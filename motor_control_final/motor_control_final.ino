#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   
#include <stdlib.h>
#define pwmpin 6
#define cruise_button 3
volatile int counter = 0;
volatile float  rpm = 0;
volatile double ini_time = 0;
volatile double exi_time = 0;
volatile float  end_time = 0; 
float mph = 0;
int maxmph = 40;

float pwm=0;
float kp=0.2;
float kd=2;
float currentspeed=0; // use mph
float error=0;
float pid=0;
float errorprevious=0; // I took static out of this, not even sure if it should set to zero every time.

int i = 0;
int max_HR = 0;
float target_high = 0.85;
float target_low = 0.80;
float cruise_speed = 0;
//int sspd = 0;
float dec = 0;
int pwm_low = 80;
//temp
const int temp_pin= A1; //set pin
float tempc; 
float tempf; 
float vout;
//threshold
int Threshold = 600;
const int PulseWire = A0;
PulseSensorPlayground pulseSensor;
int BPM = 0;
int count = 0;
//for loop declarations
 //char[2];
float sspd;
float set = 0;

//for the delay in decrement
int timer = 0;

//FOR BPM AVERAGE
float AVG_HR;
float bpm_sum=0;
//***************************************************************END VARIABLES**************************************************************

//*****************************************************************MOTOR CONTROL START***********************************************************

//**************************************************************END OF MOTOR CONTROL FUNCTION*****************************************************

//****************************************************************SETUP*********************************************************************
void setup() {
  pinMode(2, INPUT); //not sure if this is neccessary or not
  pinMode(cruise_button,INPUT);
  pinMode(temp_pin,INPUT); 
  digitalWrite(2, HIGH); // Enable pullup on digital pin 2, interrupt pin 0
  digitalWrite(pwmpin, LOW); //pwm pin 6
  digitalWrite(cruise_button,LOW); 
  attachInterrupt(0, speedometer, CHANGE); //attach interrupt to pin 2
  attachInterrupt(digitalPinToInterrupt(cruise_button), cruise, FALLING); //attach interrupt to pin 3
  Serial.begin(115200);
  Serial.setTimeout(100);
  pulseSensor.analogInput(PulseWire);   
   if (pulseSensor.begin()) 
   {
    //Serial.println("pulseSensor object created");  //This prints one time at Arduino power-up,  or on Arduino reset.  
   }
}
//**************************************************************END SETUP******************************************************************
  
//**************************************************************MAIN LOOP BEGIN************************************************************
void loop() {
 
  //Wait for entry before continuing
    if(max_HR<=20){
     Serial.println("Enter your Maximum Heart Rate");
     while(max_HR==0){ //Wait for input before running the rest of the code
         max_HR = Serial.parseFloat(); //serial communication with raspberry pi;       
        }
    }
  temp();
  //******************THIS SLOWS THE CODE DOWN BECAUSE IT WON'T CONTINUE WITHOUT HEARTRATE************
  //for(int count=0; count<=1;){
    
    int myBPM = pulseSensor.getBeatsPerMinute();  // "myBPM" hold this BPM value now. 
    if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
      BPM = myBPM;
      bpm_sum = bpm_sum + BPM;

      count= count + 1;
    }
  //}
  if(count >= 3){
    AVG_HR = (bpm_sum/4);
    count = 0;
    bpm_sum=0;
  }
  //**********************************************************BPM AVERAGE*******************************************************************
 
  //*********************************************************END AVERAGE**********************************************************************
 //************************************************************SET SPEED**********************************************************************

  mph = rpm * (26 / 12) * ((3.14 * 60) / 5280);
{   //*NOTE: replaced current_HR to AVG_HR.


  if (AVG_HR > target_high*max_HR){
    delay(100);
    sspd = cruise_speed;
  }else if (AVG_HR > target_low*max_HR){
    delay(100);
    sspd = cruise_speed;
    
  }else if(target_low*max_HR > AVG_HR){    
    //slow down in 10% increments
    delay(100);
    timer = timer + 1;
      if (timer >= 150){ //added 4-19
      dec = cruise_speed*0.1;
      sspd = sspd - dec;
      timer = 0;
    }
  }
  if (sspd <= 0){
    sspd = 0.1;
  }
}

  if (sspd > 0)
  {
    set = sspd;
  }else {
    set = set;
  }

  
//**************************************************************PID FUNCTION CALL***************************************************************
  pidfunc(set); //inputting a speed of 10MPH gives me 10 MPH out on the speedometer by adjusting the pwm values. 

  analogWrite(pwmpin, pwm);
    if(pwm >= 254 && mph < 5) // this is for bug correction. Keeps the thing working if we turn off the red button.
    {
    pwm = 0;
    }
  //*************************************************************END PID & PWM*****************************************************************
//***************SERIAL PRINTING***********
i = i + 1;
if (i>=5){

    Serial.print("AVG HR: ");
    Serial.println(AVG_HR);
    Serial.print("mph = ");
    Serial.println(mph);
    Serial.print("setspeed = ");
    Serial.println(set);
    Serial.println(" ");
    Serial.print("temp ");
    Serial.println(tempf);
    i = 0;
  }
}
//*****************************************************************END MAIN LOOP****************************************************************

//********************************************************************SPEEDOMETER FUNCTION BEGIN************************************************
//spedometer seems to be working it returns really high values once in a while, which is fine as long as we filter them out.
void speedometer() {
  float mphcalc = 0;
  counter = counter + 1;
  if (counter <= 1 ) {
    ini_time = millis();
    //delay(1);
  } else if (counter >= 10) { //have to get the second time reading right away
    exi_time = millis();
    end_time = abs(exi_time) - abs(ini_time); //larger when slower, smaller when faster.
      /*experimentally derived constant based on one hall sensor reading of 1.3x10^6, 
       don't forget that the hall sensor gives noise to the arduino unless you put a filter in place 
       I ran it through a resistor, 100 ohms, and shunt a 104 capacitor to ground*/
    rpm = (13000) / end_time;    
    counter = 0; //reset counter
  }
}
//****************************************************************END SPEEDOMETER FUNCTION*****************************************************

//***************************************************************PID FUNCTION START*********************************************************
int pidfunc(float setspeed){                  //I will change the pwm to be the speed values later. 
  if (mph <= maxmph){
    currentspeed = mph;
  }else if (mph < 0){
    currentspeed = 0;
  }else if (mph > maxmph){
    currentspeed = maxmph;
    mph = maxmph;
  }
  pid =(kp*error) + (kd*(error-errorprevious)); //this works well, I think
  error = abs(setspeed) - abs(currentspeed);
  errorprevious = error;
  if (pwm <= 255 && pwm >= pwm_low){
    pwm = pwm + pid; 
  }else if (pwm < pwm_low){
  pwm = pwm_low;
 }else if (pwm > 255){
  pwm = 255;
 }
// delay(100); // this is probably not a necessary slowdown but it can help keep it smooth
  //return constrain(pwm, 0 ,255); // ask pranay about this, I took pid and put pwm in here but it doesn't constrain to 255...
}
//********************************************************************END PID FUNCTION**********************************************************

void cruise() //This should get called on button press
{
  delay(200);
  cruise_speed = mph;
  sspd = cruise_speed;
  Serial.print("cruise speed");
  Serial.println(cruise_speed);
}

void temp(){
    vout=analogRead(temp_pin); 
    vout=(vout*5)/1023;
    tempc=(vout-1.310)/0.0225; 
    tempf=(tempc*1.8)+32; 
}
