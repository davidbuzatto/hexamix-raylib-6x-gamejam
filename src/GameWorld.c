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
#include <string.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "GameWorld.h"
#include "Macros.h"
#include "LevelTransitionAnimation.h"
#include "MergeAnimation.h"
#include "ResourceManager.h"
#include "Utils.h"

typedef enum GameState {
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_LEVEL_TRANSITION,
    GAME_STATE_GAMEOVER,
    GAME_STATE_SHOW_HELP,
} GameState;

typedef enum ColorLimit {
    COLOR_LIMIT_PRIMARY = 2,
    COLOR_LIMIT_SECONDARY = 5,
    COLOR_LIMIT_TERTIARY = 11
} ColorLimit;

typedef struct LevelInfo {
    int centerLineQuantity;
    int hexRadius;
    int pointsToNextLevel;
} LevelInfo;

static void createHexGrid( GameWorld *gw, int q, float hexRadius );
static void connectHexGrid( Hex *hexGrid, int hexCount );
static void connectHexToNeighbors( int sourceIndex, Hex *hexGrid, int hexCount );

static unsigned int pollColorQueue( void );
static void offerColorQueue( unsigned int color );
static void feedColorQueue( bool randomize, int colorLimitIndex );

static int checkAndBlend( Hex *h );
static unsigned int mostFrequentColor( unsigned int *colors, int count );
static bool isBoardFull( GameWorld *gw );
static void resetGameWorld( GameWorld *gw );

static void drawPlayingHud( GameWorld *gw );
static void drawHelpHud( GameWorld *gw );

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
    // especial
    HEX_ESPECIAL_COLOR
};

static Hex *mouseOverHex = NULL;

static Hex mouseOverDrawHex = {
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
static Hex editorDrawHex = {
    .center = { 50, 50 },
    .radius = 15,
    .apothem = 0,
    .color = HEX_RED,
    .neighbors = { 0 }
};

static LevelInfo levels[] = {
    { .centerLineQuantity = 3,  .hexRadius = 100, .pointsToNextLevel = 30   },
    { .centerLineQuantity = 5,  .hexRadius = 60,  .pointsToNextLevel = 100  },
    { .centerLineQuantity = 7,  .hexRadius = 44,  .pointsToNextLevel = 250  },
    { .centerLineQuantity = 9,  .hexRadius = 36,  .pointsToNextLevel = 500  },
    { .centerLineQuantity = 13, .hexRadius = 26,  .pointsToNextLevel = 1000 }, 
    { .centerLineQuantity = 17, .hexRadius = 21,  .pointsToNextLevel = 2000 },
    { .centerLineQuantity = 23, .hexRadius = 17,  .pointsToNextLevel = 3500 },
    { .centerLineQuantity = 31, .hexRadius = 13,  .pointsToNextLevel = 1000000 }
};
int levelQuantity = ( sizeof( levels ) / sizeof( levels[0] ) );


static unsigned int colorQueue[COLOR_QUEUE_CAPACITY] = { 0 };
static int colorQueueStart = -1;
static int colorQueueEnd = -1;
static int colorQueueSize = 0;

static MergeAnimation mergeAnimation;
static LevelTransitionAnimation levelTransitionAnimation;

static int currentLevel = 0;
static GameState state = GAME_STATE_START;
static ColorLimit colorLimit = COLOR_LIMIT_PRIMARY;
static bool randomizeColorQueueFeeder = true;
static bool showHexConnections = false;
static bool updateGrid = false;

// special hex spawn
static int specialHexCount = 0;
static int specialHexSpawn = 12;

// editor state
static int editorSelectedColor = 0;
static int editorMaxColors = 13;
static bool editorActive = false;

// help state
static int currentHelpPage = 0;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    currentLevel = clampInt( currentLevel, 0, levelQuantity - 1 );
    createHexGrid( gw, levels[currentLevel].centerLineQuantity, levels[currentLevel].hexRadius );
    connectHexGrid( gw->hexGrid, gw->hexCount );
    gw->score = currentLevel > 0 ? levels[currentLevel-1].pointsToNextLevel : 0;
    //gw->score = levels[currentLevel].pointsToNextLevel-1;

    for ( int i = 0; i < COLOR_QUEUE_CAPACITY; i++ ) {
        feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
    }

    initMergeAnimation( &mergeAnimation );
    initLevelTransitionAnimation( &levelTransitionAnimation );

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

    if ( IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyPressed( KEY_R ) ) {
        resetGameWorld( gw );
    }

    if ( state == GAME_STATE_START ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            state = GAME_STATE_PLAYING;
        }

    } else if ( state == GAME_STATE_LEVEL_TRANSITION ) {

        levelTransitionAnimation.update( &levelTransitionAnimation, delta );

        if ( !levelTransitionAnimation.running ) {
            state = GAME_STATE_PLAYING;
        }

    } else if ( state == GAME_STATE_SHOW_HELP ) {

        if ( IsKeyPressed( KEY_H ) ) {
            state = GAME_STATE_PLAYING;
        }
        if ( IsKeyPressed( KEY_LEFT ) ) {
            currentHelpPage--;
        }
        if ( IsKeyPressed( KEY_RIGHT ) ) {
            currentHelpPage++;
        }
        currentHelpPage = clampInt( currentHelpPage, 0, 1 );

    } else if ( state == GAME_STATE_PLAYING ) {

        if ( gw->score >= levels[currentLevel].pointsToNextLevel ) {
            currentLevel++;
            updateGrid = true;
        }

        if ( IsKeyPressed( KEY_H ) ) {
            currentHelpPage = 0;
            state = GAME_STATE_SHOW_HELP;
        }

        if ( IsKeyPressed( KEY_E ) ) {
            editorActive = !editorActive;
        }

        mouseOverHex = getHexByPoint( gw->hexGrid, gw->hexCount, GetMousePosition() );

        if ( !mergeAnimation.running ) {

            if ( updateGrid ) {

                levelTransitionAnimation.running = true;
                prepareLevelTransitionAnimation( 
                    &levelTransitionAnimation, 
                    levels[currentLevel-1].centerLineQuantity, 
                    levels[currentLevel-1].hexRadius, 
                    levels[currentLevel].centerLineQuantity, 
                    levels[currentLevel].hexRadius, 
                    gw->hexGrid, 
                    gw->hexCount
                );

                // prepare new grid
                createHexGrid( gw, levels[currentLevel].centerLineQuantity, levels[currentLevel].hexRadius );
                connectHexGrid( gw->hexGrid, gw->hexCount );
                mouseOverHex = NULL;
                updateGrid = false;

                // copy color data to current grid
                copyColorDataFromNextHexGrid( &levelTransitionAnimation, gw->hexGrid, gw->hexCount );

                state = GAME_STATE_LEVEL_TRANSITION;

            } else if ( isBoardFull( gw ) ) {
                state = GAME_STATE_GAMEOVER;
            }

            if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                if ( mouseOverHex != NULL && mouseOverHex->color == HEX_BLANK_COLOR ) {
                    mouseOverHex->color = pollColorQueue();
                    gw->score += checkAndBlend( mouseOverHex );
                    specialHexCount++;
                    feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
                }
            }

            if ( editorActive ) {

                if ( IsMouseButtonPressed( MOUSE_BUTTON_RIGHT ) ) {
                    if ( mouseOverHex != NULL && mouseOverHex->color == HEX_BLANK_COLOR ) {
                        mouseOverHex->color = editorDrawHex.color;
                        gw->score += checkAndBlend( mouseOverHex );
                    }
                }

                if ( IsMouseButtonPressed( MOUSE_BUTTON_MIDDLE ) ) {
                    if ( mouseOverHex != NULL ) {
                        mouseOverHex->color = HEX_BLANK_COLOR;
                    }
                }

                Vector2 mw = GetMouseWheelMoveV();
                if ( mw.y > 0 ) {
                    editorSelectedColor = ( editorSelectedColor + 1 ) % editorMaxColors;
                    editorDrawHex.color = availableColors[editorSelectedColor];
                } else if ( mw.y < 0 ) {
                    editorSelectedColor--;
                    if ( editorSelectedColor < 0 ) {
                        editorSelectedColor = editorMaxColors - 1;
                    }
                    editorDrawHex.color = availableColors[editorSelectedColor];
                }

            }

        }

        mergeAnimation.update( &mergeAnimation, delta );

    } else if ( state == GAME_STATE_GAMEOVER ) {

        if ( IsKeyPressed( KEY_R ) ) {
            resetGameWorld( gw );
        }

    }
    
}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( BLACK );

    if ( state == GAME_STATE_START ) {
        DrawTexturePro( 
            rm->startScreenTexture, 
            (Rectangle) { 0, 0, rm->startScreenTexture.width, rm->startScreenTexture.height },
            (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() },
            (Vector2) { 0 },
            0.0f,
            WHITE
        );
    } else if ( state == GAME_STATE_LEVEL_TRANSITION ) {
        levelTransitionAnimation.draw( &levelTransitionAnimation );
        drawPlayingHud( gw );
    } else if ( state == GAME_STATE_SHOW_HELP ) {
        drawHelpHud( gw );
    } else if ( state == GAME_STATE_PLAYING ) {

        drawHexGrid( gw->hexGrid, gw->hexCount, showHexConnections );

        if ( !mergeAnimation.running ) {
            if ( mouseOverHex != NULL ) {
                mouseOverDrawHex.center = mouseOverHex->center;
                mouseOverDrawHex.radius = mouseOverHex->radius;
                drawHexHighlight( &mouseOverDrawHex );
            }
        }

        mergeAnimation.draw( &mergeAnimation );
        drawPlayingHud( gw );

    } else if ( state == GAME_STATE_GAMEOVER ) {
        DrawTexturePro(
            rm->gameOverScreenTexture,
            (Rectangle) { 0, 0, rm->gameOverScreenTexture.width, rm->gameOverScreenTexture.height },
            (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() },
            (Vector2) { 0 },
            0.0f,
            WHITE
        );
    }

    EndDrawing();

}

static void createHexGrid( GameWorld *gw, int centerLineQuantity, float hexRadius ) {

    gw->hexCount = 0;
    
    float hApothem = apothem( hexRadius );
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
                initHex( &gw->hexGrid[gw->hexCount++], center, hexRadius );
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

    if ( specialHexCount == specialHexSpawn ) {
        specialHexCount = 0;
        offerColorQueue( HEX_ESPECIAL_COLOR );
        return;
    }

    if ( randomize ) {
        offerColorQueue( availableColors[GetRandomValue( 0, colorLimitIndex )] );
    } else {
        offerColorQueue( availableColors[sequentialPos % (colorLimitIndex+1)] );
        sequentialPos++;
    }

}

// cascades through the neighbors, blending as long as the accumulated color
// stays compatible with the next neighbor being scanned
static int checkAndBlend( Hex *h ) {

    unsigned int centralColor = h->color;
    unsigned int blendResults[6];
    int mergeCount = 0;

    bool needsToPrepareMergeAnimation = true;

    for ( int i = 0; i < 6; i++ ) {

        Hex *t = h->neighbors[i];

        if ( t != NULL ) {

            // every neighbor is tested against the original central color, so
            // there is no chain reaction: the star merges all compatibles at once
            unsigned int blendColor = colorBlend( centralColor, t->color );

            if ( blendColor != HEX_BLANK_COLOR ) {

                if ( needsToPrepareMergeAnimation ) {
                    prepareMergeAnimation( &mergeAnimation, h );
                    needsToPrepareMergeAnimation = false;
                }

                addMergeAnimationNeighbor( &mergeAnimation, t, blendColor );

                blendResults[mergeCount] = blendColor;
                t->color = HEX_BLANK_COLOR;

                mergeCount++;

            }

        }

    }

    if ( mergeCount > 0 ) {
        // the central hex settles on the most used blend color (random on ties)
        unsigned int finalColor = mostFrequentColor( blendResults, mergeCount );
        h->color = finalColor;
        setMergeAnimationFinalColor( &mergeAnimation, finalColor );
    }

    // a special (white) hex must never remain on the grid: when it is placed
    // with no colored neighbor to blend with, it solidifies into a random primary
    if ( h->color == HEX_ESPECIAL_COLOR ) {
        h->color = availableColors[GetRandomValue( 0, COLOR_LIMIT_PRIMARY )];
    }

    // each successful merge doubles the reward: N merges -> 2^N
    return mergeCount > 0 ? ( 1 << mergeCount ) : 0;

}

// returns the color that appears the most in the list, breaking ties randomly
static unsigned int mostFrequentColor( unsigned int *colors, int count ) {

    unsigned int tied[6];
    int tiedCount = 0;
    int bestCount = 0;

    for ( int i = 0; i < count; i++ ) {

        int occurrences = 0;
        for ( int j = 0; j < count; j++ ) {
            if ( colors[j] == colors[i] ) {
                occurrences++;
            }
        }

        if ( occurrences > bestCount ) {
            bestCount = occurrences;
            tied[0] = colors[i];
            tiedCount = 1;
        } else if ( occurrences == bestCount ) {
            bool alreadyTied = false;
            for ( int k = 0; k < tiedCount; k++ ) {
                if ( tied[k] == colors[i] ) {
                    alreadyTied = true;
                    break;
                }
            }
            if ( !alreadyTied ) {
                tied[tiedCount++] = colors[i];
            }
        }

    }

    if ( tiedCount == 0 ) {
        return HEX_BLANK_COLOR;
    }

    return tied[GetRandomValue( 0, tiedCount - 1 )];

}

static bool isBoardFull( GameWorld *gw ) {
    for ( int i = 0; i < gw->hexCount; i++ ) {
        if ( gw->hexGrid[i].color == HEX_BLANK_COLOR ) {
            return false;
        }
    }
    return true;
}

static void resetGameWorld( GameWorld *gw ) {

    currentLevel = 0;
    createHexGrid( gw, levels[currentLevel].centerLineQuantity, levels[currentLevel].hexRadius );
    connectHexGrid( gw->hexGrid, gw->hexCount );
    gw->score = 0;

    // clear and refill the color queue from scratch
    colorQueueStart = -1;
    colorQueueEnd = -1;
    colorQueueSize = 0;
    specialHexCount = 0;
    for ( int i = 0; i < COLOR_QUEUE_CAPACITY; i++ ) {
        feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
    }

    initMergeAnimation( &mergeAnimation );
    initLevelTransitionAnimation( &levelTransitionAnimation );

    mouseOverHex = NULL;
    updateGrid = false;

    state = GAME_STATE_START;

}

static void drawPlayingHud( GameWorld *gw ) {

    const int fontSize = 30;
    int spacing = 20;
    int startX = GetScreenWidth() - ( colorQueueSize * queueDrawHex.radius + ( colorQueueSize - 1 ) * spacing ) - 15;

    const char *scoreLabel = "Score: ";
    Vector2 mScoreLabel = MeasureTextEx( rm->font, scoreLabel, fontSize, 0.0f );
    DrawTextEx( rm->font, scoreLabel, (Vector2) { 15, 15 }, fontSize, 0.0f, RAYWHITE );

    Rectangle scoreRec = { mScoreLabel.x + 100, 20, 120, 20 };

    int prevPoints = 0;
    if ( currentLevel > 0 ) {
        prevPoints = levels[currentLevel-1].pointsToNextLevel;
    }
    int base = gw->score - prevPoints;
    int next = levels[currentLevel].pointsToNextLevel - prevPoints;
    float perc = base / (float) next;
    Rectangle scoreRecValue = { scoreRec.x, scoreRec.y, scoreRec.width * perc, scoreRec.height };

    DrawRectangleRounded( scoreRecValue, 1.0f, 10, PINK );
    DrawRectangleRoundedLinesEx( scoreRec, 1.0f, 10, 3, RAYWHITE );
    DrawTextEx( rm->font, TextFormat( "%05d", levels[currentLevel].pointsToNextLevel ), (Vector2) { scoreRec.x + scoreRec.width + 8, 15 }, fontSize, 0.0f, RAYWHITE );

    const char *scoreValueLabel = TextFormat( "%05d", gw->score );
    Vector2 mScoreValueLabel = MeasureTextEx( rm->font, scoreValueLabel, fontSize, 0.0f );
    DrawTextEx( rm->font, scoreValueLabel, (Vector2) { scoreRec.x - mScoreValueLabel.x - 8, 15 }, fontSize, 0.0f, RAYWHITE );

    const char *levelLabel = TextFormat( "Level %d", currentLevel + 1 );
    Vector2 mLevelLabel = MeasureTextEx( rm->font, levelLabel, fontSize, 0.0f );
    DrawTextEx( rm->font, levelLabel, (Vector2) { GetScreenWidth() / 2 - mLevelLabel.x / 2, GetScreenHeight() - 45 }, fontSize, 0.0f, RAYWHITE );

    // queue
    const char *nextColorLabel = "Next color";
    Vector2 mNextColorLabel = MeasureTextEx( rm->font, nextColorLabel, fontSize, 0.0f );
    DrawTextEx( rm->font, nextColorLabel, (Vector2) { startX - mNextColorLabel.x - 20, 15 }, fontSize, 0.0f, RAYWHITE );
    
    for ( int i = 0; i < colorQueueSize; i++ ) {
        queueDrawHex.center.x = startX + ( queueDrawHex.radius + spacing ) * i;
        queueDrawHex.center.y = 33;
        queueDrawHex.color = colorQueue[(i+colorQueueStart)%COLOR_QUEUE_CAPACITY];
        drawHex( &queueDrawHex, true );
        if ( i == 0 ) {
            drawHexHighlight( &queueDrawHex );
        }
    }

    if ( editorActive ) {
        const char *editorLabel = "Editor Mode!";
        DrawTextEx( rm->font, editorLabel, (Vector2) { 15, GetScreenHeight() - 45 }, fontSize, 0.0f, RAYWHITE );
        Vector2 mEditorLabel = MeasureTextEx( rm->font, editorLabel, fontSize, 0.0f );
        editorDrawHex.center.x = 15 + mEditorLabel.x + 20;
        editorDrawHex.center.y = GetScreenHeight() - 27;
        drawHex( &editorDrawHex, true );
        drawHexHighlight( &editorDrawHex );
    }

    // special hex
    float specialHexPerc = specialHexCount / (float) specialHexSpawn;
    Rectangle specialHexRec = { GetScreenWidth() - 40, 60, 20, 115 };
    Rectangle specialHexRecValue = { specialHexRec.x, specialHexRec.y + specialHexRec.height - ( specialHexRec.height * specialHexPerc ), specialHexRec.width, specialHexRec.height * specialHexPerc };

    float hue = (float) fmod( GetTime() * HEX_SPECIAL_HUE_SPEED, 360.0 );
    DrawRectangleRounded( specialHexRecValue, 1.0f, 10, ColorFromHSV( hue, 1.0f, 1.0f ) );
    DrawRectangleRoundedLinesEx( specialHexRec, 1.0f, 10, 3, RAYWHITE );

    const char *specialLabel = "SPECIAL";
    for ( int i = 0; i < strlen( specialLabel ); i++ ) {
        const char *labelChar = TextFormat( "%c", specialLabel[i] );
        Vector2 mChar = MeasureTextEx( rm->font, labelChar, fontSize / 2, 0.0f );
        DrawTextEx( rm->font, TextFormat( "%c", specialLabel[i] ), (Vector2) { specialHexRec.x + specialHexRec.width / 2 - mChar.x / 2, specialHexRec.y + mChar.y * i + 5}, fontSize / 2, 0.0f, BLACK );
    }

}

static void drawHelpHud( GameWorld *gw ) {

    switch ( currentHelpPage ) {
        case 0:
            DrawTexture( rm->howToMergePSTexture, 0, 0, WHITE );
            break;
        case 1:
            DrawTexture( rm->howToMergePSTTexture, 0, 0, WHITE );
            break;
        default:
            break;
    }

}
