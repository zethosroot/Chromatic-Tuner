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
constexpr int SLIGHT_OFF_CENTS = 15;
constexpr int IN_TUNE_CENTS = 5;

constexpr char* PLUGIN_NAME = "ZentNote";
constexpr char* PLUGIN_VERSION = "0.1.0-dev";
constexpr char* PLUGIN_VENDOR = "ZSOMBII Audio";

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
	const char* checkForUpdates;
};

constexpr LanguageStrings LANG_EN = {
    "Frequency", "Deviation", "Guitar", "Bass",
	"Chromatic", "cents", "Volume", "Check for updates",
};

constexpr LanguageStrings LANG_DE = {
    u8"Frequenz", u8"Abweichung", u8"Gitarre", u8"Bass",
	u8"Chromatisch", u8"cents", u8"Lautstärke", u8"Nach Updates suchen"
};

constexpr LanguageStrings LANG_HU = {
    u8"Frekvencia", u8"Eltérés", u8"Gitár", u8"Basszus",
	u8"Kromatikus", u8"cent", u8"Hangerő", u8"Frissítések keresése"
};

constexpr LanguageStrings LANGUAGES[3] = { LANG_EN, LANG_DE, LANG_HU };

#endif