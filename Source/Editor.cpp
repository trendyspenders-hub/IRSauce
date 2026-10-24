#include "Plugin.h"
#include "BinaryData.h"
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
 SauceProcessor& processor;Image skin;BrassLook look;float time=0,level=0;
 struct Dial{Slider slider;String id,label;float cx,cy;std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attachment;};
 std::vector<std::unique_ptr<Dial>> dials;
 std::vector<std::unique_ptr<ToggleButton>> toggles;
 std::vector<std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>> attachments;
public:
 bool animation=true;
 SmokerCanvas(SauceProcessor& p):processor(p){skin=ImageCache::getFromMemory(BinaryData::Smoker_jpg,BinaryData::Smoker_jpgSize);setSize(1536,1024);
  struct Spec{const char* id;const char* label;int x,y,size;};
  const Spec specs[]={ {"low","LOW",320,561,61},{"mid","MID",391,561,61},{"high","HIGH",461,561,61}, {"drive","HEAT",581,561,82},{"compression","AMOUNT",704,561,70},{"threshold","THRESHOLD",794,561,70},{"pitch","SEMITONES",898,561,70},{"double","BLEND",985,561,70},{"room","SIZE",1090,561,70},{"space","MIX",1181,561,70},{"trim","INPUT",1318,561,82},{"mix","DRY / WET",866,812,95},{"output","OUTPUT dB",1202,812,95}};
  for(auto& spec:specs){auto d=std::make_unique<Dial>();d->id=spec.id;d->label=spec.label;d->cx=spec.x;d->cy=spec.y;d->slider.setLookAndFeel(&look);d->slider.setSliderStyle(Slider::RotaryVerticalDrag);d->slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);d->slider.setRotaryParameters(MathConstants<float>::pi*1.25f,MathConstants<float>::pi*2.75f,true);d->slider.setBounds(spec.x-spec.size/2,spec.y-spec.size/2,spec.size,spec.size);d->slider.setPopupDisplayEnabled(true,true,this);d->slider.setTooltip(processor.state.getParameter(spec.id)->getName(100));d->slider.onValueChange=[this]{repaint();};addAndMakeVisible(d->slider);d->attachment=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,spec.id,d->slider);d->slider.setDoubleClickReturnValue(true,p.state.getParameter(spec.id)->convertFrom0to1(p.state.getParameter(spec.id)->getDefaultValue()));dials.push_back(std::move(d));}
  const char* ids[]={"rubOn","fireOn","pitOn","stackOn","smokeOn","prepOn"};int left[]={300,510,667,853,1044,1247};for(int i=0;i<6;i++){auto b=std::make_unique<ToggleButton>("ON");b->setColour(ToggleButton::textColourId,Colour(0xfff5dfb8));b->setColour(ToggleButton::tickColourId,Colour(0xffffae53));b->setBounds(left[i]+12,499,70,23);b->setTooltip(p.state.getParameter(ids[i])->getName(100));addAndMakeVisible(*b);attachments.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,ids[i],*b));toggles.push_back(std::move(b));}
 }
 ~SmokerCanvas()override{for(auto& d:dials)d->slider.setLookAndFeel(nullptr);}
 void tick(float dt){time+=dt;level=level*.75f+processor.outputPeak.load(std::memory_order_relaxed)*.25f;repaint();}
 void paint(Graphics& g)override{
  g.drawImage(skin,getLocalBounds().toFloat());
  auto ink=[&](String text,Rectangle<int> box,float size,bool bold=true){g.setColour(Colour(0xff301b0e));g.setFont(FontOptions(size,bold?Font::bold:Font::plain));g.drawFittedText(text,box,Justification::centred,1);};
  ink("I.R.Sauce",{680,137,232,60},42);ink("SMALL BATCH. BIG SOUND.",{685,195,222,16},11);
  const char* names[]={"THE RUB","THE FIREBOX","THE PITMASTER","DOUBLE STACK","THE SMOKEHOUSE","PREP STATION"};const char* subs[]={"3-BAND EQ","SATURATION","COMPRESSION","PITCH LAYER","STEREO REVERB","INPUT GAIN"};int x[]={312,515,679,867,1049,1253},width[]={156,129,148,147,169,139};
  for(int i=0;i<6;i++){ink(names[i],{x[i],453,width[i],24},i==4?15:17);ink(subs[i],{x[i],478,width[i],15},10);}
  ink("SAUCE MIX",{805,729,124,22},17);ink("OUTPUT",{1140,731,120,22},17);
  ink("I.R.Sauce",{353,741,79,29},17);ink("SMALL",{358,778,67,16},10);ink("BATCH",{358,794,67,16},10);
  // All numerals and the moving needle are native, replacing the generated face.
  float cx=1148,cy=209;
  for(int i=0;i<=6;i++){float a=(-120+i*40)*MathConstants<float>::pi/180;g.setColour(Colour(0xff422613));g.drawLine(cx+std::sin(a)*48,cy-std::cos(a)*48,cx+std::sin(a)*55,cy-std::cos(a)*55,1.4f);}
  ink("LEVEL",{1114,224,68,18},12);float db=Decibels::gainToDecibels(std::max(level,1e-6f));float a=jmap(jlimit(-48.f,6.f,db),-48.f,6.f,-2.1f,2.1f);g.setColour(Colour(0xffa43221));g.drawLine(cx,cy,cx+std::sin(a)*46,cy-std::cos(a)*46,2.5f);g.fillEllipse(cx-4,cy-4,8,8);
  for(auto& d:dials){g.setColour(Colour(0xffffe5bd));g.setFont(FontOptions(11,Font::bold));int labelY=int(d->cy)+(d->cy>700?49:36);g.drawFittedText(d->label,{int(d->cx)-48,labelY,96,15},Justification::centred,1);g.setFont(FontOptions(11));auto value=d->slider.getValue();String v;if(d->id=="drive"||d->id=="compression"||d->id=="double"||d->id=="room"||d->id=="space"||d->id=="mix")v=String(int(std::round(value*100)))+"%";else v=String(value,1);g.drawText(v,int(d->cx)-38,labelY+15,76,16,Justification::centred);}
  if(animation){float heat=processor.value("fireOn")>.5f?processor.value("drive"):0.f;g.saveState();g.reduceClipRegion(32,526,175,164);for(int i=0;i<20;i++){float age=std::fmod(time*(.22f+heat*.3f)+i*.271f,1.f);float ex=52+std::fmod(i*31.f,130.f)+std::sin(time+i)*3,ey=675-age*145;g.setColour(Colour(0xffffac4c).withAlpha((1-age)*heat*.65f));g.fillEllipse(ex,ey,2.5f,3.5f);}g.restoreState();float smoke=processor.value("smokeOn")>.5f?processor.value("space"):0.f;for(int i=0;i<7;i++){float age=std::fmod(time*.12f+i*.147f,1.f);float sx=1419+std::sin(time*.5f+i)*18,sy=88-age*85;g.setColour(Colour(0xffd7cabc).withAlpha(smoke*(1-age)*.10f));g.fillEllipse(sx-20-age*10,sy-12,40+age*20,24+age*20);}}
 }
};
class SauceEditor final:public AudioProcessorEditor,private Timer {
 SauceProcessor& processor;SmokerCanvas canvas;ComboBox recipes;TextButton advanced{"Advanced"};ToggleButton motion{"Animation"},bp{"Bypass"};std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;std::unique_ptr<AdvancedWindow> window;TooltipWindow tips{this,700};
public:
 SauceEditor(SauceProcessor& p):AudioProcessorEditor(p),processor(p),canvas(p){addAndMakeVisible(canvas);addAndMakeVisible(recipes);addAndMakeVisible(advanced);addAndMakeVisible(motion);addAndMakeVisible(bp);recipes.addItem("Choose recipe",1);const char* names[]={"Sweet & Clean","Low & Slow","Memphis Crunch","Hot Honey Vocals","Midnight Smoke"};for(int i=0;i<5;i++)recipes.addItem(names[i],i+2);recipes.setSelectedId(1);recipes.onChange=[this]{if(recipes.getSelectedId()>1)processor.recipe(recipes.getSelectedId()-2);};advanced.onClick=[this]{if(!window)window=std::make_unique<AdvancedWindow>(processor);window->setVisible(true);window->toFront(true);};bypassAttachment=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bp);
  motion.setToggleState(p.state.state.getProperty("animations",true),dontSendNotification);canvas.animation=motion.getToggleState();motion.onClick=[this]{canvas.animation=motion.getToggleState();processor.state.state.setProperty("animations",canvas.animation,nullptr);canvas.repaint();};setSize(1152,850);setResizable(true,true);setResizeLimits(960,722,1536,1106);startTimerHz(24);
 }
 void timerCallback()override{canvas.tick(1.f/24);repaint(0,getHeight()-32,getWidth(),32);}
 void resized()override{float scale=std::min(getWidth()/1536.f,(getHeight()-82)/1024.f);canvas.setTransform(AffineTransform::scale(scale));canvas.setTopLeftPosition(0,44);recipes.setBounds(210,8,250,28);advanced.setBounds(480,8,105,28);motion.setBounds(610,8,130,28);bp.setBounds(755,8,100,28);}
 void paint(Graphics& g)override{g.fillAll(Colour(0xff151611));g.setColour(Colour(0xfff4dfbf));g.setFont(FontOptions(18,Font::bold));g.drawText("I.R.Sauce",15,8,175,28,Justification::left);g.setFont(FontOptions(12));float db=Decibels::gainToDecibels(processor.outputPeak.load(),-90.f);String status="OUTPUT "+String(db,1)+" dBFS     |     COMPRESSION "+String(processor.gainReduction.load(),1)+" dB     |     LATENCY "+String(processor.getLatencySamples())+" samples     |     v0.3.0 BETA";g.drawText(status,16,getHeight()-31,getWidth()-32,26,Justification::left);}
};
}
juce::AudioProcessorEditor* SauceProcessor::createEditor(){return new SauceEditor(*this);}
