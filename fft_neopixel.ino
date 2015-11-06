/*
This Arduino program intends to display sound frequency on a neopixel strip, 
higher pitch correlates to a higher color frequency as 
well as being placed further down the strip. 

Designed to get input from a MAX9814 Microphone amplifier made by adafruit.

Written by Eric Smithson
*/


//FFT defines
#define LIN_OUT 1 // use the lin output function
#define FFT_N 128 // set to 128 point fft

//neopixel defines
#define PIN 6
#define MIC_PIN A0
#define B_FREQ 12 //bottom frequency - int between 0 and FFT_N/2
#define T_FREQ 30 //top frequency - int between 0 and FFT_N/2
#define PIX_NUM 29



//include libraries
#include <Adafruit_NeoPixel.h>
#include <FFT.h> // include the library

//define matricies (gives memory errors if you don't define them up here)
int height[PIX_NUM],
    noise[64]={8,6,6,5,3,4,4,4,3,4,4,3,2,3,3,4,
              2,1,2,1,3,2,3,2,1,2,3,1,2,3,4,4,
              3,2,2,2,2,2,2,1,3,2,2,2,2,2,2,2,
              2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,4 },
    eq[64]={
    255, 175,218,225,220,198,147, 99, 68, 47, 33, 22, 14,  8,  4,  2,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
    


//strip parameters
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIX_NUM, PIN, NEO_RGB + NEO_KHZ800);




void setup() {
  Serial.begin(115200); // use the serial port
  strip.begin(); //begins strip
  strip.show(); //blanks pixels
  memset(height, 0, sizeof(height));
}

void loop() {
  int i,k,maxfreq=0,maxfreqbin = 0,brightness,secondmax = 0;
  //int height[strip.numPixels()];
  
  for (i = 0 ; i < FFT_N ; i++) { // create 128 samples
    
    //read mic
    k = analogRead(A0);
    
    fft_input[2*i] = k; // put real data into even bins
    fft_input[2*i+1] = 0; // set odd bins to 0
  }
  
  //prints off the last microphone value
  Serial.println(k);
  
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_lin(); // take the output of the fft
  
  
  //observes a range of frequency indexes from B_FREQ to T_FREQ
  for (i= B_FREQ; i< T_FREQ; i++) {
    
    // print the frequency index and amplitudes
    Serial.print(i);
    Serial.print(" ");
    Serial.print(fft_lin_out[i]);
    Serial.print(" ");
    Serial.println(fft_lin_out[i]-noise[i]);
    
    
    //gets the frequency index with the highest amplitude
    if (maxfreq < max(fft_lin_out[i],maxfreq)){
      //secondmax = maxfreq;
      maxfreq = fft_lin_out[i];
      maxfreqbin = i;
    }
    
  }
  
  //prints frequency index with highest amplitude
  Serial.print(maxfreqbin);
  Serial.print(" ");
  Serial.print(maxfreq);
  Serial.println("Done");
  
  
  
  //reduces every pixel's brightness by 1
  for(int i=0; i<strip.numPixels(); i++) {
    if(height[i]>0){
      height[i]-= 25;
    }
  }
  
  //sets the height of the frequency index with the highest amplitude to max brightness
  height[map(maxfreqbin,B_FREQ,T_FREQ,0,strip.numPixels())] =+ 200;
  for(i=0;i<=(map(maxfreqbin+1,B_FREQ,T_FREQ,0,strip.numPixels())-map(maxfreqbin,B_FREQ,T_FREQ,0,strip.numPixels()));i++){
    height[map(maxfreqbin,B_FREQ,T_FREQ,0,strip.numPixels())+i] =+ 150;
  }
 
    //lightPixel(map(maxfreqbin, B_FREQ ,T_FREQ,0,strip.numPixels()),255);
    
    for(i=0;i<strip.numPixels();i++){
    lightPixel(i,height[i]);
    }
}

void lightPixel(int i, int brightness) {
  strip.setPixelColor(i, Wheel(map(i,0,strip.numPixels(),0,255), brightness));
  strip.show();
}

uint32_t Wheel(byte WheelPos, uint8_t brightness) {
  WheelPos = 255 - WheelPos;
  //int brightness =50;
  if(WheelPos < 85) {
   return strip.Color(0, (255-WheelPos * 3)*brightness/255 , (WheelPos * 3)*brightness/255);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color((WheelPos * 3)*brightness/255, 0, (255-WheelPos * 3)*brightness/255);
  } else {
   WheelPos -= 170;
   return strip.Color((255-WheelPos * 3)*brightness/255, (WheelPos * 3)*brightness/255, 0);
  }
}


