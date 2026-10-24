#include "Plugin.h"
#include "BinaryData.h"
#include "BBQVisuals.h"
using namespace juce;
namespace {
struct BrassLook final:LookAndFeel_V4 {
 void drawRotarySlider(Graphics& g,int x,int y,int w,int h,float proportion,float start,float end,Slider&)override{
  float size=float(std::min(w,h))-8,cx=x+w*.5f,cy=y+h*.5f,r=size*.5f;
  g.setColour(Colours::black.withAlpha(.7f));g.fillEllipse(cx-r-3,cy-r+2,size+6,size+6);
  g.setGradientFill(ColourGradient(Colour(0xffeed4a1),cx-r,cy-r,Colour(0xff5b3419),cx+r,cy+r,false));g.fillEllipse(cx-r,cy-r,size,size);
  g.setColour(Colour(0xffd8a65f));g.drawEllipse(cx-r,cy-r,size,size,1);
  g.setGradientFill(ColourGradient(Colour(0xff704624),cx+r,cy-r,Colour(0xffd2a065),cx-r,cy+r,false));g.fillEllipse(cx-r+5,cy-r+5,size-10,size-10);
  Path arc;arc.addCentredArc(cx,cy,r+4,r+4,0,start,start+(end-start)*proportion,true);g.setColour(Colour(0xffef9b45));g.strokePath(arc,PathStrokeType(2));
  float a=start+proportion*(end-start);g.setColour(Colour(0xffffefd3));g.drawLine(cx+std::sin(a)*r*.4f,cy-std::cos(a)*r*.4f,cx+std::sin(a)*r*.85f,cy-std::cos(a)*r*.85f,2);
 }
};
class AdvancedWindow final:public DocumentWindow {
public:AdvancedWindow(SauceProcessor& p):DocumentWindow("I.R.Sauce / Advanced Controls",Colour(0xff20241f),DocumentWindow::closeButton){setUsingNativeTitleBar(true);setContentOwned(new GenericAudioProcessorEditor(p),true);centreWithSize(580,650);setVisible(true);}void closeButtonPressed()override{setVisible(false);}
};
class SmokerCanvas final:public Component {
 SauceProcessor& processor;Image skin;BrassLook look;sauce::AnimationState visuals;
 struct Dial{Slider slider;String id,label;float cx,cy;std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attachment;};
 std::vector<std::unique_ptr<Dial>> dials;
 std::vector<std::unique_ptr<ToggleButton>> toggles;
 std::vector<std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>> attachments;
public:
 void setMode(int mode){visuals.setMode(mode);repaint();}
 void recipeChanged(){visuals.recipeChanged();repaint();}
 SmokerCanvas(SauceProcessor& p):processor(p){skin=ImageCache::getFromMemory(BinaryData::Smoker_jpg,BinaryData::Smoker_jpgSize);setSize(1536,1024);
  struct Spec{const char* id;const char* label;int x,y,size;};
  const Spec specs[]={ {"low","LOW",320,561,61},{"mid","MID",391,561,61},{"high","HIGH",461,561,61}, {"drive","HEAT",581,561,82},{"compression","AMOUNT",704,561,70},{"threshold","THRESHOLD",794,561,70},{"pitch","SEMITONES",898,561,70},{"double","BLEND",985,561,70},{"room","SIZE",1090,561,70},{"space","MIX",1181,561,70},{"trim","INPUT",1318,561,82},{"mix","DRY / WET",866,812,95},{"output","OUTPUT dB",1202,812,95}};
  for(auto& spec:specs){auto d=std::make_unique<Dial>();d->id=spec.id;d->label=spec.label;d->cx=spec.x;d->cy=spec.y;d->slider.setLookAndFeel(&look);d->slider.setSliderStyle(Slider::RotaryVerticalDrag);d->slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);d->slider.setRotaryParameters(MathConstants<float>::pi*1.25f,MathConstants<float>::pi*2.75f,true);d->slider.setBounds(spec.x-spec.size/2,spec.y-spec.size/2,spec.size,spec.size);d->slider.setPopupDisplayEnabled(true,true,this);d->slider.setTooltip(processor.state.getParameter(spec.id)->getName(100));d->slider.onValueChange=[this]{repaint();};addAndMakeVisible(d->slider);d->attachment=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,spec.id,d->slider);d->slider.setDoubleClickReturnValue(true,p.state.getParameter(spec.id)->convertFrom0to1(p.state.getParameter(spec.id)->getDefaultValue()));dials.push_back(std::move(d));}
  const char* ids[]={"rubOn","fireOn","pitOn","stackOn","smokeOn","prepOn"};int left[]={300,510,667,853,1044,1247};for(int i=0;i<6;i++){auto b=std::make_unique<ToggleButton>("ON");b->setColour(ToggleButton::textColourId,Colour(0xfff5dfb8));b->setColour(ToggleButton::tickColourId,Colour(0xffffae53));b->setBounds(left[i]+12,499,70,23);b->setTooltip(p.state.getParameter(ids[i])->getName(100));addAndMakeVisible(*b);attachments.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,ids[i],*b));toggles.push_back(std::move(b));}
 }
 ~SmokerCanvas()override{for(auto& d:dials)d->slider.setLookAndFeel(nullptr);}
 void tick(float dt){visuals.tick(dt,processor.inputPeak.load(std::memory_order_relaxed),processor.outputPeak.load(std::memory_order_relaxed),processor.transientSerial.load(std::memory_order_relaxed));repaint();}
 void paint(Graphics& g)override{
  const float fireOn=processor.value("fireOn")>.5f?1.f:0.f;
  const float heat=processor.value("drive")*fireOn,space=processor.value("smokeOn")>.5f?processor.value("space"):0.f;
  sauce::BBQVisuals::background(g,skin,visuals,heat);
  sauce::BBQVisuals::atmosphere(g,visuals,fireOn,heat,processor.value("room"),space,processor.value("mix"));
  auto ink=[&](String text,Rectangle<int> box,float size,bool bold=true){g.setColour(Colour(0xff301b0e));g.setFont(FontOptions(size,bold?Font::bold:Font::plain));g.drawFittedText(text,box,Justification::centred,1);};
  g.saveState();g.addTransform(sauce::BBQVisuals::lidTransform(visuals));
  ink("I.R.Sauce",{680,137,232,60},42);ink("SMALL BATCH. BIG SOUND.",{685,195,222,16},11);
  g.restoreState();
  const char* names[]={"THE RUB","THE FIREBOX","THE PITMASTER","DOUBLE STACK","THE SMOKEHOUSE","PREP STATION"};const char* subs[]={"3-BAND EQ","SATURATION","COMPRESSION","PITCH LAYER","STEREO REVERB","INPUT GAIN"};int x[]={312,515,679,867,1049,1253},width[]={156,129,148,147,169,139};
  for(int i=0;i<6;i++){ink(names[i],{x[i],453,width[i],24},i==4?15:17);ink(subs[i],{x[i],478,width[i],15},10);}
  ink("SAUCE MIX",{805,729,124,22},17);ink("OUTPUT",{1140,731,120,22},17);
  ink("I.R.Sauce",{353,741,79,29},17);ink("SMALL",{358,778,67,16},10);ink("BATCH",{358,794,67,16},10);
  // All numerals and the moving needle are native, replacing the generated face.
  g.saveState();g.addTransform(sauce::BBQVisuals::lidTransform(visuals));float cx=1148,cy=209;
  for(int i=0;i<=6;i++){float a=(-120+i*40)*MathConstants<float>::pi/180;g.setColour(Colour(0xff422613));g.drawLine(cx+std::sin(a)*48,cy-std::cos(a)*48,cx+std::sin(a)*55,cy-std::cos(a)*55,1.4f);}
  ink("LEVEL",{1114,224,68,18},12);float a=jmap(visuals.needleDb,-48.f,6.f,-2.1f,2.1f);g.setColour(Colour(0xffa43221));g.drawLine(cx,cy,cx+std::sin(a)*46,cy-std::cos(a)*46,2.5f);g.fillEllipse(cx-4,cy-4,8,8);
  float heldDb=Decibels::gainToDecibels(juce::jmax(visuals.heldPeak,1e-6f));float heldAngle=jmap(jlimit(-48.f,6.f,heldDb),-48.f,6.f,-2.1f,2.1f);g.setColour(visuals.heldPeak>=1?Colour(0xffd03924):Colour(0xffb37322));g.drawLine(cx+std::sin(heldAngle)*58,cy-std::cos(heldAngle)*58,cx+std::sin(heldAngle)*64,cy-std::cos(heldAngle)*64,3);g.restoreState();
  for(auto& d:dials){g.setColour(Colour(0xffffe5bd));g.setFont(FontOptions(11,Font::bold));int labelY=int(d->cy)+(d->cy>700?49:36);g.drawFittedText(d->label,{int(d->cx)-48,labelY,96,15},Justification::centred,1);g.setFont(FontOptions(11));auto value=d->slider.getValue();String v;if(d->id=="drive"||d->id=="compression"||d->id=="double"||d->id=="room"||d->id=="space"||d->id=="mix")v=String(int(std::round(value*100)))+"%";else v=String(value,1);g.drawText(v,int(d->cx)-38,labelY+15,76,16,Justification::centred);}

 }
};
class SauceEditor final:public AudioProcessorEditor,private Timer {
 SauceProcessor& processor;SmokerCanvas canvas;ComboBox recipes,motion;TextButton advanced{"Advanced"};ToggleButton bp{"Bypass"};double lastTick=0;int visualMode=1;std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;std::unique_ptr<AdvancedWindow> window;TooltipWindow tips{this,700};
public:
 SauceEditor(SauceProcessor& p):AudioProcessorEditor(p),processor(p),canvas(p){addAndMakeVisible(canvas);addAndMakeVisible(recipes);addAndMakeVisible(advanced);addAndMakeVisible(motion);addAndMakeVisible(bp);recipes.addItem("Choose recipe",1);const char* names[]={"Sweet & Clean","Low & Slow","Memphis Crunch","Hot Honey Vocals","Midnight Smoke"};for(int i=0;i<5;i++)recipes.addItem(names[i],i+2);recipes.setSelectedId(1);recipes.onChange=[this]{if(recipes.getSelectedId()>1){processor.recipe(recipes.getSelectedId()-2);canvas.recipeChanged();}};advanced.onClick=[this]{if(!window)window=std::make_unique<AdvancedWindow>(processor);window->setVisible(true);window->toFront(true);};bypassAttachment=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bp);
  motion.addItem("Animation: Off",1);motion.addItem("Animation: Subtle",2);motion.addItem("Animation: Full",3);
  visualMode=jlimit(0,2,int(p.state.state.getProperty("animationMode",bool(p.state.state.getProperty("animations",true))?1:0)));motion.setSelectedId(visualMode+1,dontSendNotification);canvas.setMode(visualMode);
  motion.onChange=[this]{visualMode=motion.getSelectedId()-1;canvas.setMode(visualMode);processor.state.state.setProperty("animationMode",visualMode,nullptr);processor.state.state.setProperty("animations",visualMode>0,nullptr);lastTick=Time::getMillisecondCounterHiRes();startTimerHz(visualMode==2?30:visualMode==1?20:12);};
  motion.setTooltip("Off stops decorative movement; functional level and peak meters stay active.");setSize(1152,850);setResizable(true,true);setResizeLimits(960,722,1536,1106);lastTick=Time::getMillisecondCounterHiRes();startTimerHz(visualMode==2?30:visualMode==1?20:12);
 }
 void advanceVisuals(float dt,bool recipeChange){if(recipeChange)canvas.recipeChanged();canvas.tick(dt);}
 void timerCallback()override{double now=Time::getMillisecondCounterHiRes();float dt=float((now-lastTick)/1000);lastTick=now;int restoredMode=jlimit(0,2,int(processor.state.state.getProperty("animationMode",visualMode)));if(restoredMode!=visualMode)motion.setSelectedId(restoredMode+1,sendNotificationSync);canvas.tick(dt);repaint(0,getHeight()-32,getWidth(),32);}
 void resized()override{float scale=std::min(getWidth()/1536.f,(getHeight()-82)/1024.f);canvas.setTransform(AffineTransform::scale(scale));canvas.setTopLeftPosition(0,44);recipes.setBounds(210,8,250,28);advanced.setBounds(480,8,105,28);motion.setBounds(600,8,190,28);bp.setBounds(810,8,100,28);}
 void paint(Graphics& g)override{g.fillAll(Colour(0xff151611));g.setColour(Colour(0xfff4dfbf));g.setFont(FontOptions(18,Font::bold));g.drawText("I.R.Sauce",15,8,175,28,Justification::left);g.setFont(FontOptions(12));float db=Decibels::gainToDecibels(processor.outputPeak.load(),-90.f);String status="OUTPUT "+String(db,1)+" dBFS     |     COMPRESSION "+String(processor.gainReduction.load(),1)+" dB     |     LATENCY "+String(processor.getLatencySamples())+" samples     |     v0.4.0 BETA";g.drawText(status,16,getHeight()-31,getWidth()-32,26,Justification::left);}
};
}
juce::AudioProcessorEditor* SauceProcessor::createEditor(){return new SauceEditor(*this);}

void advanceSauceEditorVisuals(juce::AudioProcessorEditor& editor,float seconds,bool recipeChange){if(auto* e=dynamic_cast<SauceEditor*>(&editor))e->advanceVisuals(seconds,recipeChange);}
