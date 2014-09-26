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
int lightDuration = 5;   //amount of time the light is on for in ms
int exposureLength = 190; //exposure length in ms - hopefully won't hardcode this in the end
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
  
  Serial.begin(9600);
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
  int upFrontDelay = 10;
  delay(upFrontDelay);
  doubleShiftOut(val, val2);
  delay(delayTime);
  doubleShiftOut(0,0);
  int remainderTime = exposureLength-delayTime-upFrontDelay;
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
      //startIt = 0;
      dipDelay(0,2,lightDuration);
      //digitalWrite(shutter_Release,HIGH);
      break;
    case 1:
      //digitalWrite(shutter_Release,HIGH);
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
  Serial.println("State: One Shot"); 
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
  //Serial.println("State: Shooting Sequence");
  //delay(500);
  int cases = 11;
  
  digitalWrite(shutter_Release,HIGH);
  delay(200);   
  count = 0;
  
  while(count< cases){
    setRing(count);
    int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
    while(!hotShoeHigh){
      hotShoeHigh = !digitalRead(hotShoe_Input); 
    }
    count = count++;
    if (count==(cases-1)) digitalWrite(shutter_Release,LOW);
     
  }
  
  count=0;
}

/////////////////////////////////////////////////////////////////////
////////////  Blinks the Diagnostic LED for 2 seconds //////////////
/////////////////////////////////////////////////////////////////////

void coolDown(int secs){
  Serial.println("State: Cooling Down");
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
  
  //Serial.println("State: Idle"); 
  
  int remoteHigh  = !digitalRead(remote_Input);    // negated because active low
  int buttonHigh  =  digitalRead(button_Input);    //
  int hotShoeHigh = !digitalRead(hotShoe_Input);   // negated because active low
  
  //allow test shots 
  if( !remoteHigh & !buttonHigh & hotShoeHigh & (count == 0) ) oneShot();
  
  if( count == 0 & (remoteHigh | buttonHigh) ){
     Serial.println("begin sequence");
     shootSequence();
     //delay(500);
     coolDown(2);
     //count = 1;
  } 
  
  

}





//Serial.print("remote: ");
//Serial.println(remoteVal);
//Serial.print("button: ");
//Serial.println(buttonVal);
//Serial.print("hotShoe: ");
//Serial.println(hotShoeVal);
//delay(10);
//if(!remoteVal) oneShot();
//  Serial.println("remote Input: LOW");
  //oneShot();
  //dipDelay(255,255,lightDuration);
//}else Serial.println("button Input: LOW");




// //wait until the hotshoe triggers or the button is pushed
// // ---- add functionality for detecting remote trigger
// //
//
// //if remote or button pressed start counting;
////if(digitalRead(button_Input)==HIGH || digitalRead(remote_Input)==LOW ) {
////  delay(50);
////  if(digitalRead(remote_Input)==LOW) startIt = 1;
////}
//if(digitalRead(button_Input)==HIGH) {
//  delay(50);
//  if(digitalRead(button_Input)==HIGH) startIt = 1;
//}
//
//// if( digitalRead(remote_Input)==HIGH ) {
////   startIt = 1;
//// }else startIt = 0;
// // if (count = 0 and remote or button pressed) or hotshoe triggered (regardless of count), start it or keep going
// // while(digitalRead(hotShoe_Input)==LOW || startIt&&(count==0)){
// // while(digitalRead(button_Input)==HIGH ||digitalRead(hotShoe_Input)==LOW){
// 
// digitalWrite(diagnostic_LED, startIt);
// 
// while(digitalRead(hotShoe_Input)==LOW || startIt){
//   
// //while(digitalRead(hotShoe_Input)==LOW || digitalRead(button_Input)==HIGH){
// //  digitalWrite(diagnostic_LED, startIt);
//  runIt(count);
//  if (count<11) {
//    // keep triggering the shutter
//    if(count<10) {
//      digitalWrite(shutter_Release,HIGH);
//    }else digitalWrite(shutter_Release,LOW);
//    // this will force the shutter to go off, thus force the flash to fire
//    // thus we will re-enter this loop
//    count=count++;
//  }
//  // else if at end of sequence and ring flash first, then count = 0
//  // also turn off shutter_release and wait 2 seconds
//  // blink LED for 2 Seconds
//  //
//  else if (count == 11) {
//    count = 0;
//    digitalWrite(shutter_Release,LOW);
//    startIt = 0;
//    //flash LED/stall for 2 seconds
//    digitalWrite(diagnostic_LED,HIGH);
//    delay(500);
//    digitalWrite(diagnostic_LED,LOW);
//    delay(500);
//    digitalWrite(diagnostic_LED,HIGH);
//    delay(500);
//    digitalWrite(diagnostic_LED,LOW);
//    delay(500);
//  }
// }



