#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AnimationState.h"
namespace sauce {
struct BBQVisuals {
 static juce::AffineTransform lidTransform(const AnimationState& s){const float lift=s.lidLift();return juce::AffineTransform::scale(1.f,1.f-.52f*lift,790.f,117.f).translated(0,-9*lift);}
 static juce::Path lidShape(){juce::Path p;p.startNewSubPath(294,309);p.lineTo(302,177);p.quadraticTo(310,109,367,109);p.lineTo(1251,109);p.quadraticTo(1315,110,1338,182);p.lineTo(1340,309);p.closeSubPath();return p;}
 static void background(juce::Graphics& g,const juce::Image& skin,const AnimationState& s,float heat){
  g.drawImageAt(skin,0,0);
  const float intensity=s.intensity();if(intensity<=0)return;
  // Re-sample only the food/air region. All labels and controls stay sharp.
  const float shimmer=intensity*heat*juce::jlimit(0.f,1.f,s.input*2.f);
  if(shimmer>.001f){g.saveState();g.reduceClipRegion(345,315,930,104);for(int row=0;row<104;row+=4){int dx=int(std::round(std::sin(s.time*5+row*.19f)*shimmer*3));g.drawImage(skin,345+dx,315+row,930,4,345,315+row,930,4);}g.restoreState();}
  if(s.lidLift()>.001f){
   const auto shape=lidShape();g.saveState();g.reduceClipRegion(shape);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff100f0c),790,118,juce::Colour(0xff382013),790,309,false));g.fillRect(294,109,1046,205);
   g.setColour(juce::Colour(0xff4e3323));for(int x=333;x<1320;x+=22)g.drawLine(float(x),282,float(x+11),309,2);
   g.restoreState();g.saveState();g.addTransform(lidTransform(s));g.reduceClipRegion(shape);g.drawImageAt(skin,0,0);g.restoreState();
  }
 }
 static void glow(juce::Graphics& g,float x,float y,float radius,juce::Colour colour,float alpha){
  alpha=juce::jlimit(0.f,1.f,alpha);g.setGradientFill(juce::ColourGradient(colour.withAlpha(alpha),x,y,colour.withAlpha(0.f),x+radius,y,true));g.fillEllipse(x-radius,y-radius,radius*2,radius*2);
 }
 static void atmosphere(juce::Graphics& g,const AnimationState& s,float fireOn,float heat,float room,float reverb,float sauceMix){
  const float amount=s.intensity();if(amount<=0)return;const float t=s.time;
  const float energy=fireOn*juce::jlimit(0.f,1.f,.05f+heat*.65f+s.input*.55f);
  // Flames and coals are confined to the existing firebox opening.
  g.saveState();g.reduceClipRegion(43,525,133,157);
  for(int i=0;i<5;i++){
   float x=58+i*23.f;glow(g,x,663,24,juce::Colour(0xffff8c22),amount*fireOn*(.09f+s.beat*.45f));
   float flicker=.70f+.22f*std::sin(t*8+i*1.9f)+.08f*std::sin(t*17+i);float height=(16+energy*91)*flicker;
   juce::Path f;f.startNewSubPath(x-7,676);f.cubicTo(x-14,651,x+8,654-height*.45f,x+std::sin(t*7+i)*7,676-height);f.cubicTo(x-2,653-height*.3f,x+14,654,x+7,676);f.closeSubPath();
   g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffbc55).withAlpha(amount*energy*.85f),x,676,juce::Colour(0xffef4b14).withAlpha(amount*energy*.15f),x,676-height,false));g.fillPath(f);
  }
  for(int i=0;i<24;i++){float age=std::fmod(t*(.17f+heat*.25f)+i*.193f,1.f);float x=50+std::fmod(i*31.f,115.f)+std::sin(t+i)*3;g.setColour(juce::Colour(0xffffb55e).withAlpha(amount*energy*(1-age)*.7f));g.fillEllipse(x,676-age*140,2,3);}
  g.restoreState();
  // Slow sauce flow hugs the glass edge, leaving the bottle lettering readable.
  if(sauceMix>.001f){for(int i=0;i<2;i++){float age=std::fmod(t*(.12f+sauceMix*.16f)+i*.51f,1.f);float x=428-i*76.f,y=701+age*132;juce::Path drip;drip.startNewSubPath(x,701);drip.cubicTo(x+3,724,x-3,y-16,x,y);g.setColour(juce::Colour(0xff9e3d16).withAlpha(amount*sauceMix*.85f));g.strokePath(drip,juce::PathStrokeType(2.7f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));g.fillEllipse(x-2,y-2,4,7);}}
  // Food steam follows reverb amount; room size controls its rise and spread.
  g.saveState();g.reduceClipRegion(345,314,931,109);
  for(int i=0;i<8;i++){
   float x=410+i*112.f;juce::Path steam;steam.startNewSubPath(x,414);steam.cubicTo(x-18,387,x+24*room,358,x+std::sin(t*.8f+i)*18,318);
   g.setColour(juce::Colour(0xffeadbc2).withAlpha(amount*reverb*(.11f+.025f*std::sin(t+i))));g.strokePath(steam,juce::PathStrokeType(3+room*3,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  }g.restoreState();
  for(int i=0;i<7;i++){float age=std::fmod(t*.12f+i*.147f,1.f);float x=1419+std::sin(t*.5f+i)*18,y=88-age*85;glow(g,x,y,20+age*15,juce::Colour(0xffd7cabc),amount*reverb*(1-age)*.12f);}
  // Warm background string-light highlights. No flashes or rapid strobing.
  const juce::Point<float> bulbs[]={{106,23},{217,11},{325,47},{441,30},{534,45},{686,27},{933,20}};
  for(int i=0;i<7;i++){auto p=bulbs[i];float pulse=.65f+.35f*std::sin(t*.65f+i*.7f);glow(g,p.x,p.y,18,juce::Colour(0xffffc16e),amount*(.06f+.10f*pulse));}
 }
};
}
