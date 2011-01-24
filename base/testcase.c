/*
 *  testcase.c
 *
 *  Tesztesetek, meresekkel
 *  2010-2011 - Martin Jozsef
 */

#include "testcase.h"
#include "tetranet.h"
#include "common.h"
#include "nearestp.h"
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

    printf( "%3s - %11s (%10.10s..) | ", glob_swName, glob_swDate, glob_inputFile );
    printf( "Test: %8s | ", name );
    printf( "Time: %5.2lf s | ", ( double )( stopTime - startTime ) / 1000.0 );
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

void test_nearestp( tTetranet tn ) {
    const double epsylon = 0.05;
    unsigned long i;
    tPointRef np;
    tPoint p;
    for( i = 1; i <= tn->lastPointRef; ++i ) {
        p = tetranet_getPoint( tn, i );
        p.x += epsylon;
        p.y += epsylon;
        p.z += epsylon;
        // np = nearestp_findMe(tn,p);
        np = nearestp_search( tn, p );
        if( i != np ) {
            printf( "Nearest to %ld : %ld\n", i, np );
        }
    }
}

void test_massPointLocation( tTetranet tn ) {
    tTetraRef tr;
    tPoint p;
    startClock;
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        if( tetranet_getPointLocation( tn, tetranet_getTetraMassPoint( tn, tr ) ) != tr ) {
            printf( "massPLoc fails by tr=%ld\n", tr );
        }
    }
    stopClock( "massPLoc" );
}

void test_pointLocation( tTetranet tn ) {
    const double c1 = 0.005;
    const double c2 = ( 1 - c1 ) / 3;
    unsigned long i;
    tPointRef np;
    tTetraRef tr;
    tPoint p;
    startClock();

    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tPoint a = tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 0 ) );
        tPoint b = tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 1 ) );
        tPoint c = tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 2 ) );
        tPoint d = tetranet_getPoint( tn, tetranet_getVertex( tn, tr, 3 ) );
        vector v;
        v.x = c2 * ( a.x + b.x + c.x )  + c1 * d.x;
        v.y = c2 * ( a.y + b.y + c.y ) + c1 * d.y ;
        v.z = c2 * ( a.z + b.z + c.z ) + c1 * d.z ;
        if( tetranet_getPointLocation( tn, v ) != tr ) {
            printf( "PointLoc fails by tr=%ld\n", tr );
        }
    }
    stopClock( "PointLoc" );
}

void test_delete( tTetranet tn ) {
    const unsigned long maxCount = 50000;
    unsigned long counter = 0;
    tTetraRef tr = tetranet_getLastTetraRef( tn );
    tTetraRef trn;
    tSideIndex k;

    startClock();
    while( counter < maxCount ) {
        k = 0;
        do {
            trn = tetranet_getSideNext( tn, tr, k );
            ++k;
        } while(( k <= 3 ) && ( trn == NULL_TETRA ) );

//      printf( "c = %ld tr= %ld\n", counter, tr );
        tetranet_delTetra( tn, tr );

        if( trn == NULL_TETRA ) {
            tetranet_iteratorInit( tn );
            trn = tetranet_getLastTetraRef( tn );
        }
        tr = trn;
        ++counter;
    }
    stopClock( "delete" );
}

void test_flow( tTetranet tn ) {
    const unsigned count = 200;
    const double a = 1.0;
    const double dt = 0.0001;

    double temp = 0;
    double uc, un, vc, vn, s;
    tTetraRef tr;
    tTetraRef tr0;
    tTetraRef trMaxVol;
    tSideIndex k;
    unsigned i = 0;

    startClock();

    // homogen feltoltes + legnagyobb terfogat keresese
    temp = 0;
    trMaxVol = NULL_TETRA;
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tetranet_setState( tn, tr, 0, 0.5 );
        if( tetranet_getTetraVolume( tn, tr ) > temp ) {
            trMaxVol = tr;
            temp = tetranet_getTetraVolume( tn, tr );
        }
    }

    // ertek a legnagyobb terfogatuba
    tetranet_setState( tn, trMaxVol, 0, 0.999 );

    // kezdodik a ciklus
    for( i = 0; i < count; ++i ) {
        tetranet_iteratorInit( tn );
        // beallitjuk az uj ertekeket states[2]-be
        while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
            temp = 0;
            // sajat allapot
            uc = tetranet_getState( tn, tr, 0 );
            // sajat terfogat
            vn = tetranet_getTetraVolume( tn, tr );
            for( k = 0; k <= 3; ++k ) {
                tr0 = tetranet_getSideNext( tn, tr, k );
                if( tr0 != NULL_TETRA ) {
                    // szomszed allapota
                    un = tetranet_getState( tn, tr0, 0 );
                    // kozos oldal tertulete
                    s  = tetranet_getSideArea( tn, tr, k );
                    // szomszed terfogata
                    vn = tetranet_getTetraVolume( tn, tr0 );
                    // the very secret formula
                    temp += ( -1 * a * ( uc - un ) * s * s ) / ( vc + vn );
                }
            }
            temp = dt * temp + uc;
            tetranet_setState( tn, tr, 1, temp );
        }
        // visszamasoljuk az ertekeket 2-bol 1-be
        tetranet_iteratorInit( tn );
        while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
            tetranet_setState( tn, tr, 0, tetranet_getState( tn, tr, 1 ) );
        }
    }
    stopClock( "flow" );
    printf( "Check value = %lf\n", tetranet_getState( tn, trMaxVol, 1 ) );
}






