#include<Wire.h>

//Indicating pins for all LEDs.
int led5 = 8, led6 = 7, led7 = 6, led8 = 5;

//Indicating pins for all IR sensors installed at the parking slots.
int slot5 = A0, slot6 = A1, slot7 = A2, slot8 = A3;

//Help Switch to indicate parking assistance requirement by Drivers.
int helpswitch = 2;

//Received byte of data from master.
byte rxd;

//Emergency Ramp Parameters.
int a = 13, b = 12, c = 11;

//Emergency Parameter for checking if ramp is rolled or unrolled.
bool Auth = false;

//Lock
bool lock = false;

//Slot Reservation Switch
int reserveswitch = 3;

//Safety System Enable Indicator
int safetyled = 9;

int valetbutton = 4, valetbuzzer = 10; //For valet parking system.

//Initializing the components.
void setup()
{
  
  //Setting up the Safety System Indicator LED.
  pinMode(safetyled, OUTPUT);
  
  //Creating the Slot Reservation Push Button
  pinMode(reserveswitch, INPUT);
  
  //Setting led pins to OUTPUT.
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);

  //Setting IR sensor slot pins to INPUT.
  pinMode(slot5, INPUT);
  pinMode(slot6, INPUT);
  pinMode(slot7, INPUT);
  pinMode(slot8, INPUT);
  
  //Setting up the Help Switch
  pinMode(helpswitch, INPUT);
  
  //Initializing the Emergency Ramp to Rolled Position
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, LOW);
  delay(100);
  digitalWrite(a, LOW);
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  
  //Initializing received byte of data to 0.
  rxd = 0;
  
  //Valet System Initialization
  pinMode(valetbutton, INPUT);
  pinMode(valetbuzzer, OUTPUT);
  
  //Initializing communication with the master, slave address is 2.
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  
  //Initializing serial communication through USB port with the PC.
  Serial.begin(9600);

}

//Defining what to do if an event occurs on wire through byte transmission.
void receiveEvent(int howMany)
{
  //Read the recently transmitted byte and store it in rxd.
  rxd = Wire.read();
}

void loop()
{
  
  //Reserve Switch data.
  int val = digitalRead(reserveswitch);
  if(val==0)
  {
  val = val + 6;
  Wire.beginTransmission(1);
  Wire.write(val);
  Wire.endTransmission();
  }
  
  //If rxd is equal to ONE then system is in unsafe state. Hence, switch on all the lights to improve visiblity.
  if(rxd==1)
  {
    if(!Auth)
    {
      digitalWrite(a, HIGH);
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      delay(100);
      digitalWrite(a, LOW);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      Auth = true;
    }
    digitalWrite(led5, HIGH);
    digitalWrite(led6, HIGH);
    digitalWrite(led7, HIGH);
    digitalWrite(led8, HIGH);
    digitalWrite(safetyled, LOW);
    int help = digitalRead(helpswitch);
    if(help==0)
    {
      Serial.println("Customer needs help!!!");
    }
    lock = true;
  }
  else if((rxd==3)&&(lock))
  {
    if(Auth)
    {
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    delay(100);
    digitalWrite(a, LOW);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    Auth = false;
    }
    digitalWrite(led5, LOW);
    digitalWrite(led6, LOW);
    digitalWrite(led7, LOW);
    digitalWrite(led8, LOW);
    int help = digitalRead(helpswitch);
    if(help==0)
    {
      Serial.println("Customer needs help!!!");
    }
    lock = false;
    digitalWrite(safetyled, LOW);
  }
    
  if(!lock)
  {
    //Indicating that safety system in enabled.
    if(rxd!=3)
    {
    digitalWrite(safetyled, HIGH);
    }
    else
    {
      digitalWrite(safetyled, LOW);
    }
    
    //Checking the help switch.
    int help = digitalRead(helpswitch);
    if(help==0)
    {
      Serial.println("Customer needs help!!!");
    }
    
    if(rxd==11)
    {
      Serial.println("Valet Request Received!");
      int val = digitalRead(valetbutton);
      while(val==1)
            {
              tone(valetbuzzer,55);
              delay(20);
              noTone(valetbuzzer);
              val = digitalRead(valetbutton);
              //Serial.println(val);
            }
      byte arrival = 12;
      Wire.beginTransmission(1);
      Wire.write(arrival);
      Wire.endTransmission();
     }
    
    /*int ev = digitalRead(valetbutton);
    if(ev==0)
    {
      tone(valetbuzzer,55);
      delay(200);
      noTone(valetbuzzer);
      //Serial.println("Press any key when valet arrives.");
      //char ch = 'y';
      //while(Serial.available()<=0);
      //ch = Serial.read();
      //Serial.println(ch);
      //int vno;
      Serial.println("Provide vehicle number to the valet.");
    }*/
    
    //If car is detected to be parked at slot 5, then switch ON the LED at this slot.
    if(analogRead(slot5)>500)
    {
      digitalWrite(led5,HIGH);
      delay(1);
    }
    else
    {
      digitalWrite(led5, LOW);
    }
    //If car is detected to be parked at slot 6, then switch ON the LED at this slot.
    if(analogRead(slot6)>500)
    {
      digitalWrite(led6,HIGH);
      delay(1);
    }
    else
    {
      digitalWrite(led6, LOW);
    }
    //If car is detected to be parked at slot 7, then switch ON the LED at this slot.
    if(analogRead(slot7)>500)
    {
      digitalWrite(led7,HIGH);
      delay(1);
    }
    else
    {
      digitalWrite(led7, LOW);
    }
    //If car is detected to be parked at slot 8, then switch ON the LED at this slot.
    if(analogRead(slot8)>500)
    {
      digitalWrite(led8,HIGH);
      delay(1);
    }
    else
    {
      digitalWrite(led8, LOW);
    }
    delay(5);
  }
}