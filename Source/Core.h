#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
namespace sauce {
constexpr double pi=3.14159265358979323846;
inline float gain(float db){return std::pow(10.f,db/20.f);}
struct Settings {
 float trim=0,low=0,mid=0,high=0,drive=0,threshold=-18,compression=0,attack=10,release=150,makeup=0,room=.5f,damping=.5f,space=0,mix=1,output=0;
 bool prep=true,rub=true,fire=true,pit=true,smoke=true;
};
struct Smoother {float current=0,target=0,coefficient=.999f;void prepare(double sr,float ms=20){coefficient=std::exp(-1.f/(float(sr)*ms*.001f));}void set(float x){target=x;}float next(){current=target+coefficient*(current-target);return current;}void snap(float x){current=target=x;}};
struct Biquad {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 float tick(float x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return float(y);}void reset(){z1=z2=0;}
 void configure(double sr,int type,float db){
  double f=std::min(type==0?140.:type==1?1200.:6500.,sr*.4);double w=2*pi*f/sr,c=std::cos(w),s=std::sin(w),A=std::pow(10.,db/40.),alpha=s/(2*.70710678),root=2*std::sqrt(A)*alpha,a0;
  if(type==1){alpha=s/(2*.8);b0=1+alpha*A;b1=-2*c;b2=1-alpha*A;a0=1+alpha/A;a1=-2*c;a2=1-alpha/A;}
  else if(type==0){b0=A*((A+1)-(A-1)*c+root);b1=2*A*((A-1)-(A+1)*c);b2=A*((A+1)-(A-1)*c-root);a0=(A+1)+(A-1)*c+root;a1=-2*((A-1)+(A+1)*c);a2=(A+1)+(A-1)*c-root;}
  else{b0=A*((A+1)+(A-1)*c+root);b1=-2*A*((A-1)+(A+1)*c);b2=A*((A+1)+(A-1)*c-root);a0=(A+1)-(A-1)*c+root;a1=2*((A-1)-(A+1)*c);a2=(A+1)-(A-1)*c-root;}
  b0/=a0;b1/=a0;b2/=a0;a1/=a0;a2/=a0;
 }
};
struct Comb {std::vector<float> data;size_t index=0;float filtered=0;void prepare(size_t n){data.assign(std::max(size_t(1),n),0);index=0;filtered=0;}void reset(){std::fill(data.begin(),data.end(),0);index=0;filtered=0;}float tick(float x,float feedback,float damp){float y=data[index];filtered=y*(1-damp)+filtered*damp;data[index]=x+filtered*feedback;index=(index+1)%data.size();return y;}};
struct Allpass {std::vector<float> data;size_t index=0;void prepare(size_t n){data.assign(std::max(size_t(1),n),0);index=0;}void reset(){std::fill(data.begin(),data.end(),0);index=0;}float tick(float x){float y=data[index];data[index]=x+y*.5f;index=(index+1)%data.size();return y-x;}};
class Core {
 double sr=48000;Biquad eq[2][3];Comb comb[2][8];Allpass allpass[2][4];float envelope=0;int update=0;
 enum P{Trim,Low,Mid,High,Drive,Threshold,Compression,Attack,Release,Makeup,Room,Damping,Space,Mix,Output,PrepOn,RubOn,FireOn,PitOn,SmokeOn,Count};std::array<Smoother,Count> sm;
 public:
 float reductionDb=0;
 static float gain(float db){return sauce::gain(db);}
 void targets(const Settings& s){float v[]={s.trim,s.low,s.mid,s.high,s.drive,s.threshold,s.compression,s.attack,s.release,s.makeup,s.room,s.damping,s.space,s.mix,s.output,float(s.prep),float(s.rub),float(s.fire),float(s.pit),float(s.smoke)};for(int i=0;i<Count;i++)sm[i].set(v[i]);}
 void prepare(double rate){sr=rate;for(auto& x:sm)x.prepare(sr);Settings initial;targets(initial);for(auto& x:sm)x.snap(x.target);
  int lengths[]={1116,1188,1277,1356,1422,1491,1557,1617},ap[]={556,441,341,225};for(int c=0;c<2;c++){for(int i=0;i<8;i++)comb[c][i].prepare(size_t((lengths[i]+c*23)*sr/44100));for(int i=0;i<4;i++)allpass[c][i].prepare(size_t((ap[i]+c*23)*sr/44100));}reset();}
 void reset(){previous[0]=previous[1]=0;envelope=0;update=0;reductionDb=0;for(auto& x:sm)x.snap(x.target);for(int c=0;c<2;c++){for(int i=0;i<3;i++){eq[c][i].reset();eq[c][i].configure(sr,i,sm[Low+i].current);}for(auto& a:comb[c])a.reset();for(auto& a:allpass[c])a.reset();}}
 void process(float* l,float* r,int n,const Settings& s){targets(s);for(int i=0;i<n;i++){
  float v[Count];for(int k=0;k<Count;k++)v[k]=sm[k].next();
  if((update++&15)==0)for(int c=0;c<2;c++)for(int k=0;k<3;k++)eq[c][k].configure(sr,k,v[Low+k]);
  float dry[2]={l[i],r[i]},x[2];for(int c=0;c<2;c++){
   x[c]=dry[c]*gain(v[Trim]*v[PrepOn]);float filtered=x[c];for(auto& f:eq[c])filtered=f.tick(filtered);x[c]+=v[RubOn]*(filtered-x[c]);
   // ADAA first-order waveshaper. Antiderivative avoids naive tanh aliasing.
   float shaped=shape(c,x[c],v[Drive]);x[c]+=v[FireOn]*v[Drive]*(shaped-x[c]);
  }
  float peak=std::max(std::abs(x[0]),std::abs(x[1]));float ac=std::exp(-1.f/(float(sr)*v[Attack]*.001f)),rc=std::exp(-1.f/(float(sr)*v[Release]*.001f));float co=peak>envelope?ac:rc;envelope=co*envelope+(1-co)*peak;
  float db=20*std::log10(std::max(envelope,1e-12f));float over=db-v[Threshold],knee=6,gr=over<=-knee/2?0:over>=knee/2?over:((over+knee/2)*(over+knee/2)/(2*knee));float slope=1-1/(1+v[Compression]*7);reductionDb=gr*slope*v[PitOn];float cg=gain(-reductionDb+v[Makeup]*v[Compression]*v[PitOn]);
  float mono=(x[0]+x[1])*.015f*cg;float feedback=.65f+.30f*v[Room],damp=.05f+.65f*v[Damping];
  for(int c=0;c<2;c++){x[c]*=cg;float wet=0;for(auto& a:comb[c])wet+=a.tick(mono,feedback,damp);for(auto& a:allpass[c])wet=a.tick(wet);float w=v[Space]*v[SmokeOn];x[c]=x[c]*(1-w)+wet*w;float value=(dry[c]*(1-v[Mix])+x[c]*v[Mix])*gain(v[Output]);(c?r:l)[i]=std::isfinite(value)?value:0;}
 }}
 private:
 float previous[2]{}; // input to ADAA nonlinear stage
 static double antiderivative(double x){double a=std::abs(x);return a+std::log1p(std::exp(-2*a))-std::log(2.);}
 float shape(int c,float x,float amount){double d=1+amount*15,u=x*d,prev=previous[c]*d,delta=u-prev;double y=std::abs(delta)>1e-5?(antiderivative(u)-antiderivative(prev))/delta:std::tanh((u+prev)*.5);previous[c]=x;return float(y/std::sqrt(d));}
};
}
