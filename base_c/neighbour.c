/*
 * neighbour.c
 *
 *  Created on: 2010.02.16.
 *      Author: martinj
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tetranet.h"
#include "errors.h"
#include "atvertex.h"

/// Egy oldalt leiro elem - ezek az elemek kerülnek rendezesre
typedef struct {
    tPointRef  pts[3];    ///< az oldal 3 pontja, index szerint novekvo sorrendben
    unsigned   sideIndex; ///< az oldal indexe a tetraederen belül
    tTetraRef  tetra;     ///< a tetraeder indexe
} element;

element *sarray;

// Osszehasonlito fuggveny def a qsorthoz
typedef int ( *compfn )( const void*, const void* );

// Osszehasonlito fuggveny kifejtese a qsorthoz
int compareElement( element *a, element *b ) {
    if( a->pts[0] > b->pts[0] )
        return 1;
    if( a->pts[0] < b->pts[0] )
        return -1;

    if( a->pts[1] > b->pts[1] )
        return 1;
    if( a->pts[1] < b->pts[1] )
        return -1;

    if( a->pts[2] > b->pts[2] )
        return 1;
    if( a->pts[2] < b->pts[2] )
        return -1;

    return 0;
}

/// Segedfv hibakereseshez
void printarray( long lastIndex ) {
    long i;
    long j;
    for( i = 0; i < lastIndex; i++ ) {
        printf( "%ld: ", (unsigned long)(sarray[i].tetra) );
        for( j = 0; j < 3; j++ )
            printf( "%ld ", sarray[i].pts[j] );
        printf( "\n" );
    }
}

/**
 * A tetraederek szomszedossagi viszonyainak kiszamitasa
 * - A teljes halon dolgozik; akkor kell meghivi, ha mar minden tetraeder el van tarolva.
 * - Strategia:
        - Minden oldalhoz letrehozunk egy strukturat (lasd: element), ezekbol tombot alkotunk
        - Ezt a 3 pont indexe szerint sorba rendezzuk
        - Ekkor ha egymas utan ketszer szerepel u.az a pontharmas a tombben,
            akkor a tartalmazo tetraederk szomszedosak.
 * - TODO: Kitol szarmazik ez a modszer?
 */
void neighbours_update( tTetranet tn ) {
    tTetraRef t;
    unsigned long s;
    unsigned long arraySize;
    unsigned long nrOfElements;

    // oldalak tombjenek letrehozasa
    nrOfElements = tetranet_getNumberOfTetras( tn ) * 4;
    arraySize = nrOfElements * sizeof( element );
    sarray = malloc( arraySize );

    // feltoltes
    s = 0;
    tetranet_iteratorInit( tn );
    while(( t = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        sarray[s].pts[0] = tetranet_getVertex( tn, t, 1 );
        sarray[s].pts[1] = tetranet_getVertex( tn, t, 2 );
        sarray[s].pts[2] = tetranet_getVertex( tn, t, 3 );
        sarray[s].tetra = t;
        sarray[s].sideIndex = s % 4;
        ++s;
        sarray[s].pts[0] = tetranet_getVertex( tn, t, 0 );
        sarray[s].pts[1] = tetranet_getVertex( tn, t, 2 );
        sarray[s].pts[2] = tetranet_getVertex( tn, t, 3 );
        sarray[s].tetra = t;
        sarray[s].sideIndex = s % 4;
        ++s;
        sarray[s].pts[0] = tetranet_getVertex( tn, t, 0 );
        sarray[s].pts[1] = tetranet_getVertex( tn, t, 1 );
        sarray[s].pts[2] = tetranet_getVertex( tn, t, 3 );
        sarray[s].tetra = t;
        sarray[s].sideIndex = s % 4;
        ++s;
        sarray[s].pts[0] = tetranet_getVertex( tn, t, 0 );
        sarray[s].pts[1] = tetranet_getVertex( tn, t, 1 );
        sarray[s].pts[2] = tetranet_getVertex( tn, t, 2 );
        sarray[s].tetra = t;
        sarray[s].sideIndex = s % 4;
        ++s;
    }

    // printarray( s );

    // rendezes
    qsort(( void * ) sarray, nrOfElements, sizeof( element ),
          ( compfn ) compareElement );

    // printarray();

    // visszaolvasas, feltoltes
    tTetraRef t0, t1;
    tSideIndex s0, s1;
    s = 0;
    const unsigned ptsSize = sizeof( sarray[0].pts );
    while( s < nrOfElements - 1 ) {
        if( memcmp( sarray[s].pts, sarray[s + 1].pts, ptsSize ) == 0 ) {
            t0 = sarray[s].tetra;
            t1 = sarray[s + 1].tetra;
            s0 = sarray[s].sideIndex;
            s1 = sarray[s + 1].sideIndex;
            if(( tetranet_getSideNext( tn, t0, s0 ) != NULL_TETRA ) ||
                    ( tetranet_getSideNext( tn, t1, s1 ) != NULL_TETRA ) ) {
                exitText( "Inconsistent neighbourhood data." );
            }
            tetranet_setSideNext( tn, t0, s0, t1 );
            tetranet_setSideNext( tn, t1, s1, t0 );
            // ha szomszed, akkor s+1 s+2 mar nem lehet az, atlephetjük a vizsgalatot
            ++s;
        }
        ++s;
    }

// oldalak tombjenek felszabaditasa
    free( sarray );
}

/*  Megkeresi es beallitja adott tetraeder adott oldalahoz tartozo szomszedot.
 *  Beallitja a szomszednal is a kapcsolatot.
 *  Az atVertex-bol dolgozik, feltetel, hogy oda mar helyesen fel legyenek vive a pontok.
 */
void findSideNeighbours( tTetranet tn, tTetraRef tr, tSideIndex si ) {
    tPointRef a, b, c;
    tTetraRef t0 = NULL_TETRA;
    int s0 = -1;    // amig negativ, nincs talalat. amugy a passzolo oldal indexe

    switch( si ) {
    case 0:
        a = tetranet_getVertex( tn, tr, 1 );
        b = tetranet_getVertex( tn, tr, 2 );
        c = tetranet_getVertex( tn, tr, 3 );
        break;
    case 1:
        a = tetranet_getVertex( tn, tr, 0 );
        b = tetranet_getVertex( tn, tr, 2 );
        c = tetranet_getVertex( tn, tr, 3 );
        break;
    case 2:
        a = tetranet_getVertex( tn, tr, 0 );
        b = tetranet_getVertex( tn, tr, 1 );
        c = tetranet_getVertex( tn, tr, 3 );
        break;
    case 3:
        a = tetranet_getVertex( tn, tr, 0 );
        b = tetranet_getVertex( tn, tr, 1 );
        c = tetranet_getVertex( tn, tr, 2 );
        break;
    default:
        exitText( "Index failed by findNeighbours" );
    }
    // az a ponthoz keresünk talalatot
    atVertex_init( tn, a );
    while(( s0 < 0 ) && (( t0 = atVertex_next( tn ) ) != NULL_TETRA ) ) {
        if( t0 != tr ) {
            if( a == tetranet_getVertex( tn, t0, 0 ) ) {
                if( b == tetranet_getVertex( tn, t0, 1 ) ) {
                    if( c == tetranet_getVertex( tn, t0, 2 ) ) {
                        s0 = 3;
                    } else if( c == tetranet_getVertex( tn, t0, 3 ) ) {
                        s0 = 2;
                    }
                } else if( b == tetranet_getVertex( tn, t0, 2 ) ) {
                    if( c == tetranet_getVertex( tn, t0, 3 ) ) {
                        s0 = 1;
                    }
                }
            } else if( a == tetranet_getVertex( tn, t0, 1 ) ) {
                if( b == tetranet_getVertex( tn, t0, 2 ) ) {
                    if( c == tetranet_getVertex( tn, t0, 3 ) ) {
                        s0 = 0;
                    }
                }
            }
        }
    }

    if( s0 < 0 ) {
        tetranet_setSideNext( tn, tr, si, NULL_TETRA );
    } else {
        if(( tetranet_getSideNext( tn, t0, s0 ) != NULL_TETRA ) ||
                ( tetranet_getSideNext( tn, tr, si ) != NULL_TETRA ) ) {
            exitText( "Inconsistent neighbourhood data." );
        }
        tetranet_setSideNext( tn, tr, si, t0 );
        tetranet_setSideNext( tn, t0, s0, tr );
    }
}

void neighbours_insert( tTetranet tn, tTetraRef tr ) {
    int k;
    for( k = 0; k <= 3; k++ ) {
        findSideNeighbours( tn, tr, k );
    }
}

void neighbours_delete( tTetranet tn, tTetraRef tr ) {
    tSideIndex k;
    tSideIndex j;
    tTetraRef nb;
    for( k = 0; k <= 3; k++ ) {
        if(( nb = tetranet_getSideNext( tn, tr, k ) ) != NULL_TETRA ) {
            for( j = 0; tetranet_getSideNext( tn, nb, j ) != tr; ++j ) {
                if( j > 3 )
                    exitText( "neighbours_delete error: asymmetric neighbourhood." );
            }
            tetranet_setSideNext( tn, nb, j, NULL_TETRA );
            tetranet_setSideNext( tn, tr, k, NULL_TETRA );
        }
    }
}






