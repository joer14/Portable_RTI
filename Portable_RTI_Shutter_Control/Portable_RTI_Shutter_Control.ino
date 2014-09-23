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

// I should use an opto-isolator to trigger the flash probably but really need to...
// more info on wiring... http://www.glacialwanderer.com/hobbyrobotics/?p=13

///////////////////
//////Options/////
/////////////////

int lightDuration = 75;
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
const int hotShoe_Input = 9;
const int button_Input = 8;
int count = 1;
int startIt = 0;

//diagnostic LED
const int diagnostic_LED = 13;

// input for the remote control (2.5mm jack)
// optional - can trigger simply using button or actual camera shutter

const int remote_Input;

//Output - shutter release output (3.5mm to 2.5mm)
const int shutter_Release;


void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(hotShoe_Input, INPUT);
  pinMode(diagnostic_LED, OUTPUT);
  pinMode(remote_Input, INPUT);
  pinMode(shutter_Release, OUTPUT);
  digitalWrite(hotShoe_Input,HIGH);
  digitalWrite(diagnostic_LED, LOW);

  //Zero everything so you don't have any lights stuck on.
  //
  dip(0,0);

  if(ringFlashFirst){
    count=0;
  }
}


void dip(int val, int val2){
  //val =0b11111111;
  // First output the second shift register value, then the first one
  // Since we are dealing with serial data
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
 // ---- add functionality for detecting remote trigger
 //

 //if remote or button pressed start counting;
 if(digitalRead(button_Input)==HIGH || digitalRead(remote_Input)==HIGH ) startIt = 1;

 // if (count = 0 and remote or button pressed) or hotshoe triggered (regardless of count), start it or keep going
 while(digitalRead(hotShoe_Input)==LOW || startIt&&(count==0)){

   switch (count) {
    case 0:
      dipDelay(255,255,lightDuration);
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
      dipDelay(0,1,lightDuration);
      break;
    case 10:
      dipDelay(0,2,lightDuration);
      break;
    default:
      dipDelay(0,0,lightDuration);
  }
  if (count<11) {
    // keep triggering the shutter
    shutter_Release = HIGH;
    // this will force the shutter to go off, thus force the flash to fire
    // thus we will re-enter this loop
    count=count++;
  }
  // else if at end of sequence and ring flash first, then count = 0
  // also turn off shutter_release and wait 2 seconds
  // blink LED for 2 Seconds
  //
  else if (count == 11) {
    count = (ringFlashFirst) ? 0 : 1;
    shutter_Release = LOW;
    startIt = 0;
    //flash LED/stall for 2 seconds
    Diagnostic_LED = HIGH;
    delay(500);
    Diagnostic_LED = LOW;
    delay(500);
    Diagnostic_LED = HIGH;
    delay(500);
    Diagnostic_LED = LOW;
    delay(500);
  }



}
