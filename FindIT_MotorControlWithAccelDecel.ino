#include "CytronMotorDriver.h"
#include "IRremote.h"
//Latest code (Mar 15)
/* I/O:
 *  Controller (breadboard) for 5 small buttons (B1-B5), one deadman's switch (DM_switch),
 *  master green button to supply power to the motor driver.
 *  Blue LEDs indicate when a small button has been pressed, Red LED indicated when
 *  the deadmans switch is being pressed.
 * FUNCTIONS:
 * - can set the motor to 5 different speeds (foreward)(push small buttons 1,2,3,4 or 5)
 * - can set the motor to 3 different speeds (reverse)(push combination of buttons 1,2,3 with 5)
 * - can set the motor to run at the currently set speed for a given RunTime with buttons (start with buttons 1 and 4 pressed simoultaneously)
 * - when deadman's switch is held the motor will run at the currently set speed
 * - green button turns power on and off to motor driver immediately 
 * - all braking and acceleration will be gradual
 * - speeds must be -250 to 250 in interger multiples of the DecellerationStep size (technically max spd is 255)
 */

// Motor Pin Declarations
CytronMD motor(PWM_DIR, 3, 4); // PWM = Pin 3, DIR = Pin 4.

// Pin Declarations for Button Input
const int button1_pin = 10; 
const int button2_pin = 9;
const int button3_pin = 8;
const int button4_pin = 7;
const int button5_pin = 6;
const int deadmans_pin = 5;

// The State of Buttons 1-5 (had to name them A-E else gave errors)(Dont Change These)
int BA = 0; 
int BB = 0;
int BC = 0;
int BD = 0;
int BE = 0;
int DM_switch = 0;
int spd; // this will be compared to the current speed set by the buttons to be used in accelerating the cart

// Other Variables (Can Set These)
int currentspd = 10; // variable for initialization pwm speed (do not set to zero else bad things if pushing deadman's switch immediately)
int DecellerationStepSize = 10; // when stopping, reduce speed by this much each step (0-250)
int DecellerationStepTime = 250; // when stopping, reduce speed after this long between incremental decreases
int AccellerationStepSize = 10; // when starting, increase speed by this much each step (0-250)
int AccellerationStepTime = 250; // when starting, increase speed after this long between incremental increases
int RunTime = 3000; // can set amount of time to run before decelleration begins (milliseconds)
int delay_starting = 4000; // when setting a timed run, wait for this amount of time before starting accelerating 

// -------------------------------------------------------------------------------------------------
void RunMotor(int setspd, int duration){
  motor.setSpeed(setspd);
  delay(duration + 100);          
}

// -------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600); 
  pinMode(button1_pin, INPUT); // setup button pins as inputs
  pinMode(button2_pin, INPUT);
  pinMode(button3_pin, INPUT);
  pinMode(button4_pin, INPUT);
  pinMode(button5_pin, INPUT);
  pinMode(deadmans_pin, INPUT);
}

//-------------------------------------------------------------------------------------------------
void loop() {
  do{
    BA = digitalRead(button1_pin); // read the button values until anything changes
    BB = digitalRead(button2_pin); 
    BC = digitalRead(button3_pin);
    BD = digitalRead(button4_pin);
    BE = digitalRead(button5_pin);
    DM_switch = digitalRead(deadmans_pin);
    delay(100);
  } while(BA == 0 && BB == 0 && BC == 0 && BD == 0 && BE == 0 && DM_switch == 0);

  spd = 0; // initialize this before accelerating
  
  while(DM_switch == 1){ // hold down deadman's switch to go
    if(spd != currentspd){
      while(spd < currentspd){
        spd = spd + AccellerationStepSize;
        motor.setSpeed(spd);
        delay(AccellerationStepTime);
      }
      while(spd > currentspd){
        spd = spd - AccellerationStepSize;
        motor.setSpeed(spd);
        delay(AccellerationStepTime);
      }  
    }
    
    RunMotor(currentspd,0);
    DM_switch = digitalRead(deadmans_pin);

    if(DM_switch == 0){
    currentspd = spd;
      while(currentspd > 0){
        currentspd = currentspd - DecellerationStepSize;
        motor.setSpeed(currentspd);
        delay(DecellerationStepTime);
      }
      while(currentspd < 0){
        currentspd = currentspd + DecellerationStepSize;
        motor.setSpeed(currentspd);
        delay(DecellerationStepTime);
      }
    }
  }
    
  //motor.setSpeed(0);

  // Checks Button States to Set Speeds for Reversing
  if(BA == 1 && BB == 0 && BC == 0 && BD == 0 && BE == 1)
    currentspd = -50; // 20% spd reverse
  else if(BA == 0 && BB == 1 && BC == 0 && BD == 0 && BE == 1)
    currentspd = -100; // 40% spd reverse
  else if(BA == 0 && BB == 0 && BC == 1 && BD == 0 && BE == 1)
    currentspd = -150; // 60% spd reverse

  // Checks Button States to Begin a Timed Run 
  else if(BA == 1 && BB == 0 && BC == 0 && BD == 1 && BE == 0){
    // Acceleration
    spd = 0; // initialize this before accelerating
    delay(delay_starting);
    
    while(spd < currentspd){
      spd = spd + AccellerationStepSize;
      motor.setSpeed(spd);
      delay(AccellerationStepTime);
    }
    while(spd > currentspd){
      spd = spd - AccellerationStepSize;
      motor.setSpeed(spd);
      delay(AccellerationStepTime);
    }  
    
    RunMotor(currentspd,RunTime); // run motor for a set amount time at set speed (does not include accel/deceleration time

    //Decelleration
    while(currentspd > 0){
      currentspd = currentspd - DecellerationStepSize;
      motor.setSpeed(currentspd);
      delay(DecellerationStepTime);
    }
    while(currentspd < 0){
      currentspd = currentspd + DecellerationStepSize;
      motor.setSpeed(currentspd);
      delay(DecellerationStepTime);
    }
    
    //motor.setSpeed(0);
  }

  // Checks Button States to Set Speeds for Foreward Motion
  else if(BA == 1)
    currentspd = 50; // ~20% spd
  else if(BB == 1)
    currentspd = 100; // ~40% spd
  else if(BC == 1)
    currentspd = 150; // ~60% spd
  else if(BD == 1)
    currentspd = 200; // ~80% spd
  else if(BE == 1)
    currentspd = 250; // ~100% spd
}
