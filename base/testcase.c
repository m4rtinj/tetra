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

    printf( "%2s - %11s (%10.10s..) | ", glob_swName, glob_swDate, glob_inputFile );
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
    const unsigned count = 90000;
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
#define epsylon 0.0005
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

void test_pointLocation( tTetranet tn ) {
    const double n = 10.0;
    unsigned long found = 0;
    tPointRef pr;
    tPoint p;

    p = tetranet_getPoint( tn, 1 );
    double maxX = p.x;
    double minX = p.x;
    double maxY = p.y;
    double minY = p.y;
    double maxZ = p.z;
    double minZ = p.z;

    tPointRef last = tetranet_getLastPointRef( tn );
    for( pr = 2; pr <= last; ++pr ) {
        p = tetranet_getPoint( tn, pr );
        if( p.x < minX ) minX = p.x;
        if( p.x > maxX ) maxX = p.x;
        if( p.y < minY ) minY = p.y;
        if( p.y > maxY ) maxY = p.y;
        if( p.z < minZ ) minZ = p.z;
        if( p.z > maxZ ) maxZ = p.z;
    }

    double x, y, z;
    double stepX = ( maxX - minX ) / n;
    double stepY = ( maxY - minY ) / n;
    double stepZ = ( maxZ - minZ ) / n;

    startClock();
    for( x = minX; x <= maxX; x += stepX ) {
        for( y = minY; y <= maxY; y += stepY ) {
            for( z = minZ; z <= maxZ; z += stepZ ) {
                p.x = x;
                p.y = y;
                p.z = z;
                //    printf( "%8lf %8lf %8lf -- %ld\n", x, y, z, found );
                if( tetranet_getPointLocation( tn, p ) != NULL_TETRA ) ++found;
            }
        }
    }
    stopClock( "PointLoc" );
}


unsigned long counter = 0;
void recursive_delete( tTetranet tn, tTetraRef tr, int depth, int maxDepth ) {
    if( depth > maxDepth ) return;

    /*
    Kisse kifacsart modon elobb kell letorolni a tr-t, mint a szomszedait,
    mert különben a szomjedja törölne, mint a sajat szomszedjat :)
    Ezert kellenek a segedvaltozok...
    */

    tTetraRef tr0 = tetranet_getSideNext( tn, tr, 0 );
    tTetraRef tr1 = tetranet_getSideNext( tn, tr, 1 );
    tTetraRef tr2 = tetranet_getSideNext( tn, tr, 2 );
    tTetraRef tr3 = tetranet_getSideNext( tn, tr, 3 );
    tetranet_delTetra( tn, tr );
    if( tr0 != NULL_TETRA ) recursive_delete( tn, tr0, depth + 1, maxDepth );
    if( tr1 != NULL_TETRA ) recursive_delete( tn, tr1, depth + 1, maxDepth );
    if( tr2 != NULL_TETRA ) recursive_delete( tn, tr2, depth + 1, maxDepth );
    if( tr3 != NULL_TETRA ) recursive_delete( tn, tr3, depth + 1, maxDepth );
}

void test_delete( tTetranet tn ) {
    const unsigned long maxCount = 20000;
    unsigned long counter = 0;
    tetranet_iteratorInit( tn );
    tTetraRef tr = tetranet_iteratorNext( tn );
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
            trn = tetranet_iteratorNext( tn );
        }

        tr = trn;
        ++counter;

    }
    stopClock( "delete" );
}



