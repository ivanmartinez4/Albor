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
