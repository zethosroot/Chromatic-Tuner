/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
}

TunerPluginAudioProcessorEditor::~TunerPluginAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TunerPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setFont(24.0f);

    float currentHz = audioProcessor.m_detectedHz.load(std::memory_order_relaxed);

    juce::String displayString;

    if (currentHz > 0.0f) {
        displayString = juce::String (currentHz, 1) + " Hz";
    }
    else {
        displayString = "No Signal...";
    }

    g.drawText(displayString, getLocalBounds(), juce::Justification::centred, true);
}

void TunerPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
