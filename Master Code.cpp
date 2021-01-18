#include <Wire.h>
#include <Servo.h>

bool Auth; //To control operation of emergency ramp

Servo emergencygate; //Servo Motor to operate emergency gate

int slot3 = A3, slot4 = A2; //IR sensor pins to detect whether a vehicle is lying on the tray awaiting tobe parked on the second floor yet.

int ledslot3 = 7, ledslot4 = 8; //To indicate current status of a parking slot on second floor.

int me1 = 1, me2 = 4; //Enable inputs for DC motor drivers.

int maip1 = 6, maip2 = 5, mbip1 = 11, mbip2 = 10; //Inputs to the DC motor drivers.

int emergencyramp = 2; //Input given to emergency ramp DC motor.

int firealarm = 13; //Input for Piezo Buzzer that goes ON when fire is detected.

int smokedetector = A1, smokeled = 12; //To detect and indicate an on-going rise in smoke inside parking lot.

int firedetector = A0; //Input for Temperature Sensor to detect any rise in Temperature of Parking Lot.

int emg = 9; //For Emergency Gate Servo Motor.

int slideswitch = 2; //Main Switch

//Initializng the components of Safety System on Master Arduino

void setup()
{
  //Setting up Main Switch of the System.
  pinMode(slideswitch,INPUT);
 
  //Setting up default state of emergency gate
  emergencygate.attach(emg);
  emergencygate.write(5);
  
  //Setting up inputs to the LEDs and Slot IR sensors
  pinMode(ledslot3, OUTPUT);
  pinMode(ledslot4, OUTPUT);
  pinMode(slot3, INPUT);
  pinMode(slot4, INPUT);
  
  //Providing inputs to the motor drivers for the two slots' DC motors.
  
  pinMode(me1, OUTPUT);
  digitalWrite(me1, LOW);
  pinMode(maip1, OUTPUT);
  digitalWrite(maip1, LOW);
  pinMode(maip2, OUTPUT);
  digitalWrite(maip2, LOW);
  
  pinMode(me2, OUTPUT);
  digitalWrite(me2, LOW);
  pinMode(mbip1, OUTPUT);
  digitalWrite(mbip1, LOW);
  pinMode(mbip2, OUTPUT);
  digitalWrite(mbip2, LOW);
  
  //Setting up components of safety system.
  pinMode(firedetector, INPUT);
  pinMode(firealarm, OUTPUT);
  pinMode(emergencyramp, OUTPUT);
  digitalWrite(emergencyramp, HIGH);
  
  pinMode(smokedetector, INPUT);
  pinMode(smokeled, OUTPUT);
  
  //Initially, the emergency ramp is not operated, so its state is true.
  Auth = true;
  
  //Initializing communication with all the slaves, using I2C Protocol.
  Wire.begin();
  
  //Initializing Serial Communication with the PC using USB Port.
  Serial.begin(9600);
  
}


//The following code executes for infinite amount of time, iteratively until power is shut down.
void loop()
{
  
  //Making the Main Switch control whole circuit.
  int value = digitalRead(slideswitch);
  
  if(value==0)
  {
  
  //Default assumption at start of loop is that the system is in safe state, therefore, safe = 0.
  byte safe = 0;
  
  //Detecting any excessive smoke inside parking lot using Smoke Detector.
  int value = map(analogRead(smokedetector), 300, 750, 0, 100);
  
  //If excessive smoke is detected then, parking operator is informed about the same and associated LED is switched ON, ptherwise the LED is OFF.
  if(value>=80)
  {
    Serial.println("SMOKE!");
    Serial.println(value);
    digitalWrite(smokeled, HIGH);
    safe = 1;
    Serial.println(safe);
  }  
  else if(value<80)
  {
    digitalWrite(smokeled, LOW);
  }
  
  //Detecting rise in temperature due to a fire inside parking lot, using Temperature Sensor.
  int tmp = analogRead(firedetector);
  float voltage = (tmp * 5.0)/1024;
  float milliVolt = voltage * 1000;
  float tmpCel =  (milliVolt-500)/10 ;
  
  //If excessive temperature inside parking lot is detected then Fire Alarm is raised otherwise nothing happens.
  if(tmpCel>=70.0)
  {
    Serial.println("FIRE!");
    emergencygate.write(75);
    tone(firealarm,22);
    delay(50);
    noTone(firealarm);
    safe = 1;
  }
  else
  {
    emergencygate.write(5);
    delay(10);
  }
  
  //The current safety state of parking lot is communicated to the slave using I2C Protocol, so that in case of unsafe state, parking may be blocked.
  Wire.beginTransmission(1);
  Wire.write(safe);
  Wire.endTransmission();
  Wire.beginTransmission(2);
  Wire.write(safe);
  Wire.endTransmission();
  
  //If Parking is found to be in an unsafe state then, unroll the emergency ramp and open the emergency exit gate.
  if(safe!=0)
  {
    //To indicate that a slot is permanently occupied until the emergency situation is handled.
    digitalWrite(ledslot3, HIGH);
    digitalWrite(ledslot4, HIGH);
  }
  
  //If vehicle is found to have stopped in front of the IR sensor for slot 3, then pick up its underlying tray using DC motors, and park the vehicle at slot 3.
  //Also put switch ON LED of Slot 3, and finally, bring the tray down.
  //Otherwise, maintain default state.
  if(analogRead(slot3)>500)
  {
    digitalWrite(me1, HIGH);
    delay(200);
    digitalWrite(maip1, HIGH);
    digitalWrite(maip2, LOW);
    delay(100);
    digitalWrite(maip1, LOW);
    digitalWrite(maip2, LOW);
    delay(200);
    digitalWrite(maip1, LOW);
    digitalWrite(maip2, HIGH);
    delay(100);
    digitalWrite(me1, LOW);
    digitalWrite(ledslot3, HIGH);
    delay(400);
  }
  else if(analogRead(slot3)<=500)
  {
    digitalWrite(me1, LOW);
    digitalWrite(maip1, LOW);
    digitalWrite(maip2, LOW);
    delay(1);
    digitalWrite(ledslot3, LOW);
  }
  
  //If vehicle is found to have stopped in front of the IR sensor for slot 4, then pick up its underlying tray using DC motors, and park the vehicle at slot 4.
  //Also put switch ON LED of Slot 4, and finally, bring the tray down.
  //Otherwise, maintain default state.
   if(analogRead(slot4)>500)
  {
    digitalWrite(me2, HIGH);
    delay(200);
    digitalWrite(mbip1, HIGH);
    digitalWrite(mbip2, LOW);
    delay(100);
    digitalWrite(mbip1, LOW);
    digitalWrite(mbip2, LOW);
    delay(200);
    digitalWrite(mbip1, LOW);
    digitalWrite(mbip2, HIGH);
    delay(100);
    digitalWrite(me2, LOW);
    digitalWrite(ledslot4, HIGH);
    delay(400);
  }
  else if(analogRead(slot4)<=500)
  {
    digitalWrite(me2, LOW);
    digitalWrite(mbip1, LOW);
    digitalWrite(mbip2, LOW);
    delay(1);
    digitalWrite(ledslot4, LOW);
  }
  }
  //The current switch state of parking lot is communicated to the slave using I2C Protocol, so that in case of OFF state, parking may be blocked.
  value = value+2;
  Wire.beginTransmission(1);
  Wire.write(value);
  Wire.endTransmission();
  Wire.beginTransmission(2);
  Wire.write(value);
  Wire.endTransmission();
  digitalWrite(smokeled, LOW);
  delay(10);
  
}
  
