#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Core.h"
#include "signalsmith-stretch.h"
class SauceProcessor final: public juce::AudioProcessor {
public:
 juce::AudioProcessorValueTreeState state;
 std::atomic<float> outputPeak{0},gainReduction{0},inputPeak{0};
 std::atomic<std::uint32_t> transientSerial{0};
 SauceProcessor();
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 float value(const char* id)const{return state.getRawParameterValue(id)->load();}
 const juce::String getName()const override{return "I.R.Sauce";}
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 15;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return "Custom";}void changeProgramName(int,const juce::String&)override{}
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 void prepareToPlay(double,int)override;void releaseResources()override{}void reset()override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 juce::AudioProcessorParameter* getBypassParameter()const override{return state.getParameter("bypass");}
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 bool hasEditor()const override{return true;}juce::AudioProcessorEditor* createEditor()override;
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 void recipe(int index);
private:
 sauce::Core core;signalsmith::stretch::SignalsmithStretch<float> stretch;
 juce::AudioBuffer<float> shifted,pitchInput,rawAligned,wetAligned;
 std::vector<float> delay[4];size_t write=0;int latency=0;bool firstBlock=true;
 sauce::Smoother inputGain,pitch,layer,globalMix,outputGain,bypass,limiterOn;
 float limiterGain=1,visualFast=0,visualSlow=0;int visualCooldown=0;double sampleRate=48000;
 void process(juce::AudioBuffer<float>&,bool hostBypass);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceProcessor)
};

// Offline preview/test hook; runs only on the message thread.
void advanceSauceEditorVisuals(juce::AudioProcessorEditor&,float seconds,bool recipeChange=false);
