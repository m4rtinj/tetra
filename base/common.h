/*
 *  common.h
 *  
 *  Altalanos informaciok, konstansok
 *  2010-2011 - Martin Jozsef
 */

#ifndef COMMON_H_
#define COMMON_H_

/// boolean tipus definialasa
typedef int bool;
#define FALSE 0
#define TRUE (!FALSE)

/// nagyon kicsi szam, long tipusu nulla
#define EPS 0.0000000001

/// a programvaltozat esetleg verzio jelzesere, ertekadas a main.c-ben, max 8 karakter
char glob_swName[9];
/// a parameterkent megadott fajlnev
char glob_inputFile[64];
/// a buildeles datuma, a preprocesszor szerinti formatumban
char glob_swDate[12];

void common_setGlobSwName( const char *in );
void common_setGlobSwDate( );
void common_setGlobInputFile( const char *in );

#endif
