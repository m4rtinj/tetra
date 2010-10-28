/*
 * tetranet.h
 *
 *  Created on: 23.03.2010
 *      Author: Martin József
 */

#ifndef TETRANET_H_
#define TETRANET_H_

/* -------------------------------------------------------------------------------------
 *  tipusdefiniciok
 *
 *  a megvalositott adatszerkezet fuggvenyeben modosulhatnak
 * ------------------------------------------------------------------------------------*/

#include "common.h"
#include "vector.h"
typedef vector tPoint;
typedef unsigned long int tPointRef;
typedef unsigned long int tTetraRef;
typedef unsigned int tSideIndex; /* 0..3 */

#define N_STATE 10

/// konstans az ervenytelen / nem letezo pont jelzesere
#define NULL_POINT 0

/// konstans az ervenytelen / nem letezo tetraeder jelzesere
#define NULL_TETRA 0

// tipusok a dinamikus tombok atlathatobb definialasahoz
// todo lehetne egyszerübben   double (*sideArea)[4]
typedef tTetraRef tSideNext[4];
typedef tPointRef tVertices[4];
typedef double    tSideArea[4];
typedef vector    tSideNormVect[4];
typedef int       tSideType[4];
typedef double    tStates[N_STATE];

typedef struct {
    // Dinamikus tombok a tetraederek adatainak kezelesere
    // pontok koordinatai
    tPoint        *points;
    // alkoto pontok adatai
    tVertices     *vertices;
    // hatarolo oldalak adatai
    tSideArea     *sideArea;
    tSideNormVect *sideNormVect;
    tSideType     *sideType;
    tSideNext     *sideNext;
    // magara a tetraederre vonatkozo adatok
    double        *volume;
    tStates       *states;
    tPoint        *massPoint;

    // szamossagtarolas
    tPointRef      maxPointRef;
    tTetraRef      maxTetraRef;
    tPointRef      lastPointRef;
    tTetraRef      lastTetraRef;

    // az aktualisan kezelt tetraeder indexe TODO: hasznaljuk ezt???
    tTetraRef      actTetraRef;

    // a bejaro aktualis helyzete
    tTetraRef      iteratorPos;

    // adott ponthoz tartozo tetraederek keresesehez
    void          *atVertex;
    tTetraRef     *atVertexMain;
    unsigned long *atVertexIdx;
    unsigned long  atVertexActIndex;
    unsigned long  atVertexLastIndex;

    // adott pont melyik tetraederben van? location.
    // ...
} tTetranetDescriptor;

typedef tTetranetDescriptor *tTetranet;

/* -------------------------------------------------------------------------------------
 *  fuggvenyek
 *
 *  definiciojuk allando, fuggetlen az adatszerkezettol.
 *  a fentebb definialt tipusokat hasznaljak parameterul es visszeteresi tipusul
 * ------------------------------------------------------------------------------------*/

/*
 *  felepites / bovites / torles
 */

/// ures halozatleiro keszitese
tTetranet tetranet_new( );

/// a teljes halozat inicializalasa bemeno adathalmazzal
void      tetranet_init( tTetranet tn, char *filename );

/// egy uj pont hozzaadasa a pontracshoz
tPointRef tetranet_insertPoint( tTetranet tn, tPoint p );

/// egy tetraeder hozzaadasa a meglevo halozathoz
tTetraRef tetranet_insertTetra( tTetranet tn, tPointRef pr0, tPointRef pr1, tPointRef pr2, tPointRef pr3 );

/// egy pont eltavolitasa a meglevo halozatbol
void      tetranet_delPoint( tTetranet tn, tPointRef pr );

/// egy tetraeder eltavolitasa a meglevo halozatbol
void      tetranet_delTetra( tTetranet tn, tTetraRef tr );

/*
 *  getterek: informacio kinyerese a halozatbol
 */

/// pontadatok lekerdezese a pont indexe alapjan
tPoint    tetranet_getPoint( tTetranet tn, tPointRef pr );

/// csucsok indexenek lekerdezese
tPointRef tetranet_getVertex( tTetranet tn, tTetraRef tr, unsigned vi );

/// tetraeder terfogata
double    tetranet_getTetraVolume( tTetranet tn, tTetraRef tr );

/// tetraeder sulypontja
tPoint    tetranet_getTetraMassPoint( tTetranet tn, tTetraRef tr );

/// az allapotvektor sti-edik eleme
double    tetranet_getState( tTetranet tn, tTetraRef tr, unsigned int sti );

/// oldalszomszed tetraeder
tTetraRef tetranet_getSideNext( tTetranet tn, tTetraRef tr, tSideIndex si );

/// oldal terulete
double    tetranet_getSideArea( tTetranet tn, tTetraRef tr, tSideIndex si );

/// oldal kifele mutato normalvektora
vector    tetranet_getSideNormalVector( tTetranet tn, tTetraRef tr, tSideIndex si );

/// megkeresi, hogy az adott pont melyik teraederben van
tTetraRef tetranet_getPointLocation( tTetranet tn, tPoint p );

/// tetraederek szama
unsigned long tetranet_getNrOfTetras( tTetranet tn );

/// pontok szama
unsigned long tetranet_getNrOfPoints( tTetranet tn );

/*
 *  setterek: adatmodositas a halozatban
 */

/// az allapotvektor sti-edik elemenek beallitasa
void      tetranet_setState( tTetranet tn, tTetraRef tr, unsigned int sti, double value );

/*
 *  iteratorok: tetrraederek sorozatat adjak vissza
 */

/// tetraederhalo bejarasanak inditasa
void      tetranet_iteratorInit( tTetranet tn );

/// a bejaras soran következö tetraeder
tTetraRef tetranet_iteratorNext( tTetranet tn );

/// adott ponthoz tartozo tetraederek lekerdezesenek kezdese
bool ( *tetranet_atVertexInit )( tTetranet tn, tPointRef pr );

/// csak az init utan: adott ponthoz tartozo tetraederek kozul a kovetkezo
tTetraRef( *tetranet_atVertexNext )( tTetranet tn );

#endif /* TETRANET_H_ */
