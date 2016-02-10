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

// write power managment code - keep track of 

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
//  tape down / reenforce stuff
//  check LED order
//  create and use header files like a library


///////////////////
//////Options/////
/////////////////

int serialDebug = 1;      //
int oneShotCount = 1;
int lightDuration = 50  ;   //amount of time the light is on for in ms
      // don't make it greater than 50 or else the device outputs smoke.

//int exposureLength = 190; //exposure length in ms - hopefully won't hardcode this in the end
// T3i shoots at 5FPS, so a little less than 200ms is the max time in theory for that camera
// however we also add debouncing time in the main loop, so look out for that.


/////////////////////////
//////Pin Declarations///
/////////////////////////

////Inputs////
const int remote_Input = A1;  // input for the remote control (2.5mm jack)
const int button_Input = A0;  // 
const int hotShoe_Input = A5; // 3.5mm input on top that connects to PC-Sync Cable from Flash
//////////////

////Outputs////
const int shutter_Release = 2;  // shutter release output (3.5mm to 2.5mm)
const int diagnostic_GLED = 3; //Active Low
const int diagnostic_RLED = 5; //Active Low
const int diagnostic_BLED = 13; //Active Low

////Pin assignments for each LED////
//unsigned char LEDPins[10] = {A3, 4,11,10,7, 6,9, 8,12, A4}; 

unsigned char LEDPins[10] = {A3, 6,7,8,12,11,10,9,A4,4}; 
unsigned char numOfLEDs = 10;  //Indexed at 0

//////////////////////////////
//////Some Global Variables///
//////////////////////////////

int count = 1;
int startIt = 0;

void setDiagnosticLED(int R, int G, int B){
  int compR = 255 - R;
  int compG = 255 - G;
  int compB = 255 - B;
  analogWrite(diagnostic_RLED,compR);
  analogWrite(diagnostic_GLED,compG);
  analogWrite(diagnostic_BLED,compB);
};

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
  setDiagnosticLED(255,0,0);
//  delay(500);
//  setDiagnosticLED(0,0,0);
//  oneShot();
//  delay(500);
//  setDiagnosticLED(255,0,0);
//  delay(500);
//  setDiagnosticLED(0,0,0);
//  oneShot();
//  delay(500);
//  setDiagnosticLED(255,0,0);
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
      //delay(lightDuration);
      delay(25);
   for( i=0; i< numOfLEDs; i++){
     digitalWrite(LEDPins[i], LOW);
      };
   };
};


/////////////////////////////////////////////////////////////////////
///// Called when the user hits the shutter button on the camera 
///// and tests the camera/ring light for exposure settings
/////////////////////////////////////////////////////////////////////
void oneShot(){
  if(serialDebug) Serial.println("State: One Shot"); 
  setRing(oneShotCount);
  oneShotCount = oneShotCount++;
  if (oneShotCount>10) oneShotCount=1;
//  setDiagnosticLED(0,128,128); 
//  delay(500);
//  setDiagnosticLED(0,0,0);
//  delay(500);
//  setDiagnosticLED(0,128,128); 
//  delay(2000);// just for debugging
}
///////////////////
// **** theory ****
//
// in the case that the camera is at 1/60sec for exposure
// first flash signal fires, then shutter opens for 16ms, then flash signal is low?
// then it saves to the buffer/card (this takes like 200ms or so?) 
// then it detects that it should fire again


void shootSequence(){
  if(serialDebug) Serial.println("State: Shooting Sequence");
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
      setDiagnosticLED(0,128,128); 
    }
    setDiagnosticLED(0,0,128);
    setRing(count);
    now = millis();
    Xn1 = now - previous;
    CMA = (Xn1+(count*CMA))/(count+1);
    //stopShutterTime = (10.5*CMA) + initialTime; // when should the sequency be done shooting
    stopShutterTime = ((cases-.5)*CMA) + initialTime; // when should the sequency be done shooting
  
    count = count + 1;
    if ( now > stopShutterTime) digitalWrite(shutter_Release,LOW);
    //if (count==(cases)) digitalWrite(shutter_Release,LOW);
    if(serialDebug) {
    
//      Serial.println(Xn1);
//      Serial.println(count);
    }
  }
  count=0;
}


/////////////////////////////////////////////////////////////////////
////////////  Blinks the Diagnostic LED for secs seconds //////////////
/////////////////////////////////////////////////////////////////////

void coolDown(int secs){
  if(serialDebug) Serial.println("State: Cooling Down");

  int numFlashes = (secs*1000)/250;
  int i;
  for (i=0; i<numFlashes; i++){
    setDiagnosticLED(255,0,0);
    delay(250);
    setDiagnosticLED(0,0,255);
    delay(250);
  }
  
}


void takePhoto(int msDelay){
  digitalWrite(shutter_Release,HIGH);
  delay(msDelay);
  digitalWrite(shutter_Release,LOW);
}


//1,2,
void loop() {
  
  if(serialDebug) {
    //Serial.println("State: Idle"); 
    //delay(50);
  }


//  test sequence for calibrating LEDs  
//  while(1){
//    char i;
//    for (i=0; i<10; i++){
//     Serial.println(LEDPins[i]);
//     digitalWrite(LEDPins[i], HIGH);
//     delay(5);
//     digitalWrite(LEDPins[i], LOW);
//     delay(700);
//     digitalWrite(LEDPins[i], HIGH);
//     delay(5);
//     digitalWrite(LEDPins[i], LOW);
//     delay(500);
//    }  
//  }
  
  setDiagnosticLED(0,128,0); //Set it green for Go
  
  int remoteHigh  = !digitalRead(remote_Input);    // negated because active low
  int buttonHigh  = !digitalRead(button_Input);    //
  int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
  //int buttonHigh = 0;
//  while(1){
//    buttonHigh  = digitalRead(button_Input); 
//    Serial.println(buttonHigh);
//    delay(50);
//  }
  
  count=0;
  //allow test shots 
  //if( !remoteHigh & !buttonHigh & hotShoeHigh & (count == 0) ) oneShot();
//  while(1){
//    buttonHigh  = !digitalRead(button_Input);
//    remoteHigh  = !digitalRead(remote_Input);    // negated because active low
//  
//  if(buttonHigh) {
//    Serial.println("button high");
//    delay(50);
//   };
//   if(remoteHigh) {
//    Serial.println("remote high");
//    delay(50);
//   };
//  }
  
  if( count == 0 & (remoteHigh | buttonHigh) ){
     if(serialDebug) Serial.println("begin sequence");
     //Serial.println(buttonInput)
     shootSequence();
     coolDown(3);
  } 

  // add serial 

  int incomingByte = 0; 
  if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
//                Serial.print("I received: ");
//                Serial.println(incomingByte, DEC);
            //if rec = "1"
            if(incomingByte == 49){
              shootSequence();
              Serial.println("endSerialSeq");
            }
            //if rec = "2"
            if(incomingByte == 50){
              takePhoto(250);
              Serial.println("endTakePhoto");
              Serial.println("endTakePhoto2");
              
            }
        }

  
}




