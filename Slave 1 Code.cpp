#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal.h>

bool Auth; //Session State Variable

Servo entrygate, exitgate; //Servo motors to operate entry and exit gates.

LiquidCrystal lcd1(13,12,11,10,9,6), lcd2(13,4,11,10,9,6); //lcd1 displays slot at entrance and lcd2 displays bill at exit.

int entrysensor = A0, slot1 = A1, slot2 = A2, exitsensor = A3; //IR sensors controlled by Slave Arduino

int ledslot1 = 7, ledslot2 = 8; //LED ON = Slot is occupied

int eng = 3, exg = 5; //Pins associated with the above defined servo motors.

int helpswitch = 2; //For sending an SOS signal in case of emergency.

byte rxd = 0; //For I2C protocol safety data received from Master Arduino

int nov = 0; //To save the number of vehicles currently present inside parking lot.

int reservedslots[8]; //Array that keeps track of the currently reserved slots.

//To define Parameters of vehicle occupying a slot.

struct slot
{
  int inTime, vno;
};

//To store the state of all vehicles currently present inside parking lot.

slot s[8];

//Initializing all chosen pins of Slave Arduino

void setup()
{
  
  //Initializing a Session by setting Auth variable to True.
  Auth = true;
  
  //Initializing Entry Panel LCD
  lcd1.begin(16,2);
  lcd1.setCursor(0,0);
  lcd1.print("Welcome!");
  
  //Initializing Exit Panel LCD
  lcd2.begin(16,2);
  lcd2.setCursor(0,0);
  lcd2.print("Goodbye!");
  
  //To operate Entry gate.
  entrygate.attach(eng);
  entrygate.write(5);
  
  //To operate Exit gate.
  exitgate.attach(exg);
  exitgate.write(5);
  
  //Setting up IR sensors
  pinMode(entrysensor, INPUT);
  pinMode(exitsensor, INPUT);
  pinMode(slot1, INPUT);
  pinMode(slot2, INPUT);
  
  //Starting communication with Master Arduino using I2C Protocol.
  Wire.begin(1);
  Wire.onReceive(receiveEvent);
  
  //Initially, received data is 0.
  rxd = 0;
  
  //Initially, Parking Lot is Empty. So, Parking States are initialized to 0.
  for(int i = 0; i < 8; i++)
  {
    s[i].inTime = 0;
    s[i].vno = 0;
  }
  
  //Initially none of the slots is reserved for maintenance purposes.
  for(int i = 0; i < 8; i++)
  {
    reservedslots[i] = 0;
  }
  
  //Starting Serial Communication with the PC using USB port.
  Serial.begin(9600);
  
}

//Defining what happens when an I2C event occurs.
void receiveEvent(int howMany)
{
  //Assigning received data byte the value of byte transmitted by master over the wire.
  rxd = Wire.read();
}

//Setting up the Bill Generation System that comes into picture when vehicle is going to exit the parking.
int billingSystem(int vno)
{
  int f = 0,spot;
  for(int i=0;i<8;i++)
  {
    if(s[i].vno == vno)
    {
      spot = i;
      f = 1;
      int q = spot + 1;
      Serial.println("Parking Slot: ");
      Serial.println(q);
      break;
    }
  }
  if(f==0)
  {
    return -1;
  }
  else
  {
    int t = millis();
    float diff = t - s[spot].inTime;
    float duration = diff/1000;
    Serial.println("Time of Admission: ");
    Serial.println(s[spot].inTime);
    Serial.println("Time of Exit: ");
    Serial.println(t);
    Serial.println("Parking Duration (Seconds):");
    Serial.println(duration);
    int bill = 0;
    if(duration>0.0&&duration<=5.0)
    {
      bill = 20;
    }
    else if(duration>5.0&&duration<=10.0)
    {
      bill = 30;
    }
    else if(duration>10.0&&duration<=20.0)
    {
      bill = 40;
    }
    else
    {
      bill = 100;
    }
    nov--;
    Serial.println("Bill (Rupees):");
    Serial.println(bill);
    char x[20];
    lcd2.clear();
    lcd2.print("Bill Amount: ");
    lcd2.setCursor(0,1);
    sprintf(x,"%d",bill);
    for(int i=0;x[i]!='\0';i++)
    {
      lcd2.print(x[i]);
    }
    s[spot].vno = 0;
    s[spot].inTime = 0;
    
    return 1;
  }
} 

//Algorithm to find a slot assigned to a certain vehicle.
int searchSlot(int vno)
{
  Serial.println(nov);
  if(nov==0)
  {
    return -1;
  }
  else
  {
  for(int i = 0; i < 8; i++)
  {
    if(s[i].vno == vno)
    {
      int slot = i + 1;
      return slot;
    }
  }
    return -1;
  }
}

//Algorithm to choose the most appropriate slot for assignment to a vehicle located at entrance of parking.
int assignSlot()
{
  //Choosing the slot which is closest to the entrance.
  for(int i = 0; i < 8; i++)
  {
    if(s[i].inTime==0)
    {
      return i;
    }
  }
  return -1;
}

//These activates occur iteratively for an infinite time, until power is switched off.
void loop()
{
  char ch = 'R'; 
  int i;
  if((rxd>5)&&(rxd<7))
  {
    Serial.println("If you want to Reserve a slot enter R, else, enter U to un-reserve a slot:");
    while(Serial.available()<=0);
    ch = Serial.read();
    if(ch=='R')
    {
      Serial.println("Enter the slot number to be reserved:");
      while(Serial.available()<=0);
      i = Serial.parseInt();
      i = i - 1;
      if((reservedslots[i]!=1)&&(s[i].inTime==0))
      {
      s[i].inTime = millis();
      reservedslots[i] = 1;
      i = i + 1;
      Serial.println(i);
      Serial.println("Slot Reserved!");
      }
      else
      {
        Serial.println("This slot has already been reserved!");
      }
    }
    else if(ch=='U')
    {
      Serial.println("Enter the slot number to be un-reserved:");
      while(Serial.available()<=0);
      i = Serial.parseInt();
      i = i - 1;
      if(reservedslots[i]!=0)
      {
      reservedslots[i] = 0;
      s[i].inTime = 0;
      i = i + 1;
      Serial.println(i);
      Serial.println("Slot has been freed from reservation!");
      }
      else
      {
        Serial.println("This slot has not been reserved previously.");
      }
    }
  }
  
  //In times of emergencies, the parking operator is logged out of system, however a troubled customer can still request help using help switch.
  if(!Auth)
  {
    int help = digitalRead(helpswitch);
    if(help==0)
    {
      int vno = 0;
    Serial.println("Enter the vehicle number you're looking for: ");
    if((Serial.available()>0));
    {
      vno = Serial.parseInt();
    }
    int slot = searchSlot(vno);
    if(slot==-1)
    {
      Serial.println("Invalid Vehicle Number!!!");
    }
    else
    {
      Serial.println("Parking Slot assigned to this vehicle is:");
      Serial.println(slot);
    }
    }
    //To indicate that the slots are permanently ocupied until the emergency situation is resolved. Also, increases total light inside parking lot.
    digitalWrite(ledslot1, HIGH);
    digitalWrite(ledslot2, HIGH);
    if(rxd==3)
    {
      Auth = true;
      digitalWrite(ledslot1, LOW);
      digitalWrite(ledslot2, LOW);
      lcd1.clear();
      lcd1.print("Welcome!");
      lcd2.clear();
      lcd2.print("Goodbye!");
      for(int i = 0; i < 8; i++)
      {
        s[i].inTime = 0;
        s[i].vno = 0;
      }
    }
    return;
  }
  
  //Following Code is Executed only if Auth = true.
  
  
  //If received byte from Master Arduino is equal to 1, then Parking Lot is in an UNSAFE state. Priority #1.
  if(rxd==1)
  {
    //Notifying the waiting drivers with their vehicles at entrance that parking cannot be done due to emergency.
    lcd1.clear();
    lcd1.print("PARKING FACILITY");
    lcd1.setCursor(0,1);
    lcd1.print("NOT AVAILABLE");
    lcd2.clear();
    //Notifying the drivers currently inside the parking lot to take their vehicles out through emergency exit only.
    lcd2.print("USE EMERGENCY");
    lcd2.setCursor(0,1);
    lcd2.print("EXIT");
    //Notifying the Parking Operator about emergency situation.
    Serial.println("Parking Emergency Situation!");
    //To indicate that the slots are permanently ocupied until the emergency situation is resolved. Also, increases total light inside parking lot.
    digitalWrite(ledslot1, HIGH);
    digitalWrite(ledslot2, HIGH);
    //Logging out the Parking Operator
    Auth = false;
    return;
  }
  
  //Checking if someone needs help is our Priority #2.
  int help = digitalRead(helpswitch);
  
  if(help==0)
  {
    int vno = 0;
    Serial.println("Customer Needs Help!!! Enter the vehicle number you're looking for: ");
    if((Serial.available()>0));
    {
      vno = Serial.parseInt();
    }
    int slot = searchSlot(vno);
    if(slot==-1)
    {
      Serial.println("Invalid Vehicle Number!!!");
    }
    else
    {
      Serial.println("Parking Slot assigned to this vehicle is:");
      Serial.println(slot);
    }
    delay(5);
  }
  
  //Finally, Welcoming the incoming traffic into the parking lot and assign them parking slot if parking is not full. Priority #3.
  if(analogRead(entrysensor)>500)
  {
    
    int slot = assignSlot();
    
    if(slot==-1)
    {
      entrygate.write(5);
      lcd1.clear();
      lcd1.print("Parking is FULL!");
      delay(500); 
      lcd1.clear();
      lcd1.print("Welcome!");
    }
    
    else
    {
      int vno;
      Serial.println("Enter Vehicle Number: ");
      if(Serial.available()>0);
      {
      vno = Serial.parseInt();
      }
      s[slot].vno = vno;
      s[slot].inTime = millis();
      nov++;
      Serial.println("Time of Admission: ");
      Serial.println(s[slot].inTime);
      Serial.println("Vehicle Number: ");
      Serial.println(s[slot].vno);
      int k = slot + 1;
      char x[20];
      lcd1.clear();
      lcd1.setCursor(0,0);
      lcd1.print("Slot: ");
      sprintf(x,"%d",k);
      lcd1.setCursor(0,1);
      lcd1.print(x);
      Serial.println("Is valet required? (y/n) :");
      char ch = 'y';
      while(Serial.available()<=0);
      ch = Serial.read();
      if(ch=='n')
      {
      entrygate.write(75);
      delay(600); 
      entrygate.write(5);
      }
      else
      {
          byte val = 11;
          Wire.beginTransmission(2);
          Wire.write(val);
          Wire.endTransmission();
          /*Serial.println("Has valet arrived yet? (y/n) :");
          char ch = 'y';
          while(Serial.available()<=0);
          ch = Serial.read();*/
          while(rxd!=12);
          Serial.println("Valet has arrived!");
          entrygate.write(75);
          delay(600); 
          entrygate.write(6); 
      }
      lcd1.clear();
      lcd1.print("Welcome!");
    }
  }
  
  else
  {
    entrygate.write(5);
    delay(5);
  }
  
  //Checking if the vehicle is appropriately parked at the assigned slot 1 or not.
  if(analogRead(slot1)>500)
  {
    Serial.println("Parking Detected at Slot 1");
    //Signalling that the parking slot  is currently occupied.
    digitalWrite(ledslot1, HIGH);
    delay(200); 
  }
  else if(analogRead(slot1)<=500)
  {
    //Signalling that the parking slot 1 is currently free.
    digitalWrite(ledslot1, LOW);
    delay(2); //20
  }
  
  //Checking if the vehicle is appropriately parked at the assigned slot 1 or not.
  if(analogRead(slot2)>500)
  {
    Serial.println("Parking Detected at Slot 2");
     //Signalling that the parking slot  is currently occupied.
    digitalWrite(ledslot2, HIGH);
    delay(200); 
  }
  else if(analogRead(slot2)<=500)
  {
    //Signalling that the parking slot 1 is currently free.
    digitalWrite(ledslot2, LOW);
    delay(2); 
  }
  
  //Checking whether a vehicle wants to exit the parking lot or not - Priority #4.
  if(analogRead(exitsensor)>500)
  {
    int vno;
    Serial.println("Enter vehicle number:");
    while(Serial.available()<=0);
    vno = Serial.parseInt();
    Serial.println(vno);
    //Generating the Bill based on the billingSystem() algorithm.
    int temp = billingSystem(vno);
    //int temp = 1;
    if(temp==-1)
    {
      Serial.println("Invalid Vehicle Number!");
    }
    else if(temp==1)
    {
    exitgate.write(75);
    delay(390); //500
    exitgate.write(5);
    lcd2.clear();
    lcd2.print("Goodbye");
    }
  }
  else if(analogRead(exitsensor)<500)
  {
    exitgate.write(5);
    delay(1);
  }
  
  delay(5); 
}
  
  
  
  
    
  
  