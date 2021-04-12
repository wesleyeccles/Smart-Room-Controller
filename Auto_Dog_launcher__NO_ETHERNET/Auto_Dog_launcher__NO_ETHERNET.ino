#include <SPI.h>
//#include <Ethernet.h>
#include <mac.h>
#include  <Encoder.h>
#include  <OneButton.h>
#include  "wemo.h"
#include <PWMServo.h>
# include <Adafruit_NeoPixel.h>
# include <colors.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <hue.h>

//Display 
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels
  
  #define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// HUE LIGHTS
int   hueRedGreenBlue[]= {HueRed , HueRed, HueYellow,HueGreen,HueGreen};

//Ethernet 
  //bool status;

// RANGE
  enum  State {
    FAR,
    MED,
    SHORT,
    AUTO
  };
 State Mode;

//RELAY
  const int Relay  =17;
  const int MotorDelay=2000;
  int  ballCount;

//NEOPIXEL
 const int LEDSTRIP= 7;
 const int PIXELCOUNT = 30;
  Adafruit_NeoPixel pixelStrip (PIXELCOUNT ,LEDSTRIP ,NEO_GRB + NEO_KHZ800 ); 
  const int colorArray[] ={red, orange, yellow, green,cyan, blue, indigo, violet,   red, orange, yellow, green,cyan, blue, indigo,violet,red, orange, yellow, green,cyan, blue, indigo,violet};



//SERVO
  PWMServo myServo;
  const int SERVOPIN= 20;
    int servoPos, ServoDelay,ServoCloseVal;

//LIGHT SENSOR
  const int LIGHTDIODE  = 23;
  const float ThresholdPercentage= .3; //How Sensitive is the cutoff for triggering the ball 
    int pinNumThreshold[13]; //Setting up Thredshold Values 
    int  diodeVal, currentAve, thresholdVal, j,PinAve;
    float aveDiodeVal;


//ONE BUTTON
  const int BUTTON =8;
  
  OneButton button1 (BUTTON, LOW, LOW);
  int lastPositon, currentPositon,mapPos , finalPosition  ,position; 

//TIMER
  int TimeDelay = 6000;
  unsigned int timerStart;
  bool fanIsRunning;

//OLED

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000100, B00010000,
  B00000100, B00010000,
  B01000100, B10010001,
  B01000101, B01010001,
  B01000010, B00100001,
  B01000100, B00010001,
  B01001000, B00001001,
  B01010000, B00000101,
  B00100000, B00000010,
  B01000000, B00000001,
  B00100000, B00000010,
  B00010000, B00000100,
  B00001000, B00001000,
  B00000100, B00010000,
  B00000010, B00100000,
  B00000001, B10000000, };

  
  
void setup() {
  Serial.begin(9600);
  delay(200);

//SERVO
  myServo.attach(SERVOPIN);
  myServo.write(70);                        //Set the Servo to prevent ball from falling 

//Setup PinModes                            // ensure all SPI devices start off
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

//ENCODER LEDS AND SWITCH
  pinMode(BUTTON, INPUT);
  pinMode(Relay, OUTPUT);

//ONE BUTTON
  Mode = 3;
  button1.attachClick(click1);
  button1.setClickTicks (250) ;
  button1.setPressTicks (2000) ;
  
////Ethernet Connection
//  status = Ethernet.begin(mac);
//  if (!status) {
//    Serial.printf("failed to configure Ethernet using DHCP \n");
//    //no point in continueing 
//   //while(1);
//    }
//  //print your local IP address
//  Serial.print("My IP address:");
//  for (byte thisbyte = 0; thisbyte < 4; thisbyte++) {
//    //print value of each byte of the IP address
//    Serial.print(Ethernet.localIP()[thisbyte], DEC);
//    if (thisbyte < 3) Serial.print(".");
//    }
//  Serial.println();
//
//OLED DISPLAY
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    
  }
    display.clearDisplay();
    display.display();
    display.setRotation(0);
    
    //SHOW THAT ITS CALIBRATING
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,5);  
    display.printf("SETTING UP");
    display.display();
    delay(1000);
    
    testdrawbitmap();
    
        


    
  
//Light Sensor
 pinMode(LIGHTDIODE, INPUT);
  thresholdVal = averageSetup(LIGHTDIODE,300);      //Take threshold Average For the Light sensor
     Serial.print(" Threadshold Set with a val of: ");
     Serial.println(thresholdVal);
    
 
//LED Lights
  pixelStrip.begin ();
  pixelStrip.clear ();
  pixelStrip.show ();
  pixelStrip.setBrightness (100);

  for (int i=0; i<3; i++){                        // Flash Green to show that it is calibrated and ready to go! 
    pixelStrip.fill (lime,0,30);
    pixelStrip.show ();
      delay(250);
    pixelStrip.clear ();
    pixelStrip.show ();
      delay(200);
   }

//Ball Count 
  ballCount=0;
  SETUPPrintOLED() ;
  PrintBallCountToOLED(ballCount, Mode);

//TIMER
   // switchOFF(1); //TURNS OFF WEMO FAN
   // switchOFF(2); //TURNS OFF WEMO FAN 
    fanIsRunning=false;                  
}

void loop() {
 // delay(2);
  button1.tick ();                                //Check Button for pressed
  
  PinAve=average(LIGHTDIODE, 5);                  //Average the light sensor 5 times

  //Serial.println (fanIsRunning);

  
  if (isTimerReady()){                            //Check the timer to see if its done to turn off fans 
    //switchOFF(1); //TURNS OFF WEMO FAN
    //switchOFF(2); //TURNS OFF WEMO FAN 
    fanIsRunning=false;   
  }
  
  //Serial.println (Mode); 
  
  
  if (CompareToThreshhold(PinAve, LIGHTDIODE)){ //Check the current average against the threshhold for TRUE 
    
    for (int k=1; k<=5; k++){                         //Turn ON and OFF hue lights Red > Yellow> Green 
      //  setHue(k,true,hueRedGreenBlue[k-1],255,255);
       // delay (50);
      //  setHue(k,false,hueRedGreenBlue[k-1],0,0);
          Serial.println ("Lights turning off");
     }
     
        turnOnMotor();
        MoveServo(ServoDelay);  
        PrintBallCountToOLED(ballCount,Mode);  
        Rainbow();
  }
  
   switch (Mode) {
      case FAR: 
            ServoDelay= 750;
            ServoCloseVal=80;
              pixelStrip.fill (red,0,30);
              pixelStrip.show ();
              //Serial.println ("FAR RANGE MODE"); 
         break;
          
      case MED:
            ServoDelay= 250;
            ServoCloseVal=80;
              pixelStrip.fill (yellow,0,30);
              pixelStrip.show ();   
              //Serial.println ("MED RANGE MODE");                     
         break;
          
      case SHORT: 
            myServo.write(180);
            ServoDelay= 0;
            ServoCloseVal=180;
              pixelStrip.fill (lime,0,30);
              pixelStrip.show ();       
              //Serial.println ("SHORT RANGE MODE");       
         break;
          
      default:
            
            pixelStrip.fill (blue,0,30);
            pixelStrip.show (); 
            //Serial.println ("AUTOMATIC MODE");     
          if (ballCount <=3){
            myServo.write(80);
              ServoDelay= 750;
              ServoCloseVal=80;
                display.setTextSize(1);             // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE);        // Draw white text
                display.setCursor(90,55);  
                display.printf("FAR" );
                display.display();
              
          }
          if (ballCount >3 && ballCount<6){
              ServoDelay= 250;
              ServoCloseVal=80;
                display.setTextSize(1);             // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE);        // Draw white text
                display.setCursor(90,55);  
                display.printf("MED" );
                display.display();             
         
          }
                    
          if (ballCount ==6 ){
              ServoDelay= 0;
              ServoCloseVal=180;

               if (!fanIsRunning){
                 timerStart=millis();
              //   switchON(1); //TURNS ON WEMO FAN
               //  switchON(2); //TURNS ON WEMO FAN 
                 fanIsRunning=true;  
                display.setTextSize(1);             // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE);        // Draw white text
                display.setCursor(90,55);  
                display.printf("SHORT" );
                display.display();  
               } 
          }          
         
          if (ballCount >6 ){
              ServoDelay= 0;
              ServoCloseVal=180; 
                display.setTextSize(1);             // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE);        // Draw white text
                display.setCursor(90,55);  
                display.printf("SHORT" );
                display.display();  
          }          
          break;
      }
  }


 



//ONEBUTTON 
void click1 () {
  
  Mode= Mode+1 ;
  Serial.println(Mode);
  if (Mode==4){
       Mode=0;  
   }
  if (Mode==3){
      ballCount=0; 
   }
    PrintBallCountToOLED(ballCount,Mode);
    pixelStrip.clear (); 
 }

void turnOnMotor(){
   for (int i=0;i<5; i++){                 //Pulse the motor on off quicky to turn it on
    digitalWrite(Relay, HIGH);
    delay(15);
    digitalWrite(Relay, LOW);
    delay(10);
   }
    digitalWrite(Relay, HIGH);              //Keep motor on until MoveServo turns it off
    
}

int averageSetup (int _pinNum , int _iterations) {
    unsigned int sum;
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
  delay (_servoDelay);                    //Determines how far the ball will launch
  myServo.write(180);
  delay (500);                            //Wait to turn off motor until ball has shot
  digitalWrite(Relay, LOW);               //Turn off motor
  delay(500);     
  myServo.write(ServoCloseVal);           //Close the Servo to reset 
  ballCount++;
}

void PrintBallCountToOLED(int _ballCount,int _Mode) {
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,5);  

  display.printf("Ball Count" );
  display.display();
 
  display.setTextSize(3);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(50,30);  

  display.printf("%i",_ballCount );
  display.display();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(10,55);  
  
  if (_Mode==0){                      //Show which mode it's i
    display.printf("FAR" );
    display.display();
    }
  if (_Mode==1){
    display.printf("MED" );
    display.display();
    } 
  if (_Mode==2){
    display.printf("SHORT" );
    display.display();
    } 
  if (_Mode==3){
    display.printf("AUTO" );
    display.display();
    } 
    
  delay(1);
}

void SETUPPrintOLED() {

  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(10,5);  

  display.printf("READY TO" );
   
  display.setTextSize(3);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(40,30);  

  display.printf("GO!" );
  display.display();
  delay(1000);
}

bool isTimerReady() {
      //Serial.println("checking Timer");
       
      if ((millis() -  timerStart) > TimeDelay){
        return true;
      }
      else {
        return false;
      }
    }

bool isFanRunning(){
  if (fanIsRunning){
    return true;
  }
  else{
    return false;
  }
  
}

void Rainbow(){
 for (int l=0; l<5;l++){
  for (int j=0; j<25;j++){
    for (int i=0; i<25; i++){
      pixelStrip.setPixelColor(i,colorArray[(i+j)/3]);
    }
      pixelStrip.show();
      delay(10);
  }
 }
}



void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}
