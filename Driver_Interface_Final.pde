/*Abram Huang
  ENME 351 Lab 9 Fall 2022
  *Sources*
   Animation Tutorial: http://studio.processingtogether.com/sp/pad/export/ro.9jAqLpHlv-8K3/latest
   Car: Shutterstock
   Fireflower:https://www.pngitem.com/middle/TxhTRo_super-mario-bros-mario-fire-flower-pixel-art/
  
*/        
import processing.serial.*; // add the serial library
Serial myPort; // define a serial port object to monitor

// Define initial coordinates of lane lines
float [] yPos ={0,200,400,600,800,1000};
// Initial positions of flowers
float [] yPosF={100,300,500,700,900,1100};
int VEL_Y=500;//Stores speed
long lTime=0; //Stores lapsed time
//stores all images of car states
PImage flower;
PImage carState;
PImage drive;
PImage blinkers;
PImage brake;
PImage left_Turn;
PImage right_Turn;
final float xCenter=605; //Constant Value for Car;s center position
float xPos=0; //Stores position of car
boolean disablePot; //duplicate variable from Arduino
boolean correctLeft; //duplicate variable from Arduino
boolean correctRight; //duplicate variable from Arduino




void setup(){
  size(1200, 1200); // set the window size
  lTime=millis();
  background(100);
  //grass
  fill(132,192,17);
  rect(0,0,100,height);
  rect(width-100,0,100,height);
  
  //Border Lines
  fill(255);
  rect(150,0,20,height);
  rect(width-170,0,20,height);
  
  //Flowers
  flower=loadImage("flowerImageTransp.png");
  flower.resize(83,100);
  
  drive=loadImage("drive.png");
  blinkers=loadImage("blinkers.png");
  brake=loadImage("brake.png");
  left_Turn=loadImage("left_Turn.png");
  right_Turn=loadImage("right_Turn.png");
  
  xPos=xCenter;
  carState=drive;
  
  disablePot=false;
  correctLeft=false;
  correctRight=false;
  myPort = new Serial(this, Serial.list()[2], 115200); // define input port
  myPort.clear(); // clear the port of any initial junk
 
}

void reset()
{
  background(100);
  //grass
  fill(132,192,17);
  rect(0,0,100,height);
  rect(width-100,0,100,height);
  
  //Border Lines
  fill(255);
  rect(150,0,20,height);
  rect(width-170,0,20,height);
   
}
// Function that controls how fast the car is moving by making everything move relative to it.
void road()
{
  final int cTime=millis();
  final float eTime=1e-3*(cTime-lTime);
  lTime=cTime;
  reset();
  fill(255);
  for(int i=0;i<6;i++)
  {
     yPos[i] = (yPos[i] + VEL_Y*eTime) % height;
     yPosF[i] = (yPosF[i] + VEL_Y*eTime) % height;
     //lane lines
     rect(455,yPos[i],10,100);  
     rect(765,yPos[i],10,100);
     imageMode(CORNER);
     image(flower,5,yPosF[i]);
     image(flower,width-83-5,yPosF[i]);
  }
}

void draw () {
  
  //Taken from 351 Processing Lab
 
  while (myPort.available () > 0) { // make sure port is open
    String inString = myPort.readStringUntil('\n'); // read input string
    road();
    if(inString !=null)
    {
      inString=trim(inString);
      String [] inputs=splitTokens(inString,"\t");
      
      if(inputs.length == 4)
      {
        int pot=int(inputs[0]);
        int button=int(inputs[1]);
        int speedPot=int(inputs[2]);
        int headDrop=int(inputs[3]);
        // If driver is alert, takes user input from pedal
        if(headDrop==0){
           VEL_Y=(int)map(speedPot,0,1023,300,800);
           carState=drive;  
        }
        // If driver is drowsy, locks driving wheel and slows vehicle and turns on blinkers
        if(headDrop==1 && VEL_Y>100)
        {
          VEL_Y-=20;
          carState=blinkers;
        }
        // Button to reset drowsiness detection
        if(button==0)
        {
          disablePot=false;
          correctLeft=false;
          correctRight=false;
        }
        //after user physically centers wheel again and takes input from driving wheel
        if(disablePot==false)
        {
          xPos=map(pot,0,1023,445,785);
        }
        //Checks if car crosses right lane and locks wheel and greenlights correction turn
        if(xPos>765 && correctLeft==false)
        {
          disablePot=true;
          correctLeft=true;  
        }
        //Checks if car crosse left lane and greenlights correction turn
        else if(xPos<480 && correctRight==false)
        {
           disablePot=true;
           correctRight=true;
        }
        else // if car is within the lane, facilitate normal driving condtions with inputs from wheel
        {
          imageMode(CENTER);
          image(carState,xPos,600);
        }
        //Applies correction turn to the left until the car reaches center position again
        if(correctLeft==true && xPos>xCenter)
        {
          xPos-=1;
          imageMode(CENTER);
          image(left_Turn,xPos,600);
        }
        //Applies correction turn to the right until the car reaches center position again
        else if(correctRight==true && xPos<xCenter)
        {
          xPos+=1;
          imageMode(CENTER);
          image(right_Turn,xPos,600);
        }
        //If car reaches center, stops applying correction turn
        else if(xPos==xCenter)
        {
          correctLeft=false;
          correctRight=false;
          disablePot=false;
        }     
      }
    }
  }  
}    



    
    
    
  
