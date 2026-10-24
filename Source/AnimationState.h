#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace sauce {
// Message-thread-only animation state. No graphics, clocks or random generators in the DSP callback.
struct AnimationState {
 int mode=1;float time=0,beat=0,input=0,needleDb=-48,needleVelocity=0,heldPeak=0,peakAge=0,recipeAge=10;
 std::uint32_t lastTransient=0;
 float intensity()const{return mode==0?0.f:mode==1?.38f:1.f;}
 void setMode(int m){mode=std::clamp(m,0,2);if(!mode){beat=0;recipeAge=10;}}
 void recipeChanged(){if(mode)recipeAge=0;}
 float lidLift()const{if(!mode||recipeAge>=2.4f)return 0;float t=recipeAge/2.4f;return std::sin(3.14159265358979323846f*t)*intensity();}
 void tick(float dt,float inputLevel,float outputLevel,std::uint32_t serial){
  dt=std::clamp(dt,0.f,.1f);if(mode){time+=dt;recipeAge+=dt;}
  input+= (std::clamp(inputLevel,0.f,4.f)-input)*(1-std::exp(-dt*12));
  beat*=std::exp(-dt*7);if(serial!=lastTransient){if(mode)beat=1;lastTransient=serial;}
  outputLevel=std::max(0.f,outputLevel);peakAge+=dt;
  if(outputLevel>=heldPeak){heldPeak=outputLevel;peakAge=0;}else if(peakAge>2)heldPeak*=std::exp(-dt*.7f);
  float target=std::clamp(20*std::log10(std::max(outputLevel,1e-6f)),-48.f,6.f);
  needleVelocity+=(target-needleDb)*65*dt;needleVelocity*=std::exp(-dt*11);needleDb=std::clamp(needleDb+needleVelocity*dt,-48.f,6.f);
 }
};
}
