#include "config.h"

const char* noteNamesSharp[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

const char* noteNamesFlat[12] = {
    u8"C",  u8"D♭", u8"D",  u8"E♭", u8"E",  u8"F",
    u8"G♭", u8"G",  u8"A♭", u8"A",  u8"B♭", u8"B"
};

double tuningReference = 440.0;