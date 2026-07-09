#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"

#include "Hex.h"
#include "Macros.h"
#include "Utils.h"

static void drawHexConnections( Hex *h );
static void drawHexGridConnections( Hex *hexGrid, int hexCount, int hexPos );

// especial hex rainbow render
static const float pulsePeriod = 0.6f;   // seconds per pulse
static const float pulseExpand = 0.6f;   // extra radius fraction at the pulse peak
static const float pulseMaxAlpha = 0.8f; // halo opacity at the start of each pulse

void initHex( Hex *h, Vector2 center, float radius ) {

    h->center = center;
    h->radius = radius;
    h->apothem = apothem( h->radius );
    h->color = HEX_BLANK_COLOR;

    for ( int i = 0; i < 6; i++ ) {
        h->neighbors[i] = NULL;
    }

}

void drawHex( Hex *h, bool drawSpecialEffect ) {

    if ( h->color == HEX_ESPECIAL_COLOR && drawSpecialEffect ) {

        float hue = (float) fmod( GetTime() * HEX_SPECIAL_HUE_SPEED, 360.0 );
        Color baseColor = ColorFromHSV( hue, 1.0f, 1.0f );
        DrawPoly( h->center, 6, h->radius, 90.0f, baseColor );

        // pulsing halo on top: same color, grows past the base radius while
        // fading out over each period, giving a pulsing feel
        float pulse = (float) fmod( GetTime(), pulsePeriod ) / pulsePeriod;
        float pulseRadius = h->radius * ( 1.0f + pulse * pulseExpand );
        DrawPoly( h->center, 6, pulseRadius, 90.0f, Fade( baseColor, pulseMaxAlpha * ( 1.0f - pulse ) ) );

        return;
    }

    DrawPoly( h->center, 6, h->radius, 90.0f, GetColor( h->color ) );
    unsigned int strokeColor = ColorToInt( DARKGRAY );

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
        drawHex( h, false );
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
        //DrawTriangle( v3, v2, v1, RED ); // debug
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