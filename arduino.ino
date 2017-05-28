/*
 * Sample example for Olimexino-328 Rev.C and thermistor 10 kOhm, B=3435
 * In order to work connect: 
 * one of the outputs of thermistor to 3.3V;
 * the other one to A0;
 * A0 through 10 kOhm to GND.
 * 
 *    ^ 3.3V
 *    |
 *   _|_
 *  |NTC|
 *  |10K|
 *  |___|
 *    |
 *    |
 *    +------------ A0
 *    |
 *   _|_
 *  |   |
 *  |10K|
 *  |___|
 *    |
 * ___|___  GND
 *   ___
*/

#include <VL6180X.h>
#include <Wire.h>

VL6180X sensor;
int buttonPin=8;
int motorPosition=0;


#define SLAVE_ADDRESS 0x04

int stateData=0;
int dataToSend=0;
int mm=0;

int tmp;
float r, temperature;

#define  R0  ((float)10000)
#define B ((float)3435)
// R0 = 10000 [ohm]
// B  = 3435
// T0 = 25 [C] = 298.15, [K]
// r = (ADC_MAX * R0) / (ADC_VAL) - R0
// R_ = R0 * e ^ (-B / T0), [ohm] --> const ~= 0.09919 (10K);
// T = B/ln (r/R_), [K]

//>80mm = empty

//78mm=100ml
//73mm=200ml
//67mm=300ml
//60mm=400ml
//53mm=500ml


//7mm <-> 100ml


boolean started=false;
boolean closedTemp=false;
boolean stopped=false;

int sendState=0;

void receiveStateData(int byteCount){
    stateData = Wire.read();
}

// callback for sending data
void sendData(){
  switch(sendState)
  {
    case 0:
      Wire.write(255);
    case 1:
      Wire.write((int)temperature);
      break;
    case 2:
      Wire.write(mm);
      break;
    case 3:
      Wire.write(motorPosition/100);
      sendState=0;
      return;
  }
  sendState++;
 }

/*int receiveDataFromRasp()
{
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  delay(100);
  Wire.endTransmission();
}*/

int sendDataToRasp()
{
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(sendData);
  delay(100);
  Wire.endTransmission();
}


void setup(void) 
{
  Serial.begin(9600);
  pinMode(5,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(A0, INPUT);
  pinMode(buttonPin,INPUT);
  Wire.begin();
  
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);
  
}

void close(int time)
{
  motorPosition-=time;
  digitalWrite(4,HIGH);
  analogWrite(5,240);
  delay(time);
  analogWrite(5,0);
}

void open(int time)
{
  motorPosition+=time;
  digitalWrite(4,LOW);
  analogWrite(5,240);
  delay(time);
  analogWrite(5,0);
}


void loop(void) 
{

  if(started==false)
  {
    open(2500);
    started=true;
  }
  
  tmp = analogRead (A0);
  Serial.print ("A0 = ");
  Serial.println (tmp);
  r = ((1023.0*R0)/(float)tmp)-R0;
  temperature = B/log(r/0.09919) - 273.15;  // log is ln in this case
  
  Serial.print ("T = ");
  Serial.print (temperature);
  Serial.println (", C");

  mm=sensor.readRangeSingleMillimeters();
  Serial.print(mm);
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  
  Serial.println();


  int buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    Serial.println("pressed");
    if(motorPosition>0)
    {
      close(motorPosition);
      stopped=true;
    }
  }

  if(temperature>34&&closedTemp==false&&!stopped)
  {
    close(500);
    closedTemp=true;
  }

  if(closedTemp==true&&temperature<32&&!stopped)
  {
    open(500);
    closedTemp=false;
  }

  if(!stopped&&mm<67)
  {
    close(motorPosition);
    stopped=true;
  }
  sendDataToRasp();
  delay (1000);
}


