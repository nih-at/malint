/* MPEG tables */

#include "mpeg.h"

int _mp3_jsb_tab[3][4] = {
    { 4, 8, 12, 16 },
    { 4, 8, 12, 16},
    { 0, 4,  8, 16}
};

/* number of samples per frame */
int _mp3_nsamp_tab[3] = { 384, 1152, 1152 };
