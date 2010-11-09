#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "errors.h"
#include "tetranet.h"
#include "vector.h"

//#define TEST_FILE_NAME "../data/teszt2.nas"
#define TEST_FILE_NAME "../data/szivocso_vol_tetra_hm.nas"

void explode(tTetranet tn,tTetraRef tr){
    tPointRef p0,p1,p2,p3,pm;

    p0 = tetranet_getVertex(tn,tr,0);
    p1 = tetranet_getVertex(tn,tr,1);
    p2 = tetranet_getVertex(tn,tr,2);
    p3 = tetranet_getVertex(tn,tr,3);
    pm = tetranet_insertPoint(tn,tetranet_getTetraMassPoint(tn,tr));

    tetranet_delTetra(tn,tr);

    tetranet_insertTetra(tn,p0,p1,p2,pm);
    tetranet_insertTetra(tn,p0,p1,pm,p3);
    tetranet_insertTetra(tn,p0,pm,p2,p3);
    tetranet_insertTetra(tn,pm,p1,p2,p3);
}












void selfTest( tTetranet tn ) {
    tTetraRef tet, tetSNx;

    unsigned long i = 1;
    unsigned j = 1;
    unsigned k = 0;
    double sumVolume = 0;
    double sumValue = 0;
    double v1 = 0;
    unsigned sti;

    tetranet_iteratorInit( tn );
    while(( tet = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tetranet_setState( tn, tet, 0, i );
        ++i;
    }

    printf( "%ld\n", i );
    for( j = 1; j < 99; ++j ) {  // to 99
        sti = j % 9;
        tetranet_iteratorInit( tn );
        while((( tet = tetranet_iteratorNext( tn ) ) ) != NULL_TETRA ) {
            for( k = 0; k <= 3; ++k ) {
                if(( tetSNx = tetranet_getSideNext( tn, tet, k ) ) != NULL_TETRA ) {
                    v1 = tetranet_getTetraVolume( tn, tetSNx );
                    sumVolume += v1;
                    sumValue += v1 * tetranet_getState( tn, tetSNx, sti );
                }
            }
            sti = ( sti + 1 ) % 9;
            tetranet_setState( tn, tet, sti, sumValue / sumVolume );
        }
    }
    printf( "SumValue=%lf\n", sumValue );

    // atvertex test
    tetranet_atVertexInit( tn, 2000 );
    while(( tet = tetranet_atVertexNext( tn ) ) != NULL_TETRA ) {
        printf( "%ld\n", (unsigned long) tet );
    }

    // printNet( tn );

    // location test
/*
    tPoint p;
    p = tetranet_getTetraMassPoint( tn, tetranet_getLastTetraRef(tn) );
    tet = tetranet_getPointLocation( tn, p );
    printTetra(tn,tet);

    explode(tn,tetranet_getLastTetraRef(tn));
*/
}

int main() {
    tTetranet myTNet;

    clock_t startTime = clock();
    clock_t stopTime;
    debugText( "started" );

    myTNet = tetranet_new();
    tetranet_init( myTNet, TEST_FILE_NAME );
    debugText( "tetranet init done" );

    selfTest( myTNet );
    debugText( "selftest done" );

    stopTime = clock();
    printf( "Used Time: %lf\n", (( double )( stopTime - startTime )
                                 / CLOCKS_PER_SEC ) );
    return 0;
}

