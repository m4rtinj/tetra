/**
 *  Adott ponthoz tartozo tetraederek keresese
 *  Hash tablas megoldas.
 *
 *  IndexArray
 *     |-->p1= tp11 -> tp12 ->  ... tp1n
 *     |
 *     |-->p2= tp21 -> tp22 ->  ... tp2m
 *     ...
 *     |-->pn= ...
 *
 */
#include <stdlib.h>
#include <string.h>
#include "atvertex.h"
#include "tetranet.h"
#include "common.h"
#include "errors.h"

struct tElementX {
    tTetraRef tRef;
    struct tElementX *next;
};
typedef struct tElementX tElement;

typedef struct {
    tElement **idxArr; // pointerek dinamikus tombje
    tPointRef maxPr;
    tElement *act;
} tAtVertexDesc;

/**
 *  Letrehozza a keresest gyorsito adatstrukturat. (magyaran indexel)
 */
void atVertex_update( tTetranet tn ) {
    unsigned long len;
    tTetraRef tr;
    tAtVertexDesc *atv;
    tElement *dummyPointer;

    // elokeszites
    atVertex_freeMem( tn );
    atv = malloc( sizeof( tAtVertexDesc ) );
    /* csak mert nem akarok referenciat szamossagkent hasznalni, kiszedtem:
    atv->maxPr = tetranet_getLastPointRef( tn );
    */
    atv->maxPr = tetranet_getNumberOfPoints( tn );
    len = ( atv->maxPr + 1 ) * sizeof( dummyPointer );
    atv->idxArr = malloc( len );
    memset( atv->idxArr, '\0', len );
    atv->act = NULL;
    tn->atVertex = atv;
    // feltoltes
    tetranet_iteratorInit( tn );
    while(( tr = tetranet_iteratorNext( tn ) ) != NULL_TETRA ) {
        atVertex_insert( tn, tr );
    }
}

/**
 *  Elokeszuleti lepes adott ponthoz valo kereseshez.
 *  Eloszor mindig ezt kell meghivni, csak utana lehet az atVertex_getNext-et
 *  @param p a kerdeses pont indexe
 *  @return FALSE, ha a pont nincs indexelve
 */
bool atVertex_init( tTetranet tn, tPointRef p ) {
    tAtVertexDesc *atv = tn->atVertex;
    if( p > atv->maxPr ) {
        atv->act = NULL;
        return FALSE;
    } else {
        atv->act = atv->idxArr[p];
        return TRUE;
    }
}

/**
 *  Az adott ponthoz tartozo kovetkezo tetraeder indexet adja vissza.
 *  Csak az init utan van ertelme meghivni, ott kell megadni, hogy melyik ponthoz keresunk.
 *  @return a ponthoz tartozo kovetkezo tetraeder indexe, ha hibas a pontindex, akkor NULL_TETRA
 */
tTetraRef atVertex_next( tTetranet tn ) {
    tAtVertexDesc *atv = tn->atVertex;
    if( atv->act == NULL ) {
        return NULL_TETRA;
    } else {
        tTetraRef tr = atv->act->tRef;
        atv->act = atv->act->next;
        return tr;
    }
}

void atVertex_freeMem( tTetranet tn ) {
    tAtVertexDesc *atv = tn->atVertex;
    tElement *tmp;

    if( atv != NULL ) {
        unsigned long i = 0;
        for(i=0;i<=atv->maxPr;++i){
            while(atv->idxArr[i]!=NULL){
                tmp = atv->idxArr[i]->next;
                free(atv->idxArr[i]);
                atv->idxArr[i] = tmp;
            }
        }
        free(atv->idxArr);
        free(tn->atVertex);
        tn->atVertex = NULL;
    }
}

void atVertex_insert( tTetranet tn, tTetraRef tr ) {
    tAtVertexDesc *atv = tn->atVertex;
    unsigned i;
    tElement *elem;
    tPointRef pr, k;

    for( i = 0; i <= 3; i++ ) {
        pr = tetranet_getVertex( tn, tr, i );
        if( pr > atv->maxPr ) {
            // boviteni kell az indextombot. legyen 110%-os az uj meret
            atv->idxArr = realloc( atv->idxArr, ( pr + pr / 10 + 1 ) * sizeof( elem ) );
            for( k = atv->maxPr + 1; k <= pr; ++k ) {
                atv->idxArr[k] = NULL;
            }
            atv->maxPr = pr;
        }
        elem = malloc( sizeof( tElement ) );
        elem->next = atv->idxArr[pr];
        elem->tRef = tr;
        atv->idxArr[pr] = elem;
    }
}

void atVertex_delete( tTetranet tn, tTetraRef tr ) {
    int k;
    tElement *elem;
    tElement *prev;
    tPointRef pr;
    tAtVertexDesc *atv = tn->atVertex;
    bool done = FALSE;

    for( k = 0; k <= 3; k++ ) {
        pr = tetranet_getVertex( tn, tr, k );
        elem = atv->idxArr[pr];
        prev = NULL;
        done = FALSE;
        while( !done ) {
            if( elem == NULL ) {
                exitText( "atVertex_delete error: vertex isn't indexed." );
            }
            if( elem->tRef == tr ) {
                if( prev == NULL ) {
                    atv->idxArr[pr] = elem->next;
                } else {
                    prev->next = elem->next;
                }
                free( elem );
                done = TRUE;
            }
            prev = elem;
            elem = elem->next;
        }
    }
}


