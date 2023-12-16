/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int totalNumInputChannels = getTotalNumInputChannels();
    const unsigned int bufferLength = (int) ceil(sampleRate / minimumFrequency) * 2;
    signBuffer = SignBuffer(bufferLength);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    for (int i = 0; i < FILTERCOUNT; ++i)
    {
        filters[i].prepare(spec);
    }

    UpdateFilters();
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    float* const samples = buffer.getWritePointer(0);

    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
    {
        bool sign = signbit(samples[sampleIndex]);
        if (signBuffer.WriteSignToBuffer(sign))
        {
            currentPitch = signBuffer.GetPitch(getSampleRate());
            DBG(currentPitch);
            UpdateFilters();
        }
        for (int i = 0; i < FILTERCOUNT; ++i)
        {
            samples[sampleIndex] = filters[i].processSample(samples[sampleIndex]);
        }
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::CreateParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 1; i <= FILTERCOUNT; i++)
    {
        juce::String name = getNameFromInt(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            name,
            name,
            juce::NormalisableRange<float>(-20.0f, 20.0, 0.1f, 1.0f),
            0.0f
        ));
    }

    return layout;
}

void NewProjectAudioProcessor::UpdateFilters()
{
    if (currentPitch <= 0.0) return;

    std::array<float, FILTERCOUNT> gains = GetSettings();

    for (int i = 0; i < FILTERCOUNT; ++i)
    {
        double harmonicPitch = currentPitch * (i + 1);
        if (harmonicPitch > getSampleRate() * 0.5) 
        {
            continue;
        }
        filters[i].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            getSampleRate(), harmonicPitch, 1.0, juce::Decibels::decibelsToGain(gains[i]));
    }
}

juce::String NewProjectAudioProcessor::getNameFromInt(const int Value)
{
    juce::String name = "f";
    name.append(juce::String(Value), 2);
    return name;
}

std::array<float, FILTERCOUNT> NewProjectAudioProcessor::GetSettings()
{
    std::array<float, FILTERCOUNT> settings = std::array<float, FILTERCOUNT>();
    for (int i = 0; i < FILTERCOUNT; ++i)
    {
        auto value = apvts.getRawParameterValue(getNameFromInt(i));
        if (value != nullptr) 
        {
            settings[i] = *value;
        }
    }
    return settings;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
