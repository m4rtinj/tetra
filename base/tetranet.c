/*
 * tetranet.c
 *
 *  Created on: 27.03.2010
 *      Author: Martin József
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tetranet.h"
#include "neighbour.h"
#include "errors.h"
#include "common.h"
#include "nasreader.h"
#include "atvertex.h"

/**
 * novekvo sorrendbe rakja a csucsok indexeit
 * @param p pontnegyes, itt adodik vissza a rendezett halmaz is
 */
void sortVertices( tPointRef p[4] ) {
    tPointRef temp;
    if( p[0] > p[3] ) {
        temp = p[0];
        p[0] = p[3];
        p[3] = temp;
    }
    if( p[1] > p[2] ) {
        temp = p[1];
        p[1] = p[2];
        p[2] = temp;
    }
    if( p[0] > p[1] ) {
        temp = p[0];
        p[0] = p[1];
        p[1] = temp;
    }
    if( p[2] > p[3] ) {
        temp = p[2];
        p[2] = p[3];
        p[3] = temp;
    }
    if( p[1] > p[2] ) {
        temp = p[1];
        p[1] = p[2];
        p[2] = temp;
    }
}

void addTetra( tTetranet tn, tTetraRef tr, tPointRef vertx[4] ) {
    int k = 0;
    double len = 0;
    double dotP = 0;

    // ideiglenes valtozok az oldalcsucsindexek tarolasara
    // d az oldallal szemközti csucs
    vector a, b, c, d;
    vector n;

    // a pontok indexei növekvö sorrendben
    sortVertices( vertx );

    // a csucsok tarolasa
    for( k = 0; k <= 3; k++ ) {
        tn->vertices[tr][k] = vertx[k];
    }

    // az oldalakhoz tartozo adatok szamitasa
    for( k = 0; k <= 3; k++ ) {
        // kivalasztjuk az k-adik oldalhoz tartozo pontokat; eredetileg fv: getSidePoints
        switch( k ) {
        case 0:
            a = tn->points[vertx[1]];
            b = tn->points[vertx[2]];
            c = tn->points[vertx[3]];
            d = tn->points[vertx[0]];
            break;
        case 1:
            a = tn->points[vertx[0]];
            b = tn->points[vertx[2]];
            c = tn->points[vertx[3]];
            d = tn->points[vertx[1]];
            break;
        case 2:
            a = tn->points[vertx[0]];
            b = tn->points[vertx[1]];
            c = tn->points[vertx[3]];
            d = tn->points[vertx[2]];
            break;
        case 3:
            a = tn->points[vertx[0]];
            b = tn->points[vertx[1]];
            c = tn->points[vertx[2]];
            d = tn->points[vertx[3]];
            break;
        default:
            exitText( "Index failed by getSidePoints." );
        }

        // oldal normalvektora
        n = normalOfPlane( a, b, c );

        // kifele mutasson:
        // ha az AD vektor es a normalvektor skalaris szorzata pozitiv, akkor azonos terfelbe mutatnak
        // TODO: ha pont nulla, akkor ez egy hibas (egysiku) tetraeder. itt lehetne ezt jol ellenorizni
        dotP = dotProduct( vector_diff( d, a ), n );

        if( dotP > 0 ) {
            n = negativeVector( n );
        } else {
            dotP = - dotP;
        }
        if( dotP <= EPS ) {
            exitText( "I found a 2D tetrahedron." );
        }

        len = vector_length( n );

        // egysegnyi hosszu kifele mutato normalvektor
        tn->sideNormVect[tr][k] = vector_constMult( n, 1 / len );

        // a terulet a keresztszorzat hosszanak a fele
        tn->sideArea[tr][k] = len / 2;

        // az oldal tipusa -- tovabbi informaciok hianyaban egyelore 0
        tn->sideType[tr][k] = 0;

        // ki kell kinullaznunk a szomszedot.
        tn->sideNext[tr][k] = NULL_TETRA;
    }

    // Terfogat = az oldalvektorok vegyes szorzatanak hatodresze
    // CSAK az oldalbeallitasok utan hivhato, mert az abcd vektorok utolso allapotat hasznalja
    tn->volume[tr] = abs( tripleProduct(
                              vector_diff( b, a ),
                              vector_diff( c, a ),
                              vector_diff( d, a ) ) ) / 6.0;
    // TODO tomegkozeppont
    tn->massPoint[tr] = massPoint( a, b, c, d );
}

bool isPointInTetra( tTetranet tn, tTetraRef tr, tPoint p ) {
    vector ap = vector_diff( p, tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 0 ) ) );
    return ( dotProduct( ap, tetranet_getSideNormalVector( tn, tr, 1 ) ) < 0 ) &&
           ( dotProduct( ap, tetranet_getSideNormalVector( tn, tr, 2 ) ) < 0 ) &&
           ( dotProduct( ap, tetranet_getSideNormalVector( tn, tr, 3 ) ) < 0 ) &&
           ( dotProduct(
                 vector_diff( p, tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 1 ) ) ),
                 tetranet_getSideNormalVector( tn, tr, 0 ) )
             < 0 );
}


/* **********************************
 *   Interface fuggvenyek
 * **********************************/

tTetranet tetranet_new( ) {
    tTetranet t = malloc( sizeof( tTetranetDescriptor ) );
    memset( t, '\0', sizeof( t ) );
    return t;
}

void tetranet_init( tTetranet tn, char *filename ) {
    FILE      *iniFile;
    tPoint     tempPoint;
    tPointRef  tempTetra[4];
    unsigned long i;

    iniFile = fopen( filename, "r" );
    if( iniFile == NULL ) {
        exitText( "?FILE NOT FOUND ERROR\nREADY." );
    }

    tn->lastPointRef = 0;
    tn->lastTetraRef = 0;

    tn->maxPointRef = nasreader_getPointNr( iniFile );
    tn->maxTetraRef = nasreader_getTetraNr( iniFile );

    // TODO: eleve több helyet foglani, a finomitasokhoz, pl.: +10%
    unsigned long num = tn->maxPointRef + 1;
    tn->points       = malloc( num * sizeof( tPoint ) );

    // TODO: eleve több helyet foglani, a finomitasokhoz, pl.: +10%
    num = tn->maxTetraRef + 1;
    tn->vertices     = malloc( num * sizeof( tVertices ) );
    tn->sideArea     = malloc( num * sizeof( tSideArea ) );
    tn->sideNormVect = malloc( num * sizeof( tSideNormVect ) );
    tn->sideType     = malloc( num * sizeof( tSideType ) );
    tn->sideNext     = malloc( num * sizeof( tSideNext ) );
    tn->volume       = malloc( num * sizeof( double ) );
    tn->states       = malloc( num * sizeof( tStates ) );
    tn->massPoint    = malloc( num * sizeof( tPoint ) );

    // pontok olvasasa fajlbol
    i = 0;
    nasreader_readFirstPoint( iniFile, &tempPoint );
    do {
        ++i;
        tn->points[i] = tempPoint;
    } while( nasreader_readNextPoint( iniFile, &tempPoint ) );
    tn->lastPointRef = i;

    // tetraederek olvasasa fajlbol
    i = 0;
    nasreader_readFirstTetra( iniFile, tempTetra );
    do {
        ++i;
        addTetra( tn, i, tempTetra );
    } while( nasreader_readNextTetra( iniFile, tempTetra ) );
    tn->lastTetraRef = i;

    // nem kell mar tobbet a file
    fclose( iniFile );

    // a teljes halozatra vonatkozo beallitasok
    neighbours_update( tn );

    atVertex_update( tn );
    tetranet_atVertexInit = &atVertex_init;
    tetranet_atVertexNext = &atVertex_next;
}

tPointRef tetranet_insertPoint( tTetranet tn, tPoint p ) {
    // TODO: ha mar letezik, akkor csak az indexet kerem
    if( tn->lastPointRef >= tn->maxPointRef ) {
        tn->maxPointRef = tn->maxPointRef * 2;
        tn->points = realloc( tn->points, (tn->maxPointRef + 1) * sizeof( tPoint ));
        if( tn->points == NULL ) {
            exitText( "Realloc points : error." );
        }
    }
    ++( tn->lastPointRef );
    tn->points[tn->lastPointRef] = p;
    return tn->lastPointRef;
}

void      tetranet_delPoint( tTetranet tn, tPointRef pr ) {
    // semmi, nem eri meg a macerat. igy viszont memoriazabalas. TODO
}

tTetraRef tetranet_insertTetra( tTetranet tn, tPointRef pr0, tPointRef pr1, tPointRef pr2, tPointRef pr3 ) {

    unsigned long num;
    if( tn->lastTetraRef >= tn->maxTetraRef ) {
        // TODO atgondolni, hogy a duplazas nem eros-e egy kicsit
        tn->maxTetraRef = tn->maxTetraRef * 2;
        num = tn->maxTetraRef + 1;
        tn->vertices     = realloc( tn->vertices,     num * sizeof( tVertices ) );
        tn->sideArea     = realloc( tn->sideArea,     num * sizeof( tSideArea ) );
        tn->sideNormVect = realloc( tn->sideNormVect, num * sizeof( tSideNormVect ) );
        tn->sideType     = realloc( tn->sideType,     num * sizeof( tSideType ) );
        tn->sideNext     = realloc( tn->sideNext,     num * sizeof( tSideNext ) );
        tn->volume       = realloc( tn->volume,       num * sizeof( double ) );
        tn->states       = realloc( tn->states,       num * sizeof( tStates ) );
        tn->massPoint    = realloc( tn->massPoint,    num * sizeof( tPoint ) );
        // TODO: minden tombot ellenorizni
        if( tn->massPoint == NULL ) {
            exitText( "Realloc tetras : error." );
        }
    }

    tPointRef newTetra[4];
    newTetra[0] = pr0;
    newTetra[1] = pr1;
    newTetra[2] = pr2;
    newTetra[3] = pr3;

    ++( tn->lastTetraRef );
    addTetra( tn, tn->lastTetraRef, newTetra );

    atVertex_update( tn );
    neighbours_findNeighbours( tn, tn->lastTetraRef );

    return tn->lastTetraRef;
}

void      tetranet_delTetra( tTetranet tn, tTetraRef tr ) {
    // semmi
}

tPoint    tetranet_getPoint( tTetranet tn, tPointRef pr ) {

    return tn->points[pr];
}

tPointRef tetranet_getVertex( tTetranet tn, tTetraRef tr, unsigned vi ) {
    return tn->vertices[tr][vi];
}

double    tetranet_getTetraVolume( tTetranet tn, tTetraRef tr ) {
    return tn->volume[tr];
}

tPoint    tetranet_getTetraMassPoint( tTetranet tn, tTetraRef tr ) {
    return tn->massPoint[tr];
}

double    tetranet_getState( tTetranet tn, tTetraRef tr, unsigned int sti ) {
    return tn->states[tr][sti];
}

void      tetranet_setState( tTetranet tn, tTetraRef tr, unsigned int sti, double value ) {
    tn->states[tr][sti] = value;
}

tTetraRef tetranet_getSideNext( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tn->sideNext[tr][si];
}

double    tetranet_getSideArea( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tn->sideArea[tr][si];
}

vector    tetranet_getSideNormalVector( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tn->sideNormVect[tr][si];
}

void      tetranet_iteratorInit( tTetranet tn ) {
    tn->iteratorPos = 0;
}

tTetraRef tetranet_iteratorNext( tTetranet tn ) {
    if( tn->iteratorPos >= tn->lastTetraRef ) {
        return NULL_TETRA;
    } else {
        ++( tn->iteratorPos );
        return( tn->iteratorPos );
    }
}

tTetraRef tetranet_getPointLocation( tTetranet tn, tPoint p ) {
    tTetraRef tr;
    for( tr = tn->lastTetraRef; tr != 0; --tr ) {
        if( isPointInTetra( tn, tr, p ) ) {
            return tr;
        }
    }
    return NULL_TETRA;
}

unsigned long tetranet_getNrOfTetras( tTetranet tn ) {
    return tn->lastTetraRef;
}

unsigned long tetranet_getNrOfPoints( tTetranet tn ) {
    return tn->lastPointRef;
}

