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
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "GameWorld.h"
#include "Macros.h"
#include "MergeAnimation.h"
#include "ResourceManager.h"
#include "Utils.h"

typedef enum GameState {
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_LEVEL_TRANSITION,
    GAME_STATE_GAMEOVER,
    GAME_STATE_EDITOR,
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

static void drawHud( GameWorld *gw );

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

static int currentLevel = 0;
static GameState state = GAME_STATE_PLAYING;
static ColorLimit colorLimit = COLOR_LIMIT_PRIMARY;
static bool randomizeColorQueueFeeder = true;
static bool showHexConnections = false;

// editor state
static int editorSelectedColor = 0;
static int editorMaxColors = 12;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    currentLevel = clampInt( currentLevel, 0, levelQuantity - 1 );
    createHexGrid( gw, levels[currentLevel].centerLineQuantity, levels[currentLevel].hexRadius );
    connectHexGrid( gw->hexGrid, gw->hexCount );
    gw->score = 0;

    for ( int i = 0; i < COLOR_QUEUE_CAPACITY; i++ ) {
        feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
    }

    initMergeAnimation( &mergeAnimation );

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

    if ( IsKeyPressed( KEY_F1 ) ) {
        state = GAME_STATE_EDITOR;
    }
    if ( IsKeyPressed( KEY_F2 ) ) {
        state = GAME_STATE_PLAYING;
    }

    mouseOverHex = getHexByPoint( gw->hexGrid, gw->hexCount, GetMousePosition() );

    if ( !mergeAnimation.running ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            if ( mouseOverHex != NULL && mouseOverHex->color == HEX_BLANK_COLOR ) {
                mouseOverHex->color = pollColorQueue();
                gw->score += checkAndBlend( mouseOverHex );
                feedColorQueue( randomizeColorQueueFeeder, (int) colorLimit );
            }
        }

        if ( state == GAME_STATE_EDITOR ) {

            if ( IsMouseButtonPressed( MOUSE_BUTTON_RIGHT ) ) {
                if ( mouseOverHex != NULL && mouseOverHex->color == HEX_BLANK_COLOR ) {
                    mouseOverHex->color = editorDrawHex.color;
                    gw->score += checkAndBlend( mouseOverHex );
                }
            }

            if ( IsKeyPressed( KEY_W ) ) {
                editorSelectedColor = ( editorSelectedColor + 1 ) % editorMaxColors;
                editorDrawHex.color = availableColors[editorSelectedColor];
            }

            if ( IsKeyPressed( KEY_S ) ) {
                editorSelectedColor--;
                if ( editorSelectedColor < 0 ) {
                    editorSelectedColor = editorMaxColors - 1;
                }
                editorDrawHex.color = availableColors[editorSelectedColor];
            }

        }

    }

    mergeAnimation.update( &mergeAnimation, delta );

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( BLACK );

    drawHexGrid( gw->hexGrid, gw->hexCount, showHexConnections );

    if ( !mergeAnimation.running ) {
        if ( mouseOverHex != NULL ) {
            mouseOverDrawHex.center = mouseOverHex->center;
            mouseOverDrawHex.radius = mouseOverHex->radius;
            drawHexHighlight( &mouseOverDrawHex );
        }
    }

    mergeAnimation.draw( &mergeAnimation );
    
    drawHud( gw );

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

static void drawHud( GameWorld *gw ) {

    const int fontSize = 30;
    int spacing = 20;
    int startX = GetScreenWidth() - ( colorQueueSize * queueDrawHex.radius + ( colorQueueSize - 1 ) * spacing ) - 15;

    const char *scoreLabel = "Score: ";
    Vector2 mScoreLabel = MeasureTextEx( rm->font, scoreLabel, fontSize, 0.0f );
    DrawTextEx( rm->font, scoreLabel, (Vector2) { 15, 15 }, fontSize, 0.0f, RAYWHITE );

    Rectangle scoreRec = { mScoreLabel.x + 100, 20, 120, 20 };
    DrawRectangleRoundedLinesEx( scoreRec, 1.0f, 10, 2, RAYWHITE );
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
        drawHex( &queueDrawHex );
        if ( i == 0 ) {
            drawHexHighlight( &queueDrawHex );
        }
    }

    if ( state == GAME_STATE_EDITOR ) {
        const char *editorLabel = "Editor";
        DrawTextEx( rm->font, editorLabel, (Vector2) { 15, GetScreenHeight() - 45 }, fontSize, 0.0f, RAYWHITE );
        Vector2 mEditorLabel = MeasureTextEx( rm->font, editorLabel, fontSize, 0.0f );
        editorDrawHex.center.x = 15 + mEditorLabel.x + 20;
        editorDrawHex.center.y = GetScreenHeight() - 27;
        drawHex( &editorDrawHex );
        drawHexHighlight( &editorDrawHex );
    }

    //DrawFPS( 10, GetScreenHeight() - 25 );

}
