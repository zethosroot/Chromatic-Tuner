/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "config.h"

void TunerPluginAudioProcessorEditor::timerCallback()
{
    float currentHz = audioProcessor.m_detectedHz.load(std::memory_order_relaxed);

    if (currentHz > 0.0f)
    {
        double midi = 69.0 + 12.0 * std::log2(currentHz / tuningReference);
        int nearestNote = static_cast<int>(std::round(midi));
        float targetHz = 440.0f * std::pow(2.0f, (nearestNote - 69) / 12.0f);
        float targetCents = 1200.0f * std::log2f(currentHz / targetHz);
        m_smoothedCents = 0.8f * m_smoothedCents + 0.2f * targetCents;
    }
    else
        m_smoothedCents = 0.8f * m_smoothedCents + 0.2f * 0.0f;

    repaint();
}

//==============================================================================
TunerPluginAudioProcessorEditor::TunerPluginAudioProcessorEditor (TunerPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    startTimerHz(30);

	addAndMakeVisible(m_chromButton);
	addAndMakeVisible(m_guitarButton);
	addAndMakeVisible(m_bassButton);

    m_chromButton.setButtonText(juce::String(LANGUAGES[audioProcessor.m_language.load()].chromatic));
    m_guitarButton.setButtonText(juce::String(LANGUAGES[audioProcessor.m_language.load()].guitar));
    m_bassButton.setButtonText(juce::String(LANGUAGES[audioProcessor.m_language.load()].bass));

	m_guitarButton.setRadioGroupId(TUNER_TYPE_GROUP);
	m_bassButton.setRadioGroupId(TUNER_TYPE_GROUP);
	m_chromButton.setRadioGroupId(TUNER_TYPE_GROUP);

	m_chromButton.setToggleState(true, juce::dontSendNotification);

    m_chromButton.onClick = [this] { audioProcessor.m_tunerMode.store(0); };
    m_guitarButton.onClick = [this] { audioProcessor.m_tunerMode.store(1); };
    m_bassButton.onClick = [this] { audioProcessor.m_tunerMode.store(2); };

    m_languageBox.addItem("English", 1);
    m_languageBox.addItem("Deutsch", 2);
    m_languageBox.addItem("Magyar", 3);

	m_sharpFlatBox.addItem(juce::String::fromUTF8(SHARP_SYMBOL), 1);
	m_sharpFlatBox.addItem(juce::String::fromUTF8(FLAT_SYMBOL), 2);

    m_languageBox.setSelectedId(1, juce::dontSendNotification);
	m_sharpFlatBox.setSelectedId(1, juce::dontSendNotification);

    // Capture m_languageBox by reference in the lambda
    m_languageBox.onChange = [this] {
       
        audioProcessor.m_language.store(m_languageBox.getSelectedId() - 1);
        auto lang = LANGUAGES[audioProcessor.m_language.load()];
        
        m_chromButton.setButtonText(juce::String::fromUTF8(LANGUAGES[audioProcessor.m_language.load()].chromatic));
        m_guitarButton.setButtonText(juce::String::fromUTF8(LANGUAGES[audioProcessor.m_language.load()].guitar));
		m_bassButton.setButtonText(juce::String::fromUTF8(LANGUAGES[audioProcessor.m_language.load()].bass));
    };

    m_sharpFlatBox.onChange = [this] {
        if (m_sharpFlatBox.getSelectedId() == 1) {
            audioProcessor.m_sharpPresent.store(true);
        }
        else {
            audioProcessor.m_sharpPresent.store(false);
        }
    };

	addAndMakeVisible(m_languageBox);
	addAndMakeVisible(m_sharpFlatBox);

	// Restore saved settings
    m_languageBox.setSelectedId(audioProcessor.m_language.load() + 1, juce::dontSendNotification);
}

TunerPluginAudioProcessorEditor::~TunerPluginAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TunerPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1A));

    auto lang = LANGUAGES[audioProcessor.m_language.load()];

    auto currentHz = audioProcessor.m_detectedHz.load(std::memory_order_relaxed);
    juce::String displayNote;

	int cents = 0; // Declaration early for later use.

    if (currentHz > 0.0f)
    {
        double midi = 69.0 + 12.0 * std::log2(currentHz / tuningReference);
        int nearestNote = static_cast<int>(std::round(midi));

		std::string noteName;

        if (audioProcessor.m_sharpPresent) noteName = noteNamesSharp[nearestNote % 12];
		else noteName = noteNamesFlat[nearestNote % 12];

        int octave = (nearestNote / 12) - 1;
        displayNote = juce::String(noteName) + juce::String(octave);
        float targetHz = 440.0f * std::pow(2.0f, (nearestNote - 69) / 12.0f);
        cents = static_cast<int>(std::round(1200.0f * std::log2(currentHz / targetHz)));

		float volumeDb = 20.0f * std::log10(audioProcessor.m_detectedRms.load(std::memory_order_relaxed) + 1e-6f); // Avoid log(0)

        g.setFont(14.0f);
        g.setColour(juce::Colour(0xFFAAAAAA));
        g.drawText(juce::String::fromUTF8(lang.frequency) + ": " + juce::String(currentHz, 1) + " Hz",
            240, 160, 140, 25, juce::Justification::centredLeft, true);
        g.drawText(juce::String::fromUTF8(lang.deviation) + ": " + juce::String(cents) + " " + juce::String::fromUTF8(lang.cents),
            240, 190, 140, 25, juce::Justification::centredLeft, true);
        g.drawText(juce::String::fromUTF8(lang.volume) + ": " + juce::String::formatted("%.2f", volumeDb) + " dB", 240, 220, 140, 25, juce::Justification::centredLeft, true);
    }
    else
    {
        displayNote = "--";
        g.setFont(14.0f);
        g.setColour(juce::Colour(0xFFAAAAAA));
        g.drawText(juce::String::fromUTF8(lang.frequency) + ": -- Hz", 240, 160, 140, 25, juce::Justification::centredLeft, true);
        g.drawText(juce::String::fromUTF8(lang.deviation) + ": -- " + juce::String::fromUTF8(lang.cents), 240, 190, 140, 25, juce::Justification::centredLeft, true);
		g.drawText(juce::String::fromUTF8(lang.volume) + ": -- dB", 240, 220, 140, 25, juce::Justification::centredLeft, true);
		g.setColour(juce::Colour(0xFFFFFFFF));
    }

    auto noteDisplayBounds = getLocalBounds();
    noteDisplayBounds.translate(0, -70);
    g.setFont(24.0f);

	int absoluteCents = std::abs(cents); // Absolute value for easier comparison

    if (absoluteCents <= 5) {
        g.setColour(juce::Colour(0xFF00FF00)); // Green for in-tune
    }
    else if (absoluteCents <= SLIGHT_OFF_CENTS) {
        g.setColour(juce::Colour(0xFFFFFF00)); // Yellow for slightly out of tune
    }
    else {
        g.setColour(juce::Colour(0xFFFF0000)); // Red for out of tune
	}

    g.drawText(displayNote, noteDisplayBounds, juce::Justification::centred, true);

    g.setFont(9.0f);
    g.setColour(juce::Colour(0xFF444444));
    g.drawText("ZSOMBII Audio x ZSOMBII Microsystems",
        0, getHeight() - 15, getWidth() - 5, 12,
        juce::Justification::centredRight, true);

    int barY = 100;      
    int barHeight = 20;
    int startX = 130; 
    int endX = 270;
    int width = endX - startX;

    for (int i = 0; i <= 4; i++) {
        int tickX = startX + (i * width / 4);
        int tickHeight = (i == 2) ? 12 : 7;
        g.setColour(juce::Colour(0xFF666666));
        g.drawLine(tickX, barY + barHeight - tickHeight, tickX, barY + barHeight, 1.5f);
    }

    // Cent labels

    g.setFont(9.0f);
    g.setColour(juce::Colour(0xFF666666));
    int labels[] = { -50, -25, 0, 25, 50 };
    for (int i = 0; i <= 4; i++)
    {
        int tickX = startX + (i * width / 4);
        juce::String label = (labels[i] > 0 ? "+" : "") + juce::String(labels[i]);
        g.drawText(label, tickX - 12, barY + barHeight + 2, 24, 12,
            juce::Justification::centred, true);
    }

    float fraction = juce::jlimit(0.0f, 1.0f, (m_smoothedCents + 50.0f) / 100.0f);
    int needleX = startX + (int)(fraction * width);

    juce::Colour needleColour;
    if (std::abs(m_smoothedCents) < 5.0f)       needleColour = juce::Colours::green;
    else if (std::abs(m_smoothedCents) < 15.0f) needleColour = juce::Colours::yellow;
    else needleColour = juce::Colours::red;

    g.setColour(needleColour);
	g.drawLine(needleX, barY, needleX, barY + barHeight - 2, 2.0f);

}

void TunerPluginAudioProcessorEditor::resized()
{
    m_chromButton.setBounds(30, 160, 110, 30);
    m_guitarButton.setBounds(30, 195, 110, 30);
    m_bassButton.setBounds(30, 230, 110, 30);
    m_languageBox.setBounds(280, 10, 110, 25);
    m_sharpFlatBox.setBounds(10, 10, 110, 25);
}
