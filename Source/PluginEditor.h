/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TunerPluginAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    TunerPluginAudioProcessorEditor (TunerPluginAudioProcessor&);
    ~TunerPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
	void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TunerPluginAudioProcessor& audioProcessor;

	juce::ToggleButton m_chromButton{ "Chromatic" };
    juce::ToggleButton m_guitarButton{ "Guitar" };
    juce::ToggleButton m_bassButton{ "Bass" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerPluginAudioProcessorEditor)
};
