#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "recorded_battle.h"
#include "main.h"
#include "pokemon.h"
#include "random.h"
#include "event_data.h"
#include "link.h"
#include "string_util.h"
#include "palette.h"
#include "save.h"
#include "malloc.h"
#include "util.h"
#include "task.h"
#include "test_runner.h"
#include "text.h"
#include "battle_setup.h"
#include "frontier_util.h"
#include "constants/trainers.h"
#include "constants/rgb.h"

struct PlayerInfo
{
    u32 trainerId;
    u8 name[PLAYER_NAME_LENGTH + 1];
    u8 gender;
    u16 battlerId;
    u16 language;
};

// Save data using TryWriteSpecialSaveSector is allowed to exceed SECTOR_DATA_SIZE (up to the counter field)
STATIC_ASSERT(sizeof(struct RecordedBattleSave) <= SECTOR_COUNTER_OFFSET, RecordedBattleSaveFreeSpace);

EWRAM_DATA u32 gRecordedBattleRngSeed = 0;
EWRAM_DATA u32 gBattlePalaceMoveSelectionRngValue = 0;
EWRAM_DATA static u8 sBattleRecords[MAX_BATTLERS_COUNT][BATTLER_RECORD_SIZE] = {0};
EWRAM_DATA static u16 sBattlerRecordSizes[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static u16 sBattlerPrevRecordSizes[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static u16 sBattlerSavedRecordSizes[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static u8 sRecordMode = 0;
EWRAM_DATA static u8 sLvlMode = 0;
EWRAM_DATA static u8 sFrontierFacility = 0;
EWRAM_DATA static u8 sFrontierBrainSymbol = 0;
EWRAM_DATA static MainCallback sCallback2_AfterRecordedBattle = NULL;
EWRAM_DATA u8 gRecordedBattleMultiplayerId = 0;
EWRAM_DATA static u8 sFrontierPassFlag = 0;
EWRAM_DATA static u8 sBattleScene = 0;
EWRAM_DATA static u8 sTextSpeed = 0;
EWRAM_DATA static u32 sBattleFlags = 0;
EWRAM_DATA static u32 sAI_Scripts = 0;
EWRAM_DATA static struct Pokemon sSavedPlayerParty[PARTY_SIZE] = {0};
EWRAM_DATA static struct Pokemon sSavedOpponentParty[PARTY_SIZE] = {0};
EWRAM_DATA static u16 sPlayerMonMoves[MAX_BATTLERS_COUNT / 2][MAX_MON_MOVES] = {0};
EWRAM_DATA static struct PlayerInfo sPlayers[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static bool8 sIsPlaybackFinished = 0;
EWRAM_DATA static u8 sRecordMixFriendName[PLAYER_NAME_LENGTH + 1] = {0};
EWRAM_DATA static u8 sRecordMixFriendClass = 0;
EWRAM_DATA static u16 sEasyChatSpeech[EASY_CHAT_BATTLE_WORDS_COUNT] = {0};
EWRAM_DATA static u8 sBattleOutcome = 0;

static u8 sRecordMixFriendLanguage;

static u8 GetNextRecordedDataByte(u8 *, u8 *, u8 *);
static bool32 CopyRecordedBattleFromSave(struct RecordedBattleSave *);
static void RecordedBattle_RestoreSavedParties(void);
static void CB2_RecordedBattle(void);

void RecordedBattle_Init(u8 mode)
{
    s32 i, j;

    sRecordMode = mode;
    sIsPlaybackFinished = FALSE;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        sBattlerRecordSizes[i] = 0;
        sBattlerPrevRecordSizes[i] = 0;
        sBattlerSavedRecordSizes[i] = 0;

        if (mode == B_RECORD_MODE_RECORDING)
        {
            for (j = 0; j < BATTLER_RECORD_SIZE; j++)
                sBattleRecords[i][j] = 0xFF;
            sBattleFlags = gBattleTypeFlags;
            sAI_Scripts = gBattleResources->ai->aiFlags;
        }
    }
}

void RecordedBattle_SetTrainerInfo(void)
{

}

void RecordedBattle_SetBattlerAction(u8 battlerId, u8 action)
{
    if (sBattlerRecordSizes[battlerId] < BATTLER_RECORD_SIZE && sRecordMode != B_RECORD_MODE_PLAYBACK)
        sBattleRecords[battlerId][sBattlerRecordSizes[battlerId]++] = action;
}

void RecordedBattle_ClearBattlerAction(u8 battlerId, u8 bytesToClear)
{
    s32 i;

    for (i = 0; i < bytesToClear; i++)
    {
        sBattlerRecordSizes[battlerId]--;
        sBattleRecords[battlerId][sBattlerRecordSizes[battlerId]] = 0xFF;
        if (sBattlerRecordSizes[battlerId] == 0)
            break;
    }
}

u8 RecordedBattle_GetBattlerAction(u32 actionType, u8 battlerId)
{
    if (gTestRunnerEnabled)
        BattleTest_CheckBattleRecordActionType(battlerId, sBattlerRecordSizes[battlerId], actionType);

    // Trying to read past array or invalid action byte, battle is over.
    if (sBattlerRecordSizes[battlerId] >= BATTLER_RECORD_SIZE || sBattleRecords[battlerId][sBattlerRecordSizes[battlerId]] == 0xFF)
    {
        gSpecialVar_Result = gBattleOutcome = B_OUTCOME_PLAYER_TELEPORTED; // hah
        ResetPaletteFadeControl();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        SetMainCallback2(CB2_QuitRecordedBattle);
        return 0xFF;
    }
    else
    {
        return sBattleRecords[battlerId][sBattlerRecordSizes[battlerId]++];
    }
}

u8 RecordedBattle_BufferNewBattlerData(u8 *dst)
{
    u8 i, j;
    u8 idx = 0;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (sBattlerRecordSizes[i] != sBattlerPrevRecordSizes[i])
        {
            dst[idx++] = i;
            dst[idx++] = sBattlerRecordSizes[i] - sBattlerPrevRecordSizes[i];

            for (j = 0; j < sBattlerRecordSizes[i] - sBattlerPrevRecordSizes[i]; j++)
                dst[idx++] = sBattleRecords[i][sBattlerPrevRecordSizes[i] + j];

            sBattlerPrevRecordSizes[i] = sBattlerRecordSizes[i];
        }
    }

    return idx;
}

void RecordedBattle_RecordAllBattlerData(u8 *src)
{

}

static u8 GetNextRecordedDataByte(u8 *data, u8 *idx, u8 *size)
{
    (*size)--;
    return data[(*idx)++];
}

bool32 CanCopyRecordedBattleSaveData(void)
{
    struct RecordedBattleSave *dst = AllocZeroed(sizeof(struct RecordedBattleSave));
    bool32 ret = CopyRecordedBattleFromSave(dst);
    Free(dst);
    return ret;
}

static bool32 IsRecordedBattleSaveValid(struct RecordedBattleSave *save)
{
    if (save->battleFlags == 0)
        return FALSE;
    if (save->battleFlags & BATTLE_TYPE_RECORDED_INVALID)
        return FALSE;
    if (CalcByteArraySum((void *)(save), sizeof(*save) - 4) != save->checksum)
        return FALSE;

    return TRUE;
}

static bool32 RecordedBattleToSave(struct RecordedBattleSave *battleSave, struct RecordedBattleSave *saveSector)
{
}

bool32 MoveRecordedBattleToSaveData(void)
{
}

static bool32 TryCopyRecordedBattleSaveData(struct RecordedBattleSave *dst, struct SaveSector *saveBuffer)
{
}

static bool32 CopyRecordedBattleFromSave(struct RecordedBattleSave *dst)
{
    struct SaveSector *savBuffer = AllocZeroed(SECTOR_SIZE);
    bool32 ret = TryCopyRecordedBattleSaveData(dst, savBuffer);
    Free(savBuffer);

    return ret;
}

static void CB2_RecordedBattleEnd(void)
{
}

#define tFramesToWait data[0]

static void Task_StartAfterCountdown(u8 taskId)
{
    if (--gTasks[taskId].tFramesToWait == 0)
    {
        gMain.savedCallback = CB2_RecordedBattleEnd;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

void SetVariablesForRecordedBattle(struct RecordedBattleSave *src)
{
}

void PlayRecordedBattle(void (*CB2_After)(void))
{
    struct RecordedBattleSave *battleSave = AllocZeroed(sizeof(struct RecordedBattleSave));
    if (CopyRecordedBattleFromSave(battleSave) == TRUE)
    {
        u8 taskId;

        RecordedBattle_SaveParties();
        SetVariablesForRecordedBattle(battleSave);

        taskId = CreateTask(Task_StartAfterCountdown, 1);
        gTasks[taskId].tFramesToWait = 128;

        sCallback2_AfterRecordedBattle = CB2_After;
        PlayMapChosenOrBattleBGM(FALSE);
        SetMainCallback2(CB2_RecordedBattle);
    }
    Free(battleSave);
}

#undef tFramesToWait

static void CB2_RecordedBattle(void)
{
    AnimateSprites();
    BuildOamBuffer();
    RunTasks();
}

u8 GetRecordedBattleFrontierFacility(void)
{
    return sFrontierFacility;
}

u8 GetRecordedBattleFronterBrainSymbol(void)
{
    return sFrontierBrainSymbol;
}

void RecordedBattle_SaveParties(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        sSavedPlayerParty[i] = gPlayerParty[i];
        sSavedOpponentParty[i] = gEnemyParty[i];
    }
}

static void RecordedBattle_RestoreSavedParties(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        gPlayerParty[i] = sSavedPlayerParty[i];
        gEnemyParty[i] = sSavedOpponentParty[i];
    }
}

u8 GetActiveBattlerLinkPlayerGender(void)
{
    s32 i;

    for (i = 0; i < MAX_LINK_PLAYERS; i++)
    {
        if (gLinkPlayers[i].id == gActiveBattler)
            break;
    }

    if (i != MAX_LINK_PLAYERS)
        return gLinkPlayers[i].gender;

    return 0;
}

void RecordedBattle_ClearFrontierPassFlag(void)
{
    sFrontierPassFlag = 0;
}

// Set sFrontierPassFlag to received state of FLAG_SYS_FRONTIER_PASS
void RecordedBattle_SetFrontierPassFlagFromHword(u16 flags)
{
    sFrontierPassFlag |= (flags & (1 << 15)) >> 15;
}

u8 RecordedBattle_GetFrontierPassFlag(void)
{
    return sFrontierPassFlag;
}

u8 GetBattleSceneInRecordedBattle(void)
{
    return sBattleScene;
}

u8 GetTextSpeedInRecordedBattle(void)
{
    return sTextSpeed;
}

void RecordedBattle_CopyBattlerMoves(void)
{
    s32 i;

    if (GetBattlerSide(gActiveBattler) == B_SIDE_OPPONENT)
        return;
    if (gBattleTypeFlags & (BATTLE_TYPE_RECORDED_LINK))
        return;
    if (sRecordMode == B_RECORD_MODE_PLAYBACK)
        return;

    for (i = 0; i < MAX_MON_MOVES; i++)
        sPlayerMonMoves[gActiveBattler / 2][i] = gBattleMons[gActiveBattler].moves[i];
}

// This is a special battle action only used by this function
// It shares a value with B_ACTION_SAFARI_POKEBLOCK, which can never occur in a recorded battle.
#define ACTION_MOVE_CHANGE 6

void RecordedBattle_CheckMovesetChanges(u8 mode)
{
    s32 battlerId, j, k;

    if (gBattleTypeFlags & (BATTLE_TYPE_RECORDED_LINK))
        return;

    for (battlerId = 0; battlerId < gBattlersCount; battlerId++)
    {
        // Player's side only
        if (GetBattlerSide(battlerId) != B_SIDE_OPPONENT)
        {
            if (mode == B_RECORD_MODE_RECORDING)
            {
                // Check if any of the battler's moves have changed.
                for (j = 0; j < MAX_MON_MOVES; j++)
                {
                    if (gBattleMons[battlerId].moves[j] != sPlayerMonMoves[battlerId / 2][j])
                        break;
                }
                if (j != MAX_MON_MOVES)
                {
                    // At least one of the moves has been changed
                    RecordedBattle_SetBattlerAction(battlerId, ACTION_MOVE_CHANGE);
                    for (j = 0; j < MAX_MON_MOVES; j++)
                    {
                        for (k = 0; k < MAX_MON_MOVES; k++)
                        {
                            if (gBattleMons[battlerId].moves[j] == sPlayerMonMoves[battlerId / 2][k])
                            {
                                RecordedBattle_SetBattlerAction(battlerId, k);
                                break;
                            }
                        }
                    }
                }
            }
            else // B_RECORD_MODE_PLAYBACK
            {
                if (sBattleRecords[battlerId][sBattlerRecordSizes[battlerId]] == ACTION_MOVE_CHANGE)
                {
                    u8 ppBonuses[MAX_MON_MOVES];
                    u8 moveSlots[MAX_MON_MOVES];
                    u8 mimickedMoveSlots[MAX_MON_MOVES];
                    struct ChooseMoveStruct movePp;
                    u8 ppBonusSet;

                    // We know the current action is ACTION_MOVE_CHANGE, retrieve
                    // it without saving it to move on to the next action.
                    RecordedBattle_GetBattlerAction(RECORDED_BYTE, battlerId);

                    for (j = 0; j < MAX_MON_MOVES; j++)
                        ppBonuses[j] = ((gBattleMons[battlerId].ppBonuses & (3 << (j << 1))) >> (j << 1));

                    for (j = 0; j < MAX_MON_MOVES; j++)
                    {
                        moveSlots[j] = RecordedBattle_GetBattlerAction(RECORDED_BYTE, battlerId);
                        movePp.moves[j] = gBattleMons[battlerId].moves[moveSlots[j]];
                        movePp.currentPp[j] = gBattleMons[battlerId].pp[moveSlots[j]];
                        movePp.maxPp[j] = ppBonuses[moveSlots[j]];
                        mimickedMoveSlots[j] = (gDisableStructs[battlerId].mimickedMoves & gBitTable[j]) >> j;
                    }
                    for (j = 0; j < MAX_MON_MOVES; j++)
                    {
                        gBattleMons[battlerId].moves[j] = movePp.moves[j];
                        gBattleMons[battlerId].pp[j] = movePp.currentPp[j];
                    }
                    gBattleMons[battlerId].ppBonuses = 0;
                    gDisableStructs[battlerId].mimickedMoves = 0;
                    for (j = 0; j < MAX_MON_MOVES; j++)
                    {
                        gBattleMons[battlerId].ppBonuses |= movePp.maxPp[j] << (j << 1);
                        gDisableStructs[battlerId].mimickedMoves |= mimickedMoveSlots[j] << j;
                    }

                    if (!(gBattleMons[battlerId].status2 & STATUS2_TRANSFORMED))
                    {
                        for (j = 0; j < MAX_MON_MOVES; j++)
                            ppBonuses[j] = (GetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_PP_BONUSES, NULL) & ((3 << (j << 1)))) >> (j << 1);

                        for (j = 0; j < MAX_MON_MOVES; j++)
                        {
                            movePp.moves[j] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_MOVE1 + moveSlots[j], NULL);
                            movePp.currentPp[j] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_PP1 + moveSlots[j], NULL);
                            movePp.maxPp[j] = ppBonuses[moveSlots[j]];
                        }
                        for (j = 0; j < MAX_MON_MOVES; j++)
                        {
                            SetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_MOVE1 + j, &movePp.moves[j]);
                            SetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_PP1 + j, &movePp.currentPp[j]);
                        }
                        ppBonusSet = 0;
                        for (j = 0; j < MAX_MON_MOVES; j++)
                            ppBonusSet |= movePp.maxPp[j] << (j << 1);

                        SetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_PP_BONUSES, &ppBonusSet);
                    }
                    gChosenMoveByBattler[battlerId] = gBattleMons[battlerId].moves[*(gBattleStruct->chosenMovePositions + battlerId)];
                }
            }
        }
    }
}

u32 GetAiScriptsInRecordedBattle(void)
{
    return sAI_Scripts;
}

// Used to determine when the player is allowed to press B to end a recorded battle's playback
void RecordedBattle_SetPlaybackFinished(void)
{
    sIsPlaybackFinished = TRUE;
}

bool8 RecordedBattle_CanStopPlayback(void)
{
    return (sIsPlaybackFinished == FALSE);
}

void GetRecordedBattleRecordMixFriendName(u8 *dst)
{
    s32 i;

    for (i = 0; i < PLAYER_NAME_LENGTH + 1; i++)
        dst[i] = sRecordMixFriendName[i];
    dst[PLAYER_NAME_LENGTH] = EOS;
    ConvertInternationalString(dst, sRecordMixFriendLanguage);
}

u8 GetRecordedBattleRecordMixFriendClass(void)
{
    return sRecordMixFriendClass;
}

u8 GetRecordedBattleRecordMixFriendLanguage(void)
{
    return sRecordMixFriendLanguage;
}

void RecordedBattle_SaveBattleOutcome(void)
{
    sBattleOutcome = gBattleOutcome;
}

u16 *GetRecordedBattleEasyChatSpeech(void)
{
    return sEasyChatSpeech;
}
