RustboroCity_Mart_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RustboroCity_Mart_OnLoad
	.byte 0

RustboroCity_Mart_OnLoad:
	call_if_unset FLAG_MET_DEVON_EMPLOYEE, RustboroCity_Mart_HideSpecialBallsSeller
RustboroCity_Mart_OnLoad_End:
	end

RustboroCity_Mart_HideSpecialBallsSeller:
	goto_if_set FLAG_HIDE_SPECIAL_BALLS_SELLER, RustboroCity_Mart_OnLoad_End
	setflag FLAG_HIDE_SPECIAL_BALLS_SELLER
	return

RustboroCity_Mart_EventScript_PokefanF::
	msgbox RustboroCity_Mart_Text_BuyingHealsInCaseOfShroomish, MSGBOX_NPC
	end

RustboroCity_Mart_EventScript_Boy::
	msgbox RustboroCity_Mart_Text_ShouldBuySuperPotionsInstead, MSGBOX_NPC
	end

RustboroCity_Mart_EventScript_BugCatcher::
	msgbox RustboroCity_Mart_Text_GettingEscapeRopeJustInCase, MSGBOX_NPC
	end

RustboroCity_Mart_Text_BuyingHealsInCaseOfShroomish:
	.string "I'm buying some Parlyz Heals and\n"
	.string "Antidotes.\p"
	.string "Just in case I run into Shroomish\n"
	.string "in Petalburg Woods.$"

RustboroCity_Mart_Text_ShouldBuySuperPotionsInstead:
	.string "My Pokémon evolved.\n"
	.string "It has a lot of HP now.\p"
	.string "I should buy Super Potions for it\n"
	.string "instead of ordinary Potions.$"

RustboroCity_Mart_Text_GettingEscapeRopeJustInCase:
	.string "I'm getting an Escape Rope just in\n"
	.string "case I get lost in a cave.\p"
	.string "I just need to use it to get back to\n"
	.string "the entrance.$"

