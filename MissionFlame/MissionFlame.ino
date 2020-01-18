/*
* Name: Ernst Mach and Colin Luo
* Project: Firefighting robot
* Date: June 8, 2018
* Description: Basic instructions for a firefighting robot that is controlled by an Arduino. 
* The robot enters a maze and when it reaches a wall it checks both the left and right distance 
* to determine which side to turn. Once in the maze the robot scans for both the flame and dowel. 
* When the flame is found, the robot will turn and drive to the flame, stop, blow out the flame, and 
* then turn around and go back to the original spot. When the dowel is found, the robot will turn and 
* drive up to the dowel, knock it over by turnning around, and then go back to the original spot. 
* When both the flame and dowel are down, the robot will turn around and exit the maze through the 
* same method it used to enter the maze. 
*/

#include <Servo.h>
Servo LeftServo;                            //create servo object to control left servo
Servo RightServo;                           //create servo object to control left servo

int pos = 0;                                //variable to store the servo position
int trigPin = 9;							              //trigger pin location for the ultrasonic sensor
int echoPin = 10;							              //echo pin location for the ultrasonic sensor
int duration;								                //how long it takes for the ultrasonic to recieve the return ping
int forwardDistance;						            //distance from the wall to the ultrasonic
int leftDistance;							              //variable to store the left distance when the robot checks the left side
int rightDistance;							            //variable to store the right distance when the robot checks the right side

int flameSensorPin = A5;					          //flame sensor pin location
int flameValue = 0;							            //variable to hold the value that the flame sensor picks up
int fanPin = 11;							              //pin location of the fan
int flameThreshold = 10;	                  //find average flame threshold		
int highestFlame;							              //variable to store what the highest flame value is

int objectSensor = A0;						          //pin location for the IR sensor
int objectDistance = 0;						          //variable to hold the value that the IR sensor picks up
int closestDistance = 200;	                //find the distance from the edge to the wall

boolean flameOn = true;						          //boolean saying that the flame is still active 
boolean dowelUp = true;						          //boolean saying that the dowel is not down yet

void setup() {
  RightServo.attach(6);                     //attaches the right servo on pin 6 to the servo object
  LeftServo.attach(5);                      //attaches the left servo on pin 5 to the servo object
  pinMode(trigPin, OUTPUT);                 //sets the trigPin as an Output
  pinMode(echoPin, INPUT);                  //sets the echoPin as an Input
  pinMode(flameSensorPin, INPUT);			      //sets the flameSensorPin as an Input
  pinMode(objectSensor, INPUT);				      //sets the objectSensor as an Input
}

void forwards() {
	LeftServo.write(-220);                    //tell the robot to move forward
	RightServo.write(300);
}

void stopT() {						                  //tell the robot to stop moving
	LeftServo.write(89);
	RightServo.write(90);
}

void right() {						                  //tell the robot to turn right 90 degrees
	LeftServo.write(-180);
	RightServo.write(-180);
	delay(750);
}

void left() {						                    //tell the robot to turn left 90 degrees
	LeftServo.write(180);
	RightServo.write(180);
	delay(750);
}

void turnAround(){				        	        //tells the robot to turn around 180 degrees
      right();
      right();
}

void checkUltrasonic() {			              //checks the distance using the ultrasonic and store the value found
	digitalWrite(trigPin, HIGH);
	delay(1);
	digitalWrite(trigPin, LOW);
	//measure the pulse input in echo pin
	duration = pulseIn(echoPin, HIGH);
	//distance is half the duration devided by 29.1 (from datasheet)
	forwardDistance = (duration/2) / 29.1;
}

void goForward(int stopDistance) {		    //go forwards until the robot reachs a set distance away from an object
  for(int i = 1; i > 0; i++) {        
    checkUltrasonic();                    //check the distance with the ultrasonic
    if(forwardDistance > stopDistance) {  //if the robot is still far away from where it wants to stop
      forwards();                         //move forwads
    } else {
      stopT();                            //stop moving
      break;                              //break out of for-loop
    }     
  }
}

void checkSides() {						            //turn left and right, compare both sides go in the direction of the longer path
  left();                                 //turn left
  checkUltrasonic();                      //check the distance with the ultrasonic
  leftDistance = forwardDistance;         //store the distance as the left distance
  turnAround();                           //turn to the right side
  checkUltrasonic();                      //check the distance with the ultrasonic
  rightDistance = forwardDistance;        //store the distance as the right distance
    if(leftDistance > rightDistance) {    //if the left distance is greater than right distance
      left();                             //turn to the left side
      left();
  }
}

void checkDistance() {						            //read the distance from the robot to the wall, when the disatnce changes dramatically, activate dowel takedown
  objectDistance = analogRead(objectSensor);  //scan the wall
		if(objectDistance > closestDistance ) {   //if the distance changes dramatically from the wall distance
		left();                                   //turn to the left
		goForward(5);                             //go forwards until robot is 5 cm away from dowel
		turnAround();                             //turn around to knock over dowel
		goForward(10);                            //go forwards until robot is 10 cm away from the right wall
		left();                                   //turn left to go back to original position
		dowelUp = false;                          //dowel is no longer up
	}
}

void checkFlame() {				                                              //read the flame value, find the value where the flame is the highest and take out the flame
flameValue = analogRead(flameSensorPin);                                //scan for flame value
	if(flameValue > highestFlame && flameValue > flameThreshold) {        //if there is a flame and the value is higher than current highest flame value
		highestFlame = flameValue;                                          //set the highest flame value 
	}
	if(highestFlame > flameValue + 15 && flameValue > flameThreshold) {		//when the difference between the highest flame and the current value are large enough the robot knows where the highest flame value is
		left();                                                             //turn left
		goForward(10);                                                      //go forward until there is 10 cm distance left
    stopT();                                                            //stop
    digitalWrite(fanPin, HIGH);                                         //turn on fan and run for 1 second before turning off
    delay(1000);
		digitalWrite(fanPin, LOW);
		turnAround();                                                       //turn around
		goForward(10);                                                      //go forward until there is 10 cm distance left
		left();                                                             //turn left to go back to original position
	  flameOn = false;                                                    //flame has been blown out
	}
}

void missionSetup() {   //enter the maze
  goForward(15);        //go forward until there is 15 cm distance left
  checkSides();         //check left and right to decide which way to turn
  goForward(15);        //go forward until there is 15 cm distance left
  checkSides();         //check left and right to decide which way to turn
  forwards();
  delay(2500);          //go forwards slightly to avoid seeing the wall in the beginning
}

void missionStart() {         //look for both dowel and flame
  checkUltrasonic();          //check distance
  if(forwardDistance > 5) {   //if the robot has not reached the end of the box
    forwards();               //move forwards
    if(flameOn == true) {     //if the flame has not been taken out yet
  	  checkFlame();           //look for flame
    }
    if(dowelUp == true) {     //if dowel is still up
      checkDistance();        //look for dowel
    }
  }
}

void missionEnd(){      //exit the maze
  turnAround();         //turn around
  goForward(15);        //go forward until there is 15 cm distance left
  checkSides();         //check left and right to decide which way to turn
  goForward(15);        //go forward until there is 15 cm distance left
  left();               //turn left
  forwards();           //go forwards for 6 seconds
  delay(6000);
  stopT();              //stop
}

void loop() {
  missionSetup();                                   //enter the maze
  while(dowelUp == true || flameOn == true) {       //look for flame and dowel
    missionStart();
  }
  if(dowelUp == false && flameOn == false) {        //when both flame and dowel are down exit the maze
    missionEnd();
  }
  delay(100000);                                    //avoid looping
}
