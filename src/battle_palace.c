#include "global.h"
#include "event_data.h"
#include "battle_setup.h"
#include "overworld.h"
#include "random.h"
#include "battle_tower.h"
#include "frontier_util.h"
#include "item.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/battle_frontier.h"
#include "constants/battle_palace.h"
#include "constants/frontier_util.h"
#include "constants/trainers.h"

// This file's functions.
static void InitPalaceChallenge(void);
static void GetPalaceData(void);
static void SetPalaceData(void);
static void GetPalaceCommentId(void);
static void SetPalaceOpponent(void);
static void BufferOpponentIntroSpeech(void);
static void IncrementPalaceStreak(void);
static void SavePalaceChallenge(void);
static void SetRandomPalacePrize(void);
static void GivePalacePrize(void);

// Const rom data.
static void (* const sBattlePalaceFunctions[])(void) =
{
    [BATTLE_PALACE_FUNC_INIT]               = InitPalaceChallenge,
    [BATTLE_PALACE_FUNC_GET_DATA]           = GetPalaceData,
    [BATTLE_PALACE_FUNC_SET_DATA]           = SetPalaceData,
    [BATTLE_PALACE_FUNC_GET_COMMENT_ID]     = GetPalaceCommentId,
    [BATTLE_PALACE_FUNC_SET_OPPONENT]       = SetPalaceOpponent,
    [BATTLE_PALACE_FUNC_GET_OPPONENT_INTRO] = BufferOpponentIntroSpeech,
    [BATTLE_PALACE_FUNC_INCREMENT_STREAK]   = IncrementPalaceStreak,
    [BATTLE_PALACE_FUNC_SAVE]               = SavePalaceChallenge,
    [BATTLE_PALACE_FUNC_SET_PRIZE]          = SetRandomPalacePrize,
    [BATTLE_PALACE_FUNC_GIVE_PRIZE]         = GivePalacePrize,
};

static const u16 sBattlePalaceEarlyPrizes[] =
{
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_CARBOS,
    ITEM_ZINC
};

static const u16 sBattlePalaceLatePrizes[] =
{
    ITEM_BRIGHT_POWDER,
    ITEM_WHITE_HERB,
    ITEM_QUICK_CLAW,
    ITEM_LEFTOVERS,
    ITEM_MENTAL_HERB,
    ITEM_KINGS_ROCK,
    ITEM_FOCUS_BAND,
    ITEM_SCOPE_LENS,
    ITEM_CHOICE_BAND
};

static const u32 sWinStreakFlags[][2] =
{
    {STREAK_PALACE_SINGLES_50, STREAK_PALACE_SINGLES_OPEN},
    {STREAK_PALACE_DOUBLES_50, STREAK_PALACE_DOUBLES_OPEN},
};

static const u32 sWinStreakMasks[][2] =
{
    {~(STREAK_PALACE_SINGLES_50), ~(STREAK_PALACE_SINGLES_OPEN)},
    {~(STREAK_PALACE_DOUBLES_50), ~(STREAK_PALACE_DOUBLES_OPEN)},
};

// code
void CallBattlePalaceFunction(void)
{
    sBattlePalaceFunctions[gSpecialVar_0x8004]();
}

static void InitPalaceChallenge(void)
{
}

static void GetPalaceData(void)
{
}

static void SetPalaceData(void)
{
}

static void GetPalaceCommentId(void)
{
}

static void SetPalaceOpponent(void)
{
    gTrainerBattleOpponent_A = 5 *(Random() % 255) / 64u;
    SetBattleFacilityTrainerGfxId(gTrainerBattleOpponent_A, 0);
}

static void BufferOpponentIntroSpeech(void)
{
    if (gTrainerBattleOpponent_A < FRONTIER_TRAINERS_COUNT)
        FrontierSpeechToString(gFacilityTrainers[gTrainerBattleOpponent_A].speechBefore);
}

static void IncrementPalaceStreak(void)
{
}

static void SavePalaceChallenge(void)
{
}

static void SetRandomPalacePrize(void)
{
}

static void GivePalacePrize(void)
{
}
