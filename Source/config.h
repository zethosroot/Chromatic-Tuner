#ifndef _CONFIG_H
#define _CONFIG_H

constexpr int TUNER_TYPE_GROUP = 1;

const char* noteNamesSharp[12] = {
	"C",
	"C#",
	"D",
	"D#",
	"E",
	"F",
	"F#",
	"G",
	"G#",
	"A",
	"A#",
	"B"
};

const char* noteNamesFlat[12] = {
	u8"C",  
	u8"D♭", 
	u8"D",  
	u8"E♭", 
	u8"E",  
	u8"F",
	u8"G♭", 
	u8"G",  
	u8"A♭", 
	u8"A",  
	u8"B♭", 
	u8"B"
};

struct LanguageStrings
{
    const char* frequency;
    const char* deviation;
    const char* guitar;
    const char* bass;
    const char* chromatic;
    const char* sharp;
    const char* flat;
    const char* cents;
};

constexpr LanguageStrings LANG_EN = {
    "Frequency",
    "Deviation",
    "Guitar",
    "Bass",
    "Chromatic",
    "Sharp (#)",
    "Flat (b)",
    "cents"
};

constexpr LanguageStrings LANG_DE = {
    u8"Frequenz",
    u8"Abweichung",
    u8"Gitarre",
    u8"Bass",
    u8"Chromatisch",
    u8"Kreuz (#)",
    u8"B (b)",
    u8"cents"
};

constexpr LanguageStrings LANG_HU = {
    u8"Frekvencia",
    u8"Eltérés",
    u8"Gitár",
    u8"Basszus",
    u8"Kromatikus",
    u8"Kereszt (#)",
    u8"Bé (b)",
    u8"cent"
};

constexpr LanguageStrings LANGUAGES[3] = { LANG_EN, LANG_DE, LANG_HU };

double tuningReference = 440.0; // The international standard tuning frequency for A4

#endif