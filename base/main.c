#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "tetranet.h"
#include "vector.h"
#include "testcase.h"

//#define TEST_FILE_NAME "../data/teszt2.nas"
// #define TEST_FILE_NAME "../data/szivocso_vol_tetra_hm.nas"
#define TEST_FILE_NAME "../data/fuvoka_640000_mod.bdf"

void printAll( tTetranet tn ) {
    tTetraRef tr;
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        printTetra( tn, tr );
    }
}

void selfTest( tTetranet tn ) {
    test_explode( tn );
    test_alfa( tn );
//    printAll( tn );
}

int main() {
    strncpy( version, "v5-nov18", 8 );
    startClock();
    stopClock( "start" );

    startClock();
    tTetranet myTNet;
    myTNet = tetranet_new();
    tetranet_init( myTNet, TEST_FILE_NAME );
    stopClock( "init" );

    selfTest( myTNet );
    return 0;
}

