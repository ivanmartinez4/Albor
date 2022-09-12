#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "gym_leader_rematch.h"
#include "international_string_util.h"
#include "main.h"
#include "match_call.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokenav.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "constants/songs.h"

struct Pokenav_MatchCallMenu
{
    u16 optionCursorPos;
    u16 maxOptionId;
    const u8 *matchCallOptions;
    u16 headerId;
    u16 numRegistered;
    u16 numSpecialTrainers;
    bool32 initFinished;
    u32 loopedTaskId;
    u32 (*callback)(struct Pokenav_MatchCallMenu *);
    struct PokenavMatchCallEntry matchCallEntries[MAX_REMATCH_ENTRIES - 1];
};

static u32 CB2_HandleMatchCallInput(struct Pokenav_MatchCallMenu *);
static u32 GetExitMatchCallMenuId(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleMatchCallOptionsInput(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleCheckPageInput(struct Pokenav_MatchCallMenu *);
static u32 CB2_HandleCallExitInput(struct Pokenav_MatchCallMenu *);
static u32 LoopedTask_BuildMatchCallList(s32);
static bool32 ShouldDoNearbyMessage(void);

#include "data/text/match_call_messages.h"

static const u8 sMatchCallOptionsNoCheckPage[] =
{
    MATCH_CALL_OPTION_CALL,
    MATCH_CALL_OPTION_CANCEL
};

static const u8 sMatchCallOptionsHasCheckPage[] =
{
    MATCH_CALL_OPTION_CALL,
    MATCH_CALL_OPTION_CHECK,
    MATCH_CALL_OPTION_CANCEL
};

bool32 PokenavCallback_Init_MatchCall(void)
{
    struct Pokenav_MatchCallMenu *state = AllocSubstruct(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN, sizeof(struct Pokenav_MatchCallMenu));
    if (!state)
        return FALSE;

    state->callback = CB2_HandleMatchCallInput;
    state->headerId = 0;
    state->initFinished = FALSE;
    state->loopedTaskId = CreateLoopedTask(LoopedTask_BuildMatchCallList, 1);
    return TRUE;
}

u32 GetMatchCallCallback(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->callback(state);
}

void FreeMatchCallSubstruct1(void)
{
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
}

static u32 CB2_HandleMatchCallInput(struct Pokenav_MatchCallMenu *state)
{
    int selection;

    if (JOY_REPEAT(DPAD_UP))
        return POKENAV_MC_FUNC_UP;
    if (JOY_REPEAT(DPAD_DOWN))
        return POKENAV_MC_FUNC_DOWN;
    if (JOY_REPEAT(DPAD_LEFT))
        return POKENAV_MC_FUNC_PG_UP;
    if (JOY_REPEAT(DPAD_RIGHT))
        return POKENAV_MC_FUNC_PG_DOWN;

    if (JOY_NEW(A_BUTTON))
    {
        state->callback = CB2_HandleMatchCallOptionsInput;
        state->optionCursorPos = 0;
        selection = PokenavList_GetSelectedIndex();

        if (!state->matchCallEntries[selection].isSpecialTrainer || MatchCall_HasCheckPage(state->matchCallEntries[selection].headerId))
        {
            state->matchCallOptions = sMatchCallOptionsHasCheckPage;
            state->maxOptionId = ARRAY_COUNT(sMatchCallOptionsHasCheckPage) - 1;
        }
        else
        {
            state->matchCallOptions = sMatchCallOptionsNoCheckPage;
            state->maxOptionId = ARRAY_COUNT(sMatchCallOptionsNoCheckPage) - 1;
        }

        return POKENAV_MC_FUNC_SELECT;
    }

    if (JOY_NEW(B_BUTTON))
    {
        if (GetPokenavMode() != POKENAV_MODE_FORCE_CALL_READY)
        {
            state->callback = GetExitMatchCallMenuId;
            return POKENAV_MC_FUNC_EXIT;
        }
        else
        {
            // Cant exit Match Call menu before calling Mr Stone during tutorial
            PlaySE(SE_FAILURE);
        }
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 GetExitMatchCallMenuId(struct Pokenav_MatchCallMenu *state)
{
    return POKENAV_MAIN_MENU_CURSOR_ON_MATCH_CALL;
}

static u32 CB2_HandleMatchCallOptionsInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_NEW(DPAD_UP) && state->optionCursorPos)
    {
        state->optionCursorPos--;
        return POKENAV_MC_FUNC_MOVE_OPTIONS_CURSOR;
    }

    if (JOY_NEW(DPAD_DOWN) && state->optionCursorPos < state->maxOptionId)
    {
        state->optionCursorPos++;
        return POKENAV_MC_FUNC_MOVE_OPTIONS_CURSOR;
    }

    if (JOY_NEW(A_BUTTON))
    {
        switch (state->matchCallOptions[state->optionCursorPos])
        {
        case MATCH_CALL_OPTION_CANCEL:
            state->callback = CB2_HandleMatchCallInput;
            return POKENAV_MC_FUNC_CANCEL;
        case MATCH_CALL_OPTION_CALL:
            if (GetPokenavMode() == POKENAV_MODE_FORCE_CALL_READY)
                SetPokenavMode(POKENAV_MODE_FORCE_CALL_EXIT);

            state->callback = CB2_HandleCallExitInput;
            if (ShouldDoNearbyMessage())
                return POKENAV_MC_FUNC_NEARBY_MSG;

            return POKENAV_MC_FUNC_CALL_MSG;
        case MATCH_CALL_OPTION_CHECK:
            state->callback = CB2_HandleCheckPageInput;
            return POKENAV_MC_FUNC_SHOW_CHECK_PAGE;
        }
    }

    if (JOY_NEW(B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_CANCEL;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 CB2_HandleCheckPageInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_REPEAT(DPAD_UP))
        return POKENAV_MC_FUNC_CHECK_PAGE_UP;
    if (JOY_REPEAT(DPAD_DOWN))
        return POKENAV_MC_FUNC_CHECK_PAGE_DOWN;

    if (JOY_NEW(B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_EXIT_CHECK_PAGE;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 CB2_HandleCallExitInput(struct Pokenav_MatchCallMenu *state)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        state->callback = CB2_HandleMatchCallInput;
        return POKENAV_MC_FUNC_EXIT_CALL;
    }

    return POKENAV_MC_FUNC_NONE;
}

static u32 LoopedTask_BuildMatchCallList(s32 taskState)
{
}

bool32 IsRematchEntryRegistered(int rematchIndex)
{
    if (rematchIndex < REMATCH_TABLE_ENTRIES)
        return FlagGet(FLAG_MATCH_CALL_REGISTERED + rematchIndex);

    return FALSE;
}

int IsMatchCallListInitFinished(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->initFinished;
}

int GetNumberRegistered(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->numRegistered;
}

struct PokenavMatchCallEntry *GetMatchCallList(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->matchCallEntries;
}

u16 GetMatchCallMapSec(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->matchCallEntries[index].mapSec;
}

bool32 ShouldDrawRematchPokeballIcon(int index)
{
    return FALSE;
}

int GetMatchCallTrainerPic(int index)
{
    int headerId;
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (!state->matchCallEntries[index].isSpecialTrainer)
    {
        index = GetTrainerIdxByRematchIdx(state->matchCallEntries[index].headerId);
        return gTrainers[index].trainerPic;
    }

    headerId = state->matchCallEntries[index].headerId;
    index = MatchCall_GetRematchTableIdx(headerId);
    if (index != REMATCH_TABLE_ENTRIES)
    {
        index = GetTrainerIdxByRematchIdx(index);
        return gTrainers[index].trainerPic;
    }

    index = MatchCall_GetOverrideFacilityClass(headerId);
    return gFacilityClassToPicIndex[index];
}

const u8 *GetMatchCallMessageText(int index, bool8 *newRematchRequest)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    *newRematchRequest = FALSE;
    if (!Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType))
        return gText_CallCantBeMadeHere;

    if (!state->matchCallEntries[index].isSpecialTrainer)
        *newRematchRequest = SelectMatchCallMessage(GetTrainerIdxByRematchIdx(state->matchCallEntries[index].headerId), gStringVar4);
    else
        MatchCall_GetMessage(state->matchCallEntries[index].headerId, gStringVar4);

    return gStringVar4;
}

const u8 *GetMatchCallFlavorText(int index, int checkPageEntry)
{
    int rematchId;
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (state->matchCallEntries[index].isSpecialTrainer)
    {
        rematchId = MatchCall_GetRematchTableIdx(state->matchCallEntries[index].headerId);
        if (rematchId == REMATCH_TABLE_ENTRIES)
            return MatchCall_GetOverrideFlavorText(state->matchCallEntries[index].headerId, checkPageEntry);
    }
    else
    {
        rematchId = state->matchCallEntries[index].headerId;
    }

    return gMatchCallFlavorTexts[rematchId][checkPageEntry];
}

u16 GetMatchCallOptionCursorPos(void)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    return state->optionCursorPos;
}

u16 GetMatchCallOptionId(int optionId)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    if (state->maxOptionId < optionId)
        return MATCH_CALL_OPTION_COUNT;

    return state->matchCallOptions[optionId];
}

void BufferMatchCallNameAndDesc(struct PokenavMatchCallEntry *matchCallEntry, u8 *str)
{
    const u8 *trainerName;
    const u8 *className;
    if (!matchCallEntry->isSpecialTrainer)
    {
        int index = GetTrainerIdxByRematchIdx(matchCallEntry->headerId);
        const struct Trainer *trainer = &gTrainers[index];
        int class = trainer->trainerClass;
        className = gTrainerClassNames[class];
        trainerName = trainer->trainerName;
    }
    else
    {
        MatchCall_GetNameAndDesc(matchCallEntry->headerId, &className, &trainerName);
    }

    if (className && trainerName)
    {
        u8 *str2 = GetStringClearToWidth(str, FONT_NARROW, className, 69);
        GetStringClearToWidth(str2, FONT_NARROW, trainerName, 51);
    }
    else
    {
        GetStringClearToWidth(str, FONT_NARROW, NULL, 120);
    }
}

u8 GetMatchTableMapSectionId(int rematchIndex)
{
    int mapGroup = gRematchTable[rematchIndex].mapGroup;
    int mapNum = gRematchTable[rematchIndex].mapNum;
    return Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum)->regionMapSectionId;
}

int GetIndexDeltaOfNextCheckPageDown(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    int count = 1;
    while (++index < state->numRegistered)
    {
        if (!state->matchCallEntries[index].isSpecialTrainer)
            return count;
        if (MatchCall_HasCheckPage(state->matchCallEntries[index].headerId))
            return count;

        count++;
    }

    return 0;
}

int GetIndexDeltaOfNextCheckPageUp(int index)
{
    struct Pokenav_MatchCallMenu *state = GetSubstructPtr(POKENAV_SUBSTRUCT_MATCH_CALL_MAIN);
    int count = -1;
    while (--index >= 0)
    {
        if (!state->matchCallEntries[index].isSpecialTrainer)
            return count;
        if (MatchCall_HasCheckPage(state->matchCallEntries[index].headerId))
            return count;

        count--;
    }

    return 0;
}

static bool32 ShouldDoNearbyMessage(void)
{
    return FALSE;
}
