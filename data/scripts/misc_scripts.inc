@ For scripts that don't fit thematically in other .inc files

EventScript_DisableAutoRun::
	playse SE_BOO
	waitse
	end

EventScript_EnableAutoRun::
	playse SE_PIN
	waitse
	end

EventScript_CheckFishingStreak::
	lockall
	specialvar VAR_RESULT, CheckChainFishingStreak
	buffernumberstring 0, VAR_RESULT
	msgbox gText_CurrentFishingStreak, MSGBOX_DEFAULT
	releaseall
	end

gText_CurrentFishingStreak::
	.string "Current chain fishing streak: {STR_VAR_1}.$"

EventScript_DebugItemPack::
	lockall
	setflag FLAG_SYS_POKEMON_GET
	setflag FLAG_SYS_POKEDEX_GET
	setflag FLAG_SYS_B_DASH
	setflag FLAG_RECEIVED_RUNNING_SHOES
	setflag FLAG_RECEIVED_AMULET_COIN
	setflag FLAG_RECEIVED_EXP_SHARE
	special EnableNationalPokedex
	givemon SPECIES_CHARIZARD, 100, 0
	givemon SPECIES_BLASTOISE, 100, 0
	givemon SPECIES_VENUSAUR, 100, 0
	givemon SPECIES_SNORLAX, 100, 0
	additem ITEM_AMULET_COIN, 1
	additem ITEM_EXP_SHARE, 1
	additem ITEM_RARE_CANDY, 99
	additem ITEM_MASTER_BALL, 99
	additem ITEM_HM01, 1
	additem ITEM_HM02, 1
	additem ITEM_HM03, 1
	additem ITEM_HM04, 1
	additem ITEM_HM05, 1
	additem ITEM_HM06, 1
	additem ITEM_HM07, 1
	additem ITEM_HM08, 1
	additem ITEM_GOOD_ROD, 1
	additem ITEM_MACH_BIKE, 1
	setflag FLAG_BADGE01_GET
	setflag FLAG_BADGE02_GET
	setflag FLAG_BADGE03_GET
	setflag FLAG_BADGE04_GET
	setflag FLAG_BADGE05_GET
	setflag FLAG_BADGE06_GET
	setflag FLAG_BADGE07_GET
	setflag FLAG_BADGE08_GET
	releaseall
	end
