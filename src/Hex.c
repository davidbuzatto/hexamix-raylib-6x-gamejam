#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"

#include "Hex.h"
#include "Macros.h"
#include "Utils.h"

static void drawHexConnections( Hex *h );
static void drawHexGridConnections( Hex *hexGrid, int hexCount, int hexPos );

void initHex( Hex *h, Vector2 center, float radius ) {

    h->center = center;
    h->radius = radius;
    h->apothem = apothem( h->radius );
    h->color = HEX_BLANK_COLOR;

    for ( int i = 0; i < 6; i++ ) {
        h->neighbors[i] = NULL;
    }

}

void drawHex( Hex *h ) {
    DrawPoly( h->center, 6, h->radius, 90.0f, GetColor( h->color ) );
    int strokeColor = ColorToInt( DARKGRAY );
    if ( h->color != HEX_BLANK_COLOR ) {
        strokeColor = h->color;
    }
    DrawPolyLines( h->center, 6, h->radius, 90.0f, GetColor( strokeColor ) );
}

void drawHexHighlight( Hex *h ) {
    DrawPolyLinesEx( h->center, 6, h->radius, 90.0f, 3.0f, WHITE );
}

void drawHexGrid( Hex *hexGrid, int hexCount, bool showConnections ) {
    for ( int i = 0; i < hexCount; i++ ) {
        Hex *h = &hexGrid[i];
        drawHex( h );
    }
    if ( showConnections ) {
        drawHexGridConnections( hexGrid, hexCount, -1 );
    }
}

static void drawHexConnections( Hex *h ) {
    for ( int j = 0; j < 6; j++ ) {
        Hex *t = h->neighbors[j];
        if ( t != NULL ) {
            DrawLineV( h->center, t->center, WHITE );
        }
    }
}
static void drawHexGridConnections( Hex *hexGrid, int hexCount, int hexPos ) {
    if ( hexPos >= 0 ) {
        if ( hexPos >= 0 && hexPos < hexCount ) {
            drawHexConnections( &hexGrid[hexPos] );
        }
    } else {
        for ( int i = 0; i < hexCount; i++ ) {
            drawHexConnections( &hexGrid[i] );
        }
    }
}

bool checkCollisionPointHex( Vector2 point, Hex *h ) {

    // divide the hex in 6 triangles to check
    int angle = 30;
    int angleInc = 60;

    for ( int i = 0; i < 6; i++ ) {
        Vector2 v1 = h->center;
        Vector2 v2 = {
            h->center.x + h->radius * 0.95f * cosf( DEG2RAD * angle ),
            h->center.y + h->radius * 0.95f * sinf( DEG2RAD * angle ),
        };
        Vector2 v3 = {
            h->center.x + h->radius * cosf( DEG2RAD * ( angle + angleInc ) ),
            h->center.y + h->radius * sinf( DEG2RAD * ( angle + angleInc ) ),
        };
        if ( CheckCollisionPointTriangle( point, v3, v2, v1 ) ) {
            return true;
        }
        angle += angleInc;
        DrawTriangle( v3, v2, v1, RED );
    }

    return false;

}

Hex *getHexByPoint( Hex *hexGrid, int hexCount, Vector2 point ) {
    for ( int i = 0; i < hexCount; i++ ) {
        Hex *h = &hexGrid[i];
        if ( checkCollisionPointHex( point, h ) ) {
            return h;
        }
    }
    return NULL;
}