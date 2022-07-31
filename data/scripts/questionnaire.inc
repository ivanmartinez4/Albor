.set LOCALID_MART_EMPLOYEE, 1

EventScript_Questionnaire::
	lockall
	msgbox Questionnaire_Text_FillOut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Questionnaire_EventScript_Release
	setvar VAR_0x8004, EASY_CHAT_TYPE_QUESTIONNAIRE
	call Common_ShowEasyChatScreen
	lock
	faceplayer
	goto_if_eq VAR_0x8004, 2, Questionnaire_EventScript_PlayerInputMysteryGiftPhrase
	goto_if_eq VAR_RESULT, 0, Questionnaire_EventScript_Release
	goto_if_eq VAR_RESULT, 1, Questionnaire_EventScript_ThankYou
	end

Questionnaire_EventScript_PlayerInputMysteryGiftPhrase::
	goto_if_unset FLAG_SYS_POKEDEX_GET, Questionnaire_EventScript_ThankYou
	goto_if_set FLAG_OBTAINED_PICHU_EGG, Questionnaire_EventScript_ThankYou
	applymovement LOCALID_MART_EMPLOYEE, Common_Movement_FaceDown
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_MART_EMPLOYEE, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_MART_EMPLOYEE, Common_Movement_Delay48
	waitmovement 0
	msgbox Questionnaire_Text_YouKnowThoseWordsEvent, MSGBOX_DEFAULT
	call Questionnaire_EventScript_PichuEggStart
	setflag FLAG_OBTAINED_PICHU_EGG
	msgbox Questionnaire_Text_ThankYou, MSGBOX_DEFAULT
	releaseall
	end

Questionnaire_EventScript_PichuEggStart::
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	call_if_eq VAR_RESULT, PARTY_SIZE, Questionnaire_EventScript_SendToPC
	givespecialegg SPECIES_PICHU, METLOC_FATEFUL_ENCOUNTER, MOVE_THUNDER_SHOCK, MOVE_CHARM, MOVE_SURF, MOVE_NONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox LavaridgeTown_Text_ReceivedTheEgg, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	return

Questionnaire_EventScript_SendToPC:
	msgbox Questionnaire_Text_SendToPC, MSGBOX_DEFAULT
	closemessage
	return

Questionnaire_EventScript_Release::
	releaseall
	end

Questionnaire_EventScript_ThankYou::
	applymovement LOCALID_MART_EMPLOYEE, Common_Movement_FaceDown
	waitmovement 0
	msgbox Questionnaire_Text_ThankYou, MSGBOX_DEFAULT
	releaseall
	end

Questionnaire_Text_FillOut::
	.string "There is a questionnaire.\n"
	.string "Would you like to fill it out?$"

Questionnaire_Text_ThankYou::
	.string "Thank you for taking the time to\n"
	.string "fill out our questionnaire.\p"
	.string "Your feedback will be used for\n"
	.string "future reference.$"

Questionnaire_Text_YouKnowThoseWordsEvent::
	.string "Oh, hello!\n"
	.string "You know those words?\p"
	.string "That means you must know about\n"
	.string "the MYSTERY EVENT.\p"
	.string "Sadly, MYSTERY EVENTS are over.\p"
	.string "Here, please take this POKéMON\n"
	.string "EGG for your troubles.$"

Questionnaire_Text_SendToPC::
	.string "It seems you have no room in\n"
	.string "your party. I'll have to send\l"
	.string "this EGG to your PC instead.$"
