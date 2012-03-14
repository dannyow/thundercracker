/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SIFTEO_BUDDIES_CONFIG_H_
#define SIFTEO_BUDDIES_CONFIG_H_

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameState.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Buddies {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const bool kLoadAssets = true;
const GameState kStateDefault = GAME_STATE_MAIN_MENU;
const unsigned int kNumCubes = 3; // Number of cubes used in this game
const unsigned int kMaxBuddies = 6; // Number of characters (eventually will be 15)
const unsigned int kPuzzlesPerBuddy = 5;

// Tuning
const float kStateTimeDelayShort = 1.0f; // Delay when switching between shuffle states
const float kStateTimeDelayLong = 5.0f;
const float kSwapAnimationSlide = 0.5f;

// Free Play
const float kFreePlayShakeThrottleDuration = 1.5f;

// Shuffle Mode
const float kHintTimerOnDuration = 10.0f; // Seconds before hint appears in shuffle mode
const float kHintTimerOffDuration = 1.5f; // Seconds before hint disappears in shuffle mode
const int kShuffleMaxMoves = -1; // Number of shuffles. -1 keeps going until all are shuffled.
const float kShuffleScrambleTimerDelay = 0.5f; // Time between end of swap animation and next
const float kShuffleFaceCompleteTimerDuration = 2.0f;

// Story Mode
const float kCutsceneTextDelay = 2.5f;
const float kHintBlinkTimerDuration = 0.5f; // Blink rate for story mode hints

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
