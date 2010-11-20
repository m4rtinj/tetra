#include "testcase.h"
#include "tetranet.h"
#include "common.h"
#include <sys/time.h>
#include <stdio.h>
#include <sys/resource.h>

unsigned long startTime; // ms

void startClock() {
    struct rusage rus;
    getrusage( RUSAGE_SELF, &rus );
    startTime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
}

void stopClock( char *name ) {
    struct rusage rus;
    unsigned long stopTime;
    getrusage( RUSAGE_SELF, &rus );
    stopTime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;

    printf( "Ver: %8s | ", version );
    printf( "Name: %8s | ", name );
    printf( "Time: %5.2lf s | ", (double)(stopTime - startTime)/1000.0 );
    printf( "Mem: %8ld kB\n", rus.ru_maxrss );
}

void explode( tTetranet tn, tTetraRef tr ) {
    tPointRef p0, p1, p2, p3, pm;

    p0 = tetranet_getVertex( tn, tr, 0 );
    p1 = tetranet_getVertex( tn, tr, 1 );
    p2 = tetranet_getVertex( tn, tr, 2 );
    p3 = tetranet_getVertex( tn, tr, 3 );
    pm = tetranet_insertPoint( tn, tetranet_getTetraMassPoint( tn, tr ) );

    tetranet_delTetra( tn, tr );

    tetranet_insertTetra( tn, p0, p1, p2, pm );
    tetranet_insertTetra( tn, p0, p1, pm, p3 );
    tetranet_insertTetra( tn, p0, pm, p2, p3 );
    tetranet_insertTetra( tn, pm, p1, p2, p3 );
}

void test_explode( tTetranet tn ) {
    const unsigned count = 50000;
    unsigned i;
    tTetraRef tr;

    startClock();
    tetranet_iteratorInit( tn );
    for( i = 0; i < count; ++i ) {
        tr = tetranet_iteratorNext( tn );
        explode( tn, tr );
    }
    stopClock( "explode" );
}

void test_alfa( tTetranet tn ) {
    const double a = 0.9987;
    const unsigned count = 200;
    double temp = 0;
    tTetraRef tr;
    tTetraRef tr0;
    tTetraRef trMaxVol;
    tSideIndex k;
    unsigned i = 0;

    startClock();
    // nullazas + legnagyobb terfogat keresese
    temp = 0;
    trMaxVol = NULL_TETRA;
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tetranet_setState( tn, tr, 1, 0.0 );
        if( tetranet_getTetraVolume( tn, tr ) > temp ) {
            trMaxVol = tr;
            temp = tetranet_getTetraVolume( tn, tr );
        }
    }

    // ertek a legnagyobb terfogatuba
    tetranet_setState( tn, trMaxVol, 1, 200.0 );

    for( i = 0; i < count; ++i ) {
        tetranet_iteratorInit( tn );
        // beallitjuk az uj ertekeket states[2]-be
        while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
            temp = 0;
            for( k = 0; k <= 3; ++k ) {
                tr0 = tetranet_getSideNext( tn, tr, k );
                if( tr0 != NULL_TETRA ) {
                    temp += tetranet_getState( tn, tr0, 1 );
                }
            }
            temp = ( 1 - a ) * temp + a * tetranet_getState( tn, tr, 1 );
            tetranet_setState( tn, tr, 2, temp );
        }
        // visszamasoljuk az ertekeket 2-bol 1-be
        tetranet_iteratorInit( tn );
        while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
            tetranet_setState( tn, tr, 1, tetranet_getState( tn, tr, 2 ) );
        }
    }
    stopClock( "alfa" );
    printf( "Check value = %lf\n", tetranet_getState( tn, trMaxVol, 1 ) );
}

