#include <SPI.h>
#include <Ethernet.h>
#include <mac.h>
#include  <Encoder.h>
#include  <OneButton.h>
#include  "wemo.h"
#include <PWMServo.h>
# include <Adafruit_NeoPixel.h>
# include <colors.h>

//Ethernet 
bool status;


//RELAY
const int Relay  =17;
const int MotorDelay=2000;


//NEOPIXEL
 const int LEDSTRIP= 7;
 const int PIXELCOUNT = 30;
  Adafruit_NeoPixel pixelStrip (PIXELCOUNT ,LEDSTRIP ,NEO_GRB + NEO_KHZ800 ); 

//SERVO
  PWMServo myServo;
  const int SERVOPIN= 20;
    int servoPos;

// LIGHT SENSOR
  const int LIGHTDIODE  = 23;
  const float ThresholdPercentage= .2; //How Sensitive is the cutoff for triggering the ball 
    int pinNumThreshold[13]; //Setting up Thredshold Values 
    int  diodeVal, currentAve, thresholdVal, j,PinAve;
    float aveDiodeVal;


 //ONE BUTTON
  const int BUTTON =8;
    OneButton button1 (BUTTON, LOW, LOW);
    int lastPositon, currentPositon,mapPos , finalPosition  ,position; 
    int Mode;

void setup() {
  Serial.begin(9600);
  delay(200);

//SERVO
  myServo.attach(SERVOPIN);
  myServo.write(70);                        //Set the Servo to prevent ball from falling 

//Setup PinModes
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

//ENCODER LEDS AND SWITCH
  pinMode(BUTTON, INPUT);
  pinMode(Relay, OUTPUT);

//ONE BUTTON
  Mode = 0;
  button1.attachClick(click1);
  button1.setClickTicks (250) ;
  button1.setPressTicks (2000) ;
  
////Ethernet Connection
//  status = Ethernet.begin(mac);
//  if (!status) {
//    Serial.printf("failed to configure Ethernet using DHCP \n");
//    //no point in continueing 
//    while(1);
//    }
//  //print your local IP address
//  Serial.print("My IP address:");
//  for (byte thisbyte = 0; thisbyte < 4; thisbyte++) {
//    //print value of each byte of the IP address
//    Serial.print(Ethernet.localIP()[thisbyte], DEC);
//    if (thisbyte < 3) Serial.print(".");
//    }
//  Serial.println();

  
//Light Sensor
 pinMode(LIGHTDIODE, INPUT);
  thresholdVal = averageSetup(LIGHTDIODE,300);      //Take threshold Average For the Light sensor
     Serial.print(" Threadshold Set with a val of: ");
     Serial.println(thresholdVal);
    
 
//LED Lights
  pixelStrip.begin ();
  pixelStrip.clear ();
  pixelStrip.show ();
  pixelStrip.setBrightness (255);

  for (int i=0; i<3; i++){                        // Flash Green to show that it is calibrated and ready to go! 
    pixelStrip.fill (lime,0,30);
    pixelStrip.show ();
      delay(250);
    pixelStrip.clear ();
    pixelStrip.show ();
      delay(200);
   }
}


void loop() {
  delay(2);
  button1.tick ();
 
  PinAve=average(LIGHTDIODE, 5);                  //Average the light sensor 5 times
  Serial.println (Mode);
  
  if (CompareToThreshhold(PinAve, LIGHTDIODE)){ //Check the current average against the threshhold for TRUE
      turnOnMotor();
      MoveServo(0);  
  }



  
 
}




 



//ONEBUTTON 
void click1 () {
    Mode= Mode+1 ;
    if (Mode==4){
      Mode=0;
    }
    
}

void ButtonPress(){
  if (Mode){
      turnOnMotor();
      MoveServo(1);

     
    
  }
  if (!Mode){
    
    digitalWrite(Relay, LOW);
    //switchOFF(_outletNum); //TURNS OFF WEMO
    //Serial.printf ("Outlet # %i is OFF \n" ,  _outletNum);
    //Serial.printf ("%i \n",ButtonState);
  }
}

void turnOnMotor(){
   for (int i=0;i<5; i++){
    digitalWrite(Relay, HIGH);
    delay(15);
    digitalWrite(Relay, LOW);
    delay(10);
   }
    digitalWrite(Relay, HIGH);  
    
}

int averageSetup (int _pinNum , int _iterations) {
    unsigned int sum, ave;
    sum=0; 
 for (int i = 0 ; i < _iterations ; i++){
    sum=sum + analogRead(LIGHTDIODE);   
    delay(1);
 }

  return  (sum / _iterations) ;  
}


int average (int pinNum , int iterations) {
  unsigned int sum, ave;
  sum=0; 
    for (int i = 0 ; i < iterations ; i++){
       sum = sum + analogRead(LIGHTDIODE);
       delay(1);
    }
   ave= (sum / iterations);
   return ave ;  
}

bool CompareToThreshhold(int _currentAve,int _ThresholdPos){
  if (_currentAve<(thresholdVal * ThresholdPercentage)){
      Serial.print(" Ball is present with current Average = ");
      Serial.println (_currentAve);
    return true;
  }
    
  else {
    return false;
  }
}


void MoveServo(int _servoDelay){ 
  delay (_servoDelay);
  myServo.write(180);
  delay (500);                //Wait to turn off motor until ball has shot
  digitalWrite(Relay, LOW);   //Turn off motor
  delay(500);     
  myServo.write(70);          //Close the Servo to reset 
}
