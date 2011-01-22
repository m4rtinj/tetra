/*
 *  common.c
 *  
 *  Altalanos informaciok, konstansok
 *  2010-2011 - Martin Jozsef
 */

#include <string.h>
#include "common.h"

#define DELIMITER '/'

void common_setGlobSwName( const char *in ) {
    int i = 0;
    int k = 0;
    int l = strlen( in );
    for( i = 0; i < l; i++ )
        if( in[i] == DELIMITER )
            k = i + 1;
    strncpy( glob_swName, &in[k], l - k );
}

void common_setGlobSwDate( ) {
    strncpy( glob_swDate, __DATE__, 11 );
}

void common_setGlobInputFile( const char *in ) {
    int i = 0;
    int k = 0;
    int l = strlen( in );
    for( i = 0; i < l; i++ )
        if( in[i] == DELIMITER )
            k = i + 1;
    strncpy( glob_inputFile, &in[k], l - k );
}
