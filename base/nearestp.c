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

typedef struct {
    tPointRef idx;
    double x;
    double y;
    double z;
} tmpElement;

tmpElement *tmpArray;

void printNode( node *n, int depth ) {
    if( n != NULL ) {
        if( n->left != NULL ) printf( "%d: %ld -> %ld\n", depth, n->value, n->left->value );
        if( n->right != NULL ) printf( "%d: %ld -> %ld\n", depth, n->value, n->right->value );
        if(( n->left == NULL ) && ( n->right == NULL ) ) printf( "%ld egy level.\n", n->value );
        ++depth;
        printNode( n->left, depth );
        printNode( n->right, depth );
    }
}

void printTree( tTetranet tn ) {
    printf( "\ndigraph G{" );
    printNode(( node* )( tn->nearestp ), 0 );
    printf( "}\n" );
}

// Osszehasonlito fuggveny def a qsorthoz
typedef int ( *compfn )( const void*, const void* );

// Osszehasonlito fuggveny kifejtesei a qsorthoz
int compareByX( const tmpElement *a, const tmpElement *b ) {
    double temp = ( a->x ) - ( b->x );
    if( temp > 0.0 ) return 1;
    else if( temp < 0.0 ) return -1;
    else return 0;
}

int compareByY( const tmpElement *a, const tmpElement *b ) {
    double temp = ( a->y ) - ( b->y );
    if( temp > 0.0 ) return 1;
    else if( temp < 0.0 ) return -1;
    else return 0;
}

int compareByZ( const tmpElement *a, const tmpElement *b ) {
    double temp = ( a->z ) - ( b->z );
    if( temp > 0.0 ) return 1;
    else if( temp < 0.0 ) return -1;
    else return 0;
}

node *buildKdTree( tPointRef first, tPointRef last, unsigned int depth ) {
    node *tmpNode;
    tPointRef median;

    if( last < first ) {
        return NULL;
    } else {
        // sort by axis
        switch( depth % 3 ) {
        case 0:
            qsort(( void * ) &tmpArray[first], 1 + last - first, sizeof( tmpElement ), ( compfn ) compareByX );
            break;
        case 1:
            qsort(( void * ) &tmpArray[first], 1 + last - first, sizeof( tmpElement ), ( compfn ) compareByY );
            break;
        case 2:
            qsort(( void * ) &tmpArray[first], 1 + last - first, sizeof( tmpElement ), ( compfn ) compareByZ );
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
    tn->nearestp = buildKdTree( 1, nbp, 0 );
    free( tmpArray );
//   printTree( tn );
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

    if( best == NULL_POINT ) {
        best = here->value;
    }
    tPoint phere = tetranet_getPoint( tn, here->value );
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
        pbest = tetranet_getPoint( tn, best );
        if(( d * d ) < distance( pbest, point ) ) {
            best =  kdsearch( tn, here->right, point, best, depth + 1 );
        }
    } else {
        best = kdsearch( tn, here->right, point, best, depth + 1 );
        pbest = tetranet_getPoint( tn, best );
        if(( d * d ) < distance( pbest, point ) ) {
            best =  kdsearch( tn, here->left, point, best, depth + 1 );
        }
    }
    return best;
}

tPointRef nearestp_search( tTetranet tn, tPoint p ) {
    return kdsearch( tn, tn->nearestp, p, NULL_POINT , 0 );
}

void nearestp_addPoint( tTetranet tn, tPointRef pr ) {
    unsigned int depth = 0;
    node *parent = tn->nearestp;
    node *newNode =  malloc( sizeof( node ) );
    bool found = FALSE;
    tPoint p = tetranet_getPoint( tn, pr );
    tPoint h;
    double d;

    newNode->left = NULL;
    newNode->right = NULL;
    newNode->value = pr;

    while( !found ) {
        h = tetranet_getPoint( tn, parent->value );
        switch( depth % 3 ) {
        case 0:
            d = p.x - h.x;
            break;
        case 1:
            d = p.y - h.y;
            break;
        case 2:
            d = p.z - h.z;
            break;
        }
        if( d < 0 ) {
            if( parent->left == NULL ) {
                parent->left = newNode;
                found = TRUE;
            } else {
                parent = parent->left;
            }
        } else {
            if( parent->right == NULL ) {
                parent->right = newNode;
                found = TRUE;
            } else {
                parent = parent->right;
                ++depth;
            }
        }
    }
}

void freeNode( node *n ) {
    if( n != NULL ) {
        freeNode( n->left );
        freeNode( n->right );
        free( n );
    }
}

void nearestp_free( tTetranet tn ) {
    freeNode( tn->nearestp );
    tn->nearestp = NULL;
}

