#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Macros.h"
#include "MergeAnimation.h"
#include "Hex.h"

static void update( MergeAnimation *ma, float delta );
static void draw( MergeAnimation *ma );

static const float animationUnitTime = 1.0f;

void initMergeAnimation( MergeAnimation *ma ) {
    ma->running = false;
    ma->update = update;
    ma->draw = draw;
}

void prepareMergeAnimation( MergeAnimation *ma, Hex *centralHex ) {
    ma->centralHex = *centralHex;
    ma->neighborCount = 0;
    ma->animationUnitCounter = animationUnitTime;
    ma->running = true;
    ma->prepareNextAnimationInstance = true;
}

void addMergeAnimationNeighbor( MergeAnimation *ma, Hex *neighborHex ) {

    trace( "x" );
    if ( ma->neighborCount < 6 ) {
        ma->neighborsToMerge[5 - ma->neighborCount] = *neighborHex;
        ma->neighborCount++;
    } else {
        trace( "merge animation neighbor overflow" );
    }

}

static void update( MergeAnimation *ma, float delta ) {

    if ( ma->running ) {

        Hex *neighbor = &ma->neighborsToMerge[ma->neighborCount-1];

        if ( ma->prepareNextAnimationInstance ) {
            ma->neighborToMergeStartPos = neighbor->center;
            ma->neighborToMergeStartRadius = neighbor->radius;
            ma->neighborToMergeStartColor = neighbor->color;
            ma->centralHexStartColor = ma->centralHex.color;
            ma->prepareNextAnimationInstance = false;
        }

        if ( ma->animationUnitCounter > 0.0f ) {
            ma->animationUnitCounter -= delta;
            float perc = ma->animationUnitCounter / animationUnitTime;
            Vector2 newPos = Vector2Lerp( ma->centralHex.center, ma->neighborToMergeStartPos, perc );
            float newRadius = Lerp( ma->neighborToMergeStartRadius / 10, ma->neighborToMergeStartRadius, perc );
            Color newColor = ColorLerp( GetColor( ma->neighborToMergeStartColor ), GetColor( ma->centralHexStartColor ), perc );
            neighbor->center = newPos;
            neighbor->radius = newRadius;
            ma->centralHex.color = ColorToInt( newColor );
        } else {
            ma->neighborCount--;
            ma->animationUnitCounter = animationUnitTime;
            if ( ma->neighborCount == 0 ) {
                ma->running = false;
            } else {
                ma->prepareNextAnimationInstance = true;
            }
        }

    }

}

static void draw( MergeAnimation *ma ) {
    if ( ma->running ) {
        drawHex( &ma->centralHex );
        for ( int i = 0; i < ma->neighborCount; i++ ) {
            drawHex( &ma->neighborsToMerge[i] );
        }
    }
}