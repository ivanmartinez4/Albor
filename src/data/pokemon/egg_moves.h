#define EGG_MOVES_SPECIES_OFFSET 20000
#define EGG_MOVES_TERMINATOR 0xFFFF
#define egg_moves(species, moves...) (SPECIES_##species + EGG_MOVES_SPECIES_OFFSET), moves

const u16 gEggMoves[] = {
    egg_moves(BULBASAUR,
        MOVE_AROMATIC_MIST),

    egg_moves(CHARMANDER,
        MOVE_),

    egg_moves(SQUIRTLE,
        MOVE_),

    egg_moves(CATERPIE,
        MOVE_AROMATIC_MIST),

    egg_moves(PIDGEY,
        MOVE_),

    egg_moves(RATTATA,
        MOVE_),

    egg_moves(SPEAROW,
        MOVE_),

    egg_moves(EKANS,
        MOVE_),

    egg_moves(SANDSHREW,
        MOVE_),

    egg_moves(NIDORAN_F,
        MOVE_),

    egg_moves(NIDORAN_M,
        MOVE_),

    egg_moves(VULPIX,
        MOVE_),

    egg_moves(ZUBAT,
        MOVE_),

    egg_moves(ODDISH,
        MOVE_AROMATIC_MIST),

    egg_moves(PARAS,
        MOVE_),

    egg_moves(VENONAT,
        MOVE_),

    egg_moves(DIGLETT,
        MOVE_),

    egg_moves(MEOWTH,
        MOVE_),

    egg_moves(PSYDUCK,
        MOVE_),

    egg_moves(MANKEY,
        MOVE_),

    egg_moves(GROWLITHE,
        MOVE_),

    egg_moves(POLIWAG,
        MOVE_),

    egg_moves(ABRA,
        MOVE_),

    egg_moves(MACHOP,
        MOVE_),

    egg_moves(BELLSPROUT,
        MOVE_),

    egg_moves(TENTACOOL,
        MOVE_),

    egg_moves(GEODUDE,
        MOVE_),

    egg_moves(PONYTA,
        MOVE_),

    egg_moves(SLOWPOKE,
        MOVE_),

    egg_moves(FARFETCHD,
        MOVE_),

    egg_moves(DODUO,
        MOVE_),

    egg_moves(SEEL,
        MOVE_),

    egg_moves(GRIMER,
        MOVE_),

    egg_moves(SHELLDER,
        MOVE_),

    egg_moves(GASTLY,
        MOVE_),

    egg_moves(ONIX,
        MOVE_),

    egg_moves(DROWZEE,
        MOVE_),

    egg_moves(KRABBY,
        MOVE_),

    egg_moves(EXEGGCUTE,
        MOVE_),

    egg_moves(CUBONE,
        MOVE_),

    egg_moves(LICKITUNG,
        MOVE_),

    egg_moves(KOFFING,
        MOVE_),

    egg_moves(RHYHORN,
        MOVE_),

    egg_moves(TANGELA,
        MOVE_),

    egg_moves(KANGASKHAN,
        MOVE_),

    egg_moves(HORSEA,
        MOVE_),

    egg_moves(GOLDEEN,
        MOVE_),

    egg_moves(SCYTHER,
        MOVE_),

    egg_moves(PINSIR,
        MOVE_),

    egg_moves(LAPRAS,
        MOVE_),

    egg_moves(EEVEE,
        MOVE_),

    egg_moves(OMANYTE,
        MOVE_),

    egg_moves(KABUTO,
        MOVE_),

    egg_moves(AERODACTYL,
        MOVE_),

    egg_moves(DRATINI,
        MOVE_),

    egg_moves(CHIKORITA,
        MOVE_AROMATIC_MIST),

    egg_moves(CYNDAQUIL,
        MOVE_),

    egg_moves(TOTODILE,
        MOVE_),

    egg_moves(SENTRET
        MOVE_),

    egg_moves(HOOTHOOT,
        MOVE_),

    egg_moves(LEDYBA,
        MOVE_),

    egg_moves(SPINARAK,
        MOVE_),

    egg_moves(CHINCHOU,
        MOVE_),

    egg_moves(PICHU,
        MOVE_),

    egg_moves(CLEFFA,
        MOVE_),

    egg_moves(IGGLYBUFF,
        MOVE_),

    egg_moves(TOGEPI,
        MOVE_),

    egg_moves(NATU,
        MOVE_),

    egg_moves(MAREEP,
        MOVE_),

    egg_moves(HOPPIP,
        MOVE_FAIRY_WIND),

    egg_moves(AIPOM,
        MOVE_),

    egg_moves(SUNKERN,
        MOVE_),

    egg_moves(YANMA,
        MOVE_),

    egg_moves(WOOPER,
        MOVE_),

    egg_moves(MURKROW,
        MOVE_),

    egg_moves(MISDREAVUS,
        MOVE_AROMATIC_MIST),

    egg_moves(GIRAFARIG,
        MOVE_),

    egg_moves(PINECO,
        MOVE_),

    egg_moves(DUNSPARCE,
        MOVE_),

    egg_moves(GLIGAR,
        MOVE_),

    egg_moves(SNUBBULL,
        MOVE_),

    egg_moves(QWILFISH,
        MOVE_),

    egg_moves(SHUCKLE,
        MOVE_),

    egg_moves(HERACROSS,
        MOVE_),

    egg_moves(SNEASEL,
        MOVE_),

    egg_moves(TEDDIURSA,
        MOVE_),

    egg_moves(SLUGMA,
        MOVE_),

    egg_moves(SWINUB,
        MOVE_),

    egg_moves(CORSOLA,
        MOVE_),

    egg_moves(REMORAID,
        MOVE_),

    egg_moves(DELIBIRD,
        MOVE_),

    egg_moves(SKARMORY,
        MOVE_),

    egg_moves(HOUNDOUR,
        MOVE_),

    egg_moves(PHANPY,
        MOVE_),

    egg_moves(STANTLER,
        MOVE_),

    egg_moves(TYROGUE,
        MOVE_),

    egg_moves(SMOOCHUM,
        MOVE_AROMATIC_MIST),

    egg_moves(ELEKID,
        MOVE_),

    egg_moves(MAGBY,
        MOVE_),

    egg_moves(MILTANK,
        MOVE_),

    egg_moves(LARVITAR,
        MOVE_),

    egg_moves(TREECKO,
        MOVE_),

    egg_moves(TORCHIC,
        MOVE_),

    egg_moves(MUDKIP,
        MOVE_),

    egg_moves(POOCHYENA,
        MOVE_),

    egg_moves(ZIGZAGOON,
        MOVE_),

    egg_moves(WURMPLE,
        MOVE_AROMATIC_MIST),

    egg_moves(LOTAD,
        MOVE_),

    egg_moves(SEEDOT,
        MOVE_),

    egg_moves(TAILLOW,
        MOVE_),

    egg_moves(WINGULL,
        MOVE_),

    egg_moves(RALTS,
        MOVE_),

    egg_moves(SURSKIT,
        MOVE_),

    egg_moves(SHROOMISH,
        MOVE_),

    egg_moves(SLAKOTH,
        MOVE_),

    egg_moves(NINCADA,
        MOVE_),

    egg_moves(WHISMUR,
        MOVE_),

    egg_moves(MAKUHITA,
        MOVE_),

    egg_moves(AZURILL,
        MOVE_),

    egg_moves(NOSEPASS,
        MOVE_),

    egg_moves(SKITTY,
        MOVE_),

    egg_moves(SABLEYE,
        MOVE_BABY_DOLL_EYES),

    egg_moves(MAWILE,
        MOVE_),

    egg_moves(ARON,
        MOVE_),

    egg_moves(MEDITITE,
        MOVE_),

    egg_moves(ELECTRIKE,
        MOVE_),

    egg_moves(PLUSLE,
        MOVE_),

    egg_moves(MINUN,
        MOVE_),

    egg_moves(VOLBEAT,
        MOVE_),

    egg_moves(ILLUMISE,
        MOVE_),

    egg_moves(GULPIN,
        MOVE_),

    egg_moves(CARVANHA,
        MOVE_),

    egg_moves(WAILMER,
        MOVE_),

    egg_moves(NUMEL,
        MOVE_),

    egg_moves(TORKOAL,
        MOVE_),

    egg_moves(SPOINK,
        MOVE_CRAFTY_SHIELD),

    egg_moves(SPINDA,
        MOVE_),

    egg_moves(TRAPINCH,
        MOVE_),

    egg_moves(CACNEA,
        MOVE_),

    egg_moves(SWABLU,
        MOVE_),

    egg_moves(ZANGOOSE,
        MOVE_),

    egg_moves(SEVIPER,
        MOVE_),

    egg_moves(BARBOACH,
        MOVE_),

    egg_moves(CORPHISH,
        MOVE_),

    egg_moves(LILEEP,
        MOVE_),

    egg_moves(ANORITH,
        MOVE_),

    egg_moves(FEEBAS,
        MOVE_),

    egg_moves(CASTFORM,
        MOVE_FAIRY_WIND,
        MOVE_BABY_DOLL_EYES),

    egg_moves(KECLEON,
        MOVE_),

    egg_moves(SHUPPET,
        MOVE_),

    egg_moves(DUSKULL,
        MOVE_),

    egg_moves(TROPIUS,
        MOVE_),

    egg_moves(ABSOL,
        MOVE_),

    egg_moves(SNORUNT,
        MOVE_),

    egg_moves(SPHEAL,
        MOVE_),

    egg_moves(CLAMPERL,
        MOVE_),

    egg_moves(RELICANTH,
        MOVE_),

    egg_moves(LUVDISC,
        MOVE_),

    egg_moves(BAGON,
        MOVE_),

#if P_GEN_4_POKEMON == TRUE
    egg_moves(TURTWIG,
        MOVE_),

    egg_moves(CHIMCHAR,
        MOVE_),

    egg_moves(PIPLUP,
        MOVE_),

    egg_moves(STARLY,
        MOVE_),

    egg_moves(BIDOOF,
        MOVE_),

    egg_moves(SHINX,
        MOVE_BABY_DOLL_EYES),

    egg_moves(BUDEW,
        MOVE_),

    egg_moves(CRANIDOS,
        MOVE_),

    egg_moves(SHIELDON,
        MOVE_),

    egg_moves(PACHIRISU,
        MOVE_),

    egg_moves(BUIZEL,
        MOVE_),

    egg_moves(CHERUBI,
        MOVE_),

    egg_moves(SHELLOS,
        MOVE_),

    egg_moves(DRIFLOON,
        MOVE_AROMATIC_MIST),

    egg_moves(BUNEARY,
        MOVE_),

    egg_moves(GLAMEOW,
        MOVE_),

    egg_moves(CHINGLING,
        MOVE_),

    egg_moves(STUNKY,
        MOVE_),

    egg_moves(BONSLY,
        MOVE_FAIRY_WIND),

    egg_moves(MIME_JR,
        MOVE_),

    egg_moves(HAPPINY,
        MOVE_),

    egg_moves(CHATOT,
        MOVE_),

    egg_moves(SPIRITOMB,
        MOVE_),

    egg_moves(GIBLE,
        MOVE_),

    egg_moves(MUNCHLAX,
        MOVE_),

    egg_moves(RIOLU,
        MOVE_),

    egg_moves(HIPPOPOTAS,
        MOVE_),

    egg_moves(SKORUPI,
        MOVE_),

    egg_moves(CROAGUNK,
        MOVE_),

    egg_moves(CARNIVINE,
        MOVE_),

    egg_moves(FINNEON,
        MOVE_),

    egg_moves(MANTYKE,
        MOVE_),

    egg_moves(SNOVER,
        MOVE_),
#endif

#if P_GEN_5_POKEMON == TRUE
    egg_moves(SNIVY,
        MOVE_),

    egg_moves(TEPIG,
        MOVE_),

    egg_moves(OSHAWOTT,
        MOVE_),

    egg_moves(LILLIPUP,
        MOVE_),

    egg_moves(MUNNA,
        MOVE_),

    egg_moves(BLITZLE,
        MOVE_),

    egg_moves(ROGGENROLA,
        MOVE_),

    egg_moves(WOOBAT,
        MOVE_),

    egg_moves(DRILBUR,
        MOVE_),

    egg_moves(AUDINO,
        MOVE_AROMATIC_MIST),

    egg_moves(TIMBURR,
        MOVE_),

    egg_moves(TYMPOLE,
        MOVE_),

    egg_moves(VENIPEDE,
        MOVE_),

    egg_moves(COTTONEE,
        MOVE_),

    egg_moves(SANDILE,
        MOVE_),

    egg_moves(DARUMAKA,
        MOVE_),

    egg_moves(SCRAGGY,
        MOVE_CRAFTY_SHIELD),

    egg_moves(SIGILYPH,
        MOVE_),

    egg_moves(YAMASK,
        MOVE_),

    egg_moves(TIRTOUGA,
        MOVE_),

    egg_moves(ARCHEN,
        MOVE_),

    egg_moves(ZORUA,
        MOVE_),

    egg_moves(MINCCINO,
        MOVE_),

    egg_moves(GOTHITA,
        MOVE_AROMATIC_MIST),

    egg_moves(SOLOSIS,
        MOVE_BABY_DOLL_EYES),

    egg_moves(VANILLITE,
        MOVE_),

    egg_moves(FOONGUS,
        MOVE_),

    egg_moves(FRILLISH,
        MOVE_),

    egg_moves(JOLTIK,
        MOVE_),

    egg_moves(FERROSEED,
        MOVE_),

    egg_moves(ELGYEM,
        MOVE_AROMATIC_MIST),

    egg_moves(LITWICK,
        MOVE_AROMATIC_MIST),

    egg_moves(AXEW,
        MOVE_),

    egg_moves(CUBCHOO,
        MOVE_),

    egg_moves(PAWNIARD,
        MOVE_),

    egg_moves(VULLABY,
        MOVE_),

    egg_moves(DEINO,
        MOVE_),

    egg_moves(LARVESTA,
        MOVE_),
#endif

#if P_GEN_6_POKEMON == TRUE
    egg_moves(CHESPIN,
        MOVE_),

    egg_moves(FENNEKIN,
        MOVE_),

    egg_moves(FROAKIE,
        MOVE_),

    egg_moves(BUNNELBY,
        MOVE_),

    egg_moves(FLETCHLING,
        MOVE_),

    egg_moves(LITLEO,
        MOVE_),

    egg_moves(SKIDDO,
        MOVE_),

    egg_moves(PANCHAM,
        MOVE_),

    egg_moves(HONEDGE,
        MOVE_),

    egg_moves(INKAY,
        MOVE_),

    egg_moves(HELIOPTILE,
        MOVE_),

    egg_moves(TYRUNT,
        MOVE_),

    egg_moves(AMAURA,
        MOVE_),

    egg_moves(HAWLUCHA,
        MOVE_),

    egg_moves(GOOMY,
        MOVE_),

    egg_moves(PHANTUMP,
        MOVE_),

    egg_moves(BERGMITE,
        MOVE_),

    egg_moves(NOIBAT,
        MOVE_),
#endif

#if P_GEN_7_POKEMON == TRUE
    egg_moves(ROWLET,
        MOVE_),

    egg_moves(LITTEN,
        MOVE_),

    egg_moves(POPPLIO,
        MOVE_BABY_DOLL_EYES),

    egg_moves(PIKIPEK,
        MOVE_),

    egg_moves(ROCKRUFF,
        MOVE_),

    egg_moves(MAREANIE,
        MOVE_),

    egg_moves(MUDBRAY,
        MOVE_),

    egg_moves(DEWPIDER,
        MOVE_),

    egg_moves(SALANDIT,
        MOVE_),

    egg_moves(STUFFUL,
        MOVE_),

    egg_moves(BOUNSWEET,
        MOVE_),
    
    egg_moves(ORANGURU,
        MOVE_),

    egg_moves(PASSIMIAN,
        MOVE_),

    egg_moves(SANDYGAST,
        MOVE_),

    egg_moves(MIMIKYU,
        MOVE_),

    egg_moves(DRAMPA,
        MOVE_),

    egg_moves(JANGMO_O,
        MOVE_),
#endif

#if P_GEN_8_POKEMON == TRUE
    egg_moves(GROOKEY,
        MOVE_),

    egg_moves(SCORBUNNY,
        MOVE_),

    egg_moves(SOBBLE,
        MOVE_),

    egg_moves(ROOKIDEE,
        MOVE_),

    egg_moves(BLIPBUG,
        MOVE_BABY_DOLL_EYES),

    egg_moves(WOOLOO,
        MOVE_),

    egg_moves(ROLYCOLY,
        MOVE_),

    egg_moves(SILICOBRA,
        MOVE_),

    egg_moves(TOXEL,
        MOVE_BABY_DOLL_EYES,
        MOVE_AROMATIC_MIST),

    egg_moves(HATENNA,
        MOVE_FAIRY_WIND,
        MOVE_BABY_DOLL_EYES),

    egg_moves(IMPIDIMP,
        MOVE_FAIRY_WIND,
        MOVE_BABY_DOLL_EYES),

    egg_moves(SNOM,
        MOVE_FAIRY_WIND),

    egg_moves(CUFANT,
        MOVE_),

    egg_moves(DREEPY,
        MOVE_),
#endif

    egg_moves(SANDSHREW_ALOLAN,
        MOVE_),

    egg_moves(VULPIX_ALOLAN,
        MOVE_FAIRY_WIND),

    egg_moves(ZIGZAGOON_GALARIAN,
        MOVE_),

    egg_moves(DARUMAKA_GALARIAN,
        MOVE_),

    egg_moves(COSMOG,
        MOVE_),

    EGG_MOVES_TERMINATOR
};
