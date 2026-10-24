#include "../Source/Core.h"
#include "signalsmith-stretch.h"
#include <iostream>
#include <stdexcept>
#include <random>
void require(bool x,const char* msg){if(!x)throw std::runtime_error(msg);}
float rms(const std::vector<float>& a){double sum=0;for(float x:a)sum+=x*x;return float(std::sqrt(sum/a.size()));}
float render(sauce::Settings s,float frequency,float level=0.1f){sauce::Core core;core.prepare(48000);std::vector<float> l(48000),r(48000);for(int i=0;i<48000;i++)l[i]=r[i]=level*std::sin(2*sauce::pi*frequency*i/48000);core.process(l.data(),r.data(),int(l.size()),s);std::vector<float> tail(l.begin()+24000,l.end());return rms(tail);}
int main(){try{
 for(double sr:{22050.,44100.,48000.,96000.,192000.}){sauce::Core core;core.prepare(sr);sauce::Settings s;float l[257],r[257];std::mt19937 rand(4);for(int block=0;block<200;block++){for(int i=0;i<257;i++)l[i]=r[i]=float(int(rand()%2000)-1000)/1000;float original=l[100];core.process(l,r,257,s);require(std::abs(l[100]-original)<1e-5,"Default must be unity");}
  s.drive=1;s.compression=1;s.space=.9;s.low=12;s.mid=-12;s.high=12;s.room=1;s.damping=0;for(int block=0;block<500;block++){s.low=block%2?12:-12;s.drive=block%2;s.smoke=block%3;for(int i=0;i<257;i++){l[i]=block<150?.7f*std::sin((block*257+i)*.07):0;r[i]=block<150?.3f*std::sin((block*257+i)*.13):0;}core.process(l,r,257,s);for(int i=0;i<257;i++)require(std::isfinite(l[i])&&std::isfinite(r[i])&&std::abs(l[i])<50,"Stress output finite and bounded");}
  core.reset();std::fill_n(l,257,0);std::fill_n(r,257,0);core.process(l,r,257,s);for(float v:l)require(v==0,"Reset must clear all histories");
  signalsmith::stretch::SignalsmithStretch<float> stretch;stretch.presetDefault(2,sr);stretch.setTransposeSemitones(7);float a[257]{},b[257]{};float* in[]={l,r};float* out[]={a,b};for(int k=0;k<50;k++){stretch.process(in,257,out,257);for(float v:a)require(std::isfinite(v),"Signalsmith output finite");}
 }
 sauce::Settings s;float reference=render(s,50);s.low=6;require(render(s,50)>reference*1.7f,"Low shelf boosts bass");s={};reference=render(s,1200);s.mid=-6;require(render(s,1200)<reference*.6f,"Mid cut attenuates");s={};reference=render(s,14000);s.high=6;require(render(s,14000)>reference*1.7f,"High shelf boosts treble");s={};reference=render(s,1000,.8);s.compression=1;s.threshold=-24;require(render(s,1000,.8)<reference*.4f,"Compressor reduces loud signal");
 // Linked detector must apply the same gain to unequal channels.
 sauce::Core core;core.prepare(48000);s={};s.compression=1;float l[48000],r[48000];for(int i=0;i<48000;i++){l[i]=.8f*std::sin(i*.13);r[i]=l[i]*.25f;}core.process(l,r,48000,s);for(int i=1000;i<48000;i++)require(std::abs(r[i]-l[i]*.25f)<1e-6,"Stereo link preserves channel ratio");
 std::cout<<"PASS: 5 sample rates, unity, stress automation, bounded tails, complete reset, EQ responses, compression reduction, stereo link, Signalsmith\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
