#include "global.h"
#include "battle.h"
#include "battle_factory.h"
#include "battle_factory_screen.h"
#include "event_data.h"
#include "battle_setup.h"
#include "overworld.h"
#include "frontier_util.h"
#include "battle_tower.h"
#include "random.h"
#include "constants/battle_ai.h"
#include "constants/battle_factory.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_mons.h"
#include "constants/battle_tent.h"
#include "constants/frontier_util.h"
#include "constants/layouts.h"
#include "constants/trainers.h"
#include "constants/moves.h"
#include "constants/items.h"

static bool8 sPerformedRentalSwap;

static void InitFactoryChallenge(void);
static void GetBattleFactoryData(void);
static void SetBattleFactoryData(void);
static void SaveFactoryChallenge(void);
static void SelectInitialRentalMons(void);
static void SwapRentalMons(void);
static void SetPerformedRentalSwap(void);
static void SetRentalsToOpponentParty(void);
static void SetPlayerAndOpponentParties(void);
static void SetOpponentGfxVar(void);
static void GenerateOpponentMons(void);
static void GenerateInitialRentalMons(void);
static void GetOpponentMostCommonMonType(void);
static void GetOpponentBattleStyle(void);
static void RestorePlayerPartyHeldItems(void);
static u16 GetFactoryMonId(u8 lvlMode, u8 challengeNum, bool8 useBetterRange);
static u8 GetMoveBattleStyle(u16 move);

// Number of moves needed on the team to be considered using a certain battle style
static const u8 sRequiredMoveCounts[FACTORY_NUM_STYLES - 1] = {
    [FACTORY_STYLE_PREPARATION - 1]   = 3,
    [FACTORY_STYLE_SLOW_STEADY - 1]   = 3,
    [FACTORY_STYLE_ENDURANCE - 1]     = 3,
    [FACTORY_STYLE_HIGH_RISK - 1]     = 2,
    [FACTORY_STYLE_WEAKENING - 1]     = 2,
    [FACTORY_STYLE_UNPREDICTABLE - 1] = 2,
    [FACTORY_STYLE_WEATHER - 1]       = 2
};

static const u16 sMoves_TotalPreparation[] =
{
    MOVE_SWORDS_DANCE, MOVE_GROWTH, MOVE_MEDITATE, MOVE_AGILITY, MOVE_DOUBLE_TEAM, MOVE_HARDEN,
    MOVE_MINIMIZE, MOVE_WITHDRAW, MOVE_DEFENSE_CURL, MOVE_BARRIER, MOVE_FOCUS_ENERGY, MOVE_AMNESIA,
    MOVE_ACID_ARMOR, MOVE_SHARPEN, MOVE_CONVERSION, MOVE_CONVERSION_2, MOVE_BELLY_DRUM, MOVE_PSYCH_UP,
    MOVE_CHARGE, MOVE_SNATCH, MOVE_TAIL_GLOW, MOVE_COSMIC_POWER, MOVE_IRON_DEFENSE, MOVE_HOWL, MOVE_BULK_UP, MOVE_CALM_MIND, MOVE_DRAGON_DANCE,
    MOVE_NONE
};

static const u16 sMoves_ImpossibleToPredict[] =
{
    MOVE_MIMIC, MOVE_METRONOME, MOVE_MIRROR_MOVE, MOVE_TRANSFORM, MOVE_SUBSTITUTE, MOVE_SKETCH, MOVE_CURSE,
    MOVE_PRESENT, MOVE_FOLLOW_ME, MOVE_TRICK, MOVE_ROLE_PLAY, MOVE_ASSIST, MOVE_SKILL_SWAP, MOVE_CAMOUFLAGE,
    MOVE_NONE
};

static const u16 sMoves_WeakeningTheFoe[] =
{
    MOVE_SAND_ATTACK, MOVE_TAIL_WHIP, MOVE_LEER, MOVE_GROWL, MOVE_STRING_SHOT, MOVE_SCREECH, MOVE_SMOKESCREEN, MOVE_KINESIS,
    MOVE_FLASH, MOVE_COTTON_SPORE, MOVE_SPITE, MOVE_SCARY_FACE, MOVE_CHARM, MOVE_KNOCK_OFF, MOVE_SWEET_SCENT, MOVE_FEATHER_DANCE,
    MOVE_FAKE_TEARS, MOVE_METAL_SOUND, MOVE_TICKLE,
    MOVE_NONE
};

static const u16 sMoves_HighRiskHighReturn[] =
{
    MOVE_GUILLOTINE, MOVE_HORN_DRILL, MOVE_DOUBLE_EDGE, MOVE_HYPER_BEAM, MOVE_COUNTER, MOVE_FISSURE,
    MOVE_BIDE, MOVE_SELF_DESTRUCT, MOVE_SKY_ATTACK, MOVE_EXPLOSION, MOVE_FLAIL, MOVE_REVERSAL, MOVE_DESTINY_BOND,
    MOVE_PERISH_SONG, MOVE_PAIN_SPLIT, MOVE_MIRROR_COAT, MOVE_MEMENTO, MOVE_GRUDGE, MOVE_FACADE, MOVE_FOCUS_PUNCH,
    MOVE_BLAST_BURN, MOVE_HYDRO_CANNON, MOVE_OVERHEAT, MOVE_FRENZY_PLANT, MOVE_PSYCHO_BOOST, MOVE_VOLT_TACKLE,
    MOVE_NONE
};

static const u16 sMoves_Endurance[] =
{
    MOVE_MIST, MOVE_RECOVER, MOVE_LIGHT_SCREEN, MOVE_HAZE, MOVE_REFLECT, MOVE_SOFT_BOILED, MOVE_REST, MOVE_PROTECT,
    MOVE_DETECT, MOVE_ENDURE, MOVE_MILK_DRINK, MOVE_HEAL_BELL, MOVE_SAFEGUARD, MOVE_BATON_PASS, MOVE_MORNING_SUN,
    MOVE_SYNTHESIS, MOVE_MOONLIGHT, MOVE_SWALLOW, MOVE_WISH, MOVE_INGRAIN, MOVE_MAGIC_COAT, MOVE_RECYCLE, MOVE_REFRESH,
    MOVE_MUD_SPORT, MOVE_SLACK_OFF, MOVE_AROMATHERAPY, MOVE_WATER_SPORT,
    MOVE_NONE
};

static const u16 sMoves_SlowAndSteady[] =
{
    MOVE_SING, MOVE_SUPERSONIC, MOVE_DISABLE, MOVE_LEECH_SEED, MOVE_POISON_POWDER, MOVE_STUN_SPORE, MOVE_SLEEP_POWDER,
    MOVE_THUNDER_WAVE, MOVE_TOXIC, MOVE_HYPNOSIS, MOVE_CONFUSE_RAY, MOVE_GLARE, MOVE_POISON_GAS, MOVE_LOVELY_KISS, MOVE_SPORE,
    MOVE_SPIDER_WEB, MOVE_SWEET_KISS, MOVE_SPIKES, MOVE_SWAGGER, MOVE_MEAN_LOOK, MOVE_ATTRACT, MOVE_ENCORE, MOVE_TORMENT,
    MOVE_FLATTER, MOVE_WILL_O_WISP, MOVE_TAUNT, MOVE_YAWN, MOVE_IMPRISON, MOVE_SNATCH, MOVE_TEETER_DANCE, MOVE_GRASS_WHISTLE, MOVE_BLOCK,
    MOVE_NONE
};

static const u16 sMoves_DependsOnTheBattlesFlow[] =
{
    MOVE_SANDSTORM, MOVE_RAIN_DANCE, MOVE_SUNNY_DAY, MOVE_HAIL, MOVE_WEATHER_BALL,
    MOVE_NONE
};

// Excludes FACTORY_STYLE_NONE
static const u16 *const sMoveStyles[FACTORY_NUM_STYLES - 1] =
{
    [FACTORY_STYLE_PREPARATION - 1]   = sMoves_TotalPreparation,
    [FACTORY_STYLE_SLOW_STEADY - 1]   = sMoves_SlowAndSteady,
    [FACTORY_STYLE_ENDURANCE - 1]     = sMoves_Endurance,
    [FACTORY_STYLE_HIGH_RISK - 1]     = sMoves_HighRiskHighReturn,
    [FACTORY_STYLE_WEAKENING - 1]     = sMoves_WeakeningTheFoe,
    [FACTORY_STYLE_UNPREDICTABLE - 1] = sMoves_ImpossibleToPredict,
    [FACTORY_STYLE_WEATHER - 1]       = sMoves_DependsOnTheBattlesFlow,
};

static void (* const sBattleFactoryFunctions[])(void) =
{
    [BATTLE_FACTORY_FUNC_INIT]                   = InitFactoryChallenge,
    [BATTLE_FACTORY_FUNC_GET_DATA]               = GetBattleFactoryData,
    [BATTLE_FACTORY_FUNC_SET_DATA]               = SetBattleFactoryData,
    [BATTLE_FACTORY_FUNC_SAVE]                   = SaveFactoryChallenge,
    [BATTLE_FACTORY_FUNC_SELECT_RENT_MONS]       = SelectInitialRentalMons,
    [BATTLE_FACTORY_FUNC_SWAP_RENT_MONS]         = SwapRentalMons,
    [BATTLE_FACTORY_FUNC_SET_SWAPPED]            = SetPerformedRentalSwap,
    [BATTLE_FACTORY_FUNC_SET_OPPONENT_MONS]      = SetRentalsToOpponentParty,
    [BATTLE_FACTORY_FUNC_SET_PARTIES]            = SetPlayerAndOpponentParties,
    [BATTLE_FACTORY_FUNC_SET_OPPONENT_GFX]       = SetOpponentGfxVar,
    [BATTLE_FACTORY_FUNC_GENERATE_OPPONENT_MONS] = GenerateOpponentMons,
    [BATTLE_FACTORY_FUNC_GENERATE_RENTAL_MONS]   = GenerateInitialRentalMons,
    [BATTLE_FACTORY_FUNC_GET_OPPONENT_MON_TYPE]  = GetOpponentMostCommonMonType,
    [BATTLE_FACTORY_FUNC_GET_OPPONENT_STYLE]     = GetOpponentBattleStyle,
    [BATTLE_FACTORY_FUNC_RESET_HELD_ITEMS]       = RestorePlayerPartyHeldItems,
};

static const u32 sWinStreakFlags[][2] =
{
    {STREAK_FACTORY_SINGLES_50, STREAK_FACTORY_SINGLES_OPEN},
    {STREAK_FACTORY_DOUBLES_50, STREAK_FACTORY_DOUBLES_OPEN},
};

static const u32 sWinStreakMasks[][2] =
{
    {~(STREAK_FACTORY_SINGLES_50), ~(STREAK_FACTORY_SINGLES_OPEN)},
    {~(STREAK_FACTORY_DOUBLES_50), ~(STREAK_FACTORY_DOUBLES_OPEN)},
};

static const u8 sFixedIVTable[][2] =
{
    {3, 6},
    {6, 9},
    {9, 12},
    {12, 15},
    {15, 18},
    {21, 31},
    {31, 31},
    {31, 31},
};

static const u16 sInitialRentalMonRanges[][2] =
{
    // Level 50
    {FRONTIER_MON_GRIMER,     FRONTIER_MON_FURRET_1},   // 110 - 199
    {FRONTIER_MON_DELCATTY_1, FRONTIER_MON_CLOYSTER_1}, // 162 - 266
    {FRONTIER_MON_DELCATTY_2, FRONTIER_MON_CLOYSTER_2}, // 267 - 371
    {FRONTIER_MON_DUGTRIO_1,  FRONTIER_MON_SLAKING_1},  // 372 - 467
    {FRONTIER_MON_DUGTRIO_2,  FRONTIER_MON_SLAKING_2},  // 468 - 563
    {FRONTIER_MON_DUGTRIO_3,  FRONTIER_MON_SLAKING_3},  // 564 - 659
    {FRONTIER_MON_DUGTRIO_4,  FRONTIER_MON_SLAKING_4},  // 660 - 755
    {FRONTIER_MON_DUGTRIO_1,  FRONTIER_MONS_HIGH_TIER}, // 372 - 849

    // Open level
    {FRONTIER_MON_DUGTRIO_1, FRONTIER_MON_SLAKING_1}, // 372 - 467
    {FRONTIER_MON_DUGTRIO_2, FRONTIER_MON_SLAKING_2}, // 468 - 563
    {FRONTIER_MON_DUGTRIO_3, FRONTIER_MON_SLAKING_3}, // 564 - 659
    {FRONTIER_MON_DUGTRIO_4, FRONTIER_MON_SLAKING_4}, // 660 - 755
    {FRONTIER_MON_DUGTRIO_1, NUM_FRONTIER_MONS - 1},  // 372 - 881
    {FRONTIER_MON_DUGTRIO_1, NUM_FRONTIER_MONS - 1},  // 372 - 881
    {FRONTIER_MON_DUGTRIO_1, NUM_FRONTIER_MONS - 1},  // 372 - 881
    {FRONTIER_MON_DUGTRIO_1, NUM_FRONTIER_MONS - 1},  // 372 - 881
};

// code
void CallBattleFactoryFunction(void)
{
    sBattleFactoryFunctions[gSpecialVar_0x8004]();
}

static void InitFactoryChallenge(void)
{
}

static void GetBattleFactoryData(void)
{
}

static void SetBattleFactoryData(void)
{
}

static void SaveFactoryChallenge(void)
{
}

static void SelectInitialRentalMons(void)
{
    ZeroPlayerPartyMons();
    DoBattleFactorySelectScreen();
}

static void SwapRentalMons(void)
{
    DoBattleFactorySwapScreen();
}

static void SetPerformedRentalSwap(void)
{
    sPerformedRentalSwap = TRUE;
}

static void GenerateOpponentMons(void)
{
}

static void SetOpponentGfxVar(void)
{
    SetBattleFacilityTrainerGfxId(gTrainerBattleOpponent_A, 0);
}

static void SetRentalsToOpponentParty(void)
{
}

static void SetPlayerAndOpponentParties(void)
{
}

static void GenerateInitialRentalMons(void)
{
}

// Determines if the upcoming opponent has a single most-common
// type in its party. If there are two different types that are
// tied, then the opponent is deemed to have no preferred type,
// and NUMBER_OF_MON_TYPES is the result.
static void GetOpponentMostCommonMonType(void)
{
    u8 i;
    u8 typeCounts[NUMBER_OF_MON_TYPES];
    u8 mostCommonTypes[2];

    gFacilityTrainerMons = gBattleFrontierMons;

    // Count the number of times each type occurs in the opponent's party.
    for (i = TYPE_NORMAL; i < NUMBER_OF_MON_TYPES; i++)
        typeCounts[i] = 0;
    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        u32 species = gFacilityTrainerMons[gFrontierTempParty[i]].species;
        typeCounts[gSpeciesInfo[species].types[0]]++;
        if (gSpeciesInfo[species].types[0] != gSpeciesInfo[species].types[1])
            typeCounts[gSpeciesInfo[species].types[1]]++;
    }

    // Determine which are the two most-common types.
    // The second most-common type is only updated if
    // its count is equal to the most-common type.
    mostCommonTypes[0] = 0;
    mostCommonTypes[1] = 0;
    for (i = 1; i < NUMBER_OF_MON_TYPES; i++)
    {
        if (typeCounts[mostCommonTypes[0]] < typeCounts[i])
            mostCommonTypes[0] = i;
        else if (typeCounts[mostCommonTypes[0]] == typeCounts[i])
            mostCommonTypes[1] = i;
    }

    if (typeCounts[mostCommonTypes[0]] != 0)
    {
        // The most-common type must be strictly greater than
        // the second-most-common type, or the top two must be
        // the same type.
        if (typeCounts[mostCommonTypes[0]] > typeCounts[mostCommonTypes[1]])
            gSpecialVar_Result = mostCommonTypes[0];
        else if (mostCommonTypes[0] == mostCommonTypes[1])
            gSpecialVar_Result = mostCommonTypes[0];
        else
            gSpecialVar_Result = NUMBER_OF_MON_TYPES;
    }
    else
    {
        gSpecialVar_Result = NUMBER_OF_MON_TYPES;
    }
}

static void GetOpponentBattleStyle(void)
{
    u8 i, j, count;
    u8 stylePoints[FACTORY_NUM_STYLES];

    count = 0;
    gFacilityTrainerMons = gBattleFrontierMons;
    for (i = 0; i < FACTORY_NUM_STYLES; i++)
        stylePoints[i] = 0;

    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        u16 monId = gFrontierTempParty[i];
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u8 battleStyle = GetMoveBattleStyle(gFacilityTrainerMons[monId].moves[j]);
            stylePoints[battleStyle]++;
        }
    }

    gSpecialVar_Result = FACTORY_STYLE_NONE;
    for (i = 1; i < FACTORY_NUM_STYLES; i++)
    {
        if (stylePoints[i] >= sRequiredMoveCounts[i - 1])
        {
            gSpecialVar_Result = i;
            count++;
        }
    }

    // Has no singular style
    if (count > 2)
        gSpecialVar_Result = FACTORY_NUM_STYLES;
}

static u8 GetMoveBattleStyle(u16 move)
{
    const u16 *moves;
    u8 i, j;

    for (i = 0; i < ARRAY_COUNT(sMoveStyles); i++)
    {
        for (j = 0, moves = sMoveStyles[i]; moves[j] != MOVE_NONE; j++)
        {
            if (moves[j] == move)
                return i + 1;
        }
    }
    return FACTORY_STYLE_NONE;
}

bool8 InBattleFactory(void)
{
    return gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_FACTORY_PRE_BATTLE_ROOM
        || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_FACTORY_BATTLE_ROOM;
}

static void RestorePlayerPartyHeldItems(void)
{
}

// Get the IV to use for the opponent's pokémon.
// The IVs get higher for each subsequent challenge and for
// the last trainer in each challenge. Noland is an exception
// to this, as he uses the IVs that would be used by the regular
// trainers 2 challenges ahead of the current one.
// Due to a mistake in FillFactoryFrontierTrainerParty, the
// challenge number used to determine the IVs for regular trainers
// is Battle Tower's instead of Battle Factory's.
u8 GetFactoryMonFixedIV(u8 challengeNum, bool8 isLastBattle)
{
    u8 ivSet;
    bool8 useHigherIV = isLastBattle ? TRUE : FALSE;

    if (challengeNum > 8)
        ivSet = 7;
    else
        ivSet = challengeNum;

    return sFixedIVTable[ivSet][useHigherIV];
}

void FillFactoryBrainParty(void)
{
}

static u16 GetFactoryMonId(u8 lvlMode, u8 challengeNum, bool8 useBetterRange)
{
    u16 numMons, monId;
    u16 adder; // Used to skip past early mons for open level

    if (lvlMode == FRONTIER_LVL_50)
        adder = 0;
    else
        adder = 8;

    if (challengeNum < 7)
    {
        if (useBetterRange)
        {
            numMons = (sInitialRentalMonRanges[adder + challengeNum + 1][1] - sInitialRentalMonRanges[adder + challengeNum + 1][0]) + 1;
            monId = Random() % numMons;
            monId += sInitialRentalMonRanges[adder + challengeNum + 1][0];
        }
        else
        {
            numMons = (sInitialRentalMonRanges[adder + challengeNum][1] - sInitialRentalMonRanges[adder + challengeNum][0]) + 1;
            monId = Random() % numMons;
            monId += sInitialRentalMonRanges[adder + challengeNum][0];
        }
    }
    else
    {
        u16 challenge = challengeNum;
        if (challenge != 7)
            challenge = 7; // why bother assigning it above at all

        numMons = (sInitialRentalMonRanges[adder + challenge][1] - sInitialRentalMonRanges[adder + challenge][0]) + 1;
        monId = Random() % numMons;
        monId += sInitialRentalMonRanges[adder + challenge][0];
    }

    return monId;
}

u8 GetNumPastRentalsRank(u8 battleMode, u8 lvlMode)
{
}

u32 GetAiScriptsInBattleFactory(void)
{
}

void SetMonMoveAvoidReturn(struct Pokemon *mon, u16 moveArg, u8 moveSlot)
{
    u16 move = moveArg;
    if (moveArg == MOVE_RETURN)
        move = MOVE_FRUSTRATION;
    SetMonMoveSlot(mon, move, moveSlot);
}
