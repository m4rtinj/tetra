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
    double volume = tripleProduct(
                        vector_diff( b, a ),
                        vector_diff( c, a ),
                        vector_diff( d, a ) ) / 6.0;
    if( volume < 0 ) {
        tn->volume[tr] = -volume;
    } else {
        tn->volume[tr] = volume;
    }


    // TODO tomegkozeppont
    tn->massPoint[tr] = massPoint( a, b, c, d );

    // tetraederek szama
    ++( tn->numberOfTetras );
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
    memset( t, '\0', sizeof( tTetranetDescriptor ) );
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

    tn->numberOfPoints = 0;
    tn->numberOfTetras = 0;

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
    tn->numberOfPoints = i;

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
        tn->points = realloc( tn->points, ( tn->maxPointRef + 1 ) * sizeof( tPoint ) );
        if( tn->points == NULL ) {
            exitText( "Realloc points : error." );
        }
    }
    ++( tn->lastPointRef );
    ++( tn->numberOfPoints );
    tn->points[tn->lastPointRef] = p;
    return tn->lastPointRef;
}

void      tetranet_delPoint( tTetranet tn, tPointRef pr ) {
    // semmi, nem eri meg a macerat. igy viszont memoriazabalas. TODO
}


/*
F: first free ref
M: max ref (arraysize-1)
L: last used ref

F == M ?
    bovites
    M az utolso uj elem
F-re beirjuk az uj elemet
amig F<=L es F nem ures
    F++
 */
tTetraRef tetranet_insertTetra( tTetranet tn, tPointRef pr0, tPointRef pr1, tPointRef pr2, tPointRef pr3 ) {
    if( tn->lastTetraRef >= tn->maxTetraRef ) {
        unsigned long num;
        tn->maxTetraRef = tn->maxTetraRef * 2;  // TODO atgondolni, hogy a duplazas nem eros-e egy kicsit
        num = tn->maxTetraRef + 1;
        tn->vertices     = realloc( tn->vertices,     num * sizeof( tVertices ) );
        if( tn->vertices == NULL )
            exitText( "Realloc vertices : error." );
        tn->sideArea     = realloc( tn->sideArea,     num * sizeof( tSideArea ) );
        if( tn->sideArea == NULL )
            exitText( "Realloc sideArea : error." );
        tn->sideNormVect = realloc( tn->sideNormVect, num * sizeof( tSideNormVect ) );
        if( tn->sideNormVect == NULL )
            exitText( "Realloc sideNormVect : error." );
        tn->sideType     = realloc( tn->sideType,     num * sizeof( tSideType ) );
        if( tn->sideType == NULL )
            exitText( "Realloc sideType : error." );
        tn->sideNext     = realloc( tn->sideNext,     num * sizeof( tSideNext ) );
        if( tn->sideNext == NULL )
            exitText( "Realloc sideNext : error." );
        tn->volume       = realloc( tn->volume,       num * sizeof( double ) );
        if( tn->volume == NULL )
            exitText( "Realloc volume : error." );
        tn->states       = realloc( tn->states,       num * sizeof( tStates ) );
        if( tn->states == NULL )
            exitText( "Realloc massPoint : error." );
        tn->massPoint    = realloc( tn->massPoint,    num * sizeof( tPoint ) );
        if( tn->massPoint == NULL )
            exitText( "Realloc tetras : error." );
    }

    tPointRef newTetra[4];
    newTetra[0] = pr0;
    newTetra[1] = pr1;
    newTetra[2] = pr2;
    newTetra[3] = pr3;

    ++( tn->lastTetraRef );
    addTetra( tn, tn->lastTetraRef, newTetra );

    // elobb atvertex, csak utana neighbours, mert utobbi elobbit hasznalja !!!
    atVertex_insert( tn, tn->lastTetraRef );
    neighbours_insert( tn, tn->lastTetraRef );

    return tn->lastTetraRef;
}

void      tetranet_delTetra( tTetranet tn, tTetraRef tr ) {
    // toroljuk a szomszednyilvantartasbol es az atvertex-bol
    neighbours_delete( tn, tr );
    atVertex_delete( tn, tr );

    if( tr != tn->lastTetraRef ) {
        // az üres helyre masoljuk az utolsot
        tTetraRef tl = tn->lastTetraRef;

        neighbours_delete( tn, tl );
        atVertex_delete( tn, tl );

        addTetra( tn, tr, tn->vertices[tl] );

        atVertex_insert( tn, tr );
        neighbours_insert( tn, tr );
    }
    --( tn->lastTetraRef );
    --( tn->numberOfTetras );
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

void      tetranet_setSideNext( tTetranet tn, tTetraRef tr, tSideIndex si, tTetraRef nb ) {
    tn->sideNext[tr][si] = nb;
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

tTetraRef tetranet_getLastTetraRef( tTetranet tn ) {
    return tn->lastTetraRef;
}

tPointRef tetranet_getLastPointRef( tTetranet tn ) {
    return tn->lastPointRef;
}

unsigned long tetranet_getNumberOfTetras( tTetranet tn ) {
    return tn->numberOfTetras;
}

unsigned long tetranet_getNumberOfPoints( tTetranet tn ) {
    return tn->numberOfPoints;
}

void printNet( tTetranet tn ) {
    tTetraRef tr;
    printf( "Printout tetranet.\n" );
    tetranet_iteratorInit( tn );
    while((( tr = tetranet_iteratorNext( tn ) ) ) != NULL_TETRA ) {
        printf( "[%ld] ve: %ld %ld %ld %ld ",
                tr,
                tetranet_getVertex( tn, tr, 0 ),
                tetranet_getVertex( tn, tr, 1 ),
                tetranet_getVertex( tn, tr, 2 ),
                tetranet_getVertex( tn, tr, 3 ) );
        printf( "nb: %ld %ld %ld %ld ",
                tetranet_getSideNext( tn, tr, 0 ),
                tetranet_getSideNext( tn, tr, 1 ),
                tetranet_getSideNext( tn, tr, 2 ),
                tetranet_getSideNext( tn, tr, 3 ) );
        printf( "vol: %lf ", tetranet_getTetraVolume( tn, tr ) );
        printf( "\n" );
    }
}

