#ifndef GUARD_CONSTANTS_OPPONENTS_H
#define GUARD_CONSTANTS_OPPONENTS_H

#define TRAINER_NONE                          0

// NOTE: Because each Trainer uses a flag to determine when they are defeated, there is only space for 9 additional trainers before trainer flag space overflows
//       More space can be made by shifting flags around in constants/flags.h or changing how trainer flags are handled
//       MAX_TRAINERS_COUNT can be increased but will take up additional saveblock space

#define TRAINERS_COUNT                        1
#define MAX_TRAINERS_COUNT                  864

#endif  // GUARD_CONSTANTS_OPPONENTS_H
