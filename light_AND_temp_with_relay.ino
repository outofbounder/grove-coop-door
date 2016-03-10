#include <EEPROM.h>

//TEMP SENSOR ON A0
const int tempSensorPin = 0;
int a;
int del=1000;                // duration between temperature readings
int B=3975;                  //B value of the thermistor
float resistance;
float temp;
int tempThresh=19;
int tempValue;

//LIGHT SENSOR ON A1
const int lightSensorPin = 1;  
const int dayLightThresh=150;       //the threshold for mornings
const int dayLightThresh=40;       //the threshold for nights
bool isDaytime=false;
int daytimeMinutes=0;
int nightMinutes=0;

//RELAY1 ON D2
const int relayUp = 2;

//RELAY2 on D3
const int relayDown = 3;

//BUTTON ON D5
const int buttonPin = 4;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status
bool doorOverride = false;   // variable for reading the override status(button pressed)

//LOCALS
bool isRunning = false;
//int doorValue = EEPROM.read(0);    //read from EEPROM
//int doorState = 0;    //from button 

void setup()
{
  setupRelays();
  pinMode(buttonPin, INPUT);   
}
void setupRelays() {
  pinMode(relayUp, OUTPUT);
  pinMode(relayDown, OUTPUT);
  Serial.begin(9600);  
}
void loop()
{  
  if(isRunning==false)
  {
    Startup();
  }
  CheckDaylight();
}
void Startup(){
  //print a startup message
  Serial.println("Starting Up...");
  
  //get startup values
  tempValue = GetTemp();
  
  printMessage("[Light Threshold]= ",lightThresh);
  printMessage("[Light Value]= ",analogRead(lightSensorPin));
  Serial.println();
  printMessage("[Temp Threshold]= ",tempThresh);
  printMessage("[Temp Value]= ",tempValue);
  Serial.println();
  printMessage("[Door Value]= ", isDaytime);
  Serial.println("[0=Off, 1=On]");
  
  //set status if ok
  isRunning = true;
}
void CheckDaylight()
{
 //printMessage("Getting Light...", nightMinutes);
 //Serial.print(" minutes elapsed...");
  
  int lightValue = getAverageLightInAMinute();
  tempValue = GetTemp();
  
  if(lightValue>lightThresh && tempValue>tempThresh)  //day time and above temp
  {        
    //only open if door is not already open to prevent motor binding!!
    //only open if button was not pushed to open it
   // if (isDaytime==false && doorValue!=1)
    if (isDaytime==false && doorOverride==false)
    {
      openCoopDoor(); //opens relay
      printMessage("Good Morning! ",lightValue);       
    }
    else
    {
      printMessage("Sunshine-What a day! [Light Value=", lightValue);
      Serial.println("]");
      daytimeMinutes++;
    }
  }
  else  //the light value has dropped below threshold    
  {
    //close if daytime (door opened during session)
    //close if door was left open (restarted) 
    if (isDaytime==true)
    {
      String msg="Good Night Birdies! Door was open for " + daytimeMinutes;
      printMessage(msg, lightValue);
      closeCoopDoor(); 
    }
    else
    {
      printMessage("Zzzz... [Light Value=",lightValue);
      Serial.println("]");
      nightMinutes++;
    }
  }  
}
void checkButtonPress(){
  int pollDelayInterval=1000;
  int pollCount=5;
  for (int i=0;i<pollCount;i++) //loops 5 times, delays 1000ms for a total of 5sec
  {
    delay(pollDelayInterval); //wait
    
    // read the state of the pushbutton value:
    buttonState = digitalRead(buttonPin);

    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState == 1) { 
      doorOverride = true; //set override flag 
      if(isDaytime==false){ //closed, open it   
        openCoopDoor();
        Serial.println("opened by button ");
      }
      else {
        closeCoopDoor();
        Serial.println("closed by button ");
      }
    } 
    else {
      Serial.write(".");
     // CloseCoopDoor();
    }
  }
}
int getAverageLightInAMinute()
{
  int pollDelayInterval=5000;
  int pollCount=12;
  int totalValues= 0;
  int avg;
  
  for (int i=0;i<pollCount;i++) //loops 12 times, delays 5000ms for a total of 1min
  {
     int theValue = analogRead(lightSensorPin); //the light sensor is attached to analog 1
     totalValues+=theValue;
     //delay(pollDelayInterval);
     //do 5 sec button look
     checkButtonPress();
     float percentComplete = round((i*100)/12);
     Serial.println(percentComplete);
  }
  avg = totalValues/pollCount;
  return avg;
}
int GetTemp()
{
  a=analogRead(tempSensorPin);  //gets temp
  resistance=(float)(1023-a)*10000/a; 
  temp=1/(log(resistance/10000)/B+1/298.15)-273.15;
  delay(del);
  
  //convert to f and print to screen
  temp = (temp*1.8)+32;
  
  return temp;
}
//MISC PRINT MSG
void printMessage(String _text, int _value)
{  
    Serial.print(_text);
    Serial.print(_value);
}
void printMessage(String _text, float _temp, int _light)
{
    Serial.print(_text);
    Serial.print("The indoor temp is: ");
    Serial.print(_temp);
    Serial.print(". Taken while solar is ");
    Serial.print(_light);
    Serial.println();
}
//RELAY ACTIONS
void openCoopDoor()
{
    digitalWrite(relayUp, HIGH); //also turn on relay
    delay(3000);   //keep open for x amount of seconds
    digitalWrite(relayUp, LOW);  //turn off relay
    //set EEPROM so we know what state door is in on start
 //   EEPROM.write(0,1); //ROM0 is turned on so we know
    //set daytime to true now that the door is open
    isDaytime = true;
}
void closeCoopDoor()
{
    digitalWrite(relayDown, HIGH); //also turn on relay
    delay(2800);   //keep open for x amount of seconds
    digitalWrite(relayDown, LOW);  //turn off relay
    //set EEPROM so we know what state door is in on start
//    EEPROM.write(0,0); //ROM0 is turned on so we know
    //set daytime to false now that door is closed
    isDaytime = false;
}
