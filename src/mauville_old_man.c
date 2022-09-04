#include "global.h"
#include "main.h"
#include "constants/songs.h"
#include "constants/event_objects.h"
#include "mauville_old_man.h"
#include "event_data.h"
#include "string_util.h"
#include "text.h"
#include "easy_chat.h"
#include "script.h"
#include "random.h"
#include "event_scripts.h"
#include "task.h"
#include "menu.h"
#include "m4a.h"
#include "bard_music.h"
#include "sound.h"
#include "strings.h"
#include "overworld.h"
#include "field_message_box.h"
#include "script_menu.h"
#include "trader.h"
#include "m4a.h"
#include "constants/mauville_old_man.h"

static void InitGiddyTaleList(void);
static void StartBardSong(bool8 useTemporaryLyrics);
static void Task_BardSong(u8 taskId);
static void StorytellerSetup(void);
static void Storyteller_ResetFlag(void);

static u8 sSelectedStory;

struct BardSong gBardSong;

static EWRAM_DATA u16 sUnknownBardRelated = 0;
static EWRAM_DATA struct MauvilleManStoryteller * sStorytellerPtr = NULL;
static EWRAM_DATA u8 sStorytellerWindowId = 0;

static const u16 sDefaultBardSongLyrics[BARD_SONG_LENGTH] = {
    EC_WORD_SHAKE
};

static const u8 *const sGiddyAdjectives[] = {
    GiddyText_SoPretty,
    GiddyText_SoDarling,
    GiddyText_SoRelaxed,
    GiddyText_SoSunny,
    GiddyText_SoDesirable,
    GiddyText_SoExciting,
    GiddyText_SoAmusing,
    GiddyText_SoMagical
};

// Non-random lines Giddy can say. Not all are strictly
// questions, but most are, and the player will receive
// a Yes/No prompt afterwards regardless.
static const u8 *const sGiddyQuestions[GIDDY_MAX_QUESTIONS] = {
    GiddyText_ISoWantToGoOnAVacation,
};

static void SetupBard(void)
{
}

static void SetupHipster(void)
{
}

static void SetupStoryteller(void)
{
    StorytellerSetup();
}

static void SetupGiddy(void)
{
}

static void SetupTrader(void)
{
    TraderSetup();
}

void SetMauvilleOldMan(void)
{
    u16 trainerId = (gSaveBlock2Ptr->playerTrainerId[1] << 8) | gSaveBlock2Ptr->playerTrainerId[0];


    // Determine man based on the last digit of the player's trainer ID.
    switch ((trainerId % 10) / 2)
    {
    case MAUVILLE_MAN_BARD:
        SetupBard();
        break;
    case MAUVILLE_MAN_HIPSTER:
        SetupHipster();
        break;
    case MAUVILLE_MAN_TRADER:
        SetupTrader();
        break;
    case MAUVILLE_MAN_STORYTELLER:
        SetupStoryteller();
        break;
    case MAUVILLE_MAN_GIDDY:
        SetupGiddy();
        break;
    }
    SetMauvilleOldManObjEventGfx();
}

u8 GetCurrentMauvilleOldMan(void)
{
}

void Script_GetCurrentMauvilleMan(void)
{
    gSpecialVar_Result = GetCurrentMauvilleOldMan();
}

void HasBardSongBeenChanged(void)
{
}

void SaveBardSongLyrics(void)
{
}

// Copies lyrics into gStringVar4
static void PrepareSongText(void)
{
}

void PlayBardSong(void)
{
    StartBardSong(gSpecialVar_0x8004);
    ScriptContext_Stop();
}

void GetHipsterSpokenFlag(void)
{
}

void SetHipsterSpokenFlag(void)
{
}

void HipsterTryTeachWord(void)
{
    u16 phrase = GetNewHipsterPhraseToTeach();

    if (phrase == EC_EMPTY_WORD)
    {
        gSpecialVar_Result = FALSE;
    }
    else
    {
        CopyEasyChatWord(gStringVar1, phrase);
        gSpecialVar_Result = TRUE;
    }
}

void GiddyShouldTellAnotherTale(void)
{
}

void GenerateGiddyLine(void)
{
}

static void InitGiddyTaleList(void)
{
}

static void ResetBardFlag(void)
{
}

static void ResetHipsterFlag(void)
{
}

static void ResetTraderFlag(void)
{
    Trader_ResetFlag();
}

static void ResetStorytellerFlag(void)
{
    Storyteller_ResetFlag();
}

void ResetMauvilleOldManFlag(void)
{
    switch (GetCurrentMauvilleOldMan())
    {
    case MAUVILLE_MAN_BARD:
        ResetBardFlag();
        break;
    case MAUVILLE_MAN_HIPSTER:
        ResetHipsterFlag();
        break;
    case MAUVILLE_MAN_STORYTELLER:
        ResetStorytellerFlag();
        break;
    case MAUVILLE_MAN_TRADER:
        ResetTraderFlag();
        break;
    case MAUVILLE_MAN_GIDDY:
        break;
    }
    SetMauvilleOldManObjEventGfx();
}

// States and task data for Task_BardSong.
// The function BardSing receives this task as an
// argument and reads its state as well.
enum {
    BARD_STATE_INIT,
    BARD_STATE_WAIT_BGM,
    BARD_STATE_GET_WORD,
    BARD_STATE_HANDLE_WORD,
    BARD_STATE_WAIT_WORD,
    BARD_STATE_PAUSE,
};

#define tState              data[0]
#define tWordState          data[1]
#define tDelay              data[2]
#define tCharIndex          data[3]
#define tCurrWord           data[4]
#define tUseTemporaryLyrics data[5]

#define MACRO1(a) (((a) & 3) + (((a) / 8) & 1))
#define MACRO2(a) (((a) % 4) + (((a) / 8) & 1))

static void StartBardSong(bool8 useTemporaryLyrics)
{
    u8 taskId = CreateTask(Task_BardSong, 80);

    gTasks[taskId].tUseTemporaryLyrics = useTemporaryLyrics;
}

static void EnableTextPrinters(void)
{
    gDisableTextPrinters = FALSE;
}

static void DisableTextPrinters(struct TextPrinterTemplate * printer, u16 renderCmd)
{
    gDisableTextPrinters = TRUE;
}

static void DrawSongTextWindow(const u8 *str)
{
    DrawDialogueFrame(0, FALSE);
    AddTextPrinterParameterized(0, FONT_NORMAL, str, 0, 1, 1, DisableTextPrinters);
    gDisableTextPrinters = TRUE;
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void BardSing(struct Task *task, struct BardSong *song)
{
}

static void Task_BardSong(u8 taskId)
{
}

void SetMauvilleOldManObjEventGfx(void)
{
    VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_BARD);
}

// Language fixers?

void SanitizeMauvilleOldManForRuby(union OldMan * oldMan)
{
    s32 i;
    u8 playerName[PLAYER_NAME_LENGTH + 1];

    switch (oldMan->common.id)
    {
    case MAUVILLE_MAN_TRADER:
    {
        struct MauvilleOldManTrader * trader = &oldMan->trader;
        for (i = 0; i < NUM_TRADER_ITEMS; i++)
        {
            if (trader->language[i] == LANGUAGE_JAPANESE)
                ConvertInternationalString(trader->playerNames[i], LANGUAGE_JAPANESE);
        }
        break;
    }
    case MAUVILLE_MAN_STORYTELLER:
    {
        struct MauvilleManStoryteller * storyteller = &oldMan->storyteller;
        for (i = 0; i < NUM_STORYTELLER_TALES; i++)
        {
            if (storyteller->gameStatIDs[i] != 0)
            {
                memcpy(playerName, storyteller->trainerNames[i], PLAYER_NAME_LENGTH);
                playerName[PLAYER_NAME_LENGTH] = EOS;
                if (IsStringJapanese(playerName))
                {
                    memset(playerName, CHAR_SPACE, PLAYER_NAME_LENGTH + 1);
                    StringCopy(playerName, gText_Friend);
                    memcpy(storyteller->trainerNames[i], playerName, PLAYER_NAME_LENGTH);
                    storyteller->language[i] = GAME_LANGUAGE;
                }
            }
        }
        break;
    }
    }
}

void SanitizeReceivedEmeraldOldMan(union OldMan * oldMan, u32 version, u32 language)
{
    u8 playerName[PLAYER_NAME_LENGTH + 1];
    s32 i;
    if (oldMan->common.id == MAUVILLE_MAN_STORYTELLER && language == LANGUAGE_JAPANESE)
    {
        struct MauvilleManStoryteller * storyteller = &oldMan->storyteller;

        for (i = 0; i < NUM_STORYTELLER_TALES; i++)
        {
            if (storyteller->gameStatIDs[i] != 0)
            {
                memcpy(playerName, storyteller->trainerNames[i], PLAYER_NAME_LENGTH);
                playerName[PLAYER_NAME_LENGTH] = EOS;
                if (IsStringJapanese(playerName))
                    storyteller->language[i] = LANGUAGE_JAPANESE;
                else
                    storyteller->language[i] = GAME_LANGUAGE;
            }
        }
    }
}

void SanitizeReceivedRubyOldMan(union OldMan * oldMan, u32 version, u32 language)
{
    bool32 isRuby = (version == VERSION_SAPPHIRE || version == VERSION_RUBY);

    switch (oldMan->common.id)
    {
    case MAUVILLE_MAN_TRADER:
    {
        struct MauvilleOldManTrader * trader = &oldMan->trader;
        s32 i;

        if (isRuby)
        {
            for (i = 0; i < NUM_TRADER_ITEMS; i++)
            {
                u8 *str = trader->playerNames[i];
                if (str[0] == EXT_CTRL_CODE_BEGIN && str[1] == EXT_CTRL_CODE_JPN)
                {
                    StripExtCtrlCodes(str);
                    trader->language[i] = LANGUAGE_JAPANESE;
                }
                else
                    trader->language[i] = language;
            }
        }
        else
        {
            for (i = 0; i < NUM_TRADER_ITEMS; i++)
            {
                if (trader->language[i] == LANGUAGE_JAPANESE)
                {
                    StripExtCtrlCodes(trader->playerNames[i]);
                }
            }
        }
    }
    break;
    case MAUVILLE_MAN_STORYTELLER:
    {

        struct MauvilleManStoryteller * storyteller = &oldMan->storyteller;
        s32 i;

        if (isRuby)
        {
            for (i = 0; i < NUM_STORYTELLER_TALES; i++)
            {
                if (storyteller->gameStatIDs[i] != 0)
                    storyteller->language[i] = language;
            }
        }
    }
    break;
    case MAUVILLE_MAN_BARD:
    {
        struct MauvilleManBard * bard = &oldMan->bard;

        if (isRuby)
        {
            bard->language = language;
        }
    }
    break;
    case MAUVILLE_MAN_HIPSTER:
    {
        struct MauvilleManHipster * hipster = &oldMan->hipster;

        if (isRuby)
        {
            hipster->language = language;
        }
    }
    break;
    case MAUVILLE_MAN_GIDDY:
    {
        struct MauvilleManGiddy * giddy = &oldMan->giddy;

        if (isRuby)
        {
            giddy->language = language;
        }
    }
    break;
    }
}

struct Story
{
    u8 stat;
    u8 minVal;
    const u8 *title;
    const u8 *action;
    const u8 *fullText;
};

static const struct Story sStorytellerStories[] = {
    // The 50 below is replaced with GAME_STAT_SAVED_GAME
    {
        50, 1,
        MauvilleCity_PokemonCenter_1F_Text_SavedGameTitle,
        MauvilleCity_PokemonCenter_1F_Text_SavedGameAction,
        MauvilleCity_PokemonCenter_1F_Text_SavedGameStory
    },
    {
        GAME_STAT_STARTED_TRENDS, 1,
        MauvilleCity_PokemonCenter_1F_Text_TrendsStartedTitle,
        MauvilleCity_PokemonCenter_1F_Text_TrendsStartedAction,
        MauvilleCity_PokemonCenter_1F_Text_TrendsStartedStory
    },
    {
        GAME_STAT_PLANTED_BERRIES, 1,
        MauvilleCity_PokemonCenter_1F_Text_BerriesPlantedTitle,
        MauvilleCity_PokemonCenter_1F_Text_BerriesPlantedAction,
        MauvilleCity_PokemonCenter_1F_Text_BerriesPlantedStory
    },
    {
        GAME_STAT_TRADED_BIKES, 1,
        MauvilleCity_PokemonCenter_1F_Text_BikeTradesTitle,
        MauvilleCity_PokemonCenter_1F_Text_BikeTradesAction,
        MauvilleCity_PokemonCenter_1F_Text_BikeTradesStory
    },
    {
        GAME_STAT_GOT_INTERVIEWED, 1,
        MauvilleCity_PokemonCenter_1F_Text_InterviewsTitle,
        MauvilleCity_PokemonCenter_1F_Text_InterviewsAction,
        MauvilleCity_PokemonCenter_1F_Text_InterviewsStory
    },
    {
        GAME_STAT_TRAINER_BATTLES, 1,
        MauvilleCity_PokemonCenter_1F_Text_TrainerBattlesTitle,
        MauvilleCity_PokemonCenter_1F_Text_TrainerBattlesAction,
        MauvilleCity_PokemonCenter_1F_Text_TrainerBattlesStory
    },
    {
        GAME_STAT_POKEMON_CAPTURES, 1,
        MauvilleCity_PokemonCenter_1F_Text_PokemonCaughtTitle,
        MauvilleCity_PokemonCenter_1F_Text_PokemonCaughtAction,
        MauvilleCity_PokemonCenter_1F_Text_PokemonCaughtStory
    },
    {
        GAME_STAT_FISHING_CAPTURES, 1,
        MauvilleCity_PokemonCenter_1F_Text_FishingPokemonCaughtTitle,
        MauvilleCity_PokemonCenter_1F_Text_FishingPokemonCaughtAction,
        MauvilleCity_PokemonCenter_1F_Text_FishingPokemonCaughtStory
    },
    {
        GAME_STAT_HATCHED_EGGS, 1,
        MauvilleCity_PokemonCenter_1F_Text_EggsHatchedTitle,
        MauvilleCity_PokemonCenter_1F_Text_EggsHatchedAction,
        MauvilleCity_PokemonCenter_1F_Text_EggsHatchedStory
    },
    {
        GAME_STAT_EVOLVED_POKEMON, 1,
        MauvilleCity_PokemonCenter_1F_Text_PokemonEvolvedTitle,
        MauvilleCity_PokemonCenter_1F_Text_PokemonEvolvedAction,
        MauvilleCity_PokemonCenter_1F_Text_PokemonEvolvedStory
    },
    {
        GAME_STAT_USED_POKECENTER, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedPokemonCenterTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedPokemonCenterAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedPokemonCenterStory
    },
    {
        GAME_STAT_RESTED_AT_HOME, 1,
        MauvilleCity_PokemonCenter_1F_Text_RestedAtHomeTitle,
        MauvilleCity_PokemonCenter_1F_Text_RestedAtHomeAction,
        MauvilleCity_PokemonCenter_1F_Text_RestedAtHomeStory
    },
    {
        GAME_STAT_ENTERED_SAFARI_ZONE, 1,
        MauvilleCity_PokemonCenter_1F_Text_SafariGamesTitle,
        MauvilleCity_PokemonCenter_1F_Text_SafariGamesAction,
        MauvilleCity_PokemonCenter_1F_Text_SafariGamesStory
    },
    {
        GAME_STAT_USED_CUT, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedCutTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedCutAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedCutStory
    },
    {
        GAME_STAT_USED_ROCK_SMASH, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedRockSmashTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedRockSmashAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedRockSmashStory
    },
    {
        GAME_STAT_MOVED_SECRET_BASE, 1,
        MauvilleCity_PokemonCenter_1F_Text_MovedBasesTitle,
        MauvilleCity_PokemonCenter_1F_Text_MovedBasesAction,
        MauvilleCity_PokemonCenter_1F_Text_MovedBasesStory
    },
    {
        GAME_STAT_USED_SPLASH, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedSplashTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedSplashAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedSplashStory
    },
    {
        GAME_STAT_USED_STRUGGLE, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedStruggleTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedStruggleAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedStruggleStory
    },
    {
        GAME_STAT_SLOT_JACKPOTS, 1,
        MauvilleCity_PokemonCenter_1F_Text_SlotJackpotsTitle,
        MauvilleCity_PokemonCenter_1F_Text_SlotJackpotsAction,
        MauvilleCity_PokemonCenter_1F_Text_SlotJackpotsStory
    },
    {
        GAME_STAT_CONSECUTIVE_ROULETTE_WINS, 2,
        MauvilleCity_PokemonCenter_1F_Text_RouletteWinsTitle,
        MauvilleCity_PokemonCenter_1F_Text_RouletteWinsAction,
        MauvilleCity_PokemonCenter_1F_Text_RouletteWinsStory
    },
    {
        GAME_STAT_ENTERED_BATTLE_TOWER, 1,
        MauvilleCity_PokemonCenter_1F_Text_BattleTowerChallengesTitle,
        MauvilleCity_PokemonCenter_1F_Text_BattleTowerChallengesAction,
        MauvilleCity_PokemonCenter_1F_Text_BattleTowerChallengesStory
    },
    {
        GAME_STAT_POKEBLOCKS, 1,
        MauvilleCity_PokemonCenter_1F_Text_MadePokeblocksTitle,
        MauvilleCity_PokemonCenter_1F_Text_MadePokeblocksAction,
        MauvilleCity_PokemonCenter_1F_Text_MadePokeblocksStory
    },
    {
        GAME_STAT_ENTERED_CONTEST, 1,
        MauvilleCity_PokemonCenter_1F_Text_EnteredContestsTitle,
        MauvilleCity_PokemonCenter_1F_Text_EnteredContestsAction,
        MauvilleCity_PokemonCenter_1F_Text_EnteredContestsStory
    },
    {
        GAME_STAT_WON_CONTEST, 1,
        MauvilleCity_PokemonCenter_1F_Text_WonContestsTitle,
        MauvilleCity_PokemonCenter_1F_Text_WonContestsAction,
        MauvilleCity_PokemonCenter_1F_Text_WonContestsStory
    },
    {
        GAME_STAT_SHOPPED, 1,
        MauvilleCity_PokemonCenter_1F_Text_TimesShoppedTitle,
        MauvilleCity_PokemonCenter_1F_Text_TimesShoppedAction,
        MauvilleCity_PokemonCenter_1F_Text_TimesShoppedStory
    },
    {
        GAME_STAT_USED_ITEMFINDER, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedItemFinderTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedItemFinderAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedItemFinderStory
    },
    {
        GAME_STAT_GOT_RAINED_ON, 1,
        MauvilleCity_PokemonCenter_1F_Text_TimesRainedTitle,
        MauvilleCity_PokemonCenter_1F_Text_TimesRainedAction,
        MauvilleCity_PokemonCenter_1F_Text_TimesRainedStory
    },
    {
        GAME_STAT_CHECKED_POKEDEX, 1,
        MauvilleCity_PokemonCenter_1F_Text_CheckedPokedexTitle,
        MauvilleCity_PokemonCenter_1F_Text_CheckedPokedexAction,
        MauvilleCity_PokemonCenter_1F_Text_CheckedPokedexStory
    },
    {
        GAME_STAT_RECEIVED_RIBBONS, 1,
        MauvilleCity_PokemonCenter_1F_Text_ReceivedRibbonsTitle,
        MauvilleCity_PokemonCenter_1F_Text_ReceivedRibbonsAction,
        MauvilleCity_PokemonCenter_1F_Text_ReceivedRibbonsStory
    },
    {
        GAME_STAT_JUMPED_DOWN_LEDGES, 1,
        MauvilleCity_PokemonCenter_1F_Text_LedgesJumpedTitle,
        MauvilleCity_PokemonCenter_1F_Text_LedgesJumpedAction,
        MauvilleCity_PokemonCenter_1F_Text_LedgesJumpedStory
    },
    {
        GAME_STAT_WATCHED_TV, 1,
        MauvilleCity_PokemonCenter_1F_Text_TVWatchedTitle,
        MauvilleCity_PokemonCenter_1F_Text_TVWatchedAction,
        MauvilleCity_PokemonCenter_1F_Text_TVWatchedStory
    },
    {
        GAME_STAT_CHECKED_CLOCK, 1,
        MauvilleCity_PokemonCenter_1F_Text_CheckedClockTitle,
        MauvilleCity_PokemonCenter_1F_Text_CheckedClockAction,
        MauvilleCity_PokemonCenter_1F_Text_CheckedClockStory
    },
    {
        GAME_STAT_WON_POKEMON_LOTTERY, 1,
        MauvilleCity_PokemonCenter_1F_Text_WonLotteryTitle,
        MauvilleCity_PokemonCenter_1F_Text_WonLotteryAction,
        MauvilleCity_PokemonCenter_1F_Text_WonLotteryStory
    },
    {
        GAME_STAT_USED_DAYCARE, 1,
        MauvilleCity_PokemonCenter_1F_Text_UsedDaycareTitle,
        MauvilleCity_PokemonCenter_1F_Text_UsedDaycareAction,
        MauvilleCity_PokemonCenter_1F_Text_UsedDaycareStory
    },
    {
        GAME_STAT_RODE_CABLE_CAR, 1,
        MauvilleCity_PokemonCenter_1F_Text_RodeCableCarTitle,
        MauvilleCity_PokemonCenter_1F_Text_RodeCableCarAction,
        MauvilleCity_PokemonCenter_1F_Text_RodeCableCarStory
    },
    {
        GAME_STAT_ENTERED_HOT_SPRINGS, 1,
        MauvilleCity_PokemonCenter_1F_Text_HotSpringsTitle,
        MauvilleCity_PokemonCenter_1F_Text_HotSpringsAction,
        MauvilleCity_PokemonCenter_1F_Text_HotSpringsStory
    }
};

static const s32 sNumStories = ARRAY_COUNT(sStorytellerStories);

static void StorytellerSetup(void)
{
}

static void Storyteller_ResetFlag(void)
{
}

static u32 StorytellerGetGameStat(u8 stat)
{
    if (stat == 50)
        stat = GAME_STAT_SAVED_GAME;
    return GetGameStat(stat);
}

static const struct Story *GetStoryByStat(u32 stat)
{
    s32 i;

    for (i = 0; i < sNumStories; i++)
    {
        if (sStorytellerStories[i].stat == stat)
            return &sStorytellerStories[i];
    }
    return &sStorytellerStories[sNumStories - 1];
}

static const u8 *GetStoryTitleByStat(u32 stat)
{
    return GetStoryByStat(stat)->title;
}

static const u8 *GetStoryTextByStat(u32 stat)
{
    return GetStoryByStat(stat)->fullText;
}

static const u8 *GetStoryActionByStat(u32 stat)
{
    return GetStoryByStat(stat)->action;
}

static u8 GetFreeStorySlot(void)
{
    u8 i;

    for (i = 0; i < NUM_STORYTELLER_TALES; i++)
    {
        if (sStorytellerPtr->gameStatIDs[i] == 0)
            break;
    }
    return i;
}

static u32 StorytellerGetRecordedTrainerStat(u32 trainer)
{
    u8 *ptr = sStorytellerPtr->statValues[trainer];

    return ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
}

static void StorytellerSetRecordedTrainerStat(u32 trainer, u32 val)
{
    u8 *ptr = sStorytellerPtr->statValues[trainer];

    ptr[0] = val;
    ptr[1] = val >> 8;
    ptr[2] = val >> 16;
    ptr[3] = val >> 24;
}

static bool32 HasTrainerStatIncreased(u32 trainer)
{
    if (StorytellerGetGameStat(sStorytellerPtr->gameStatIDs[trainer]) > StorytellerGetRecordedTrainerStat(trainer))
        return TRUE;
    else
        return FALSE;
}

static void GetStoryByStattellerPlayerName(u32 player, void *dst)
{
    u8 *name = sStorytellerPtr->trainerNames[player];

    memset(dst, EOS, PLAYER_NAME_LENGTH + 1);
    memcpy(dst, name, PLAYER_NAME_LENGTH);
}

static void StorytellerSetPlayerName(u32 player, const u8 *src)
{
    u8 *name = sStorytellerPtr->trainerNames[player];
    memset(name, EOS, PLAYER_NAME_LENGTH);
    memcpy(name, src, PLAYER_NAME_LENGTH);
}


static void StorytellerRecordNewStat(u32 player, u32 stat)
{
    sStorytellerPtr->gameStatIDs[player] = stat;
    StorytellerSetPlayerName(player, gSaveBlock2Ptr->playerName);
    StorytellerSetRecordedTrainerStat(player, StorytellerGetGameStat(stat));
    ConvertIntToDecimalStringN(gStringVar1, StorytellerGetGameStat(stat), STR_CONV_MODE_LEFT_ALIGN, 10);
    StringCopy(gStringVar2, GetStoryActionByStat(stat));
    sStorytellerPtr->language[player] = gGameLanguage;
}

static void ScrambleStatList(u8 *arr, s32 count)
{
    s32 i;

    for (i = 0; i < count; i++)
        arr[i] = i;
    for (i = 0; i < count; i++)
    {
        u32 a = Random() % count;
        u32 b = Random() % count;
        u8 temp;
        SWAP(arr[a], arr[b], temp);
    }
}

static bool8 StorytellerInitializeRandomStat(void)
{
    u8 storyIds[sNumStories];
    s32 i, j;

    ScrambleStatList(storyIds, sNumStories);
    for (i = 0; i < sNumStories; i++)
    {
        u8 stat = sStorytellerStories[storyIds[i]].stat;
        u8 minVal = sStorytellerStories[storyIds[i]].minVal;

        for (j = 0; j < NUM_STORYTELLER_TALES; j++)
        {
            if (sStorytellerPtr->gameStatIDs[j] == stat)
                break;
        }
        if (j == NUM_STORYTELLER_TALES && StorytellerGetGameStat(stat) >= minVal)
        {
            sStorytellerPtr->alreadyRecorded = TRUE;
            if (GetFreeStorySlot() == NUM_STORYTELLER_TALES)
                StorytellerRecordNewStat(sSelectedStory, stat);
            else
                StorytellerRecordNewStat(GetFreeStorySlot(), stat);
            return TRUE;
        }
    }
    return FALSE;
}

static void StorytellerDisplayStory(u32 player)
{
    u8 stat = sStorytellerPtr->gameStatIDs[player];

    ConvertIntToDecimalStringN(gStringVar1, StorytellerGetRecordedTrainerStat(player), STR_CONV_MODE_LEFT_ALIGN, 10);
    StringCopy(gStringVar2, GetStoryActionByStat(stat));
    GetStoryByStattellerPlayerName(player, gStringVar3);
    ConvertInternationalString(gStringVar3, sStorytellerPtr->language[player]);
    ShowFieldMessage(GetStoryTextByStat(stat));
}

static void PrintStoryList(void)
{
    s32 i;
    s32 width = GetStringWidth(FONT_NORMAL, gText_Exit, 0);
    for (i = 0; i < NUM_STORYTELLER_TALES; i++)
    {
        s32 curWidth;
        u16 gameStatID = sStorytellerPtr->gameStatIDs[i];

        if (gameStatID == 0)
            break;
        curWidth = GetStringWidth(FONT_NORMAL, GetStoryTitleByStat(gameStatID), 0);
        if (curWidth > width)
            width = curWidth;
    }
    sStorytellerWindowId = CreateWindowFromRect(0, 0, ConvertPixelWidthToTileWidth(width), GetFreeStorySlot() * 2 + 2);
    SetStandardWindowBorderStyle(sStorytellerWindowId, FALSE);
    for (i = 0; i < NUM_STORYTELLER_TALES; i++)
    {
        u16 gameStatID = sStorytellerPtr->gameStatIDs[i];
        if (gameStatID == 0)
            break;
        AddTextPrinterParameterized(sStorytellerWindowId, FONT_NORMAL, GetStoryTitleByStat(gameStatID), 8, 16 * i + 1, TEXT_SKIP_DRAW, NULL);
    }
    AddTextPrinterParameterized(sStorytellerWindowId, FONT_NORMAL, gText_Exit, 8, 16 * i + 1, TEXT_SKIP_DRAW, NULL);
    InitMenuInUpperLeftCornerNormal(sStorytellerWindowId, GetFreeStorySlot() + 1, 0);
    CopyWindowToVram(sStorytellerWindowId, COPYWIN_FULL);
}

static void Task_StoryListMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 selection;

    switch (task->data[0])
    {
    case 0:
        PrintStoryList();
        task->data[0]++;
        break;
    case 1:
        selection = Menu_ProcessInput();
        if (selection == MENU_NOTHING_CHOSEN)
            break;
        if (selection == MENU_B_PRESSED || selection == GetFreeStorySlot())
        {
            gSpecialVar_Result = 0;
        }
        else
        {
            gSpecialVar_Result = 1;
            sSelectedStory = selection;
        }
        ClearToTransparentAndRemoveWindow(sStorytellerWindowId);
        DestroyTask(taskId);
        ScriptContext_Enable();
        break;
    }
}

// Sets gSpecialVar_Result to TRUE if player selected a story
void StorytellerStoryListMenu(void)
{
    CreateTask(Task_StoryListMenu, 80);
}

void Script_StorytellerDisplayStory(void)
{
    StorytellerDisplayStory(sSelectedStory);
}

u8 StorytellerGetFreeStorySlot(void)
{
}

// Returns TRUE if stat has increased
bool8 StorytellerUpdateStat(void)
{
}

bool8 HasStorytellerAlreadyRecorded(void)
{
}

bool8 Script_StorytellerInitializeRandomStat(void)
{
}

