#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "errors.h"
#include "tetranet.h"
#include "vector.h"
#include "testcase.h"

//#define TEST_FILE_NAME "../data/teszt2.nas"
#define TEST_FILE_NAME "../data/szivocso_vol_tetra_hm.nas"

void selfTest( tTetranet tn ) {
   // test_alfa(tn);
    test_explode(tn);
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

