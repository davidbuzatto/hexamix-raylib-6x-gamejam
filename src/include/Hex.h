#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

typedef struct Hex Hex;

struct Hex {
    Vector2 center;
    float radius;
    float apothem;
    unsigned int color;
    Hex *neighbors[6];
};

void initHex( Hex *h, Vector2 center, float radius );
void drawHex( Hex *h, bool drawSpecialEffect );
void drawHexHighlight( Hex *h );
void drawHexGrid( Hex *hexGrid, int hexCount, bool showConnections );
bool checkCollisionPointHex( Vector2 point, Hex *h );
Hex *getHexByPoint( Hex *hexGrid, int hexCount, Vector2 point );