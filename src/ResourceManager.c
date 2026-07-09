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
    //_rm.howToMergePSTexture = LoadTexture( "resources/images/howto/how-to-merge-ps-white.png" );
    //_rm.howToMergePSTTexture = LoadTexture( "resources/images/howto/how-to-merge-pst-white.png" );

    //_rm.soundExample = LoadSound( "resources/sfx/sound.wav" );
    //_rm.musicExample = LoadMusicStream( "resources/musics/music.ogg" );

}

void unloadResourcesResourceManager( void ) {

    UnloadFont( _rm.font );
    UnloadTexture( _rm.startScreenTexture );
    UnloadTexture( _rm.howToMergePSTexture );
    UnloadTexture( _rm.howToMergePSTTexture );

    //UnloadSound( _rm.soundExample );
    //UnloadMusicStream( _rm.musicExample );
    
}