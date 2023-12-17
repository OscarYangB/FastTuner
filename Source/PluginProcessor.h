/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SignBuffer.h"

constexpr auto FILTERCOUNT = 10;

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
        CreateParameterLayout();

    juce::AudioProcessorValueTreeState apvts
    {
        *this,
        nullptr,
        "Parameters",
        CreateParameterLayout()
    };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)

    const double minimumFrequency = 73.42;
    SignBuffer signBuffer;

    double currentPitch = 0.0;

    juce::dsp::IIR::Filter<float> filters[FILTERCOUNT] = {};

    void UpdateFilters();

    bool isValidPitch(const double pitch, const double sampleRate);

    static juce::String getNameFromInt(const int Value);

    std::array<float, FILTERCOUNT> GetSettings();

    int lastValidFilter = -1;
};
