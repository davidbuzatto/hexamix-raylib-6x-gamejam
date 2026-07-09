/**
 * @file ResourceManager.c
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>

#include "raylib/raylib.h"

#include "ResourceManager.h"
#include "SoundPool.h"

static ResourceManager _rm = { 0 };  // mutable; owned exclusively by this module
const ResourceManager * const rm = &_rm;

void loadResourcesResourceManager( void ) {
    
    _rm.font = LoadFontEx( "resources/fonts/Fredoka/static/Fredoka-Bold.ttf", 30, NULL, 250 );

    _rm.startScreenTexture = LoadTexture( "resources/images/screens/start.png" );
    SetTextureFilter( _rm.startScreenTexture, TEXTURE_FILTER_BILINEAR );
    _rm.gameOverScreenTexture = LoadTexture( "resources/images/screens/gameover.png" );
    SetTextureFilter( _rm.gameOverScreenTexture, TEXTURE_FILTER_BILINEAR );

    _rm.howToMergePSTexture = LoadTexture( "resources/images/howto/how-to-merge-ps-black.png" );
    _rm.howToMergePSTTexture = LoadTexture( "resources/images/howto/how-to-merge-pst-black.png" );

    _rm.placeSoundPool = createSoundPool( "resources/sfx/kenney/click4.ogg", 3 );
    _rm.placeFailSoundPool = createSoundPool( "resources/sfx/kenney/highDown.ogg", 3 );
    _rm.mergeSoundPool = createSoundPool( "resources/sfx/kenney/powerUp4.ogg", 6 );
    _rm.specialHexSoundPool = createSoundPool( "resources/sfx/kenney/zapTwoTone2.ogg", 2 );
    _rm.explodingSoundPool = createSoundPool( "resources/sfx/kenney/minimize_006.ogg", 2 );
    _rm.showingSoundPool = createSoundPool( "resources/sfx/kenney/maximize_006.ogg", 2 );

    _rm.gameOverSound = LoadSound( "resources/sfx/kenney/explosionCrunch_000.ogg" );

    //_rm.bgMusic = LoadMusicStream( "resources/musics/kenney/alpha-dance.ogg" );
    _rm.bgMusic = LoadMusicStream( "resources/musics/kenney/retro-mystic.ogg" );

}

void unloadResourcesResourceManager( void ) {

    UnloadFont( _rm.font );
    UnloadTexture( _rm.startScreenTexture );
    UnloadTexture( _rm.howToMergePSTexture );
    UnloadTexture( _rm.howToMergePSTTexture );

    destroySoundPool( _rm.placeSoundPool );
    destroySoundPool( _rm.placeFailSoundPool );
    destroySoundPool( _rm.mergeSoundPool );
    destroySoundPool( _rm.specialHexSoundPool );
    destroySoundPool( _rm.explodingSoundPool );
    destroySoundPool( _rm.showingSoundPool );

    UnloadMusicStream( _rm.bgMusic );
    
}