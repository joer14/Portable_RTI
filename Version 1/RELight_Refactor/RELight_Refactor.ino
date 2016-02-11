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

// out of place thought:
// consider using current over driving with PWM by PWM ~OE on the shift registers

///////////////////
//////Options/////
/////////////////

int serialDebug = 1;      //
int lightDuration = 75;   //amount of time the light is on for in ms
//int exposureLength = 190; //exposure length in ms - hopefully won't hardcode this in the end
// T3i shoots at 5FPS, so a little less than 200ms is the max time in theory for that camera
// however we also add debouncing time in the main loop, so look out for that.

// Ring Flash Mode
// 1 => fire all of the lights on the first photo (total of 11 photos)
// 0 => 1 LED per photo, 10 photos.
int ringFlashFirst = 1;


/////////////////////////
//////Pin Declarations///
/////////////////////////

////Shift Registers (Outputs) ///
int latchPin = 11;
int clockPin = 10;
int dataPin = 12;
/////////////////////////////////

////Inputs////
const int hotShoe_Input = 9;
const int button_Input = 8;
const int remote_Input = 6;  // input for the remote control (2.5mm jack)
//////////////

///Outputs////
const int diagnostic_LED = 13;
const int shutter_Release = 7;  // shutter release output (3.5mm to 2.5mm)
/////////////

///Some Global Variables////
int count = 1;
int startIt = 0;



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
 
  doubleShiftOut(0,0);

  if(ringFlashFirst){
    count=0;
  };
  
  
  digitalWrite(shutter_Release,LOW);
  
  if(serialDebug) Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only
  //}


// Test Shutter Release  
//  delay(1000);
//  digitalWrite(shutter_Release,HIGH);
//  delay(1000);
//  digitalWrite(shutter_Release,LOW);
//   
}

/////////////////////////////////////////////////
////Shifts byte Values out to shift registers////
/////////////////////////////////////////////////

void doubleShiftOut(int val, int val2){
  //val =0b11111111;
  // First output the second shift register value, then the first one
  // since we are dealing with serial data
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, val2);
  shiftOut(dataPin, clockPin, MSBFIRST, val);
  digitalWrite(latchPin, HIGH);

};

void dipDelay( int val, int val2, int delayTime){
  int upFrontDelay = 0;
  delay(upFrontDelay);
  doubleShiftOut(val, val2);
  delay(delayTime);
  doubleShiftOut(0,0);
  //int remainderTime = exposureLength-delayTime-upFrontDelay;
  //delay(remainderTime);
};


///////////////////////////////////////////////////////
/////// 1. Displays a given state's value on        ///
///////    the ring light for light duration        ///
/////// 2. turns off LED and waits until expsoure   ///
///////    length has finished. Then returns        ///
///////////////////////////////////////////////////////

void setRing(int LEDstate){
  
    switch (LEDstate) {
     
    case 0:
      //dipDelay(0,2,lightDuration);
      // Useful to display alt light 1 for debugging
      //dipDelay(31,255,lightDuration);
      
      dipDelay(32,0,lightDuration);
      
      break;
    case 1:
      dipDelay(0,1,lightDuration);
      break;
    case 2:
      //dipDelay(2,0,lightDuration);
      dipDelay(64,0,lightDuration);
      
      break;
    case 3:
      dipDelay(4,0,lightDuration);
      break;
    case 4:
      dipDelay(8,0,lightDuration);
      break;
    case 5:
      //dipDelay(16,0,lightDuration);
      dipDelay(0,2,lightDuration);
      
      break;
    case 6:
      //dipDelay(32,0,lightDuration);
      dipDelay(2,0,lightDuration);
      
      break;
    case 7:
      //dipDelay(64,0,lightDuration);
      dipDelay(128,0,lightDuration);
      
      break;
    case 8:
      //dipDelay(128,0,lightDuration);
      dipDelay(1,0,lightDuration);
      
      break;
    case 9:
      //dipDelay(0,1,lightDuration);
      dipDelay(16,0,lightDuration);
      
      break;
    case 10:
      dipDelay(255,255,lightDuration);
      break;
    default:
      dipDelay(0,0,lightDuration);
  };
};

/////////////////////////////////////////////////////////////////////
///// Called when the user hits the shutter button on the camera 
///// and tests the camera/ring light for exposure settings
/////////////////////////////////////////////////////////////////////
void oneShot(){
  if(serialDebug) Serial.println("State: One Shot"); 
  setRing(0);//turn all LEDs on
  //delay(2000);// just for debugging
}
///////////////////
// **** theory ****
//
// in the case that the camera is at 1/60sec for exposure
// first flash signal fires, then shutter opens for 16ms, then flash signal is low?
// then it saves to the buffer/card (this takes like 200ms or so?) 
// then it detects that it should fire again


void shootSequence(){
  //if(serialDebug) Serial.println("State: Shooting Sequence");
  //delay(500);
  //int cases = 11;
  //Set cases to 12 to have a dead state at the end for no LED exposure
  int cases = 12;
  unsigned long initialTime = millis();
  unsigned long CMA = 0;  // current Cummulative Moving Average (elapsed time between captures) 
  unsigned long CMAn1 = 0; // n+1 cummulative Moving Average
  unsigned long Xn1 = 0;   // most recent delay
  unsigned long n = 0;     // current state, ranges from 0 to 10;
  //unsigned long currentExpectedTermination = (11*CMAn1) + initialTime; // when should the sequency be done shooting
  //unsigned long stopShutterTime = (10.5*CMAn1) + initialTime; // when should the sequency be done shooting
  unsigned long currentExpectedTermination = (cases*CMAn1) + initialTime; // when should the sequency be done shooting
  unsigned long stopShutterTime = ((cases-.5)*CMAn1) + initialTime; // when should the sequency be done shooting
  
  unsigned long now;
  unsigned long previous;
  
  digitalWrite(shutter_Release,HIGH);
  //setRing(0);
  //delay(150);   
  count = 0;
  
  while(count< cases){
    previous = millis();
    int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
    while(!hotShoeHigh){
      hotShoeHigh = !digitalRead(hotShoe_Input); 
    }
    setRing(count);
    now = millis();
    Xn1 = now - previous;
    CMA = (Xn1+(count*CMA))/(count+1);
    //stopShutterTime = (10.5*CMA) + initialTime; // when should the sequency be done shooting
    stopShutterTime = ((cases-.5)*CMA) + initialTime; // when should the sequency be done shooting
  
    count = count++;
    if ( now > stopShutterTime) digitalWrite(shutter_Release,LOW);
    //if (count==(cases)) digitalWrite(shutter_Release,LOW);
    if(serialDebug) Serial.println(Xn1);
  }
  
  count=0;
}


/////////////////////////////////////////////////////////////////////
////////////  Blinks the Diagnostic LED for 2 seconds //////////////
/////////////////////////////////////////////////////////////////////

void coolDown(int secs){
  if(serialDebug) Serial.println("State: Cooling Down");
  doubleShiftOut(0,0);                  //clear Display just in case it isn't
  int numFlashes = (secs*1000)/250;
  int i;
  for (i=0; i<numFlashes; i++){
    digitalWrite(diagnostic_LED,HIGH);
    delay(250);
    digitalWrite(diagnostic_LED,LOW);
    delay(250);
  }
  
}

void loop() {
  
  //if(serialDebug) Serial.println("State: Idle"); 
  
  int remoteHigh  = !digitalRead(remote_Input);    // negated because active low
  int buttonHigh  =  digitalRead(button_Input);    //
  int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
  
  //allow test shots 
  if( !remoteHigh & !buttonHigh & hotShoeHigh & (count == 0) ) oneShot();
  
  if( count == 0 & (remoteHigh | buttonHigh) ){
     if(serialDebug) Serial.println("begin sequence");
     shootSequence();
     //delay(500);
     coolDown(2);
     //count = 1;
  } 
  
  

}





//if(serialDebug) Serial.print("remote: ");
//if(serialDebug) Serial.println(remoteVal);
//if(serialDebug) Serial.print("button: ");
//if(serialDebug) Serial.println(buttonVal);
//if(serialDebug) Serial.print("hotShoe: ");
//if(serialDebug) Serial.println(hotShoeVal);
//delay(10);
//if(!remoteVal) oneShot();
//  if(serialDebug) Serial.println("remote Input: LOW");
  //oneShot();
  //dipDelay(255,255,lightDuration);
//}else if(serialDebug) Serial.println("button Input: LOW");



