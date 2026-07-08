/**
 * @file GameWorld.c
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "GameWorld.h"
#include "Macros.h"
#include "ResourceManager.h"
#include "Utils.h"

typedef enum ColorLimit {
    COLOR_LIMIT_PRIMARY = 2,
    COLOR_LIMIT_SECONDARY = 5,
    COLOR_LIMIT_TERTIARY = 11
} ColorLimit;

static void createHexGrid( GameWorld *gw, int q, float radius );
static void connectHexGrid( Hex *hexGrid, int hexCount );
static void connectHexToNeighbors( int sourceIndex, Hex *hexGrid, int hexCount );

static unsigned int pollColorQueue( void );
static void offerColorQueue( unsigned int color );
static void feedColorQueue( bool randomize, int colorLimitIndex );

static int checkAndBlend( Hex *h );

static void drawHud( GameWorld *gw );

static Hex *mouseOverHex = NULL;
static Hex mouseHoverDrawHex = {
    .center = { 0 },
    .radius = 0,
    .apothem = 0,
    .color = 0,
    .neighbors = { 0 }
};
static Hex queueDrawHex = {
    .center = { 0 },
    .radius = 15,
    .apothem = 0,
    .color = 0,
    .neighbors = { 0 }
};

static int gridQuantities[] = { 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41 };
static int gridRadii[] = { 100, 60, 44, 36, 30, 26, 23, 21, 19, 18, 17, 16, 15, 14, 13, 12, 11, 11, 10, 10 };

static unsigned int availableColors[] = {
    // primary
    HEX_RED,
    HEX_YELLOW,
    HEX_BLUE,
    // secondary
    HEX_ORANGE,
    HEX_GREEN,
    HEX_PURPLE,
    // tertiary
    HEX_RED_ORANGE, 
    HEX_YELLOW_ORANGE, 
    HEX_YELLOW_GREEN, 
    HEX_BLUE_GREEN, 
    HEX_BLUE_PURPLE, 
    HEX_RED_PURPLE, 
};

static unsigned int colorQueue[COLOR_QUEUE_CAPACITY] = { 0 };
static int colorQueueStart = -1;
static int colorQueueEnd = -1;
static int colorQueueSize = 0;

static int gridId = 2;
static ColorLimit colorLimit = COLOR_LIMIT_PRIMARY;
static bool randomizeColorQueueFeeder = true;
static bool showHexConnections = false;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    int gridIdCount = ( sizeof( gridQuantities ) / sizeof( gridQuantities[0] ) );

    for ( int i = 0; i < gridIdCount; i++ ) {
        trace( 
            "%d %.2f %.2f %.2f", 
            gridQuantities[i], 
            gridRadii[i], 
            apothem( gridRadii[i] ), 
            apothem( gridRadii[i] ) * 2 * gridQuantities[i]
        );
    }

    gridId = clampInt( gridId, 0, gridIdCount - 1 );

    createHexGrid( gw, gridQuantities[gridId], gridRadii[gridId] );
    connectHexGrid( gw->hexGrid, gw->hexCount );
    gw->score = 0;

    for ( int i = 0; i < COLOR_QUEUE_CAPACITY; i++ ) {
        feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
    }

    return gw;

}

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw ) {
    free( gw );
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {
    
    mouseOverHex = getHexByPoint( gw->hexGrid, gw->hexCount, GetMousePosition() );

    if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
        if ( mouseOverHex != NULL && mouseOverHex->color == HEX_BLANK_COLOR ) {
            mouseOverHex->color = pollColorQueue();
            gw->score += checkAndBlend( mouseOverHex );
            feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
        }
    }

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( BLACK );

    drawHexGrid( gw->hexGrid, gw->hexCount, showHexConnections );

    if ( mouseOverHex != NULL ) {
        mouseHoverDrawHex.center = mouseOverHex->center;
        mouseHoverDrawHex.radius = mouseOverHex->radius;
        drawHexHighlight( &mouseHoverDrawHex );
    }
    
    drawHud( gw );

    EndDrawing();

}

static void createHexGrid( GameWorld *gw, int centerLineQuantity, float radius ) {

    gw->hexCount = 0;
    
    float hApothem = apothem( radius );
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
            if ( gw->hexCount < MAX_HEX_GRID_COUNT ) {
                initHex( &gw->hexGrid[gw->hexCount++], center, radius );
            }
        }
        if ( decrease ) {
            lineQuantity--;
        } else {
            lineQuantity++;
        }
    }

}

static void connectHexGrid( Hex *hexGrid, int hexCount ) {
    for ( int i = 0; i < hexCount; i++ ) {
        connectHexToNeighbors( i, hexGrid, hexCount );
    }
}

static void connectHexToNeighbors( int sourceIndex, Hex *hexGrid, int hexCount ) {

    // probe scan
    Hex *source = &hexGrid[sourceIndex];
    int angle = 0;
    int angleInc = 60;
    int targetFoundCount = 0;

    for ( int p = 0; p < 6; p++ ) {
        Vector2 probe = {
            source->center.x + ( source->apothem * 2 ) * cosf( DEG2RAD * angle ),
            source->center.y + ( source->apothem * 2 ) * sinf( DEG2RAD * angle )
        };
        for ( int t = 0; t < hexCount; t++ ) {
            Hex *target = &hexGrid[t];
            if ( source != target ) {
                if ( CheckCollisionCircles( probe, 10, target->center, 10 ) ) {
                    source->neighbors[targetFoundCount++] = target;
                    break;
                }
            }
        }
        angle += angleInc;
        if ( targetFoundCount == 6 ) {
            return;
        }
    }

}

static unsigned int pollColorQueue( void ) {
    unsigned int color = HEX_BLANK_COLOR;
    if ( colorQueueStart != -1 ) {
        color = colorQueue[colorQueueStart%COLOR_QUEUE_CAPACITY];
        colorQueueStart++;
        if ( colorQueueStart > colorQueueEnd ) {
            colorQueueStart = -1;
            colorQueueEnd = -1;
        }
        colorQueueSize--;
    } else {
        trace( "empty queue!" );
    }
    return color;
}

static void offerColorQueue( unsigned int color ) {
    if ( colorQueueSize < COLOR_QUEUE_CAPACITY ) {
        if ( colorQueueStart == -1 ) {
            colorQueueStart = 0;
            colorQueueEnd = 0;
            colorQueue[colorQueueEnd] = color;
        } else {
            colorQueueEnd++;
            colorQueue[colorQueueEnd%COLOR_QUEUE_CAPACITY] = color;
        }
        colorQueueSize++;
    }
}

static void feedColorQueue( bool randomize, int colorLimitIndex ) {

    static int sequentialPos = 0;

    if ( randomize ) {
        offerColorQueue( availableColors[GetRandomValue( 0, colorLimitIndex )] );
    } else {
        offerColorQueue( availableColors[sequentialPos % (colorLimitIndex+1)] );
        sequentialPos++;
    }

}

// checks all neightbors and blend with the last compatible
static int checkAndBlend( Hex *h ) {

    int points = 0;
    int lastBlendColor =  HEX_BLANK_COLOR;

    for ( int i = 0; i < 6; i++ ) {
        Hex *t = h->neighbors[i];
        if ( t != NULL ) {
            int blendColor = colorBlend( h->color, t->color );
            if ( blendColor != HEX_BLANK_COLOR ) {
                lastBlendColor = blendColor;
                t->color = HEX_BLANK_COLOR;
                points++;
            }
        }
    }

    if ( lastBlendColor != HEX_BLANK_COLOR ) {
        h->color = lastBlendColor;
    }

    return points;

}

static void drawHud( GameWorld *gw ) {

    const int fontSize = 20;
    int spacing = 20;
    int startX = GetScreenWidth() - ( colorQueueSize * queueDrawHex.radius + ( colorQueueSize - 1 ) * spacing ) - 10;

    DrawText( TextFormat( "Score: %d", gw->score ), 15, 15, fontSize, RAYWHITE );
    const char *nextColorLabel = "Next color ->";
    int w = MeasureText( nextColorLabel, fontSize );
    DrawText( nextColorLabel, startX - w - 20, 15, fontSize, RAYWHITE );
    
    for ( int i = 0; i < colorQueueSize; i++ ) {
        queueDrawHex.center.x = startX + ( queueDrawHex.radius + spacing ) * i;
        queueDrawHex.center.y = 25;
        queueDrawHex.color = colorQueue[(i+colorQueueStart)%COLOR_QUEUE_CAPACITY];
        drawHex( &queueDrawHex );
        if ( i == 0 ) {
            drawHexHighlight( &queueDrawHex );
        }
    }

    DrawFPS( 10, GetScreenHeight() - 25 );

}
