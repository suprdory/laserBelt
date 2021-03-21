#include <FastLED.h>
#define NUM_LEDS 60
#define DATA_PIN 21    //21 is A3 on Pro micro  //use 14 on uno
CRGB leds[NUM_LEDS];
CRGB rgb1,rgb2,rgb3;
byte gHue, gPos, gVal, gHue0, gPos0, aMag;
byte col1,col2,col3,scl;
CHSV color,colorLoop;
CRGBPalette16 currentPalette( PartyColors_p );
CHSVPalette16 loopPalette( PartyColors_p );
#include <Wire.h>
#include "MPU6050.h"
#define NHIST 100
#define NSMOOTH 5
float w=0;
float k=1;
char ax,ay,az,dx,dy,dz;
char sx,sy,sz;
byte Hc;
char Hx[NHIST],Hy[NHIST],Hz[NHIST];
int16_t Tx,Ty,Tz;
int16_t count=0;
byte Sc;
char Sx[NHIST],Sy[NHIST],Sz[NHIST];
int16_t STx,STy,STz;
char pat=1,pattern=2;
int16_t aa[3];           // Stores the 16-bit signed accelerometer sensor output
MPU6050lib mpu;
void setup()
{
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  start_MPU6050();
  randomSeed(analogRead(1));
  col1=random(0,256);
  col2=random(0,256);
  col3=random(0,256);
  pat=2;
  w=.1;
  k=2.;
}

void loop()
{ 
  update_accel();
  if(count%16000==0) {
  pattern=random(1,3);
}

if(pattern==1){ 
  pattern1();
}
if(pattern==2){ 
  pattern2();
}

  count++;
}

void pattern2() {

    if(count%4000==0) {
      pat=random(1,5);
      Serial.print("pat: "); Serial.println((int8_t)pat); 
    }

  if (count%100==0) {
    cycle_loop_palette();
  }
  
  if (count%5==0){
    fadeall(200);
  }

  if (count%5==0){
    get_Mag();
    for(int i=0;i<NUM_LEDS;i++) {
  
      colorLoop=ColorFromPalette(loopPalette, 3*i*(256/NUM_LEDS)+(count >> 1));
      color=CHSV(colorLoop.h,255,5*aMag*sq(cos8(6*i*(256/NUM_LEDS)+(count >> 2))/255.));  
      // color=CHSV(colorLoop.h,255,50);
      if(leds[i]<color) {
        leds[i]=color;
      }
    }
  }
  
  if (count%5==0){  
    switch (pat) {
      case 1:
        shift_down();
      break;
      case 2:  
        shift_up();
      break;
      case 3:
        shift_out();
      break;
      case 4:
        shift_in();
      break;
    }
   }
  FastLED.show();
}

void pattern1() {

  if (count%50==0) {
    cycle_palette();
  }
  
  if(count%4000==0) {
    pat=random(1,5);
    //Serial.print("pat: "); Serial.println((int8_t)pat); 
  }
   
  if(count%10==0) {
    get_Mag();
    switch (pat) {
      case 1:
        shift_down();
        leds[NUM_LEDS-1]=ColorFromPalette(currentPalette, aMag);
      break;
      case 2:  
        shift_up();
        leds[0]=ColorFromPalette(currentPalette, aMag);
      break;
      case 3:
        shift_out();
        leds[NUM_LEDS/2]=ColorFromPalette(currentPalette, aMag);
      break;
      case 4:
        shift_in();
        leds[NUM_LEDS-1]=ColorFromPalette(currentPalette, aMag);
        leds[0]=ColorFromPalette(currentPalette, aMag);
      break;
    }
    FastLED.show();
  }
}

void cycle_palette() {
  col1=col1-2;
  col2=col2-1;
  col3=col3+3;
  SetupColPalette();
}
void cycle_loop_palette() {
  col1=col1-2;
  col2=col2-1;
  col3=col3+3;
  SetupLoopPalette();
}

void shift_in() {
  for (int i=NUM_LEDS/2;i<(NUM_LEDS-1);i++){
    leds[i]=leds[i+1];
  } 
  for (int i=NUM_LEDS/2;i>0;i--){
    leds[i]=leds[i-1];
  } 
}
void shift_out() {
  for (int i=NUM_LEDS-1;i>NUM_LEDS/2;i--){
    leds[i]=leds[i-1];
  } 
  for (int i=0;i<NUM_LEDS/2;i++){
    leds[i]=leds[i+1];
  } 
}
void shift_up() {
    for (int i=NUM_LEDS-1;i>0;i--){
    leds[i]=leds[i-1];
  }
}
void shift_down() {
  for (int i=0;i<NUM_LEDS-1;i++){
    leds[i]=leds[i+1];
  }
}
                      
void SetupColPalette() {
  hsv2rgb_rainbow(CHSV( col1, 255, 0  ), rgb1);
  hsv2rgb_rainbow(CHSV( col3, 255, 255), rgb3);
  hsv2rgb_rainbow(CHSV( col2, 255, 255), rgb2);
  currentPalette = CRGBPalette16( 
                      rgb1, 
                      rgb2,
                      rgb3); 
}

void SetupLoopPalette()
{
  loopPalette = CHSVPalette16( 
                      CHSV( col3, 255, 255), 
                      CHSV( col2, 255, 255),
                      CHSV( col3, 255, 255)); 
}

void fadeall(byte scl) { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(scl); } }

float mapF(float x, float in_min, float in_max, float out_min, float out_max) {
// for positive x x< in_min returns out_min
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void update_accel() {
  mpu.readAccelData(aa);  // Read the x/y/z adc values 
  if(count%2==0){
  high_pass();}
  low_pass();
}
void get_Mag() {
  aMag=mapF(constrain(sqrt(((max(sx,0))+max(sy,0)+max(sz,0))),3,10),3.0,10.0,0,255);
}

void high_pass() {
  // high pass filter raw acc readings
  ax=constrain(aa[0]/100,-128,127);
  ay=constrain(aa[1]/100,-128,127);
  az=constrain(aa[2]/100,-128,127);
  Hc++;
  if (Hc>=NHIST) {Hc=0;}
  Tx-=Hx[Hc];
  Hx[Hc]=ax;
  Tx+= Hx[Hc];
  Ty-=Hy[Hc];
  Hy[Hc]=ay;
  Ty+= Hy[Hc];  
  Tz-=Hz[Hc];
  Hz[Hc]=az;
  Tz+= Hz[Hc];
  dx=ax-(Tx)/NHIST;
  dy=ay-(Ty)/NHIST;
  dz=az-(Tz)/NHIST;
}
void low_pass() {
  // low pass filters raw acc readings
  Sc++;
  if (Sc>=NSMOOTH) {Sc=0;}
  STx-=Sx[Sc];
  Sx[Sc]=dx;
  STx+= Sx[Sc];
  STy-=Sy[Sc];
  Sy[Sc]=dy;
  STy+= Sy[Sc];  
  STz-=Sz[Sc];
  Sz[Sc]=dz;
  STz+= Sz[Sc];
  sx=(STx)/NSMOOTH;
  sy=(STy)/NSMOOTH;
  sz=(STz)/NSMOOTH;
}


void print_vars() {
//  Serial.print(" aax: "); Serial.print(aa[0]); 
//  Serial.print(" aay: "); Serial.print(aa[1]);
//  Serial.print(" aaz: "); Serial.print(aa[2]); Serial.println(); 

  Serial.print("amag: "); Serial.print(aMag);  Serial.println(); 
  
//  Serial.print("ax: "); Serial.print((int8_t)ax); 
//  Serial.print(" ay: "); Serial.print((int8_t)ay);
//  Serial.print(" az: "); Serial.print((int8_t)az);
  
//  Serial.print(" sx: "); Serial.print((int8_t)sx); 
//  Serial.print(" sy: "); Serial.print((int8_t)sy);
//  Serial.print(" sz: "); Serial.print((int8_t)sz); Serial.println(); 
}

void start_MPU6050() {
  Wire.begin();
  Serial.begin(9600);
  
  // Read the WHO_AM_I register, this is a good test of communication
  uint8_t c = mpu.readByte(MPU6050_ADDRESS, WHO_AM_I_MPU6050);  // Read WHO_AM_I register for MPU-6050
  if (c == 0x68) { // WHO_AM_I should always be 0x68  {   
    mpu.initMPU6050(); Serial.println("MPU6050 initialized for active data mode...."); // Initialize device for active mode read of acclerometer, gyroscope, and temperature
  }
  else
  {
    Serial.print("Could not connect to MPU6050: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
}
