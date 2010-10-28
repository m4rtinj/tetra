/**
 *  Adott ponthoz tartozo tetraederek keresese
 *
 */
#include <stdlib.h>
#include "atvertex.h"
#include "tetranet.h"
#include "common.h"

/// Egy pont-tetraeder part leiro elem - ezek az elemek kerülnek rendezesre
typedef struct {
    tPointRef p;   ///< csucs
    tTetraRef t;   ///< tetraeder
} pair;

// Osszehasonlito fuggveny def a qsorthoz
typedef int ( *compfn )( const void*, const void* );

// Osszehasonlito fuggveny kifejtese a qsorthoz
int comparePair( pair *a, pair *b ) {
    return a->p - b->p;
}

/**
 *  Letrehozza a keresest gyorsito adatstrukturat. (magyaran indexel)
 */
void atVertex_update( tTetranet tn ) {

    tTetraRef *tArray = NULL;
    tPointRef *iArray = NULL;

    pair *tempArray = NULL;
    unsigned long i, k;
    tTetraRef nrOfTetras = tn->lastTetraRef;

    // memoriafoglalas a tetraederindexeknek
    tArray = malloc( nrOfTetras * 4 * sizeof( tTetraRef ) );
    // memoriafoglalas az indexek indexeinek
    iArray = malloc(( tn->lastPointRef + 3 ) * sizeof( tPointRef ) ); // igen, + 3, hogy lassuk a veget is.,
    // ideiglen tomb, az update vegen fel kell szabaditani
    tempArray = malloc( nrOfTetras * 4 * sizeof( pair ) );

    unsigned long idx = 0;
    for( i = 1; i <= nrOfTetras; ++i ) {
        for( k = 0; k <= 3; ++k ) {
            tempArray[idx].p = tn->vertices[i][k];
            tempArray[idx].t = i;
            ++idx;
        }
    }

    // rendezes
    qsort(( void * ) tempArray,
          nrOfTetras * 4,
          sizeof( pair ),
          ( compfn ) comparePair );

    tPointRef temp = NULL_POINT;
    for( idx = 0; idx < nrOfTetras * 4; ++idx ) {
        tArray[idx] = tempArray[idx].t;
        if( temp != tempArray[idx].p ) {
            temp = tempArray[idx].p;
            iArray[tempArray[idx].p] = idx;
        }
    }
    // a +1. elem arra kell, hogy az utolso sorozat veget is megtalaljuk
    iArray[tn->lastPointRef + 1] = nrOfTetras * 4;
    // a +2. elem arra kell, hogy tudjuk, hogy hol az indextomb utolso eleme
    iArray[tn->lastPointRef + 2] = 0;
    free( tempArray );

    free( tn->atVertexMain );
    free( tn->atVertexIdx );
    tn->atVertexMain = tArray;
    tn->atVertexIdx  = iArray;
}

/**
 *  Elokeszuleti lepes adott ponthoz valo kereseshez.
 *  Eloszor mindig ezt kell meghivni, csak utana lehet az atVertex_getNext-et
 *  @param p a kerdeses pont indexe
 *  @return FALSE, ha pont nem letezik.
 */
bool atVertex_init( tTetranet tn, tPointRef p ) {
    if( p > tn->lastPointRef ) {
        tn->atVertexActIndex = 0;
        return FALSE;
    } else {
        tn->atVertexActIndex = tn->atVertexIdx[p];
        tn->atVertexLastIndex = tn->atVertexIdx[p+1] - 1;
        return TRUE;
    }
}

/**
 *  Az adott ponthoz tartozo elso tetraeder indexet adja vissza.
 *  Eloszor mindig ezt kell meghivni, csak utana lehet a atVertex_getNext-et
 *  Parametere nincs, a atVertex_getFirst-ben megadott ponthoz keres.
 *  @return a ponthoz tartozo elso tetraeder indexe, ha hibas a pontindex, akkor NULL_TETRA
 */
tTetraRef atVertex_next( tTetranet tn ) {
    if( tn->atVertexActIndex <= tn->atVertexLastIndex ) {
        tTetraRef tr = tn->atVertexMain[tn->atVertexActIndex];
        ++( tn->atVertexActIndex );
        return tr;
    } else return NULL_TETRA;
}

void atVertex_freeMem( tTetranet tn ) {
    free( tn->atVertexMain );
    free( tn->atVertexIdx );
}

void atVertex_insert( tTetranet tn, tTetraRef tr ) {
    atVertex_freeMem( tn );
    atVertex_update( tn );
}

