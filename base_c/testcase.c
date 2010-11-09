#include "testcase.h"
#include "tetranet.h"
#include <time.h>

clock_t startTime;

void startClock() {
    startTime = clock();
}

void stopClock() {
    stopTime = clock;
    printf( "Used Time: %lf\n",
            (( double )( stopTime - startTime ) / CLOCKS_PER_SEC ) );
}

void testcase_explode( tTetranet tn, tTetraRef tr ) {
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

void testcase_explode( tTetranet tn, tTetraRef tr ) {


void testcase_alfa( tTetranet tn ) {
    const double a = 0.2;
    const unsigned nr = 99;
    double temp = 0;
    tTetraRef tr;
    tSideIndex k;
    unsigned i = 0;

    startClock();
    // nullazas
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tetranet_setState( tn, tr, 1, 0.0 );
    }

    tetranet_iteratorInit( tn );
    tr = tetranet_iteratorNext( tn );
    tetranet_setState( tn, tr, 1, 200.0 );

    for( i = 0; i <= nr; ++i ) {
        tetranet_iteratorInit( tn );
        // beallitjuk az uj ertekeket states[2]-be
        while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
            temp = 0;
            for( k = 0; k <= 3; ++k ) {
                temp += tetranet_getState( tn, tetranet_getSideNext( tn, tr, k ), 1 );
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
    stopClock();
}

