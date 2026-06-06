/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "config.h"

void TunerPluginAudioProcessorEditor::timerCallback() {
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

	m_guitarButton.setRadioGroupId(TUNER_TYPE_GROUP);
	m_bassButton.setRadioGroupId(TUNER_TYPE_GROUP);
	m_chromButton.setRadioGroupId(TUNER_TYPE_GROUP);

	m_guitarButton.setToggleState(true, juce::dontSendNotification);

}

TunerPluginAudioProcessorEditor::~TunerPluginAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TunerPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1A));

    auto currentHz = audioProcessor.m_detectedHz.load(std::memory_order_relaxed);
    juce::String displayNote;

    if (currentHz > 0.0f)
    {
        double midi = 69.0 + 12.0 * std::log2(currentHz / tuningReference);
        int nearestNote = static_cast<int>(std::round(midi));
        std::string noteName = noteNames[nearestNote % 12];
        int octave = (nearestNote / 12) - 1;
        displayNote = juce::String(noteName) + juce::String(octave);
        float targetHz = 440.0f * std::pow(2.0f, (nearestNote - 69) / 12.0f);
        int cents = static_cast<int>(std::round(1200.0f * std::log2(currentHz / targetHz)));

        g.setFont(14.0f);
        g.setColour(juce::Colour(0xFFAAAAAA));
        g.drawText("Frequency: " + juce::String(currentHz, 1) + " Hz",
            240, 160, 140, 25, juce::Justification::centredLeft, true);
        g.drawText("Deviation: " + juce::String(cents) + " cents",
            240, 190, 140, 25, juce::Justification::centredLeft, true);
    }
    else
    {
        displayNote = "Idle";
        g.setFont(14.0f);
        g.setColour(juce::Colour(0xFFAAAAAA));
        g.drawText("Frequency: --", 240, 160, 140, 25, juce::Justification::centredLeft, true);
        g.drawText("Deviation: -- cents", 240, 190, 140, 25, juce::Justification::centredLeft, true);
    }

    auto noteDisplayBounds = getLocalBounds();
    noteDisplayBounds.translate(0, -70);
    g.setFont(24.0f);
    g.setColour(juce::Colours::white);
    g.drawText(displayNote, noteDisplayBounds, juce::Justification::centred, true);
}

void TunerPluginAudioProcessorEditor::resized()
{
    m_chromButton.setBounds(30, 160, 80, 30);
    m_guitarButton.setBounds(30, 195, 80, 30);
    m_bassButton.setBounds(30, 230, 80, 30);
}
