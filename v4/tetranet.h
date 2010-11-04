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
typedef unsigned int tSideIndex; /* 0..3 */

#define N_STATE 10

/// konstans az ervenytelen / nem letezo pont jelzesere
#define NULL_POINT NULL

/// konstans az ervenytelen / nem letezo tetraeder jelzesere
#define NULL_TETRA NULL

// a tetraeder struktura definicioja
typedef struct tTetra tTetra;
typedef struct tSide tSide;

typedef tTetra *tTetraRef;

struct tSide {
    double area;
    vector normVect;
    int type;
    tTetraRef next;
};

struct tTetra {
    long vertices[4];
    tSide sides[4]; // a szemkozti pont indexe az oldalindex
    double volume;
    tPoint massPoint;
    double states[N_STATE];
    tTetraRef next;
    tTetraRef prev;
};

// tipusok a dinamikus tombok atlathatobb definialasahoz
// todo lehetne egyszerübben, pl: double (*sideArea)[4]
typedef tTetraRef tSideNext[4];
typedef tPointRef tVertices[4];
typedef double    tSideArea[4];
typedef vector    tSideNormVect[4];
typedef int       tSideType[4];
typedef double    tStates[N_STATE];

typedef struct {
    // pontok koordinatai: dinamikus tömb
    tPoint        *points;
    // tetraederek lancolt listaja
    tTetra        *tetras;

    // szamossagtarolas
    tPointRef      maxPointRef;    // a tomb utolso cimezheto helye
// tTetraRef      maxTetraRef;    // a tomb utolso cimezheto helye
    tPointRef      lastPointRef;   // az utolso hasznalt elem indexe
    tTetraRef      lastTetraRef;   // a lanc utolso elemenek cime
    unsigned long  numberOfPoints;
    unsigned long  numberOfTetras; // a tetraederek szama;

    // a bejaro aktualis helyzete
    tTetraRef      iteratorPos;

    // adott ponthoz tartozo tetraederek keresesehez
    void          *atVertex;
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

/// utolso hasznalt tetraeder ref
tTetraRef tetranet_getLastTetraRef( tTetranet tn );

/// utolso hasznalt pont ref
tPointRef tetranet_getLastPointRef( tTetranet tn );

/// a tetraederek szama
unsigned long tetranet_getNumberOfTetras( tTetranet tn );

/// a pontok szama
unsigned long tetranet_getNumberOfPoints( tTetranet tn );

/*
 *  setterek: adatmodositas a halozatban
 */

/// az allapotvektor sti-edik elemenek beallitasa
void      tetranet_setState( tTetranet tn, tTetraRef tr, unsigned int sti, double value );

/// az szomszedossag beallitasa
/**
 ez a setter itt nem elegans, mert nem szabad user altal hivni.
 szukseges megis, hogy a neighbours modul adatszerkezettol fuggetlenul irni tudja a szomszedossagi viszonyokat.
 tombos megoldas eseten a teljes tomb kikerulhetne a neighboursba, es akkor nem kellene,
 de listas esetben a szomszedossag a tetraeder strukrura egy eleme, igy nem.
*/
void      tetranet_setSideNext( tTetranet tn, tTetraRef tr, tSideIndex si, tTetraRef neighbour );


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

// csak teszteleshez
void printTetra( tTetranet tn, tTetraRef tr );
void printNet( tTetranet tn );

#endif /* TETRANET_H_ */
