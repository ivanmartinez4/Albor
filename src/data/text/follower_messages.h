extern const u8 EventScript_FollowerIsShivering[];
extern const u8 EventScript_FollowerNostalgia[];
extern const u8 EventScript_FollowerHopping[];
extern const u8 EventScript_FollowerJumpOnPlayer[];
extern const u8 EventScript_FollowerCuddling[];
extern const u8 EventScript_FollowerShiverCuddling[];
extern const u8 EventScript_FollowerGetCloser[];
extern const u8 EventScript_FollowerPokingPlayer[];
extern const u8 EventScript_FollowerLookAround[];
extern const u8 EventScript_FollowerLookAway[];
extern const u8 EventScript_FollowerLookAwayBark[];
extern const u8 EventScript_FollowerLookAwayPoke[];
extern const u8 EventScript_FollowerPokeGround[];
extern const u8 EventScript_FollowerStartled[];
extern const u8 EventScript_FollowerFastHopping[];
extern const u8 EventScript_FollowerDizzy[];
extern const u8 EventScript_FollowerLookAroundScared[];
extern const u8 EventScript_FollowerDance[];
extern const u8 EventScript_FollowerLookUp[];

// 'Generic', unconditional happy messages
static const u8 sHappyMsg00[] = _("{STR_VAR_1} está haciéndote\ncosquillas.");
static const u8 sHappyMsg01[] = _("{STR_VAR_1} es feliz pero tímido.");
static const u8 sHappyMsg02[] = _("{STR_VAR_1} va dando saltitos.");
static const u8 sHappyMsg03[] = _("A {STR_VAR_1} se le ve sereno.");
static const u8 sHappyMsg04[] = _("A{STR_VAR_1} le encanta\npasear contigo.");
static const u8 sHappyMsg05[] = _("{STR_VAR_1} está radiante.");
static const u8 sHappyMsg06[] = _("{STR_VAR_1} parece muy feliz.");
static const u8 sHappyMsg07[] = _("{STR_VAR_1} está haciendo un esfuerzo.");
static const u8 sHappyMsg08[] = _("{STR_VAR_1} está oliendo\nlos aromas del aire..");
static const u8 sHappyMsg09[] = _("{STR_VAR_1} da saltos de alegría.");
static const u8 sHappyMsg10[] = _("¡{STR_VAR_1} se siente genial!");
static const u8 sHappyMsg11[] = _("Tu {STR_VAR_1} ha percibido humo.");
static const u8 sHappyMsg12[] = _("{STR_VAR_1} está jugueteando.");
static const u8 sHappyMsg13[] = _("Se está desperezando {STR_VAR_1}.");
static const u8 sHappyMsg14[] = _("{STR_VAR_1} parece que quiere liderar.");
static const u8 sHappyMsg15[] = _("{STR_VAR_1} hace lo que pueda para seguirte.");
static const u8 sHappyMsg16[] = _("{STR_VAR_1} quiere abrazarte.");
static const u8 sHappyMsg17[] = _("{STR_VAR_1} está lleno de vida.");
static const u8 sHappyMsg18[] = _("{STR_VAR_1} no deja de mirarte.");
static const u8 sHappyMsg19[] = _("¡{STR_VAR_1} está tan feliz que no\npuede ni tenerse en pie!");
static const u8 sHappyMsg20[] = _("{STR_VAR_1} asiente lentamente.");
static const u8 sHappyMsg21[] = _("¡{STR_VAR_1} está muy ansioso!");
static const u8 sHappyMsg22[] = _("{STR_VAR_1} se está concentrando\npara oír algo.");
static const u8 sHappyMsg23[] = _("{STR_VAR_1} parece muy interesado.");
static const u8 sHappyMsg24[] = _("{STR_VAR_1} se está esforzando para seguirte.");
static const u8 sHappyMsg25[] = _("{STR_VAR_1} te mira radiante.");
static const u8 sHappyMsg26[] = _("{STR_VAR_1} no deja de sonreír.");
static const u8 sHappyMsg27[] = _("A tu {STR_VAR_1} le encanta el\nolor de las flores.");
static const u8 sHappyMsg28[] = _("¡{STR_VAR_1} es feliz a tu lado!");
static const u8 sHappyMsg29[] = _("{STR_VAR_1} te echó una mirada feliz.");
static const u8 sHappyMsg30[] = _("{STR_VAR_1} te está dando caricias.");
  // Conditional messages begin here, index 31
static const u8 sHappyMsg31[] = _("A tu {STR_VAR_1} le encanta el tiempo.");
static const u8 sHappyMsg32[] = _("A {STR_VAR_1} se le ve\nmuy seguro de sí mismo.");

const struct FollowerMsgInfo gFollowerHappyMessages[] = {
    {sHappyMsg00, EventScript_FollowerPokingPlayer},
    {sHappyMsg01}, {sHappyMsg02}, {sHappyMsg03}, {sHappyMsg04}, {sHappyMsg05}, {sHappyMsg06}, {sHappyMsg07},
    {sHappyMsg08, EventScript_FollowerLookAround},
    {sHappyMsg09, EventScript_FollowerHopping},
    {sHappyMsg10}, {sHappyMsg11},
    {sHappyMsg12, EventScript_FollowerPokingPlayer},
    {sHappyMsg13, EventScript_FollowerLookAround},
    {sHappyMsg14}, {sHappyMsg15},
    {sHappyMsg16, EventScript_FollowerCuddling},
    {sHappyMsg17}, {sHappyMsg18},
    {sHappyMsg19, EventScript_FollowerFastHopping},
    {sHappyMsg20}, {sHappyMsg21}, {sHappyMsg22}, {sHappyMsg23}, {sHappyMsg24}, {sHappyMsg25}, {sHappyMsg26}, {sHappyMsg27}, {sHappyMsg28}, {sHappyMsg29},
    {sHappyMsg30, EventScript_FollowerCuddling},
    {sHappyMsg31}, {sHappyMsg32},
};

// Unconditional neutral messages
static const u8 sNeutralMsg00[] = _("{STR_VAR_1} está rebuscando\nen el suelo.");
static const u8 sNeutralMsg01[] = _("{STR_VAR_1} está en guardia.");
static const u8 sNeutralMsg02[] = _("{STR_VAR_1} está ensimismado.");
static const u8 sNeutralMsg03[] = _("{STR_VAR_1} está despistado.");
static const u8 sNeutralMsg04[] = _("Tu {STR_VAR_1} ha bostezado sonoramente.");
static const u8 sNeutralMsg05[] = _("Tu {STR_VAR_1} no deja de\nmirar a todos lados.");
static const u8 sNeutralMsg06[] = _("{STR_VAR_1} parece disfrutar del camino.");
static const u8 sNeutralMsg07[] = _("¡{STR_VAR_1} está lleno de energía!.");
static const u8 sNeutralMsg08[] = _("¡{STR_VAR_1} soltó un grito de batalla!");
static const u8 sNeutralMsg09[] = _("{STR_VAR_1} está bailoteando.");
static const u8 sNeutralMsg10[] = _("{STR_VAR_1} parece ansioso.");
static const u8 sNeutralMsg11[] = _("{STR_VAR_1} mira a lo lejos..");
static const u8 sNeutralMsg12[] = _("¡{STR_VAR_1} está muy atento!");
static const u8 sNeutralMsg13[] = _("{STR_VAR_1} está levantando la vista.");

const struct FollowerMsgInfo gFollowerNeutralMessages[] = {
    {sNeutralMsg00, EventScript_FollowerPokeGround},
    {sNeutralMsg01},
    {sNeutralMsg02, EventScript_FollowerLookAway},
    {sNeutralMsg03, EventScript_FollowerLookAround},
    {sNeutralMsg04},
    {sNeutralMsg05, EventScript_FollowerLookAround},
    {sNeutralMsg06}, {sNeutralMsg07}, {sNeutralMsg08},
    {sNeutralMsg09, EventScript_FollowerDance},
    {sNeutralMsg10},
    {sNeutralMsg11, EventScript_FollowerLookAway},
    {sNeutralMsg12},
    {sNeutralMsg13, EventScript_FollowerLookAwayBark},
};

// Unconditional sad messages
static const u8 sSadMsg00[] = _("{STR_VAR_1} está falto de fuerzas..");
static const u8 sSadMsg01[] = _("¡{STR_VAR_1} te está\ndando pisotones!");
static const u8 sSadMsg02[] = _("{STR_VAR_1} parece un poco cansado.");
  // Conditional messages begin, index 3
static const u8 sSadMsg03[] = _("{STR_VAR_1} está descontento.");
static const u8 sSadMsg04[] = _("{STR_VAR_1} está que se cae al suelo.");
static const u8 sSadMsg05[] = _("{STR_VAR_1} parece a punto\nde caer rendido.");
static const u8 sSadMsg06[] = _("A {STR_VAR_1} le está costando\nseguirte el ritmo…");
static const u8 sSadMsg07[] = _("{STR_VAR_1} está nervioso.");

const struct FollowerMsgInfo gFollowerSadMessages[] = {
    {sSadMsg00, EventScript_FollowerDizzy},
    {sSadMsg01}, {sSadMsg02},
    {sSadMsg03}, {sSadMsg04}, {sSadMsg05}, {sSadMsg06}, {sSadMsg07},
};

// Unconditional upset messages
static const u8 sUpsetMsg00[] = _("{STR_VAR_1} parece infeliz…");
static const u8 sUpsetMsg01[] = _("{STR_VAR_1} está haciendo carantoñas.");
static const u8 sUpsetMsg02[] = _("Tu {STR_VAR_1} parece un poco frío contigo.");
  // Conditional messages, index 3
static const u8 sUpsetMsg03[] = _("{STR_VAR_1} se refugia de la\nlluvia como puede.");

const struct FollowerMsgInfo gFollowerUpsetMessages[] = {
    {sUpsetMsg00}, {sUpsetMsg01},
    {sUpsetMsg02, EventScript_FollowerIsShivering},
    {sUpsetMsg03},
};

// Unconditional angry messages
static const u8 sAngryMsg00[] = _("¡{STR_VAR_1} está gruñendo!");
static const u8 sAngryMsg01[] = _("¡{STR_VAR_1} está haciendo pucheros!");
static const u8 sAngryMsg02[] = _("{STR_VAR_1} está triste por\alguna razón.");
static const u8 sAngryMsg03[] = _("Tu {STR_VAR_1} te gira la cara.");
static const u8 sAngryMsg04[] = _("{STR_VAR_1} está llorando.");

const struct FollowerMsgInfo gFollowerAngryMessages[] = {
    {sAngryMsg00}, {sAngryMsg01}, {sAngryMsg02},
    {sAngryMsg03, EventScript_FollowerLookAway},
    {sAngryMsg04},
};

// Unconditional pensive messages
static const u8 sPensiveMsg00[] = _("{STR_VAR_1} está mirando al suelo.");
static const u8 sPensiveMsg01[] = _("{STR_VAR_1} inspecciona el terreno.");
static const u8 sPensiveMsg02[] = _("{STR_VAR_1} busca algo.");
static const u8 sPensiveMsg03[] = _("{STR_VAR_1} parece dormirse…");
static const u8 sPensiveMsg04[] = _("{STR_VAR_1} está dando vueltas.");
static const u8 sPensiveMsg05[] = _("{STR_VAR_1} mira para todos lados.");
static const u8 sPensiveMsg06[] = _("¡{STR_VAR_1} bosteza ruidosamente!");
static const u8 sPensiveMsg07[] = _("{STR_VAR_1} se está relajando.");
static const u8 sPensiveMsg08[] = _("{STR_VAR_1} no va a ningún sitio\nsin mirarte.");
static const u8 sPensiveMsg09[] = _("{STR_VAR_1} está pensativo.");
static const u8 sPensiveMsg10[] = _("{STR_VAR_1} está centrado en ti.");
static const u8 sPensiveMsg11[] = _("{STR_VAR_1} mira fijamente a lo lejos.");
static const u8 sPensiveMsg12[] = _("{STR_VAR_1} rebusca por el suelo.");
static const u8 sPensiveMsg13[] = _("Tu {STR_VAR_1} quiere volver a casa.");
static const u8 sPensiveMsg14[] = _("¡{STR_VAR_1} tiene ganas de luchar!");
static const u8 sPensiveMsg15[] = _("{STR_VAR_1} está concentrado.");
static const u8 sPensiveMsg16[] = _("{STR_VAR_1} sigue el camino.");
static const u8 sPensiveMsg17[] = _("{STR_VAR_1} parece inseguro…");
static const u8 sPensiveMsg18[] = _("{STR_VAR_1} mira tus huellas.");
static const u8 sPensiveMsg19[] = _("{STR_VAR_1} is staring straight into\nyour eyes.");

const struct FollowerMsgInfo gFollowerPensiveMessages[] = {
    {sPensiveMsg00},
    {sPensiveMsg01, EventScript_FollowerLookAround},
    {sPensiveMsg02}, {sPensiveMsg03}, {sPensiveMsg04},
    {sPensiveMsg05, EventScript_FollowerLookAround},
    {sPensiveMsg06}, {sPensiveMsg07}, {sPensiveMsg08}, {sPensiveMsg09}, {sPensiveMsg10},
    {sPensiveMsg11, EventScript_FollowerLookAway},
    {sPensiveMsg12, EventScript_FollowerPokeGround},
    {sPensiveMsg13, EventScript_FollowerLookAway},
    {sPensiveMsg14}, {sPensiveMsg15}, {sPensiveMsg16}, {sPensiveMsg17}, {sPensiveMsg18}, {sPensiveMsg19},
};

// All 'love' messages are unconditional
static const u8 sLoveMsg00[] = _("¡{STR_VAR_1} quiere ir más cerca de ti!");
static const u8 sLoveMsg01[] = _("{STR_VAR_1} se está ruborizando.");
static const u8 sLoveMsg02[] = _("¡{STR_VAR_1} te ha abrazado de repente!");
static const u8 sLoveMsg03[] = _("¡{STR_VAR_1} tiene ganas de jugar!");
static const u8 sLoveMsg04[] = _("{STR_VAR_1} se frota contra ti.");
static const u8 sLoveMsg05[] = _("{STR_VAR_1} ronronea.");
static const u8 sLoveMsg06[] = _("¡{STR_VAR_1} te está lamiendo!");
static const u8 sLoveMsg07[] = _("¡{STR_VAR_1} te adora!");
static const u8 sLoveMsg08[] = _("A {STR_VAR_1} le encanta acercarse a ti.");
static const u8 sLoveMsg09[] = _("{STR_VAR_1} va siempre cerca tuya.");

const struct FollowerMsgInfo gFollowerLoveMessages[] = {
    {sLoveMsg00, EventScript_FollowerGetCloser},
    {sLoveMsg01},
    {sLoveMsg02, EventScript_FollowerCuddling},
    {sLoveMsg03},
    {sLoveMsg04, EventScript_FollowerCuddling},
    {sLoveMsg05},
    {sLoveMsg06, EventScript_FollowerCuddling},
    {sLoveMsg07},
    {sLoveMsg08, EventScript_FollowerGetCloser},
    {sLoveMsg09},
};

// Unconditional surprised messages
static const u8 sSurpriseMsg00[] = _("¡{STR_VAR_1} parece a punto de caer rendido!");
static const u8 sSurpriseMsg01[] = _("¡{STR_VAR_1} dió un bote encima tuya!");
static const u8 sSurpriseMsg02[] = _("{STR_VAR_1} no se ha acostumbrado\na su nombre aún.");
static const u8 sSurpriseMsg03[] = _("{STR_VAR_1} está olisqueando.");
static const u8 sSurpriseMsg04[] = _("Tu {STR_VAR_1} percibe algo cerca.");
static const u8 sSurpriseMsg05[] = _("¡{STR_VAR_1} ha sentido algo y está aullando!");
static const u8 sSurpriseMsg06[] = _("¡{STR_VAR_1} ha recuperado las fuerzas!");
static const u8 sSurpriseMsg07[] = _("¡{STR_VAR_1} está gruñendo!");
static const u8 sSurpriseMsg08[] = _("¡{STR_VAR_1} se dió la vuelta de repente!");
static const u8 sSurpriseMsg09[] = _("¡Vaya susto se ha llevado {STR_VAR_1}!");
static const u8 sSurpriseMsg10[] = _("¡Algo parece que huele bien!\n¿Eh, {STR_VAR_1}?");
static const u8 sSurpriseMsg11[] = _("{STR_VAR_1} parece pensativo.");
static const u8 sSurpriseMsg12[] = _("Ten cuidado, {STR_VAR_1}.");
static const u8 sSurpriseMsg13[] = _("{STR_VAR_1} lleva cuidado\nde no caerse.");
static const u8 sSurpriseMsg14[] = _("{STR_VAR_1} avanza cauteloso…");
static const u8 sSurpriseMsg15[] = _("{STR_VAR_1} se prepara para luchar.");
static const u8 sSurpriseMsg16[] = _("¡{STR_VAR_1} siente algo extraño!");
static const u8 sSurpriseMsg17[] = _("¡{STR_VAR_1} está asustado!");
static const u8 sSurpriseMsg18[] = _("{STR_VAR_1} siente una presencia inusual…");
static const u8 sSurpriseMsg19[] = _("{STR_VAR_1} está un poco tenso.");
  // Conditional messages, index 20
static const u8 sSurpriseMsg20[] = _("La lluvia parece sorprender a {STR_VAR_1}.");

const struct FollowerMsgInfo gFollowerSurpriseMessages[] = {
    {sSurpriseMsg00},
    {sSurpriseMsg01, EventScript_FollowerPokingPlayer},
    {sSurpriseMsg02}, {sSurpriseMsg03}, {sSurpriseMsg04}, {sSurpriseMsg05}, {sSurpriseMsg06},
    {sSurpriseMsg07, EventScript_FollowerLookAwayBark},
    {sSurpriseMsg08, EventScript_FollowerLookAway},
    {sSurpriseMsg09},
    {sSurpriseMsg10, EventScript_FollowerLookAround},
    {sSurpriseMsg11}, {sSurpriseMsg12}, {sSurpriseMsg13}, {sSurpriseMsg14}, {sSurpriseMsg15}, {sSurpriseMsg16},
    {sSurpriseMsg17, EventScript_FollowerCuddling},
    {sSurpriseMsg18},
    {sSurpriseMsg19, EventScript_FollowerLookAround},
    {sSurpriseMsg20},
};

// Unconditional curious messages
static const u8 sCuriousMsg00[] = _("Tu {STR_VAR_1} no para de buscar algo.");
static const u8 sCuriousMsg01[] = _("¡Tu {STR_VAR_1} ha corrido hacia ti!");
static const u8 sCuriousMsg02[] = _("¿Hay algo cerca, {STR_VAR_1}?");
static const u8 sCuriousMsg03[] = _("{STR_VAR_1} juega con las piedrecitas.");
static const u8 sCuriousMsg04[] = _("{STR_VAR_1} está buscando algo.");
static const u8 sCuriousMsg05[] = _("{STR_VAR_1} está lloriqueando.");
static const u8 sCuriousMsg06[] = _("{STR_VAR_1} parece dubitativo…");

const struct FollowerMsgInfo gFollowerCuriousMessages[] = {
    {sCuriousMsg00, EventScript_FollowerLookAround},
    {sCuriousMsg01, EventScript_FollowerPokingPlayer},
    {sCuriousMsg02}, {sCuriousMsg03},
    {sCuriousMsg04, EventScript_FollowerLookAround},
    {sCuriousMsg05}, {sCuriousMsg06},
};

// Unconditional music messages
static const u8 sMusicMsg00[] = _("¡{STR_VAR_1} está muy ágil!");
static const u8 sMusicMsg01[] = _("¡{STR_VAR_1} no para!");
static const u8 sMusicMsg02[] = _("¡Tu {STR_VAR_1} empezó a\nbailar de felicidad!");
static const u8 sMusicMsg03[] = _("¡Qué bien le sienta\na {STR_VAR_1} pasear contigo!");
static const u8 sMusicMsg04[] = _("{STR_VAR_1} quiere jugar contigo..");
static const u8 sMusicMsg05[] = _("{STR_VAR_1} parece disfrutar del paseo.");
static const u8 sMusicMsg06[] = _("{STR_VAR_1} bailotea.");
static const u8 sMusicMsg07[] = _("{STR_VAR_1} te mordisquea los pies.");
static const u8 sMusicMsg08[] = _("{STR_VAR_1} te está mirando.");
static const u8 sMusicMsg09[] = _("{STR_VAR_1} está esforzándose\npara impresionarte.");
static const u8 sMusicMsg10[] = _("¡Vaya {STR_VAR_1} más bailón tienes!");
static const u8 sMusicMsg11[] = _("{STR_VAR_1} es muy jovial.");
static const u8 sMusicMsg12[] = _("{STR_VAR_1} brincotea.");
static const u8 sMusicMsg13[] = _("Tu {STR_VAR_1} ha reconocido un olor familiar…");
// Conditional music messages, index 14
static const u8 sMusicMsg14[] = _("A {STR_VAR_1} le encanta la lluvia.");

const struct FollowerMsgInfo gFollowerMusicMessages[] = {
    {sMusicMsg00, EventScript_FollowerLookAround},
    {sMusicMsg01},
    {sMusicMsg02, EventScript_FollowerDance},
    {sMusicMsg03},
    {sMusicMsg04, EventScript_FollowerHopping},
    {sMusicMsg05, EventScript_FollowerHopping},
    {sMusicMsg06}, {sMusicMsg07}, {sMusicMsg08}, {sMusicMsg09},
    {sMusicMsg10, EventScript_FollowerDance},
    {sMusicMsg11},
    {sMusicMsg12, EventScript_FollowerHopping},
    {sMusicMsg13, EventScript_FollowerNostalgia},
    {sMusicMsg14}
};


static const u8 sPoisonedMsg00[] = _("{STR_VAR_1} está sufriendo por el veneno.");

const struct FollowerMsgInfo gFollowerPoisonedMessages[] = {
    {sPoisonedMsg00, EventScript_FollowerIsShivering},
};
