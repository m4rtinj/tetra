/*
 *  nasreader.c
 *
 *  Created on: 06.10.2010
 *      Author: martinj
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "nasreader.h"
#include "common.h"
#include "errors.h"
#include "tetranet.h"

#define MAX_LINE 99

char line[MAX_LINE + 1];
char strTemp[9] = "01234567\0";

inline void line2point( tPoint *p ) {
    // a stringet csak a deklaracioban zartuk le,
    // ezert a terminatort nem szabad felulirni
    strncpy( strTemp, &line[24], 8 );
    p->x = atof( strTemp );
    strncpy( strTemp, &line[32], 8 );
    p->y = atof( strTemp );
    strncpy( strTemp, &line[40], 8 );
    p->z = atof( strTemp );
    // TODO mi van, ha az atof hibaval ter vissza?
}

inline void line2tetra( tPointRef *p ) {
    // a stringet csak a deklaracioban zartuk le,
    // ezert a terminatort nem szabad felulirni
    strncpy( strTemp, &line[24], 8 );
    p[0] = atoi( strTemp );
    strncpy( strTemp, &line[32], 8 );
    p[1] = atoi( strTemp );
    strncpy( strTemp, &line[40], 8 );
    p[2] = atoi( strTemp );
    strncpy( strTemp, &line[48], 8 );
    p[3] = atoi( strTemp );
    // TODO atoi hibajelzesek figyelese
}

unsigned long int nasreader_getPointNr( FILE *f ) {
    unsigned long int i = 0;
    rewind( f );
    do {
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "No GRID line found in the input file." );
        }
    } while( strncmp( line, "GRID", 4 ) != 0 );
    do {
        ++i;
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "Suddenly EOF by reading Grids." );
        }
    }  while( strncmp( line, "GRID", 4 ) == 0 );
    return i;
}

unsigned long int nasreader_getTetraNr( FILE *f ) {
    unsigned long int i = 0;
    rewind( f );
    do {
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "No CTETRA line found in the input file." );
        }
    } while( strncmp( line, "CTETRA", 6 ) != 0 );
    do {
        ++i;
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "Suddenly EOF by reading Grids." );
        }
    } while( strncmp( line, "CTETRA", 6 ) == 0 );
    return i;
}

void nasreader_readFirstPoint( FILE *f, tPoint *p ) {
    rewind( f );
    while( TRUE ) {
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "No GRID line found in the input file." );
        }
        if( strncmp( line, "GRID", 4 ) == 0 ) {
            break;
        }
    }
    line2point( p );
}

/*
 * 0       8       16      24      32      40
 * |       |       |       |       |       |       |
 * GRID          27        93.9291 -23.035893.43689
 *
 */
bool nasreader_readNextPoint( FILE *f, tPoint *p ) {
    // a kovetkezo GRID sor keresese
    if( !fgets( line, MAX_LINE, f ) ) {
        exitText( "Suddenly end of file during reading grids." );
    }
    if( strncmp( line, "GRID", 4 ) != 0 ) {
        return FALSE;
    }
    line2point( p );
    return TRUE;
}

void nasreader_readFirstTetra( FILE *f, tPointRef *p ) {
    rewind( f );
    while( TRUE ) {
        if( !fgets( line, MAX_LINE, f ) ) {
            exitText( "No CTETRA line found in the input file." );
        }
        if( strncmp( line, "CTETRA", 6 ) == 0 ) {
            break;
        }
    }
    line2tetra( p );
}

/*
 * 0       8       16      24      32      40      48
 * |       |       |       |       |       |       |
 * CTETRA         1       8   21185   20773     380   20488
 *
 */
bool nasreader_readNextTetra( FILE *f, tPointRef *p ) {
    // a kovetkezo CTETRA sor keresese
    if( !fgets( line, MAX_LINE, f ) ) {
        exitText( "Suddenly end of file during reading grids." );
    }
    if( strncmp( line, "CTETRA", 6 ) != 0 ) {
        return FALSE;
    }
    line2tetra( p );
    return TRUE;
}
