#include "errors.h"
#include <stdio.h>
#include <stdlib.h>

void errorText( char *text ) {
    printf( "ERROR: %s\n", text );
}

void exitText( char *text ) {
    errorText( text );
    exit( EXIT_FAILURE );
}

void debugText( char *text ) {
    printf( "DEBUG: %s\n", text );
}
