#ifndef _CONFIG_H
#define _CONFIG_H

constexpr int TUNER_TYPE_GROUP = 1;
constexpr float RMS_THRESHOLD = 0.002f;
constexpr float GUITAR_FREQ_MIN = 60.0f;
constexpr float GUITAR_FREQ_MAX = 1400.0f;
constexpr float BASS_FREQ_MIN = 25.0f;
constexpr float BASS_FREQ_MAX = 400.0f;
constexpr float CHROM_FREQ_MIN = 20.0f;
constexpr float CHROM_FREQ_MAX = 4200.0f;
constexpr int SLIGHT_OFF_CENTS = 25;

extern const char* noteNamesSharp[12];
extern const char* noteNamesFlat[12];
extern double tuningReference;

constexpr char* SHARP_SYMBOL = u8"♯";
constexpr char* FLAT_SYMBOL = u8"♭";

struct LanguageStrings
{
    const char* frequency;
    const char* deviation;
    const char* guitar;
    const char* bass;
    const char* chromatic;
    const char* cents;
    const char* volume;
};

constexpr LanguageStrings LANG_EN = {
    "Frequency", "Deviation", "Guitar", "Bass",
	"Chromatic", "cents", "Volume"
};

constexpr LanguageStrings LANG_DE = {
    u8"Frequenz", u8"Abweichung", u8"Gitarre", u8"Bass",
	u8"Chromatisch", u8"cents", u8"Lautstärke"
};

constexpr LanguageStrings LANG_HU = {
    u8"Frekvencia", u8"Eltérés", u8"Gitár", u8"Basszus",
	u8"Kromatikus", u8"cent", u8"Hangerő"
};

constexpr LanguageStrings LANGUAGES[3] = { LANG_EN, LANG_DE, LANG_HU };

#endif