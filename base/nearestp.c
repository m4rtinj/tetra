/*
 * nearestp.c
 *
 * a kd-tree wikipedia szocikk alapjan: kiegyensulyozott kd-tree
 *
 *  Created on: 26.09.2010
 *      Author: martinj
 */

#include "nearestp.h"
#include "tetranet.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct sctNode {
    tPointRef value;
    struct sctNode *left;
    struct sctNode *right;
} node;

node *rootNode;

// Osszehasonlito fuggveny def a qsorthoz
typedef int ( *compfn )( const void*, const void* );

// Osszehasonlito fuggveny kifejtesei a qsorthoz
int compareByX( tPoint *a, tPoint *b ) {
    return a->x > b->x;
}
int compareByY( tPoint *a, tPoint *b ) {
    return a->y > b->y;
}
int compareByZ( tPoint *a, tPoint *b ) {
    return a->z > b->z;
}

typedef struct {
    tPointRef idx;
    double x;
    double y;
    double z;
} tmpElement;

tmpElement *tmpArray;

node *buildKdTree( tPointRef first, tPointRef last, unsigned int depth ) {
    node *tmpNode;
    tPointRef median;

    if( last < first ) {
        return NULL;
    } else {
        // sort by axis
        switch( depth % 3 ) {
        case 0:
            qsort(( void * ) &tmpArray[first], last - first, sizeof( tmpElement ), ( compfn ) compareByX );
            break;
        case 1:
            qsort(( void * ) &tmpArray[first], last - first, sizeof( tmpElement ), ( compfn ) compareByY );
            break;
        case 2:
            qsort(( void * ) &tmpArray[first], last - first, sizeof( tmpElement ), ( compfn ) compareByZ );
            break;
        }
        // Sort point list and choose median as pivot element
        median = ( first + last ) / 2;
        tmpNode = malloc( sizeof( node ) );
        tmpNode->value = tmpArray[median].idx;
        tmpNode->left = buildKdTree( first, median - 1, depth + 1 );
        tmpNode->right = buildKdTree( median + 1, last, depth + 1 );

        return tmpNode;
    }
}

void nearestp_update( tTetranet tn ) {
    tPointRef i = 1;
    tPoint p;
    unsigned long nbp = tetranet_getNumberOfPoints( tn );
    tmpArray = malloc(( nbp + 1 ) * sizeof( tmpElement ) );
    for( i = nbp; i != 0; --i ) {
        p = tetranet_getPoint( tn, i );
        tmpArray[i].idx = i;
        tmpArray[i].x = p.x;
        tmpArray[i].y = p.y;
        tmpArray[i].z = p.z;
    }
    rootNode = buildKdTree( 1, nbp, 0 );
    free( tmpArray );
}

inline double distance( tPoint a, tPoint b ) {
    return ( b.x - a.x ) * ( b.x - a.x ) +
           ( b.y - a.y ) * ( b.y - a.y ) +
           ( b.z - a.z ) * ( b.z - a.z );
}

tPointRef kdsearch( tTetranet tn, node *here, tPoint point, tPointRef best, unsigned int depth ) {
    if( here == NULL ) {
        return best;
    }
    tPoint phere = tetranet_getPoint( tn, here->value );
    if( best == NULL_POINT ) {
        best = here->value;
    }
    tPoint pbest = tetranet_getPoint( tn, best );
    if( distance( phere, point ) < distance( pbest, point ) ) {
        best = here->value;
        pbest = tetranet_getPoint( tn, best );
    }

    double d;
    switch( depth % 3 ) {
    case 0:
        d = point.x - phere.x;
        break;
    case 1:
        d = point.y - phere.y;
        break;
    case 2:
        d = point.z - phere.z;
        break;
    }

    if( d < 0 ) {
        best = kdsearch( tn, here->left, point, best, depth + 1 );
        if(( d * d ) < distance( pbest, point ) ) {
            best =  kdsearch( tn, here->right, point, best, depth + 1 );
        }
    } else {
        best = kdsearch( tn, here->right, point, best, depth + 1 );
        if(( d * d ) < distance( pbest, point ) ) {
            best =  kdsearch( tn, here->left, point, best, depth + 1 );
        }
    }

    return best;
}

tPointRef nearestp_search( tTetranet tn, tPoint p ) {
    tPointRef pr;
    pr = kdsearch( tn, rootNode, p, NULL_POINT , 0 );
    return pr;
}

void nearestp_addPoint( tTetranet tn, tPointRef p ) {

}




