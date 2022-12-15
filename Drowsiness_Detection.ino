/*Abram Huang
  ENME 351 Lab 9 Fall 2022
  *Sources*
  ADLX335 Data Collection Tutorial:https://lastminuteengineers.com/adxl335-accelerometer-arduino-tutorial/
  ADLX335 Pitch Calculation:https://www.electronicwings.com/arduino/adxl335-accelerometer-interfacing-with-arduino-uno

*/


#include <math.h>
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
const int buzzer=2;
const int off=3;
const int pot=3;
const int reset=7;
const int pedal=5;;
const int A=10; //pink
const int A_prime=9; //orange
const int B=11; //yellow
const int B_prime=12; //blue
int totalSteps=2048;
int prevStep=0;
int potValue=0;
int xPos=0;
int press=0;//stores button state
int xCenter=605;//stores center position of car
bool disablePot=false;//enables and disable the potentiometers during turn correction
bool correctLeft=false;//true if car crosses right boundary
bool correctRight=false;//true if car crosses left boundary

String guiInputs=""; //stores inputs that will be sent to processing


// initialize minimum and maximum Raw Ranges for each axis
int RawMin = 0;
int RawMax = 1023;

// Take multiple samples to reduce noise
const int sampleSize = 10;
// Take samples and return the average
int ReadSamples(int pin)
{
	long reading = 0;
	analogRead(pin);
	delay(1);
	for (int i = 0; i < sampleSize; i++)
	{
	  reading += analogRead(pin);
	}
	return reading/sampleSize;
}

// Checks if car is leaving lane, alerts driver, and locks wheel.
void checkLane()
{
  if(xPos<480)
  {
    tone(buzzer,50);
    correctRight=true;
    disablePot=true;
   
  }
  else if(xPos>765)
  {
    tone(buzzer,50);
    correctLeft=true;
    disablePot=true;
  }
  
}
// Helper function for nonblocking delays
void millisDelay()
{
  long currentTime=millis();
  while(millis()<currentTime+2.5)
  {

  }
}

// Wave Drive Clockwise rotation
void clockwise()
{
  digitalWrite(A,HIGH);
  millisDelay();
  digitalWrite(A,LOW);
  digitalWrite(B,HIGH);
  millisDelay();
  digitalWrite(B,LOW);
  digitalWrite(A_prime,HIGH);
  millisDelay();
  digitalWrite(A_prime,LOW);
  digitalWrite(B_prime,HIGH);
  millisDelay();
  digitalWrite(B_prime,LOW);
}
//Wave Drive Counterclockwise rotation
void counterclockwise()
{
  digitalWrite(A_prime,HIGH);
  millisDelay();
  digitalWrite(A_prime,LOW);
  digitalWrite(B,HIGH);
  millisDelay();
  digitalWrite(B,LOW);
  millisDelay();
  digitalWrite(A,HIGH);
  millisDelay();
  digitalWrite(A,LOW);
  digitalWrite(B_prime,HIGH);
  millisDelay();
  digitalWrite(B_prime,LOW);
}
//Rotates CW by a number of specified steps
void rotateCW(int steps){
  for(int i=0;i<steps;i++)
  {
    clockwise();
    
  }
}
//Rotates CCW by a number of specified steps
void rotateCCW(int steps){
  for(int i=0;i<steps;i++)
  {
    counterclockwise();
    
  }
}

void setup() 
{
  pinMode(pedal,INPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(off,INPUT_PULLUP);
  pinMode(reset,INPUT_PULLUP);
  pinMode(xInput,INPUT);
  pinMode(yInput,INPUT);
  pinMode(zInput,INPUT);
  pinMode(LED_BUILTIN, OUTPUT); //used for testing

  pinMode(A,OUTPUT);
  pinMode(A_prime,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(B_prime,OUTPUT);
	Serial.begin(115200);
}


void loop() 
{
  
  String guiInputs="";
	//Read raw values
  analogReference(EXTERNAL); //Accelerometer reads using 3.3 V reference
	int xRaw = ReadSamples(xInput);
	int yRaw = ReadSamples(yInput);
	int zRaw = ReadSamples(zInput);
  analogReference(DEFAULT);

	// Convert raw values to 'milli-Gs"
	long xScaled = map(xRaw, RawMin, RawMax, -3000, 3000);
	long yScaled = map(yRaw, RawMin, RawMax, -3000, 3000);
	long zScaled = map(zRaw, RawMin, RawMax, -3000, 3000);

	// re-scale to fractional Gs
	float xAccel = xScaled / 1000.0;
	float yAccel = yScaled / 1000.0;
	float zAccel = zScaled / 1000.0;
  potValue=ReadSamples(pot);//averages potentiomer ADC data to reduce noise

  guiInputs+=(String)potValue+ "\t";

  //if wheel is not locked, reads difference in position of wheel and updates car position accordingly to a CW or CCW rotation of the wheel
  if(disablePot==false){
    xPos=map(potValue,0,1023,445,785);
    int stepValue=map(potValue,28,995,0,totalSteps);
    int diff=stepValue-prevStep;
    prevStep=stepValue;
    if(diff>2)
    {
      rotateCW(10); 
    }
    else if(diff<-2)
    { 
      rotateCCW(10);
    }
  }
  //Check if car leaves lane and updates conditions
  checkLane();
  
  //If car leaves lane to the right, locks driving wheel and applies correction left turn.
  if (correctLeft) 
  {
    disablePot=true;
    rotateCCW(90);
    xPos=xCenter;
    correctLeft=false;
    
  }
  //If car leaves lane to the left, locks driving wheel and applies correction turn.
  if (correctRight)
  {
    disablePot=true;
    rotateCW(90);
    xPos=xCenter;
    correctRight=false;
  }
  //Stops correction rotation
  if(xPos==xCenter)
  {
    correctLeft=false;
    correctRight=false;
  }
  // Button for user toggle off lane keep assist and alarm.
  int resetInput=digitalRead(reset);
  if(resetInput==0){
    disablePot=false;
    xPos=xCenter;
    noTone(buzzer);
  }
  guiInputs+=(String)resetInput+ "\t";
  // Setting for optional adjustment of speed, used for testing
  int speedPot=ReadSamples(pedal);
  guiInputs+=(String)speedPot+ "\t";

  long pitch =(((atan2(zAccel,xAccel) * 180) / 3.14 ) + 180 ); /* Formula for pitch */

  int headDrop=0; //stores state head position drops
 
  if((pitch>240 && pitch<350)|| pitch<10){// Stores a general estimate of ranges through trial and error in which a 30 degree drop in pitch is sensed, turns alarm on
    tone(buzzer,50);
    headDrop=1;
    digitalWrite(LED_BUILTIN,HIGH);
    
  }
  else{
    headDrop=0;
  }
  guiInputs+=(String)headDrop+"\t";
  //Button to turn off drowsiness detection
  if(digitalRead(off)==LOW){
  
    noTone(buzzer);
    digitalWrite(LED_BUILTIN,LOW);
  }
  Serial.println(guiInputs);
  
}




