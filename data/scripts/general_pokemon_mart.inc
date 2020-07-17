General_Poke_Mart_Script::
	lock
	faceplayer
	goto_if_set FLAG_BADGE08_GET, General_Poke_Mart_Badge8
	goto_if_set FLAG_BADGE07_GET, General_Poke_Mart_Badge7
	goto_if_set FLAG_BADGE05_GET, General_Poke_Mart_Badge5
	goto_if_set FLAG_BADGE03_GET, General_Poke_Mart_Badge3
	goto_if_set FLAG_BADGE02_GET, General_Poke_Mart_Badge2
	goto_if_set FLAG_BADGE01_GET, General_Poke_Mart_Badge1
	goto_if_unset FLAG_ADVENTURE_STARTED, Oldale_Poke_Mart
	message gText_HowMayIServeYou
	waitmessage
	pokemart NoBadge
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_RustboroSpecial::
	message gText_HowMayIServeYou
	waitmessage
	pokemart RustboroSpecial
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge8:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge8
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge7:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge7
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge5:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge5
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge3:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge3
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge2:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge2
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

General_Poke_Mart_Badge1:
	message gText_HowMayIServeYou
	waitmessage
	pokemart Badge1
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Oldale_Poke_Mart::
	message gText_HowMayIServeYou
	waitmessage
	pokemart Oldale
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Badge8:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_REVIVE
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_HARBOR_MAIL
	.2byte ITEM_NONE
	release
	end

Badge7:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_HARBOR_MAIL
	.2byte ITEM_NONE
	release
	end

Badge5:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_HARBOR_MAIL
	.2byte ITEM_NONE
	release
	end

Badge3:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_X_ACCURACY
	.2byte ITEM_GUARD_SPEC
	.2byte ITEM_DIRE_HIT
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_HARBOR_MAIL
	.2byte ITEM_NONE
	release
	end

Badge2:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_HARBOR_MAIL
	.2byte ITEM_NONE
	release
	end

Badge1:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_NONE
	release
	end

NoBadge:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_NONE
	release
	end

Oldale:
	.2byte ITEM_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_NONE
	release
	end

RustboroSpecial:
	.2byte ITEM_REPEAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_NEST_BALL
	.2byte ITEM_NET_BALL
	.2byte ITEM_DIVE_BALL
	.2byte ITEM_LUXURY_BALL
	.2byte ITEM_NONE
	release
	end
