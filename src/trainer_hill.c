#include "global.h"
#include "malloc.h"
#include "battle.h"
#include "battle_tower.h"
#include "battle_setup.h"
#include "ereader_helpers.h"
#include "event_data.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_message_box.h"
#include "international_string_util.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "trainer_hill.h"
#include "window.h"
#include "util.h"
#include "constants/battle_ai.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/moves.h"
#include "constants/trainers.h"
#include "constants/trainer_hill.h"
#include "constants/trainer_types.h"

#define HILL_MAX_TIME 215999 // 60 * 60 * 60 - 1

struct FloorTrainers
{
    u8 name[HILL_TRAINERS_PER_FLOOR][HILL_TRAINER_NAME_LENGTH + 1];
    u8 facilityClass[HILL_TRAINERS_PER_FLOOR];
};

static EWRAM_DATA struct {
    u8 floorId;
    struct TrainerHillChallenge challenge;
    struct TrainerHillFloor floors[NUM_TRAINER_HILL_FLOORS];
} *sHillData = NULL;

static EWRAM_DATA struct FloorTrainers *sFloorTrainers = NULL;
EWRAM_DATA u32 *gTrainerHillVBlankCounter = NULL;

// This file's functions.
static void TrainerHillStartChallenge(void);
static void GetOwnerState(void);
static void GiveChallengePrize(void);
static void CheckFinalTime(void);
static void TrainerHillResumeTimer(void);
static void TrainerHillSetPlayerLost(void);
static void TrainerHillGetChallengeStatus(void);
static void BufferChallengeTime(void);
static void GetAllFloorsUsed(void);
static void GetInEReaderMode(void);
static void IsTrainerHillChallengeActive(void);
static void ShowTrainerHillPostBattleText(void);
static void SetAllTrainerFlags(void);
static void GetGameSaved(void);
static void SetGameSaved(void);
static void ClearGameSaved(void);
static void GetChallengeWon(void);
static void TrainerHillSetMode(void);
static void SetUpDataStruct(void);
static void FreeDataStruct(void);
static void SetTimerValue(u32 *dst, u32 val);
static u32 GetTimerValue(u32 *src);
static void SetTrainerHillMonLevel(struct Pokemon *mon, u8 level);
static u16 GetPrizeItemId(void);

// const data
#include "data/battle_frontier/trainer_hill.h"

struct
{
    u8 trainerClass;
    u8 musicId;
} static const sTrainerClassesAndMusic[] =
{
    {TRAINER_CLASS_TEAM_AQUA, TRAINER_ENCOUNTER_MUSIC_AQUA},
    {TRAINER_CLASS_AQUA_ADMIN, TRAINER_ENCOUNTER_MUSIC_AQUA},
    {TRAINER_CLASS_AQUA_LEADER, TRAINER_ENCOUNTER_MUSIC_AQUA},
    {TRAINER_CLASS_AROMA_LADY, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_BATTLE_GIRL, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_SWIMMER_F, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_POKEFAN, TRAINER_ENCOUNTER_MUSIC_TWINS},
    {TRAINER_CLASS_DRAGON_TAMER, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_COOLTRAINER, TRAINER_ENCOUNTER_MUSIC_COOL},
    {TRAINER_CLASS_GUITARIST, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_SAILOR, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_TWINS, TRAINER_ENCOUNTER_MUSIC_TWINS},
    {TRAINER_CLASS_INTERVIEWER, TRAINER_ENCOUNTER_MUSIC_INTERVIEWER},
    {TRAINER_CLASS_RUIN_MANIAC, TRAINER_ENCOUNTER_MUSIC_HIKER},
    {TRAINER_CLASS_GENTLEMAN, TRAINER_ENCOUNTER_MUSIC_RICH},
    {TRAINER_CLASS_SWIMMER_M, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_POKEMANIAC, TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {TRAINER_CLASS_BLACK_BELT, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_OLD_COUPLE, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_BUG_MANIAC, TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {TRAINER_CLASS_CAMPER, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_KINDLER, TRAINER_ENCOUNTER_MUSIC_HIKER},
    {TRAINER_CLASS_TEAM_MAGMA, TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {TRAINER_CLASS_MAGMA_ADMIN, TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {TRAINER_CLASS_MAGMA_LEADER, TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {TRAINER_CLASS_LASS, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_BUG_CATCHER, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_NINJA_BOY, TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {TRAINER_CLASS_RICH_BOY, TRAINER_ENCOUNTER_MUSIC_RICH},
    {TRAINER_CLASS_HEX_MANIAC, TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {TRAINER_CLASS_BEAUTY, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_LADY, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_PARASOL_LADY, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_PICNICKER, TRAINER_ENCOUNTER_MUSIC_GIRL},
    {TRAINER_CLASS_PKMN_BREEDER, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_COLLECTOR, TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {TRAINER_CLASS_PKMN_RANGER, TRAINER_ENCOUNTER_MUSIC_COOL},
    {TRAINER_CLASS_RIVAL, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_YOUNG_COUPLE, TRAINER_ENCOUNTER_MUSIC_GIRL},
    {TRAINER_CLASS_PSYCHIC, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_SR_AND_JR, TRAINER_ENCOUNTER_MUSIC_TWINS},
    {TRAINER_CLASS_ELITE_FOUR, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_YOUNGSTER, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_EXPERT, TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {TRAINER_CLASS_TRIATHLETE, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_BIRD_KEEPER, TRAINER_ENCOUNTER_MUSIC_COOL},
    {TRAINER_CLASS_FISHERMAN, TRAINER_ENCOUNTER_MUSIC_HIKER},
    {TRAINER_CLASS_CHAMPION, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_TUBER_M, TRAINER_ENCOUNTER_MUSIC_MALE},
    {TRAINER_CLASS_TUBER_F, TRAINER_ENCOUNTER_MUSIC_GIRL},
    {TRAINER_CLASS_SIS_AND_BRO, TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {TRAINER_CLASS_HIKER, TRAINER_ENCOUNTER_MUSIC_HIKER},
    {TRAINER_CLASS_LEADER, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {TRAINER_CLASS_SCHOOL_KID, TRAINER_ENCOUNTER_MUSIC_MALE},
};

static const u16 sPrizeListRareCandy1[]  = {ITEM_RARE_CANDY,       ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListLuxuryBall1[] = {ITEM_LUXURY_BALL,      ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListMaxRevive1[]  = {ITEM_MAX_REVIVE,       ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListMaxEther1[]   = {ITEM_MAX_ETHER,        ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListElixir1[]     = {ITEM_ELIXIR,           ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListRoar[]        = {ITEM_TM05_PLAY_ROUGH,        ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListSludgeBomb[]  = {ITEM_TM36_SLUDGE_BOMB, ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListToxic[]       = {ITEM_TM06_TOXIC,       ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListSunnyDay[]    = {ITEM_TM11_METAL_CLAW,   ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListEarthQuake[]  = {ITEM_TM26_EARTHQUAKE,  ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};

static const u16 sPrizeListRareCandy2[]  = {ITEM_RARE_CANDY,       ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListLuxuryBall2[] = {ITEM_LUXURY_BALL,      ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListMaxRevive2[]  = {ITEM_MAX_REVIVE,       ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListMaxEther2[]   = {ITEM_MAX_ETHER,        ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListElixir2[]     = {ITEM_ELIXIR,           ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListBrickBreak[]  = {ITEM_TM31_BRICK_BREAK, ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListTorment[]     = {ITEM_TM41_TORMENT,     ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListSkillSwap[]   = {ITEM_TM48_SKILL_SWAP,  ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListGigaDrain[]   = {ITEM_TM19_NASTY_PLOT,  ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};
static const u16 sPrizeListAttract[]     = {ITEM_TM45_ATTRACT,     ITEM_ETHER, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FLUFFY_TAIL, ITEM_GREAT_BALL};

static const u16 *const sPrizeLists1[NUM_TRAINER_HILL_PRIZE_LISTS] =
{
    sPrizeListRareCandy1,
    sPrizeListLuxuryBall1,
    sPrizeListMaxRevive1,
    sPrizeListMaxEther1,
    sPrizeListElixir1,
    sPrizeListRoar,
    sPrizeListSludgeBomb,
    sPrizeListToxic,
    sPrizeListSunnyDay,
    sPrizeListEarthQuake
};

static const u16 *const sPrizeLists2[NUM_TRAINER_HILL_PRIZE_LISTS] =
{
    sPrizeListRareCandy2,
    sPrizeListLuxuryBall2,
    sPrizeListMaxRevive2,
    sPrizeListMaxEther2,
    sPrizeListElixir2,
    sPrizeListBrickBreak,
    sPrizeListTorment,
    sPrizeListSkillSwap,
    sPrizeListGigaDrain,
    sPrizeListAttract
};

static const u16 *const *const sPrizeListSets[] =
{
    sPrizeLists1,
    sPrizeLists2
};

static const u16 sEReader_Pal[] = INCBIN_U16("graphics/trainer_hill/ereader.gbapal");
static const u8 sRecordWinColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static const struct TrainerHillChallenge *const sChallengeData[NUM_TRAINER_HILL_MODES] =
{
    [HILL_MODE_NORMAL]  = &sChallenge_Normal,
    [HILL_MODE_VARIETY] = &sChallenge_Variety,
    [HILL_MODE_UNIQUE]  = &sChallenge_Unique,
    [HILL_MODE_EXPERT]  = &sChallenge_Expert,
};

static void (* const sHillFunctions[])(void) =
{
    [TRAINER_HILL_FUNC_START]                 = TrainerHillStartChallenge,
    [TRAINER_HILL_FUNC_GET_OWNER_STATE]       = GetOwnerState,
    [TRAINER_HILL_FUNC_GIVE_PRIZE]            = GiveChallengePrize,
    [TRAINER_HILL_FUNC_CHECK_FINAL_TIME]      = CheckFinalTime,
    [TRAINER_HILL_FUNC_RESUME_TIMER]          = TrainerHillResumeTimer,
    [TRAINER_HILL_FUNC_SET_LOST]              = TrainerHillSetPlayerLost,
    [TRAINER_HILL_FUNC_GET_CHALLENGE_STATUS]  = TrainerHillGetChallengeStatus,
    [TRAINER_HILL_FUNC_GET_CHALLENGE_TIME]    = BufferChallengeTime,
    [TRAINER_HILL_FUNC_GET_ALL_FLOORS_USED]   = GetAllFloorsUsed,
    [TRAINER_HILL_FUNC_GET_IN_EREADER_MODE]   = GetInEReaderMode,
    [TRAINER_HILL_FUNC_IN_CHALLENGE]          = IsTrainerHillChallengeActive,
    [TRAINER_HILL_FUNC_POST_BATTLE_TEXT]      = ShowTrainerHillPostBattleText,
    [TRAINER_HILL_FUNC_SET_ALL_TRAINER_FLAGS] = SetAllTrainerFlags,
    [TRAINER_HILL_FUNC_GET_GAME_SAVED]        = GetGameSaved,
    [TRAINER_HILL_FUNC_SET_GAME_SAVED]        = SetGameSaved,
    [TRAINER_HILL_FUNC_CLEAR_GAME_SAVED]      = ClearGameSaved,
    [TRAINER_HILL_FUNC_GET_WON]               = GetChallengeWon,
    [TRAINER_HILL_FUNC_SET_MODE]              = TrainerHillSetMode,
};

static const u8 *const sModeStrings[NUM_TRAINER_HILL_MODES] =
{
    [HILL_MODE_NORMAL]  = gText_NormalTagMatch,
    [HILL_MODE_VARIETY] = gText_VarietyTagMatch,
    [HILL_MODE_UNIQUE]  = gText_UniqueTagMatch,
    [HILL_MODE_EXPERT]  = gText_ExpertTagMatch,
};

static const struct ObjectEventTemplate sTrainerObjectEventTemplate =
{
    .graphicsId = OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL,
    .elevation = 3,
    .movementType = MOVEMENT_TYPE_LOOK_AROUND,
    .movementRangeX = 1,
    .movementRangeY = 1,
    .trainerType = TRAINER_TYPE_NORMAL,
};

static const u32 sNextFloorMapNum[NUM_TRAINER_HILL_FLOORS] =
{
    [TRAINER_HILL_1F - 1] = MAP_NUM(TRAINER_HILL_2F),
    [TRAINER_HILL_2F - 1] = MAP_NUM(TRAINER_HILL_3F),
    [TRAINER_HILL_3F - 1] = MAP_NUM(TRAINER_HILL_4F),
    [TRAINER_HILL_4F - 1] = MAP_NUM(TRAINER_HILL_ROOF)
};
static const u8 sTrainerPartySlots[HILL_TRAINERS_PER_FLOOR][PARTY_SIZE / 2] =
{
    {0, 1, 2},
    {3, 4, 5}
};

void CallTrainerHillFunction(void)
{
    SetUpDataStruct();
    sHillFunctions[gSpecialVar_0x8004]();
    FreeDataStruct();
}

void ResetTrainerHillResults(void)
{
}

static u8 GetFloorId(void)
{
    return gMapHeader.mapLayoutId - LAYOUT_TRAINER_HILL_1F;
}

u8 GetTrainerHillOpponentClass(u16 trainerId)
{
    u8 id = trainerId - 1;

    return gFacilityClassToTrainerClass[sFloorTrainers->facilityClass[id]];
}

void GetTrainerHillTrainerName(u8 *dst, u16 trainerId)
{
    s32 i;
    u8 id = trainerId - 1;

    for (i = 0; i < HILL_TRAINER_NAME_LENGTH + 1; i++)
        dst[i] = sFloorTrainers->name[id][i];
}

u8 GetTrainerHillTrainerFrontSpriteId(u16 trainerId)
{
    u8 id, facilityClass;

    SetUpDataStruct();
    id = trainerId - 1;
    facilityClass = sHillData->floors[sHillData->floorId].trainers[id].facilityClass;
    FreeDataStruct();

    return gFacilityClassToPicIndex[facilityClass];
}

void InitTrainerHillBattleStruct(void)
{
}

void FreeTrainerHillBattleStruct(void)
{
    TRY_FREE_AND_SET_NULL(sFloorTrainers);
}

static void SetUpDataStruct(void)
{
}

static void FreeDataStruct(void)
{
    TRY_FREE_AND_SET_NULL(sHillData);
}

void CopyTrainerHillTrainerText(u8 which, u16 trainerId)
{
    u8 id, floorId;

    SetUpDataStruct();
    floorId = GetFloorId();
    id = trainerId - 1;

    switch (which)
    {
    case TRAINER_HILL_TEXT_INTRO:
        FrontierSpeechToString(sHillData->floors[floorId].trainers[id].speechBefore);
        break;
    case TRAINER_HILL_TEXT_PLAYER_LOST:
        FrontierSpeechToString(sHillData->floors[floorId].trainers[id].speechWin);
        break;
    case TRAINER_HILL_TEXT_PLAYER_WON:
        FrontierSpeechToString(sHillData->floors[floorId].trainers[id].speechLose);
        break;
    case TRAINER_HILL_TEXT_AFTER:
        FrontierSpeechToString(sHillData->floors[floorId].trainers[id].speechAfter);
        break;
    }

    FreeDataStruct();
}

static void TrainerHillStartChallenge(void)
{
}

static void GetOwnerState(void)
{
}

static void GiveChallengePrize(void)
{
}

// If bestTime > timer, the challenge was completed faster and its a new record
// Otherwise the owner says it was a slow time and to complete it faster next time
static void CheckFinalTime(void)
{
}

static void TrainerHillResumeTimer(void)
{
}

static void TrainerHillSetPlayerLost(void)
{
}

static void TrainerHillGetChallengeStatus(void)
{
}

static void BufferChallengeTime(void)
{
}

// Returns TRUE if all 4 floors are used
// Returns FALSE otherwise, and buffers the number of floors used
// The only time fewer than all 4 floors are used is for the JP-exclusive E-Reader and Default modes
static void GetAllFloorsUsed(void)
{
    SetUpDataStruct();
    if (sHillData->challenge.numFloors != NUM_TRAINER_HILL_FLOORS)
    {
        ConvertIntToDecimalStringN(gStringVar1, sHillData->challenge.numFloors, STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_Result = FALSE;
    }
    else
    {
        gSpecialVar_Result = TRUE;
    }

    FreeDataStruct();
}

// May have been dummied. Every time this is called a conditional for var result occurs afterwards
// Relation to E-Reader is an assumption, most dummied Trainer Hill code seems to be JP E-Reader mode related
static void GetInEReaderMode(void)
{
    SetUpDataStruct();
    gSpecialVar_Result = FALSE;
    FreeDataStruct();
}

bool8 InTrainerHillChallenge(void)
{
}

static void IsTrainerHillChallengeActive(void)
{
    if (!InTrainerHillChallenge())
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = TRUE;
}

void PrintOnTrainerHillRecordsWindow(void)
{
}

// Leftover from Fire Red / Leaf Green as in these games,
// the timer had to be xored by the encryption key in Sav2.
static u32 GetTimerValue(u32 *src)
{
    return *src;
}

static void SetTimerValue(u32 *dst, u32 val)
{
    *dst = val;
}

void LoadTrainerHillObjectEventTemplates(void)
{
}

bool32 LoadTrainerHillFloorObjectEventScripts(void)
{
    SetUpDataStruct();
    // Something may have been dummied here
    FreeDataStruct();
    return TRUE;
}

static u16 GetMetatileForFloor(u8 floorId, u32 x, u32 y, u32 floorWidth) // floorWidth is always 16
{
    bool8 impassable;
    u16 metatile;
    u16 elevation;

    impassable = (sHillData->floors[floorId].map.collisionData[y] >> (15 - x) & 1);
    metatile = sHillData->floors[floorId].map.metatileData[floorWidth * y + x] + NUM_METATILES_IN_PRIMARY;
    elevation = 3 << MAPGRID_ELEVATION_SHIFT;

    return ((impassable << MAPGRID_COLLISION_SHIFT) & MAPGRID_COLLISION_MASK) | elevation | (metatile & MAPGRID_METATILE_ID_MASK);
}

void GenerateTrainerHillFloorLayout(u16 *mapArg)
{
    s32 y, x;
    u16 *src, *dst;
    u8 mapId = GetCurrentTrainerHillMapId();

    if (mapId == TRAINER_HILL_ENTRANCE)
    {
        InitMapFromSavedGame();
        return;
    }

    SetUpDataStruct();
    if (mapId == TRAINER_HILL_ROOF)
    {
        InitMapFromSavedGame();
        FreeDataStruct();
        return;
    }

    mapId = GetFloorId();
    src = gMapHeader.mapLayout->map;
    gBackupMapLayout.map = mapArg;
    // Dimensions include border area loaded beyond map
    gBackupMapLayout.width = HILL_FLOOR_WIDTH + 15;
    gBackupMapLayout.height = HILL_FLOOR_HEIGHT + 14;
    dst = mapArg + 224;

    // First 5 rows of the map (Entrance / Exit) are always the same
    for (y = 0; y < HILL_FLOOR_HEIGHT_MARGIN; y++)
    {
        for (x = 0; x < HILL_FLOOR_WIDTH; x++)
            dst[x] = src[x];
        dst += 31;
        src += 16;
    }

    // Load the 16x16 floor-specific layout
    for (y = 0; y < HILL_FLOOR_HEIGHT_MAIN; y++)
    {
        for (x = 0; x < HILL_FLOOR_WIDTH; x++)
            dst[x] = GetMetatileForFloor(mapId, x, y, HILL_FLOOR_WIDTH);
        dst += 31;
    }

    RunOnLoadMapScript();
    FreeDataStruct();
}

bool32 InTrainerHill(void)
{
    bool32 ret;

    if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_1F
        || gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_2F
        || gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_3F
        || gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_4F)
        ret = TRUE;
    else
        ret = FALSE;

    return ret;
}

u8 GetCurrentTrainerHillMapId(void)
{
    u8 mapId;

    if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_1F)
        mapId = TRAINER_HILL_1F;
    else if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_2F)
        mapId = TRAINER_HILL_2F;
    else if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_3F)
        mapId = TRAINER_HILL_3F;
    else if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_4F)
        mapId = TRAINER_HILL_4F;
    else if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_ROOF)
        mapId = TRAINER_HILL_ROOF;
    else if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_HILL_ENTRANCE)
        mapId = TRAINER_HILL_ENTRANCE;
    else
        mapId = 0;

    return mapId;
}

const struct WarpEvent* SetWarpDestinationTrainerHill4F(void)
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(TRAINER_HILL_4F), MAP_NUM(TRAINER_HILL_4F));

    return &header->events->warps[1];
}

// For warping from the roof in challenges where the 4F is not the final challenge floor
// This would only occur in the JP-exclusive Default and E-Reader challenges
const struct WarpEvent* SetWarpDestinationTrainerHillFinalFloor(u8 warpEventId)
{
    u8 numFloors;
    const struct MapHeader *header;

    if (warpEventId == 1)
        return &gMapHeader.events->warps[1];

    numFloors = GetNumFloorsInTrainerHillChallenge();
    if (numFloors == 0 || numFloors > NUM_TRAINER_HILL_FLOORS)
        numFloors = NUM_TRAINER_HILL_FLOORS;

    header = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(TRAINER_HILL_4F), sNextFloorMapNum[numFloors - 1]);
    return &header->events->warps[0];
}

u16 LocalIdToHillTrainerId(u8 localId)
{
}

bool8 GetHillTrainerFlag(u8 objectEventId)
{
}

void SetHillTrainerFlag(void)
{
}

const u8 *GetTrainerHillTrainerScript(void)
{
    return TrainerHill_EventScript_TrainerBattle;
}

static void ShowTrainerHillPostBattleText(void)
{
    CopyTrainerHillTrainerText(TRAINER_HILL_TEXT_AFTER, gSpecialVar_LastTalked);
    ShowFieldMessageFromBuffer();
}

static void CreateNPCTrainerHillParty(u16 trainerId, u8 firstMonId)
{
    u8 trId, level;
    s32 i, floorId, partySlot;

    if (trainerId == 0 || trainerId > HILL_TRAINERS_PER_FLOOR)
        return;

    trId = trainerId - 1;
    SetUpDataStruct();
    level = GetHighestLevelInPlayerParty();
    floorId = GetFloorId();
    for (i = firstMonId, partySlot = 0; i < firstMonId + PARTY_SIZE / 2; i++, partySlot++)
    {
        u8 id = sTrainerPartySlots[trId][partySlot];
        struct Pokemon *mon = &gEnemyParty[i];

        CreateBattleTowerMon(mon, &sHillData->floors[floorId].trainers[trId].mons[id]);
        SetTrainerHillMonLevel(mon, level);
    }

    FreeDataStruct();
}

void FillHillTrainerParty(void)
{
    ZeroEnemyPartyMons();
    CreateNPCTrainerHillParty(gTrainerBattleOpponent_A, 0);
}

void FillHillTrainersParties(void)
{
    ZeroEnemyPartyMons();
    CreateNPCTrainerHillParty(gTrainerBattleOpponent_A, 0);
    CreateNPCTrainerHillParty(gTrainerBattleOpponent_B, PARTY_SIZE / 2);
}

u8 GetTrainerEncounterMusicIdInTrainerHill(u16 trainerId)
{
    s32 i;
    u8 trId, facilityClass;

    SetUpDataStruct();
    trId = trainerId - 1;
    facilityClass = sHillData->floors[sHillData->floorId].trainers[trId].facilityClass;
    FreeDataStruct();

    for (i = 0; i < ARRAY_COUNT(sTrainerClassesAndMusic); i++)
    {
        if (sTrainerClassesAndMusic[i].trainerClass == gFacilityClassToTrainerClass[facilityClass])
            return sTrainerClassesAndMusic[i].musicId;
    }

    return 0;
}

static void SetTrainerHillMonLevel(struct Pokemon *mon, u8 level)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];

    SetMonData(mon, MON_DATA_EXP, &exp);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    CalculateMonStats(mon);
}

u8 GetNumFloorsInTrainerHillChallenge(void)
{
    u8 floors;

    SetUpDataStruct();
    floors = sHillData->challenge.numFloors;
    FreeDataStruct();

    return floors;
}

static void SetAllTrainerFlags(void)
{
}

// Palette never loaded, OnTrainerHillEReaderChallengeFloor always FALSE
void TryLoadTrainerHillEReaderPalette(void)
{
    if (OnTrainerHillEReaderChallengeFloor() == TRUE)
        LoadPalette(sEReader_Pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
}

static void GetGameSaved(void)
{
}

static void SetGameSaved(void)
{
}

static void ClearGameSaved(void)
{
}

// Always FALSE
bool32 OnTrainerHillEReaderChallengeFloor(void)
{
    if (!InTrainerHillChallenge() || GetCurrentTrainerHillMapId() == TRAINER_HILL_ENTRANCE)
        return FALSE;

    GetInEReaderMode();
    if (gSpecialVar_Result == FALSE)
        return FALSE;
    else
        return TRUE;
}

static void GetChallengeWon(void)
{
}

static void TrainerHillSetMode(void)
{
}

// Determines which prize list to use from the set of prize lists.
static u8 GetPrizeListId(bool8 allowTMs)
{
    u8 prizeListId, i, modBy;

    // The initial selection depends on the trainer numbers for the completed challenge.
    // These don't change with the available challenge modes, so Normal/Unique will always
    // have a prizeListId of 8, and Variety/Expert will have a prizeListId of 24.
    prizeListId = 0;
    for (i = 0; i < NUM_TRAINER_HILL_FLOORS; i++)
    {
        prizeListId ^= sHillData->floors[i].trainerNum1 & 0x1F;
        prizeListId ^= sHillData->floors[i].trainerNum2 & 0x1F;
    }

    // In practice, the conditional below is always true.
    // The 2nd half of the lists in both sets of lists all have a TM as the "grand prize", while the 1st half do not,
    // so taking the mod of the (total / 2) ensures that a prize list without a TM will be used.
    if (allowTMs)
        modBy = NUM_TRAINER_HILL_PRIZE_LISTS;
    else
        modBy = NUM_TRAINER_HILL_PRIZE_LISTS / 2;

    prizeListId %= modBy;
    return prizeListId;
}

static u16 GetPrizeItemId(void)
{
}
