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
    #define CHECK(i, j) {if( p[0] > p[3] ){temp = p[0]; p[0] = p[3]; p[3] = temp;}}
    CHECK(0, 3)
    CHECK(1, 2)
    CHECK(0, 1)
    CHECK(2, 3)
    CHECK(1, 2)
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
        tn->tetras[tr].vertices[k] = vertx[k];
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
            exitText( "Index failed by getting sidePoints." );
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
        tn->tetras[tr].sides[k].normVect = vector_constMult( n, 1 / len );

        // a terulet a keresztszorzat hosszanak a fele
        tn->tetras[tr].sides[k].area = len / 2;

        // az oldal tipusa -- tovabbi informaciok hianyaban egyelore 0
        tn->tetras[tr].sides[k].type = 0;

        // ki kell nullaznunk a szomszedot.
        tn->tetras[tr].sides[k].next = NULL_TETRA;
    }

    // Terfogat = az oldalvektorok vegyes szorzatanak hatodresze
    // CSAK az oldalbeallitasok utan allhat, mert az abcd vektorok utolso allapotat hasznalja
    double volume = tripleProduct(
                        vector_diff( b, a ),
                        vector_diff( c, a ),
                        vector_diff( d, a ) ) / 6.0;
    if( volume < 0 ) {
        tn->tetras[tr].volume = -volume;
    } else {
        tn->tetras[tr].volume = volume;
    }

    // tomegkozeppont
    tn->tetras[tr].massPoint = massPoint( a, b, c, d );

 //   tn->lastTetraRef = newTetra; TODO javitani ezt, mert itt nincs jo helyen!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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
  //  tn->points       = malloc( num * sizeof( tPoint ) );
// TODO
    num = tn->maxTetraRef + 1;
    tn->vertices     = malloc( num * sizeof( tVertices ) );

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

inline bool isTheSamePoint( tPoint p1, tPoint p2 ) {
    return (( p2.x - p1.x ) * ( p2.x - p1.x ) +
            ( p2.y - p1.y ) * ( p2.y - p1.y ) +
            ( p2.z - p1.z ) * ( p2.z - p1.z ) ) < EPS;
}

tPointRef tetranet_insertPoint( tTetranet tn, tPoint p ) {
    tPointRef k;
    for( k = 1; k<=tn->lastPointRef;k++){
        if(isTheSamePoint(p,tn->points[k])){
            return k;
            }
     }

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

tTetraRef tetranet_insertTetra( tTetranet tn, tPointRef pr0, tPointRef pr1, tPointRef pr2, tPointRef pr3 ) {
    tPointRef vertx[4];
    vertx[0] = pr0;
    vertx[1] = pr1;
    vertx[2] = pr2;
    vertx[3] = pr3;

    addTetra( tn, vertx );

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
    return tr->vertices[vi];
}

double    tetranet_getTetraVolume( tTetranet tn, tTetraRef tr ) {
    return tr->volume;
}

tPoint    tetranet_getTetraMassPoint( tTetranet tn, tTetraRef tr ) {
    return tr->massPoint;
}

double    tetranet_getState( tTetranet tn, tTetraRef tr, unsigned int sti ) {
    return tr->states[sti];
}

void      tetranet_setState( tTetranet tn, tTetraRef tr, unsigned int sti, double value ) {
    tr->states[sti] = value;
}

tTetraRef tetranet_getSideNext( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tr->sides[si].next;
}

void      tetranet_setSideNext( tTetranet tn, tTetraRef tr, tSideIndex si, tTetraRef nb ) {
    tr->sides[si].next = nb;
}

double    tetranet_getSideArea( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tr->sides[si].area;
}

vector    tetranet_getSideNormalVector( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    return tr->sides[si].normVect;
}

void      tetranet_iteratorInit( tTetranet tn ) {
    tn->iteratorPos = tn->tetras;
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

void printTetra( tTetranet tn, tTetraRef tr ) {
    printf( "[%ld] ve: %ld %ld %ld %ld ",
            ( unsigned long )tr ,
            tetranet_getVertex( tn, tr, 0 ),
            tetranet_getVertex( tn, tr, 1 ),
            tetranet_getVertex( tn, tr, 2 ),
            tetranet_getVertex( tn, tr, 3 ) );
    printf( "nb: %ld %ld %ld %ld ",
            ( unsigned long )( tetranet_getSideNext( tn, tr, 0 ) ),
            ( unsigned long )( tetranet_getSideNext( tn, tr, 1 ) ),
            ( unsigned long )( tetranet_getSideNext( tn, tr, 2 ) ),
            ( unsigned long )( tetranet_getSideNext( tn, tr, 3 ) ) );
    printf( "vol: %lf ", tetranet_getTetraVolume( tn, tr ) );
    printf( "\n" );
}

void printNet( tTetranet tn ) {
    tTetraRef tr = tn->tetras;
    while( tr != NULL_TETRA ) {
        printTetra( tn, tr );
        tr = tr->next;
    }
}


