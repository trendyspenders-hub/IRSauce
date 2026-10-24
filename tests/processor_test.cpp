#include "../Source/Plugin.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
void require(bool x,const char* why){if(!x)throw std::runtime_error(why);}
void parameter(SauceProcessor& p,const char* id,float v){auto* x=p.state.getParameter(id);x->setValueNotifyingHost(x->convertTo0to1(v));}
void latencyTest(bool dryMix,bool bypass){SauceProcessor p;if(dryMix)parameter(p,"mix",0);parameter(p,"bypass",bypass?1:0);p.prepareToPlay(48000,127);int delay=p.getLatencySamples();juce::MidiBuffer midi;std::vector<float> history(12000);double maxError=0;int t=0;for(int block=0;block<80;block++){int n=block%2?1:257;juce::AudioBuffer<float> b(2,n);for(int i=0;i<n;i++){float x=.05f*std::sin((t+i)*.071f);history[t+i]=x;b.setSample(0,i,x);b.setSample(1,i,-x);}p.processBlock(b,midi);for(int i=0;i<n;i++){float want=t+i>=delay?history[t+i-delay]:0;maxError=std::max(maxError,double(std::abs(b.getSample(0,i)-want)));require(std::abs(b.getSample(0,i)+b.getSample(1,i))<1e-6,"Stereo opposite channels preserved");}t+=n;}require(maxError<2e-5,"Dry/default/bypass latency alignment");}
int main(int argc,char** argv){try{
 if(argc==3&&std::string(argv[1])=="--render"){
  juce::ScopedJuceInitialiser_GUI init;SauceProcessor p;p.prepareToPlay(48000,512);p.recipe(1);std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());editor->setVisible(true);auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f);juce::File file=juce::File::getCurrentWorkingDirectory().getChildFile(argv[2]);auto stream=file.createOutputStream();require(bool(stream),"Open screenshot file");juce::PNGImageFormat fmt;require(fmt.writeImageToStream(image,*stream),"Write screenshot");std::cout<<"Rendered native editor: "<<file.getFullPathName()<<"\n";return 0;
 }
 for(auto sr:{44100.,48000.,96000.}){SauceProcessor p;p.prepareToPlay(sr,128);juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,1025);p.recipe(2);for(int block=0;block<120;block++){for(int i=0;i<1025;i++){float x=2*std::sin((i+block*1025)*.14);b.setSample(0,i,x);b.setSample(1,i,x*.5f);}p.processBlock(b,midi);for(int c=0;c<2;c++)for(int i=0;i<1025;i++)require(std::isfinite(b.getSample(c,i))&&std::abs(b.getSample(c,i))<=.981f,"Limiter and oversized buffers");}p.reset();b.clear();p.processBlock(b,midi);for(int i=0;i<1025;i++)require(b.getSample(0,i)==0,"Processor reset clears tails");}
 latencyTest(false,false);latencyTest(true,false);latencyTest(false,true);
 SauceProcessor p;p.recipe(3);parameter(p,"low",4.2f);parameter(p,"attack",23);p.state.state.setProperty("animations",false,nullptr);juce::MemoryBlock state;p.getStateInformation(state);SauceProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));require(std::abs(restored.value("low")-4.2)<.02,"State restores EQ");require(std::abs(restored.value("attack")-23)<.02,"State restores advanced compression");require(!bool(restored.state.state.getProperty("animations",true)),"State restores animation preference");
 p.prepareToPlay(48000,64);juce::AudioBuffer<float> zero(2,0);juce::MidiBuffer midi;p.processBlock(zero,midi);
 std::cout<<"PASS: native processor at 44.1/48/96 kHz, oversized/zero buffers, limiter, reset, latency nulls, stereo, state recall\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
