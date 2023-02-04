#ifndef GUARD_CONFIG_POKEMON_H
#define GUARD_CONFIG_POKEMON_H

#define P_SHEDINJA_BALL         GEN_LATEST  // Since Gen 4, Shedinja requires a Poké Ball for its evolution. In Gen 3, Shedinja inherits Nincada's Ball.
#define P_LEGENDARY_PERFECT_IVS GEN_LATEST  // Since Gen 6, Legendaries, Mythicals and Ultra Beasts found in the wild or given through gifts have at least 3 perfect IVs.
#define P_KADABRA_EVERSTONE     GEN_LATEST  // Since Gen 4, Kadabra can evolve even when holding an Everstone.
#define P_NIDORAN_M_DITTO_BREED GEN_LATEST  // Since Gen 5, when Nidoran♂ breeds with Ditto it can produce Nidoran♀ offspring. Before, it would only yield male offspring. This change also applies to Volbeat.
#define P_SHINY_BASE_CHANCE     GEN_LATEST  // Since Gen 6, the base chances of encountering a Shiny Pokémon was raised to 1/4096. This config adds an extra roll to the calculation, which effectively does the same thing.
#define P_BALL_INHERITING           GEN_LATEST  // Since Gen 6, Eggs from the Daycare will inherit the Poké Ball from their mother. From Gen7 onwards, the father can pass it down as well, as long as it's of the same species as the mother.

#endif // GUARD_CONFIG_POKEMON_H
