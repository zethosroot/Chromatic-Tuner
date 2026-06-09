/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "config.h"

//==============================================================================
TunerPluginAudioProcessor::TunerPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
	m_pitchTracker{} // Initialize the pitch tracker struct
#endif
{
}

TunerPluginAudioProcessor::~TunerPluginAudioProcessor()
{
}

//==============================================================================
const juce::String TunerPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TunerPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TunerPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TunerPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TunerPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TunerPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TunerPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TunerPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TunerPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void TunerPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TunerPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    m_ringBuffer.fill(0.0f); // Hold a sample to work with
    m_ringWritePos = 0; // Indexing
    m_sampleRate = sampleRate; // Set the sample rate.
    m_bufferFull = false; // Buffer is empty

    dywapitch_inittracking(&m_pitchTracker);
}

void TunerPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TunerPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TunerPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* data = buffer.getReadPointer(0);
    int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; i++) {
        m_ringBuffer[m_ringWritePos] = data[i];
        m_ringWritePos = (m_ringWritePos + 1) % YIN_BUFFER_SIZE;

        if (m_ringWritePos == 0) m_bufferFull = true;
    }

    if (m_bufferFull) {

        // RMS noise gate
        float sum = 0.0f;
        for (int i = 0; i < YIN_BUFFER_SIZE; ++i) {
            sum += (float)(m_ringBuffer[i] * m_ringBuffer[i]);
        }
        float rms = std::sqrt(sum / YIN_BUFFER_SIZE);

        m_detectedRms.store(rms, std::memory_order_relaxed);

		float rmsThreshold = RMS_THRESHOLD; // Default RMS threshold value

        if (m_smoothedHz > 0.0f) {
            if (m_smoothedHz > 200) rmsThreshold = 0.002f; // Higher threshold for higher frequencies
			else if (m_smoothedHz > 100) rmsThreshold = 0.005f; // Medium threshold for mid frequencies
			else rmsThreshold = 0.01f; // Lower threshold for lower frequencies
        }


        if (rms < RMS_THRESHOLD) // RMS threshold
        {
            
            m_silenceCounter++;
            if (m_silenceCounter > SILENCE_HOLD_FRAMES)
                m_detectedHz.store(-1.0, std::memory_order_relaxed);
			m_bufferFull = false;
            return;
        }
        else {
			m_silenceCounter = 0; // Reset silence counter if signal is above threshold
        }

		double frequency = dywapitch_computepitch(&m_pitchTracker, m_ringBuffer.data(), 0, YIN_BUFFER_SIZE);

        int mode = m_tunerMode.load(std::memory_order_relaxed);

        float minHz, maxHz;

        switch (mode) {
            case 1: // Guitar thresholds
                minHz = GUITAR_FREQ_MIN;
                maxHz = GUITAR_FREQ_MAX;
                break;
            case 2: // Bass thresholds
                minHz = BASS_FREQ_MIN;
                maxHz = BASS_FREQ_MAX;
                break;
            default: // Chromatic thresholds
                minHz = CHROM_FREQ_MIN;
                maxHz = CHROM_FREQ_MAX;
        }

        if (frequency >= minHz && frequency <= maxHz) {
            
            if (m_smoothedHz < 0.0f) {
                m_smoothedHz = frequency;
            }
            else {
                m_smoothedHz = 0.7 * m_smoothedHz + 0.3 * frequency;
            }
           
            m_detectedHz.store(m_smoothedHz, std::memory_order_relaxed);
        }
        else {
            m_detectedHz.store(-1.0f, std::memory_order_relaxed);
			m_smoothedHz = -1.0f; // Reset smoothed frequency when out of range
		}

		m_bufferFull = false;
    }
}

//==============================================================================
bool TunerPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TunerPluginAudioProcessor::createEditor()
{
    return new TunerPluginAudioProcessorEditor (*this);
}

//==============================================================================

void TunerPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeInt(m_language.load());
    stream.writeInt(m_tunerMode.load());
    stream.writeBool(m_sharpPresent.load());
}

void TunerPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, sizeInBytes, false);
    m_language.store(stream.readInt());
    m_tunerMode.store(stream.readInt());
    m_sharpPresent.store(stream.readBool());
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TunerPluginAudioProcessor();
}
