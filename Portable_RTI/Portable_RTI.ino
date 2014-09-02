// Code for a prototype portable RTI rig
// Joe Rowley, August 2014 for Prof. James Davis
// at the University of California, Santa Cruz. 
// Using an Arduino Micro, 2 8-bit Shift Registers
// 10 mosfets, 10 Cree MRK 12V, 700ma LEDs
// 12V Regulator, 4 CR123A 3.2V batteries
// I should have some constant current limited but don't
// [really don't do that... it's a bad idea not to have a limiter on it]
// 3.5mm Female jack is used as an on/off switch and as an input for the hotshoe
// Contact joer14@gmail.com if you have any questions. 

///////////////////
//////Options/////
/////////////////

int lightDuration = 50;
// How long do you want the LEDs to be on for, measured in milliseconds 
// T3i shoots at 5FPS, so a little less than 200ms is the max time in theory for that camera
// however we also add debouncing time in the main loop, so look out for that. 

// Ring Flash Mode
// 1 => fire all of the lights on the first photo (total of 11 photos)
// 0 => 1 LED per photo, 10 photos. 
int ringFlashFirst = 1;



int latchPin = 11;
int clockPin = 10;
int dataPin = 12;
const int HotShoe_Pin = 9;
const int buttonPin = 8;
int count = 1;
const int LED_Pin = 13;


void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(HotShoe_Pin, INPUT);
  pinMode(LED_Pin, OUTPUT);
  digitalWrite(HotShoe_Pin,HIGH);  
  digitalWrite(LED_Pin, LOW);
  
  //Zero everything so you don't have any lights stuck on. 
  //
  dip(0,0);
  
  if(ringFlashFirst){
    count=0;
  }
}


void dip(int val, int val2){
  //val =0b11111111;
  //First output the second shift register value, then the first one.
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, val2);
  shiftOut(dataPin, clockPin, MSBFIRST, val);
  
  digitalWrite(latchPin, HIGH);
};

void dipDelay( int val, int val2, int delayTime){
  dip(val, val2);
  delay(delayTime);
  dip(0,0);
};


void loop() {
 
 //wait until the hotshoe triggers or the button is pushed
  
 while(digitalRead(HotShoe_Pin)==LOW || digitalRead(buttonPin)==HIGH){
 
   // I use a switch because it is simple and easy to play with,
   // but really could use a loop, bit shifting,
   // any number of more elegant solutions. 
   
   switch (count) {
    //display zero
    case 0:
      dipDelay(255,255,50);
      delay(200);
    break;
    case 1:
      dipDelay(1,0,lightDuration);
      break;
    case 2:
      dipDelay(2,0,lightDuration);
      break;
    case 3:
      dipDelay(4,0,lightDuration);
      break;
    case 4:
      dipDelay(8,0,lightDuration);
      break;
    case 5:
      dipDelay(16,0,lightDuration);
      break;
    case 6:
      dipDelay(32,0,lightDuration);
      break;
    case 7:
      dipDelay(64,0,lightDuration);
      break;
    case 8:
      dipDelay(128,0,lightDuration);
      break;
    case 9:
      dipDelay(0,1,100);
      break;
    case 10:
      dipDelay(0,2,lightDuration);
      break;
    
    default: 
      dipDelay(0,0,lightDuration);
  }
  if (count<11) {
    count=count++;
  }
  //debounce delay 
  //50 ms delay for debouncing. might not be necessary. 
  delay(50);
  
  //if count ==11 and ring flash first, then count = 0;
  if(count == 11) count = (ringFlashFirst) ? 0 : 1;
 };
}
