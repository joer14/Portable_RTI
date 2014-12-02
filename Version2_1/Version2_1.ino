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


//TODO
//  Play with button code more and see if we need to solder it to +5V instead of ground...
//  fix extra shot at end
//  clean up code
//  change some vars to #defines
//  add LED status - make a function to make this easier?
    //  green is ready to shoot
    //  blue is shooting actively
    //  red alt blue is cooling down

//  fix button, add functionality to it?
//    maybe shoot and entire sequence or door a cool pattern 

///////////////////
//////Options/////
/////////////////

int serialDebug = 1;      //
int lightDuration = 50;   //amount of time the light is on for in ms
//int exposureLength = 190; //exposure length in ms - hopefully won't hardcode this in the end
// T3i shoots at 5FPS, so a little less than 200ms is the max time in theory for that camera
// however we also add debouncing time in the main loop, so look out for that.


/////////////////////////
//////Pin Declarations///
/////////////////////////

////Inputs////
const int remote_Input = A0;  // input for the remote control (2.5mm jack)
const int button_Input = A1;  // 
const int hotShoe_Input = A5; // 3.5mm input on top that connects to PC-Sync Cable from Flash
//////////////

////Outputs////
const int shutter_Release = 2;  // shutter release output (3.5mm to 2.5mm)
const int diagnostic_GLED = 3; //Active Low
const int diagnostic_RLED = 5; //Active Low
const int diagnostic_BLED = 13; //Active Low

//High Power LEDs//

//const int LED01 = A3;
//const int LED02 = A4;
//const int LED03 =  4;
//const int LED04 =  6;
//const int LED05 =  7;
//const int LED06 =  8;
//const int LED07 =  9;
//const int LED08 =  10;
//const int LED09 =  11;
//const int LED10 =  12;

unsigned char LEDPins[10] = {A3, A4, 4, 6, 7, 8,9,10,11,12}; 
unsigned char numOfLEDs = 10;  //Indexed at 0

//////////////////////////////
//////Some Global Variables///
//////////////////////////////

int count = 1;
int startIt = 0;

void setup() {
  
  //Setup Inputs
  pinMode(hotShoe_Input, INPUT);
  pinMode(remote_Input, INPUT);
  pinMode(button_Input, INPUT);
  
  //Pull up the remote and button
  digitalWrite(remote_Input,HIGH);
  digitalWrite(button_Input,HIGH);
  digitalWrite(hotShoe_Input,HIGH);
  
  //Status LED Outputs
  pinMode(diagnostic_RLED, OUTPUT);
  pinMode(diagnostic_RLED, OUTPUT);
  pinMode(diagnostic_RLED, OUTPUT);
  
  //Display Initially a Red LED
  digitalWrite(diagnostic_RLED, LOW);
  digitalWrite(diagnostic_RLED, HIGH);
  digitalWrite(diagnostic_RLED, HIGH);
  
  //Setup Shutter Release Output
  pinMode(shutter_Release, OUTPUT);  
  digitalWrite(shutter_Release,LOW);
  
  //Setup all the high powered LEDs as outputs and turn them off
  unsigned char i;
  for( i=0; i< numOfLEDs; i++){
    pinMode(LEDPins[i],OUTPUT);
    digitalWrite(LEDPins[i], LOW);
  };
  
  if(serialDebug) Serial.begin(9600);
  
  
   
}

void dipDelay( int val, int val2, int delayTime){
  int upFrontDelay = 0;
  delay(upFrontDelay);
  //doubleShiftOut(val, val2);
  delay(delayTime);
  //doubleShiftOut(0,0);
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
   unsigned char i;
   if (LEDstate < 10){
        digitalWrite(LEDPins[LEDstate], HIGH);
        delay(lightDuration);
        digitalWrite(LEDPins[LEDstate], LOW);
   }else if(LEDstate == 10) {
     for( i=0; i< numOfLEDs; i++){
       digitalWrite(LEDPins[i], HIGH);
      };
      delay(lightDuration);
   for( i=0; i< numOfLEDs; i++){
     digitalWrite(LEDPins[i], LOW);
      };
   };
};

void setDiagnosticLED(int R, int G, int B){
  int compR = 255 - R;
  int compG = 255 - G;
  int compB = 255 - B;
  analogWrite(diagnostic_RLED,compR);
  analogWrite(diagnostic_GLED,compG);
  analogWrite(diagnostic_BLED,compB);
};

/////////////////////////////////////////////////////////////////////
///// Called when the user hits the shutter button on the camera 
///// and tests the camera/ring light for exposure settings
/////////////////////////////////////////////////////////////////////
void oneShot(){
  if(serialDebug) Serial.println("State: One Shot"); 
  setRing(10);
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

  int numFlashes = (secs*1000)/250;
  int i;
  for (i=0; i<numFlashes; i++){
    //digitalWrite(diagnostic_LED,HIGH);
    delay(250);
    //digitalWrite(diagnostic_LED,LOW);
    delay(250);
  }
  
}

void testLED(){
  if(serialDebug) Serial.println("State: Test LED");
  delay(4000);
}

void loop() {
  
  if(serialDebug) {
    //Serial.println("State: Idle"); 
    //delay(50);
  }
  while(1){
    setDiagnosticLED(255,0,0);
    delay(1000);
    setDiagnosticLED(0,255,0);
    delay(1000);
    setDiagnosticLED(0,0,255);
    delay(1000);
  
  };
  int remoteHigh  = !digitalRead(remote_Input);    // negated because active low
  int buttonHigh  =  !digitalRead(button_Input);    //
  int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
  count=0;
  //allow test shots 
  if( !remoteHigh & !buttonHigh & hotShoeHigh & (count == 0) ) oneShot();
  //if(buttonHigh) testLED();
  
  if( count == 0 & (remoteHigh | buttonHigh) ){
     if(serialDebug) Serial.println("begin sequence");
     shootSequence();
     //delay(500);
     coolDown(2);
     //count = 1;
  } 
  
  int fade =5;
  int brightness = 0;
    
//  while(1){
//      unsigned char i;
//      for( i=0; i< numOfLEDs; i++){
//        Serial.println(LEDPins[i]);
//        digitalWrite(LEDPins[i], HIGH);
//        delay(50);
//        digitalWrite(LEDPins[i], LOW);
//        delay(2000);  
//    };
//    analogWrite(diagnostic_RLED,255-fade);
//    analogWrite(diagnostic_BLED,fade);
//    analogWrite(diagnostic_GLED,fade/2);
//   
//    int a = 255-brightness;
//    int b = brightness;
//    int c = brightness/2;
//    analogWrite(diagnostic_RLED,255);
//    analogWrite(diagnostic_BLED,c);
//    analogWrite(diagnostic_GLED, a);
//    
//    brightness = brightness + fade;
//    if (brightness == 0 || brightness == 255) {
//      fade = -fade ; 
//    }  
//    delay(300);
//    
//    
//    
//    if (brightness  = 120){
//      unsigned char i;
//      analogWrite(diagnostic_RLED,0);
//      analogWrite(diagnostic_BLED,255);
//      analogWrite(diagnostic_GLED,255);
//      delay(250);
//    
//      for( i=0; i< numOfLEDs; i++){
//        //pinMode(LEDPins[i],OUTPUT);
//        digitalWrite(LEDPins[i], HIGH);
//      };
//      //digitalWrite(4,HIGH);
//      delay(50);
//      for( i=0; i< numOfLEDs; i++){
//        //pinMode(LEDPins[i],OUTPUT);
//        digitalWrite(LEDPins[i], LOW);
//      };
//      analogWrite(diagnostic_RLED,255);
//      analogWrite(diagnostic_BLED,c);
//      analogWrite(diagnostic_GLED,a);
//      
//      delay(3000);
//    }
// nbh  }
  
  

}




