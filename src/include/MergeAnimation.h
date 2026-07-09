#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

#include "Hex.h"

typedef struct MergeAnimation MergeAnimation;

struct MergeAnimation {

    Hex centralHex;
    Hex neighborsToMerge[6];
    int neighborCount;
    float animationUnitCounter;
    bool running;

    bool prepareNextAnimationInstance;
    Vector2 neighborToMergeStartPos;
    float neighborToMergeStartRadius;
    unsigned int neighborToMergeStartColor;
    unsigned int centralHexStartColor;

    void (*update)( MergeAnimation *ma, float delta );
    void (*draw)( MergeAnimation *ma );

};

void initMergeAnimation( MergeAnimation *ma );
void prepareMergeAnimation( MergeAnimation *ma, Hex *centralHex );
void addMergeAnimationNeighbor( MergeAnimation *ma, Hex *neighborHex );
