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
void TunerPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xFF1A1A1A));

	g.setColour(juce::Colours::white);

    g.setFont(24.0f);

    auto currentHz = audioProcessor.m_detectedHz.load(std::memory_order_relaxed);

    juce::String displayNote;

    if (currentHz > 0.0f) {
        displayNote = juce::String (currentHz, 1) + " Hz";
    }
    else {
        displayNote = "Idle";
    }

	auto noteDisplayBounds = getLocalBounds(); 

    noteDisplayBounds.translate(0, -70);

    g.drawText(displayNote, noteDisplayBounds, juce::Justification::centred, true);

    
}

void TunerPluginAudioProcessorEditor::resized()
{
    m_chromButton.setBounds(100, 180, 80, 30);
    m_guitarButton.setBounds(190, 180, 80, 30);
    m_bassButton.setBounds(280, 180, 80, 30);
}
