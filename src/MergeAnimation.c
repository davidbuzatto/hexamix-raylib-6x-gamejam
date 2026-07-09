#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Macros.h"
#include "MergeAnimation.h"
#include "Hex.h"

static void update( MergeAnimation *ma, float delta );
static void draw( MergeAnimation *ma );

static const float animationUnitTime = 0.2f;

void initMergeAnimation( MergeAnimation *ma ) {
    ma->running = false;
    ma->update = update;
    ma->draw = draw;
}

void prepareMergeAnimation( MergeAnimation *ma, Hex *centralHex ) {
    ma->centralHex = *centralHex;
    ma->centralOriginalColor = centralHex->color;
    ma->neighborCount = 0;
    ma->currentIndex = 5;
    ma->animationUnitCounter = animationUnitTime;
    ma->running = true;
    ma->prepareNextAnimationInstance = true;
    ma->inFinalTransition = false;
}

void setMergeAnimationFinalColor( MergeAnimation *ma, unsigned int finalColor ) {
    ma->finalColor = finalColor;
}

void addMergeAnimationNeighbor( MergeAnimation *ma, Hex *neighborHex, unsigned int blendColor ) {

    if ( ma->neighborCount < 6 ) {
        ma->neighborsToMerge[5 - ma->neighborCount] = *neighborHex;
        ma->neighborBlendColor[5 - ma->neighborCount] = blendColor;
        ma->neighborCount++;
    } else {
        trace( "merge animation neighbor overflow" );
    }

}

static void update( MergeAnimation *ma, float delta ) {

    if ( !ma->running ) {
        return;
    }

    if ( !ma->inFinalTransition ) {

        // per neighbor phase: the neighbor slides into the center while the
        // central hex pulses from its original color to the blend result and
        // back to the original color
        Hex *neighbor = &ma->neighborsToMerge[ma->currentIndex];

        if ( ma->prepareNextAnimationInstance ) {
            ma->neighborToMergeStartPos = neighbor->center;
            ma->neighborToMergeStartRadius = neighbor->radius;
            ma->prepareNextAnimationInstance = false;
        }

        if ( ma->animationUnitCounter > 0.0f ) {
            ma->animationUnitCounter -= delta;
            float perc = ma->animationUnitCounter / animationUnitTime;
            // sine gives a smooth fade in / fade out for the color pulse
            float pulse = sinf( ( 1.0f - perc ) * PI );
            // cosine ease-in-out remaps perc keeping the same endpoints
            float easedPerc = ( 1.0f - cosf( perc * PI ) ) / 2.0f;
            Vector2 newPos = Vector2Lerp( ma->centralHex.center, ma->neighborToMergeStartPos, easedPerc );
            float newRadius = Lerp( ma->neighborToMergeStartRadius / 10, ma->neighborToMergeStartRadius, easedPerc );
            Color newColor = ColorLerp( GetColor( ma->centralOriginalColor ), GetColor( ma->neighborBlendColor[ma->currentIndex] ), pulse );
            neighbor->center = newPos;
            neighbor->radius = newRadius;
            ma->centralHex.color = ColorToInt( newColor );
        } else {
            ma->neighborCount--;
            ma->currentIndex--;
            ma->animationUnitCounter = animationUnitTime;
            if ( ma->neighborCount == 0 ) {
                ma->inFinalTransition = true;
                ma->centralHex.color = ma->centralOriginalColor;
            } else {
                ma->prepareNextAnimationInstance = true;
            }
        }

    } else {

        // final phase: the central hex transitions from its original color to
        // the most used blend color and settles there
        if ( ma->animationUnitCounter > 0.0f ) {
            ma->animationUnitCounter -= delta;
            float perc = ma->animationUnitCounter / animationUnitTime;
            // cosine ease-in-out for a smooth fade to the final color
            float eased = ( 1.0f - cosf( ( 1.0f - perc ) * PI ) ) / 2.0f;
            Color newColor = ColorLerp( GetColor( ma->centralOriginalColor ), GetColor( ma->finalColor ), eased );
            ma->centralHex.color = ColorToInt( newColor );
        } else {
            ma->centralHex.color = ma->finalColor;
            ma->running = false;
            ma->inFinalTransition = false;
        }

    }

}

static void draw( MergeAnimation *ma ) {
    if ( ma->running ) {
        drawHex( &ma->centralHex );
        for ( int i = ma->currentIndex - ma->neighborCount + 1; i <= ma->currentIndex; i++ ) {
            drawHex( &ma->neighborsToMerge[i] );
        }
    }
}