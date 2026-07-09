#pragma once

#include "raylib/raylib.h"

typedef struct {
    int quantity;
    int current;
    Sound *pool;
} SoundPool;

SoundPool *createSoundPool( const char *soundPath, int quantity );
void destroySoundPool( SoundPool *soundPool );
void playSoundFromSoundPool( SoundPool *soundPool );