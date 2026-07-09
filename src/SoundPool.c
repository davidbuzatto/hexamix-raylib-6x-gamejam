#include <stdlib.h>

#include "raylib/raylib.h"

#include "Macros.h"
#include "SoundPool.h"

SoundPool *createSoundPool( const char *soundPath, int quantity ) {

    SoundPool *new = (SoundPool*) malloc( sizeof( SoundPool ) );

    new->quantity = quantity;
    new->current = 0;

    new->pool = (Sound*) malloc( sizeof( Sound ) * quantity );

    new->pool[0] = LoadSound( soundPath );
    for ( int i = 1; i < quantity; i++ ) {
        new->pool[i] = LoadSoundAlias( new->pool[0] );
    }

    return new;

}

void destroySoundPool( SoundPool *soundPool ) {
    if ( soundPool != NULL ) {
        if ( soundPool->pool != NULL ) {
            for ( int i = 1; i < soundPool->quantity; i++ ) {
                UnloadSoundAlias( soundPool->pool[i] );
            }
            UnloadSound( soundPool->pool[0] );
            free( soundPool->pool );
        }
        free( soundPool );
    }
}

void playSoundFromSoundPool( SoundPool *soundPool ) {
    PlaySound( soundPool->pool[soundPool->current%soundPool->quantity]);
    soundPool->current++;
}