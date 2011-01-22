/*
 *  atvertex.h
 *  
 *  Adott ponthoz tartozo tetraederek keresese.
 *  2010-2011 - Martin Jozsef
 */

#ifndef ATVERTEX_H_
#define ATVERTEX_H_

#include "tetranet.h"
#include "common.h"

/**
 *  Letrehozza a keresest gyorsito adatstrukturat. (magyaran indexel)
 */
void atVertex_update( tTetranet tn );

/**
 *  Elokeszuleti lepes adott ponthoz valo kereseshez.
 *  Eloszor mindig ezt kell meghivni, csak utana lehet a atVertex_getNext-et
 *  @param p a kerdeses pont indexe
 *  @return TRUE, ha van a ponthoz tetraeder tarolva.
 */
bool atVertex_init( tTetranet tn, tPointRef p );

/**
 *  Az adott ponthoz tartozo kovetkezo tatraeder indexet adja vissza.
 *  Parametere nincs, a atVertex_getFirst-ben megadott ponthoz keres.
 *  @return a ponthoz tartozo kovetkezo tetraeder indexe, ha nincs tobb, akkor NULL_TETRA
 */
tTetraRef atVertex_next( tTetranet tn );

/**
 *  A lefoglalt területek felszabaditasa
 */
void atVertex_free( tTetranet tn );

/**
 *  Uj tetraeder indexelese
 *  A rutin johiszemuen feltetelezi, hogy a tetraeder meg nem szerepel.
 */
void atVertex_insert( tTetranet tn, tTetraRef tr );

/**
 *  Tetraeder eltavolitasa az atvertex nyilvantartasbol
 *  A rutin johiszemuen feltetelezi, hogy a tetraeder korabban fel lett veve.
 */
void atVertex_delete( tTetranet tn, tTetraRef tr );
#endif /* ATVERTEX_H_ */

