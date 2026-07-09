#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Hex.h"
#include "LevelTransitionAnimation.h"
#include "Macros.h"
#include "Utils.h"

static void update( LevelTransitionAnimation *lta, float delta );
static void initSimpleHex( SimpleHex *h, Vector2 center, float radius );
static void draw( LevelTransitionAnimation *lta );
static void drawSimpleHex( SimpleHex* h, unsigned int strokeColor );
static void drawSimpleHexGrid( SimpleHex *hexGrid, int hexCount, unsigned int strokeColor );
static void prepareSimpleHexGrid( SimpleHex *hexGrid, int centerLineQuantity, float hexRadius, int *hexCount );
static void calculateTargetAttributes( LevelTransitionAnimation *lta );

static float explodingTime = 1.0f;
static float explodingCounter = 0.0f;

static float showingTime = 1.0f;
static float showingCounter = 0.0f;
static unsigned int showingColor = 0;

void initLevelTransitionAnimation( LevelTransitionAnimation *lta ) {
    lta->running = false;
    lta->update = update;
    lta->draw = draw;
}

void prepareLevelTransitionAnimation( LevelTransitionAnimation *lta, int chgCenterLineQuantity, float chgHexRadius, int nhgCenterLineQuantity, float nhgHexRadius, Hex *hexGrid, int hexCount ) {

    lta->chgCenterLineQuantity = chgCenterLineQuantity;
    lta->chgCount = 0;
    lta->chgHexRadius = chgHexRadius;
    lta->nhgCenterLineQuantity = nhgCenterLineQuantity;
    lta->nhgCount = 0;
    lta->nhgHexRadius = nhgHexRadius;
    lta->running = true;

    prepareSimpleHexGrid( lta->currentHexGrid, lta->chgCenterLineQuantity, lta->chgHexRadius, &lta->chgCount );
    prepareSimpleHexGrid( lta->nextHexGrid, lta->nhgCenterLineQuantity, lta->nhgHexRadius, &lta->nhgCount );

    // the current grid colors must be loaded before scattering them into the
    // next grid, otherwise calculateTargetAttributes copies blank colors
    if ( lta->chgCount == hexCount ) {
        for ( int i = 0; i < lta->chgCount; i++ ) {
            lta->currentHexGrid[i].color = hexGrid[i].color;
        }
    } else {
        trace( "incompatible sizes!" );
    }

    calculateTargetAttributes( lta );

    explodingCounter = 0.0f;
    showingCounter = 0.0f;
    showingColor = ColorToInt( BLANK );
    lta->state = LTA_STATE_EXPLODING_CURRENT_GRID;

}

void copyColorDataFromNextHexGrid( LevelTransitionAnimation *lta, Hex *hexGrid, int hexCount ) {

    if ( lta->nhgCount == hexCount ) {
        for ( int i = 0; i < lta->nhgCount; i++ ) {
            hexGrid[i].color = lta->nextHexGrid[i].color;
        }
    } else {
        trace( "incompatible sizes!" );
    }

}

static void update( LevelTransitionAnimation *lta, float delta ) {

    if ( !lta->running ) {
        return;
    }

    if ( lta->state == LTA_STATE_EXPLODING_CURRENT_GRID ) {

        float perc = explodingCounter / explodingTime;
        float easedPerc = ( 1.0f - cosf( perc * PI ) ) / 2.0f;

        for ( int i = 0; i < lta->chgCount; i++ ) {
            SimpleHex *h = &lta->currentHexGrid[i];
            h->currentCenter = Vector2Lerp( h->startCenter, h->targetCenter, easedPerc );
            h->currentRadius = Lerp( h->startRadius, h->targetRadius, easedPerc );
        }

        explodingCounter += delta;

        if ( explodingCounter >= explodingTime ) {
            lta->state = LTA_STATE_SHOWING_NEXT_GRID;
            for ( int i = 0; i < lta->chgCount; i++ ) {
                SimpleHex *h = &lta->currentHexGrid[i];
                h->currentCenter = h->targetCenter;
            }
        }

    } else if ( lta->state == LTA_STATE_SHOWING_NEXT_GRID ) {

        float perc = showingCounter / showingTime;
        float easedPerc = ( 1.0f - cosf( perc * PI ) ) / 2.0f;
        float alpha = 1.0f * easedPerc;
        alpha = Clamp( alpha, 0.0f, 1.0f );

        showingColor = ColorToInt( Fade( DARKGRAY, alpha ) );
        showingCounter += delta;

        if ( showingCounter >= showingTime ) {
            // end on this same frame so the game switches to the playing state
            // and draws the real grid, avoiding a frame with nothing drawn
            lta->state = LTA_STATE_FINISHED;
            lta->running = false;
        }

    }

}

static void initSimpleHex( SimpleHex *h, Vector2 center, float radius ) {
    h->currentCenter = center;
    h->startCenter = center;
    h->currentRadius = radius;
    h->startRadius = radius;
    h->apothem = apothem( h->currentRadius );
    h->color = HEX_BLANK_COLOR;
}

static void draw( LevelTransitionAnimation *lta ) {

    if ( lta->running ) {
        if ( lta->state == LTA_STATE_EXPLODING_CURRENT_GRID ) {
            drawSimpleHexGrid( lta->currentHexGrid, lta->chgCount, ColorToInt( DARKGRAY ) );
        } else if ( lta->state == LTA_STATE_SHOWING_NEXT_GRID ) {
            drawSimpleHexGrid( lta->currentHexGrid, lta->chgCount, ColorToInt( DARKGRAY ) );
            drawSimpleHexGrid( lta->nextHexGrid, lta->nhgCount, showingColor );
        }
    }

}

static void drawSimpleHex( SimpleHex *h, unsigned int strokeColor ) {
    DrawPoly( h->currentCenter, 6, h->currentRadius, 90.0f, GetColor( h->color ) );
    if ( h->color != HEX_BLANK_COLOR ) {
        strokeColor = h->color;
    }
    DrawPolyLines( h->currentCenter, 6, h->currentRadius, 90.0f, GetColor( strokeColor ) );
}

static void drawSimpleHexGrid( SimpleHex *hexGrid, int hexCount, unsigned int strokeColor ) {
    for ( int i = 0; i < hexCount; i++ ) {
        SimpleHex *h = &hexGrid[i];
        drawSimpleHex( h, strokeColor );
    }
}

static void prepareSimpleHexGrid( SimpleHex *hexGrid, int centerLineQuantity, float hexRadius, int *hexCount ) {

    int hc = 0;
    
    float hApothem = apothem( hexRadius );
    float vApothem = apothem( 2 * hApothem );
    float startX = GetScreenWidth() / 2 - ( hApothem * 2 * centerLineQuantity ) / 2 + hApothem;
    float startY = GetScreenHeight() / 2 - ( vApothem * centerLineQuantity ) / 2 + vApothem / 2;
    
    bool decrease = false;
    int linesAboveAndBelow = centerLineQuantity / 2;
    int lineQuantity = centerLineQuantity - linesAboveAndBelow;

    for ( int i = 0; i < centerLineQuantity; i++ ) {
        if ( i == linesAboveAndBelow ) {
            decrease = true;
        }
        int offset = ( centerLineQuantity - lineQuantity ) * hApothem;
        for ( int j = 0; j < lineQuantity; j++ ) {
            Vector2 center = { startX + hApothem * 2 * j + offset, startY + vApothem * i };
            if ( hc < MAX_HEX_GRID_COUNT ) {
                initSimpleHex( &hexGrid[hc++], center, hexRadius );
            }
        }
        if ( decrease ) {
            lineQuantity--;
        } else {
            lineQuantity++;
        }
    }

    *hexCount = hc;

}

static void calculateTargetAttributes( LevelTransitionAnimation *lta ) {

    // random positions
    int *posArray = (int*) malloc( sizeof( int ) * lta->nhgCount );
    for ( int i = 0; i < lta->nhgCount; i++ ) {
        posArray[i] = i;
    }
    for ( int i = 0; i < lta->nhgCount; i++ ) {
        int randomPos = GetRandomValue( 0, lta->nhgCount-1 );
        int temp = posArray[i];
        posArray[i] = posArray[randomPos];
        posArray[randomPos] = temp;
    }

    for ( int i = 0; i < lta->chgCount; i++ ) {
        int pos = posArray[i];
        lta->currentHexGrid[i].targetCenter = lta->nextHexGrid[pos].startCenter;
        lta->currentHexGrid[i].targetRadius = lta->nextHexGrid[pos].startRadius;
        lta->nextHexGrid[pos].color = lta->currentHexGrid[i].color;
    }

    free( posArray );

}
