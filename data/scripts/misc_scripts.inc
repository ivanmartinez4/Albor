@ For scripts that don't fit thematically in other .inc files

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_DisableAutoRun::
	playse SE_BOO
	waitse
	end

EventScript_EnableAutoRun::
	playse SE_PIN
	waitse
	end

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_CheckFishingStreak::
	lockall
	specialvar VAR_RESULT, CheckChainFishingStreak
	buffernumberstring 0, VAR_RESULT
	msgbox EventScript_CheckFishingStreak_Text_CurrentFishingStreak, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CheckFishingStreak_Text_CurrentFishingStreak::
	.string "Current chain fishing streak: {STR_VAR_1}.$"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_CheckSavefileSize::
	special CheckSavefileSize
	msgbox EventScript_Text_CheckSavefileSize
	release
	end

EventScript_Text_CheckSavefileSize:
	.string "SaveBlock1 size: {STR_VAR_1}/15872 bytes.\nSaveBlock2 size: {STR_VAR_2}/3968 bytes.$"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_CheckEVs::
	lockall
	faceplayer
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, PARTY_SIZE
	goto_if_ge EventScript_Cancel
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	compare VAR_RESULT, SPECIES_EGG
	goto_if_eq EventScript_CheckEVs_Egg
	special CheckMonEVs
	msgbox EventScript_CheckEVs_Text_Evaluation
	msgbox EventScript_Text_ComeBackAnytime
	releaseall
	end

EventScript_Cancel:
	lockall
	faceplayer
	msgbox EventScript_Text_ComeBackAnytime
	releaseall
	end

EventScript_CheckEVs_Egg:
	lockall
	faceplayer
	msgbox EventScript_CheckEVs_Text_Egg
	releaseall
	end

EventScript_CheckEVs_Text_Evaluation:
	.string "Your POKéMON has: {STR_VAR_1} HP EVs,\n"
	.string "{STR_VAR_2} ATK EVs and {STR_VAR_3} DEF EVs.\p"
	.string "Let's see what else…\p"
	.string "It also has: {STR_VAR_4} SPD EVs,\n"
	.string "{STR_VAR_5} SP. ATK EVs and {STR_VAR_6} SP. DEF EVs.$"

EventScript_CheckEVs_Text_Egg:
	.string "Sorry, Pokémon Eggs don't\n"
	.string "have EVs.$"

EventScript_Text_ComeBackAnytime:
	.string "Come back anytime.$"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_CheckIVs::
	lockall
	faceplayer
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, PARTY_SIZE
	goto_if_ge EventScript_Cancel
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	compare VAR_RESULT, SPECIES_EGG
	goto_if_eq EventScript_CheckIVs_Egg
	special CheckMonIVs
	msgbox EventScript_CheckIVs_Text_Evaluation
	msgbox EventScript_Text_ComeBackAnytime
	releaseall
	end

EventScript_CheckIVs_Egg:
	lockall
	faceplayer
	msgbox EventScript_CheckIVs_Text_Egg
	releaseall
	end

EventScript_CheckIVs_Text_Evaluation:
	.string "Your POKéMON has: {STR_VAR_1} HP IVs,\n"
	.string "{STR_VAR_2} ATK IVs and {STR_VAR_3} DEF IVs.\p"
	.string "Let's see what else…\p"
	.string "It also has: {STR_VAR_4} SPD IVs,\n"
	.string "{STR_VAR_5} SP. ATK IVs and {STR_VAR_6} SP. DEF IVs.$"

EventScript_CheckIVs_Text_Egg:
	.string "Sorry, Pokémon Eggs don't\n"
	.string "have IVs.$"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

EventScript_ForceEggHatch::
	lockall
	faceplayer
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, PARTY_SIZE
	goto_if_eq EventScript_Text_ComeBackAnytime
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	compare VAR_RESULT, SPECIES_EGG
	goto_if_ne EventScript_ForceEggHatch_CantForceHatch
	special EggHatch
	waitstate
	faceplayer @ For some reason, if I don't re-cast faceplayer the NPC is able to move around.
	msgbox EventScript_Text_ComeBackAnytime
	releaseall
	end
EventScript_ForceEggHatch_CantForceHatch:
	lock
	msgbox EventScript_ForcedEggHatch_Text_NotAnEgg
	release
	end

EventScript_ForcedEggHatch_Text_NotAnEgg:
	.string "That's not a Pokémon Egg.$"
