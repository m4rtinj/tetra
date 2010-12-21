#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "errors.h"
#include "tetranet.h"
#include "vector.h"
#include "testcase.h"

void printAll( tTetranet tn ) {
    tTetraRef tr;
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        printTetra( tn, tr );
    }
}

void selfTest( tTetranet tn ) {
    test_explode( tn );
//    test_pointLocation(tn);
    test_alfa( tn );
//    printAll( tn );
}

void help() {
    printf( "Test software to check large tetrahedron networks.\n" );
    printf( "2010 - Martin Jozsef\n\n" );
    printf( "Usage: %s nas_file_name\n", glob_swName );
    exit( EXIT_FAILURE );
}

int main( int argc, char *argv[] ) {
    common_setGlobSwName( argv[0] );
    if( argc < 2 ) help();
    common_setGlobSwDate();
    common_setGlobInputFile( argv[1] );

    startClock();
    stopClock( "start" );

    startClock();
    tTetranet myTNet;
    myTNet = tetranet_new();
    tetranet_init( myTNet, argv[1] );
    stopClock( "init" );

    selfTest( myTNet );

    startClock();
    tetranet_free(myTNet);
    stopClock("free");
    printf("\n");
    return 0;
}

