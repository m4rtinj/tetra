#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "errors.h"
#include "tetranet.h"
#include "vector.h"

#define TEST_FILE_NAME "../data/teszt2.nas"
//#define TEST_FILE_NAME "teszt2.nas"

/*
 *  Egy gyors teszt, sebessegmeresere es TODO: helyesseg/teljesseg ellenorzesere

void selfTest( tTetranet tn ) {
    tTetraRef tet, tetSNx;

    unsigned long i = 1;
    unsigned j = 1;
    unsigned k = 0;
    double sumVolume = 0;
    double sumValue = 0;
    double v1 = 0;

    tetranet_iteratorInit( tn );
    while(( tet = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        tetranet_setState( tn, tet, 0, i );
        ++i;
    }

    printf("%ld\n",i);
    for( j = 1; j < 99; ++j ) {  // to 99
        tetranet_iteratorInit( tn );
        while((( tet = tetranet_iteratorNext( tn ) ) ) != NULL_TETRA ) {
            for( k = 0; k <= 3; ++k ) {
                if(( tetSNx = tetranet_getSideNext( tn, tet, k ) ) != NULL_TETRA ) {
                    v1 = tetranet_getTetraVolume( tn, tetSNx );
                    sumVolume += v1;
                    sumValue += v1 * tetranet_getState( tn, tetSNx, j - 1 );
                }
            }
            tetranet_setState( tn, tet, j, sumValue / sumVolume );
        }
    }
    printf( "SumValue=%lf\n", sumValue );
}
*/
/*
void selfTest() {
    double volume = 0;
    double area = 0;
    tIndex idx = 0;
    int i;


    for( idx = 1; idx <= numberOfTetraeders; ++idx ) {
        volume += tetranet_getVolume( idx );
    }
    printf( "volume = %lf\n", volume );

    for( idx = 1; idx <= numberOfTetraeders; ++idx ) {
        for( i = 0; i <= 3; ++i ) {
            if( tetranet_getSideNext( idx, i ) == NULL_TETRA ) {
                area += tetranet_getSideArea( idx, i );
            }
        }
    }
    printf( "area = %lf\n", area );
}
*/



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
    tetranet_atVertexInit( tn, 2 );
    while(( tet = tetranet_atVertexNext( tn ) ) != NULL_TETRA ) {
        printf( "%ld\n", tet );
    }

    printNet( tn );

    // location test
    tPoint p;
    tPointRef pr;
    p = tetranet_getTetraMassPoint( tn, 1 );
    tet = tetranet_getPointLocation( tn, p );
    printf( "location: %ld\n", tet );

    // insert test
    pr = tetranet_insertPoint( tn, p );
    tetranet_delTetra( tn, 1 );
    printNet( tn );
    tet = tetranet_insertTetra( tn, 4, 3, 1, pr );
    tet = tetranet_insertTetra( tn, 4, 3, 2, pr );
    tet = tetranet_insertTetra( tn, 4, 2, 1, pr );
    tet = tetranet_insertTetra( tn, 2, 3, 1, pr );
    printNet( tn );
    tetranet_delTetra( tn, 2 );
    printNet( tn );

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

