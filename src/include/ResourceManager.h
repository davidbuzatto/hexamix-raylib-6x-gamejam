/**
 * @file ResourceManager.h
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager struct and function declarations.
 * 
 * @copyright Copyright (c) 2026
 */
#pragma once

#include "raylib/raylib.h"

typedef struct ResourceManager {

    Font font;

    Texture2D startScreenTexture;
    Texture2D gameOverScreenTexture;

    Texture2D howToMergePSTexture;
    Texture2D howToMergePSTTexture;

    /*Texture2D textureExample;
    Sound soundExample;
    Music musicExample;*/
    
} ResourceManager;

// Read-only outside this module. Use rm->field to access resources.
extern const ResourceManager * const rm;

/**
 * @brief Load global game resources, linking them in the global instance of
 * ResourceManager called rm.
 */
void loadResourcesResourceManager( void );

/**
 * @brief Unload global game resources.
 */
void unloadResourcesResourceManager( void );