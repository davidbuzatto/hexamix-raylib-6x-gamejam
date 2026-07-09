#pragma once

#define trace( ... ) TraceLog( LOG_INFO, __VA_ARGS__ );

#define MAX_HEX_GRID_COUNT 1500
#define COLOR_QUEUE_CAPACITY 5

#define HEX_SPECIAL_HUE_SPEED 180.0f

#define HEX_BLANK_COLOR    0x00000000
#define HEX_ESPECIAL_COLOR 0xffffffff

// final color palette
// primary
#define HEX_RED           0xdf202eff
#define HEX_YELLOW        0xe9d90eff
#define HEX_BLUE          0x006fd7ff

// secondary
#define HEX_ORANGE        0xef8c24ff // red + yellow
#define HEX_GREEN         0x22a744ff // yellow + blue
#define HEX_PURPLE        0x9b14c6ff // blue + red

// tertiary
#define HEX_RED_ORANGE    0xed5f18ff // red + orange
#define HEX_YELLOW_ORANGE 0xf3b01dff // yellow + orange
#define HEX_YELLOW_GREEN  0x98cc00ff // yellow + green
#define HEX_BLUE_GREEN    0x2ebcb2ff // blue + green
#define HEX_BLUE_PURPLE   0x680ee3ff // blue + purple
#define HEX_RED_PURPLE    0xce12a6ff // red + purple

/* color palette 1
// primary
#define HEX_RED           0xd62028ff
#define HEX_YELLOW        0xfced23ff
#define HEX_BLUE          0x3f61abff

// secondary
#define HEX_ORANGE        0xf6841dff // red + yellow
#define HEX_GREEN         0x05b250ff // yellow + blue
#define HEX_PURPLE        0x723b96ff // blue + red

// tertiary
#define HEX_RED_ORANGE    0xf26623ff // red + orange
#define HEX_YELLOW_ORANGE 0xfcb413ff // yellow + orange
#define HEX_YELLOW_GREEN  0x8dc541ff // yellow + green
#define HEX_BLUE_GREEN    0x2ebcb2ff // blue + green
#define HEX_BLUE_PURPLE   0x4c499bff // blue + purple
#define HEX_RED_PURPLE    0xb03b94ff // red + purple
*/

/* color palette 2
// primary
#define HEX_RED           0xdf202eff
#define HEX_YELLOW        0xe9d90eff
#define HEX_BLUE          0x006fd7ff

// secondary
#define HEX_ORANGE        0xef8c24ff // red + yellow
#define HEX_GREEN         0x22a744ff // yellow + blue
#define HEX_PURPLE        0x9b14c6ff // blue + red

// tertiary
#define HEX_RED_ORANGE    0xed5f18ff // red + orange
#define HEX_YELLOW_ORANGE 0xf3b01dff // yellow + orange
#define HEX_YELLOW_GREEN  0x98cc00ff // yellow + green
#define HEX_BLUE_GREEN    0x009c8bff // blue + green
#define HEX_BLUE_PURPLE   0x680ee3ff // blue + purple
#define HEX_RED_PURPLE    0xce12a6ff // red + purple
*/