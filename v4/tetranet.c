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
#include "nearestp.h"

/**
 * novekvo sorrendbe rakja a csucsok indexeit
 * @param p pontnegyes, itt adodik vissza a rendezett halmaz is
 */
void sortVertices( tPointRef p[4] ) {
    tPointRef temp;
#define CHECK(i, j) {if( p[i] > p[j] ){temp = p[i]; p[i] = p[j]; p[j] = temp;}}
    CHECK( 0, 3 )
    CHECK( 1, 2 )
    CHECK( 0, 1 )
    CHECK( 2, 3 )
    CHECK( 1, 2 )
}

void addTetra( tTetranet tn, tPointRef vertx[4] ) {
    int k = 0;
    double len = 0;
    double dotP = 0;

    tTetraRef newTetra = malloc( sizeof( tTetra ) );
    if( newTetra == NULL ) {
        exitText( "Error by malloc for newTetra." );
    }

    // ideiglenes valtozok az oldalcsucsindexek tarolasara
    // d az oldallal szemközti csucs
    vector a, b, c, d;
    vector n;

    // a pontok indexei növekvö sorrendben
    sortVertices( vertx );

    // a csucsok tarolasa
    for( k = 0; k <= 3; k++ ) {
        newTetra->vertices[k] = vertx[k];
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
        newTetra->sides[k].normVect = vector_constMult( n, 1 / len );

        // a terulet a keresztszorzat hosszanak a fele
        newTetra->sides[k].area = len / 2;

        // az oldal tipusa -- tovabbi informaciok hianyaban egyelore 0
        newTetra->sides[k].type = 0;

        // ki kell nullaznunk a szomszedot.
        newTetra->sides[k].next = NULL_TETRA;
    }

    // Terfogat = az oldalvektorok vegyes szorzatanak hatodresze
    // CSAK az oldalbeallitasok utan allhat, mert az abcd vektorok utolso allapotat hasznalja
    double volume = tripleProduct(
                        vector_diff( b, a ),
                        vector_diff( c, a ),
                        vector_diff( d, a ) ) / 6.0;
    if( volume < 0 ) {
        newTetra->volume = -volume;
    } else {
        newTetra->volume = volume;
    }

    // tomegkozeppont
    newTetra->massPoint = massPoint( a, b, c, d );

    // prev / next
    newTetra->prev = tn->lastTetraRef;
    newTetra->next = NULL_TETRA;
    if( tn->lastTetraRef != NULL_TETRA ) {
        tn->lastTetraRef->next = newTetra;
    } else {
        // itt adjuk meg a lanc kezdoelemet.
        newTetra->prev = NULL_TETRA;
        tn->tetras = newTetra;
    }
    tn->lastTetraRef = newTetra;

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

    tn->maxPointRef = nasreader_getPointNr( iniFile );
    tn->numberOfTetras = 0;
    tn->numberOfPoints = 0;

    // TODO: eleve több helyet foglani, a finomitasokhoz, pl.: +10%
    unsigned long num = tn->maxPointRef + 1;
    tn->points       = malloc( num * sizeof( tPoint ) );

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
    nasreader_readFirstTetra( iniFile, tempTetra );
    do {
        addTetra( tn, tempTetra );
    } while( nasreader_readNextTetra( iniFile, tempTetra ) );

    // nem kell mar tobbet a file
    fclose( iniFile );

    // a teljes halozatra vonatkozo beallitasok
    neighbours_update( tn );

    atVertex_update( tn );
    tetranet_atVertexInit = &atVertex_init;
    tetranet_atVertexNext = &atVertex_next;

    nearestp_update( tn );
}

inline bool isTheSamePoint( tPoint p1, tPoint p2 ) {
    return (( p2.x - p1.x ) * ( p2.x - p1.x ) +
            ( p2.y - p1.y ) * ( p2.y - p1.y ) +
            ( p2.z - p1.z ) * ( p2.z - p1.z ) ) < EPS;
}

tPointRef tetranet_insertPoint( tTetranet tn, tPoint p ) {
    tPointRef k = nearestp_search( tn, p );
    if( isTheSamePoint( p, tn->points[k] ) ) {
        return k;
    } else {
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
        nearestp_addPoint( tn, tn->lastPointRef );
        return tn->lastPointRef;
    }
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
    // atlancolas
    if( tr->next != NULL_TETRA ) {
        tr->next->prev = tr->prev;
    }
    if( tr->prev != NULL_TETRA ) {
        tr->prev->next = tr->next;
    }
    // ha ez volt az utolso, akkor valtozik az utolsot jelzo is
    if( tr == tn->lastTetraRef ) {
        tn->lastTetraRef = tn->lastTetraRef->prev;
    }
    // ha ez volt az elso, akkor valtozik a belepesi pont is
    if( tr == tn->tetras ) {
        tn->tetras = tn->tetras->next;
    }

    free( tr );
    // tetraederek szama
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
    tTetraRef tr = tn->iteratorPos;
    if( tr != NULL_TETRA ) {
        tn->iteratorPos = tr->next;
    }
    return tr;
}

tTetraRef tetranet_getPointLocation( tTetranet tn, tPoint p ) {
    tTetraRef ntr;
    tPointRef npr = nearestp_search( tn, p );
    atVertex_init( tn, npr );
    while(( ntr = atVertex_next( tn ) ) != NULL_TETRA ) {
        if( isPointInTetra( tn, ntr, p ) ) {
            return ntr;
        }
    }
    tTetraRef tr = tn->tetras;
    while( tr != NULL_TETRA ) {
        if( isPointInTetra( tn, tr, p ) ) {
            return tr;
        }
        tr = tr->next;
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

void freeTetra( tTetraRef tr ) {
    tTetraRef tmp;
    while(tr!=NULL_TETRA){
        tmp = tr->next;
        free(tr);
        tr = tmp;
    }
}

void tetranet_free( tTetranet tn ) {
    atVertex_free( tn );
    nearestp_free( tn );
    free( tn->points );
    freeTetra( tn->tetras );
    free( tn );
}

void printTetra( tTetranet tn, tTetraRef tr ) {
    printf( "[%7ld] ve: %6ld %6ld %6ld %6ld ",
            ( unsigned long )tr,
            tetranet_getVertex( tn, tr, 0 ),
            tetranet_getVertex( tn, tr, 1 ),
            tetranet_getVertex( tn, tr, 2 ),
            tetranet_getVertex( tn, tr, 3 ) );
    printf( "nb: %7ld %7ld %7ld %7ld ",
            ( unsigned long ) tetranet_getSideNext( tn, tr, 0 ) ,
            ( unsigned long ) tetranet_getSideNext( tn, tr, 1 ) ,
            ( unsigned long ) tetranet_getSideNext( tn, tr, 2 ) ,
            ( unsigned long ) tetranet_getSideNext( tn, tr, 3 ) );
    printf( "vol: %5.2lf ", tetranet_getTetraVolume( tn, tr ) );
    printf( "sta: %8.4lf ", tetranet_getState( tn, tr, 1 ) );
    printf( "\n" );
}

void printNet( tTetranet tn ) {
    tTetraRef tr = tn->tetras;
    while( tr != NULL_TETRA ) {
        printTetra( tn, tr );
        tr = tr->next;
    }
}


