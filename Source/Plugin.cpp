#include "Plugin.h"
using namespace juce;
AudioProcessorValueTreeState::ParameterLayout SauceProcessor::layout(){
 AudioProcessorValueTreeState::ParameterLayout l;
 auto add=[&](const char* id,const char* name,float low,float high,float initial,float step=.01f){l.add(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,NormalisableRange<float>{low,high,step},initial));};
 add("trim","Prep Station / Input dB",-24,24,0);add("low","The Rub / Low dB",-12,12,0);add("mid","The Rub / Mid dB",-12,12,0);add("high","The Rub / High dB",-12,12,0);
 add("drive","Firebox / Heat",0,1,0,.001);add("compression","Pitmaster / Amount",0,1,0,.001);add("threshold","Pitmaster / Threshold dB",-48,0,-18);
 add("attack","Pitmaster / Attack ms",1,100,10);add("release","Pitmaster / Release ms",20,1000,150);add("makeup","Pitmaster / Makeup dB",0,18,0);
 add("pitch","Double Stack / Semitones",-12,12,0);add("double","Double Stack / Blend",0,1,0,.001);
 add("room","Smokehouse / Size",0,1,.5,.001);add("damping","Smokehouse / Damping",0,1,.5,.001);add("space","Smokehouse / Reverb Mix",0,1,0,.001);
 add("mix","Sauce / Global Mix",0,1,1,.001);add("output","Serving Level / Output dB",-24,12,0);
 const char* ids[]={"prepOn","rubOn","fireOn","pitOn","stackOn","smokeOn","limiter","bypass"};
 const char* names[]={"Prep Station / Enabled","The Rub / Enabled","Firebox / Enabled","Pitmaster / Enabled","Double Stack / Enabled","Smokehouse / Enabled","Output / Safety Limiter","Global / Bypass"};
 for(int i=0;i<8;i++)l.add(std::make_unique<AudioParameterBool>(ParameterID{ids[i],1},names[i],i!=7));return l;
}
SauceProcessor::SauceProcessor():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),state(*this,nullptr,"IRSauce",layout()){}
bool SauceProcessor::isBusesLayoutSupported(const BusesLayout& b)const{return b.getMainInputChannelSet()==AudioChannelSet::stereo()&&b.getMainOutputChannelSet()==AudioChannelSet::stereo();}
void SauceProcessor::prepareToPlay(double rate,int block){sampleRate=rate;core.prepare(rate);stretch.presetDefault(2,rate);latency=stretch.inputLatency()+stretch.outputLatency();setLatencySamples(latency);
 int size=std::max(64,block);for(auto* b:{&shifted,&pitchInput,&rawAligned,&wetAligned})b->setSize(2,size);for(auto& d:delay)d.assign(latency+1,0);
 for(auto* s:{&inputGain,&pitch,&layer,&globalMix,&outputGain,&bypass,&limiterOn})s->prepare(rate);reset();}
void SauceProcessor::reset(){core.reset();stretch.reset();for(auto& d:delay)std::fill(d.begin(),d.end(),0);write=0;limiterGain=1;firstBlock=true;outputPeak=0;gainReduction=0;}
void SauceProcessor::processBlock(AudioBuffer<float>& b,MidiBuffer&){process(b,false);}
void SauceProcessor::processBlockBypassed(AudioBuffer<float>& b,MidiBuffer&){process(b,true);}
void SauceProcessor::process(AudioBuffer<float>& b,bool hostBypass){ScopedNoDenormals no;auto count=b.getNumSamples();if(count==0)return;if(delay[0].empty()||b.getNumChannels()<2){b.clear();return;}
 inputGain.set(sauce::gain(value("trim")*(value("prepOn")>.5f)));pitch.set(value("pitch"));layer.set(value("stackOn")>.5f?value("double"):0);globalMix.set(value("mix"));outputGain.set(sauce::gain(value("output")));bypass.set(hostBypass||value("bypass")>.5f?1.f:0.f);limiterOn.set(value("limiter"));
 if(firstBlock){for(auto* s:{&inputGain,&pitch,&layer,&globalMix,&outputGain,&bypass,&limiterOn})s->snap(s->target);firstBlock=false;}
 sauce::Settings settings;settings.low=value("low");settings.mid=value("mid");settings.high=value("high");settings.drive=value("drive");settings.compression=value("compression");settings.threshold=value("threshold");settings.attack=value("attack");settings.release=value("release");settings.makeup=value("makeup");settings.room=value("room");settings.damping=value("damping");settings.space=value("space");settings.rub=value("rubOn")>.5f;settings.fire=value("fireOn")>.5f;settings.pit=value("pitOn")>.5f;settings.smoke=value("smokeOn")>.5f;
 float peak=0;const float recovery=std::exp(-1.f/(float(sampleRate)*.1f));
 // Chunking bounds pitch updates and permits hosts to exceed their advertised block size.
 for(int offset=0;offset<count;offset+=64){int n=std::min(64,count-offset);float* ptr[]={b.getWritePointer(0,offset),b.getWritePointer(1,offset)};float* in[]={pitchInput.getWritePointer(0),pitchInput.getWritePointer(1)};float* shiftedOut[]={shifted.getWritePointer(0),shifted.getWritePointer(1)};
  for(int i=0;i<n;i++){float trim=inputGain.next();auto read=(write+1)%delay[0].size();for(int c=0;c<2;c++){float raw=std::isfinite(ptr[c][i])?ptr[c][i]:0;in[c][i]=raw*trim;rawAligned.setSample(c,i,delay[c][read]);wetAligned.setSample(c,i,delay[c+2][read]);delay[c][write]=raw;delay[c+2][write]=in[c][i];}write=read;}
  float semitones=0;for(int i=0;i<n;i++)semitones=pitch.next();stretch.setTransposeSemitones(semitones);stretch.process(in,n,shiftedOut,n);
  for(int i=0;i<n;i++){float blend=layer.next();for(int c=0;c<2;c++)ptr[c][i]=wetAligned.getSample(c,i)*(1-blend)+shiftedOut[c][i]*blend;}
  core.process(ptr[0],ptr[1],n,settings);
  for(int i=0;i<n;i++){float mix=globalMix.next(),out=outputGain.next(),bp=bypass.next(),safe=limiterOn.next();float result[2];for(int c=0;c<2;c++)result[c]=(rawAligned.getSample(c,i)*(1-mix)+ptr[c][i]*mix)*out;
   float m=std::max(std::abs(result[0]),std::abs(result[1]));float needed=m>.98f?.98f/m:1.f;limiterGain=needed<limiterGain?needed:recovery*limiterGain+(1-recovery)*needed;
   for(int c=0;c<2;c++){float limited=result[c]*(1-safe+safe*limiterGain);float x=limited*(1-bp)+rawAligned.getSample(c,i)*bp;ptr[c][i]=std::isfinite(x)?x:0;peak=std::max(peak,std::abs(ptr[c][i]));}
  }
 }
 outputPeak.store(peak,std::memory_order_relaxed);gainReduction.store(core.reductionDb,std::memory_order_relaxed);
}
void SauceProcessor::getStateInformation(MemoryBlock& d){auto xml=state.copyState().createXml();copyXmlToBinary(*xml,d);}
void SauceProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(state.state.getType())){auto restored=ValueTree::fromXml(*xml);state.replaceState(restored);}}
void SauceProcessor::recipe(int index){
 auto set=[&](const char* id,float v){auto* p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();};
 for(auto id:{"trim","low","mid","high","drive","compression","pitch","double","space","makeup"})set(id,0);
 set("threshold",-18);set("attack",10);set("release",150);set("room",.5);set("damping",.5);set("mix",1);set("output",-3);set("bypass",0);set("limiter",1);for(auto id:{"prepOn","rubOn","fireOn","pitOn","stackOn","smokeOn"})set(id,1);
 if(index==0){set("high",1.5);set("compression",.2);}if(index==1){set("drive",.22);set("compression",.35);set("space",.14);}if(index==2){set("drive",.7);set("low",2);set("high",-2);set("compression",.45);}if(index==3){set("high",2);set("drive",.15);set("compression",.4);set("pitch",.09);set("double",.23);set("space",.15);}if(index==4){set("high",-4);set("pitch",-12);set("double",.3);set("room",.9);set("space",.55);}
}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SauceProcessor();}
