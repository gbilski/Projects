
#include <Arduino.h>
#include <Servo.h>
#include <Servo.h>

#define nexSerial Serial2

Servo servo0; //creates first servo object to control servo
Servo servo1; //creates second servo object to control servo

static uint32_t oldtime=millis();   // for the timeout
byte SpeedyResponse[106]; //The data buffer for the serial data
byte ByteNumber;  // pointer to which byte number we are reading currently
byte ResponseLength; // how long response is asked from speeduinoSeria3
unsigned int AFR_RAW;   //AFR value
float o2val;   //AFR value
int MAP;    //MAP from speeduino
int BARO;   //barometer from speeduino
int TPS;    //TPS percent from speeduino
float OIL;    //Oil pressure speeduino
float TEMP;   //Coolant Temperature
int RPM;
float IAT;  //intake air temperature
float FUEL;   //Fuel pressure spedino
int pinAC = 2;    //from the AC switch
int pinACoff = 4; //from the AC fan switch
int pinACon = 3;  //AC on/off signal to Speeduino
int pinLights = 24;  //Park lights on
boolean pinLightsstate = LOW;
boolean pinLightsnew = LOW;
float boost1;
float boost;
//volatile float peakboost = 0; // Set peak memory to 0. We don't save this to flash so  resets on start up
volatile int peakboost = 0; // Set peak memory to 0. We don't save this to flash so  resets on start up
unsigned int VOLT;
float voltval;
int servo0_val;
int servo1_val;
int speed_val;
int rpm_val;
int VSS;


void setup(){
 Serial.begin(115200);
 Serial1.begin(115200);  // baudrate for Speeduino is 115200
 nexSerial.begin(38400);  // for the Nextion screen

 // Servo for dash gauge
 servo0.attach(8);  //output to servo on pin 9
 servo1.attach(10);  //output to servo on pin 11

 //AC control pins
 pinMode(pinACon, OUTPUT);
 pinMode(pinAC, INPUT_PULLUP);
 pinMode(pinACoff, INPUT_PULLUP);

 //Park lights
 pinMode(pinLights, INPUT_PULLUP);

// zero the data so it's not just random garbageRPM
 boost = 0;
 boost1 = 0;
 o2val = 0;
 MAP = 0;
 BARO = 0;
 TPS = 0;
 OIL = 0;
 FUEL = 0;
 VOLT = 0;
 TEMP = 0;
 IAT = 0;
 ResponseLength = 104;
 delay(500);
// requestData(); // all set. Start requesting data from speeduino
}

//Send r to request data from Speeduio
void requestData() {
  Serial1.write("r"); //new type real time data.
  Serial1.write(0x00); //Speeduino TS canID, not used atm.
  Serial1.write(0x30);  //command type, 0x30 for real time data.
  Serial1.write(4); //offset for the data. LSB
  Serial1.write(0x00); // offset is in 2 bytes. LSB first.
  Serial1.write(ResponseLength);  //how many bytes we need back. LSB
  Serial1.write(0x00); // number of bytes is in 2 bytes. LSB first.
}

//display the needed values in serial monitor for debugging
void displayData(){
  int pinACstate = digitalRead(pinAC);
  int pinACoffstate = digitalRead(pinACoff);
  Serial.print ("MAP:"); Serial.print (MAP); Serial.print("  ");
  Serial.print ("BARO:"); Serial.print (BARO); Serial.print ("  ");
  Serial.print ("AFR:"); Serial.print (o2val); Serial.print("  ");
  Serial.print ("OIL:"); Serial.print (OIL); Serial.print("  ");
  Serial.print ("FUEL:"); Serial.print (FUEL); Serial.print("  ");
  Serial.print ("RPM "); Serial.print (RPM); Serial.print("\t");
  Serial.print ("VSS "); Serial.print (VSS); Serial.print("\t");
  Serial.print ("TPS %:"); Serial.print (TPS); Serial.print("  ");
  Serial.print ("pinAC:"); Serial.print (pinACstate); Serial.print("  ");
  Serial.print ("pinACoff:"); Serial.print (pinACoffstate); Serial.print("  ");
  Serial.print ("TEMP:"); Serial.print (TEMP,0); Serial.print("  ");
  Serial.print ("IAT:"); Serial.print (IAT,0); Serial.print("  ");
  Serial.print ("VOLT:"); Serial.print (voltval); Serial.println();
}

void servoData() {
  servo0_val = map(RPM, 0, 7500, 2500, 550);  //converts RPM to output value in micro seconds
  servo0.writeMicroseconds(servo0_val);  //sets the servo position
  delay(25);
//  servo1_val = map(currentStatus.VSS, 0, 210, 2500, 550);
//  servo1.writeMicroseconds(servo1_val);
//  delay(25);
}

void processData(){   // To process data for display
  if(SpeedyResponse[1] == 0x30){ //only process if serial response array has the right start point.
   MAP          = ((SpeedyResponse[3] << 8)| (SpeedyResponse[2])); // 5,4 MAP high and low byte
   BARO         = SpeedyResponse[38]; // 40
   boost        = float(MAP - 100)/100;
   RPM          = ((SpeedyResponse[13] << 8)| (SpeedyResponse[12])); // 15,14 RPM high and low byte
   AFR_RAW      = SpeedyResponse[8]; //10
   o2val        = float(AFR_RAW)/10; //
   TPS          = SpeedyResponse[22]; //TPS percent value
   FUEL         = SpeedyResponse[101]; // 103
   OIL          = SpeedyResponse[102]; // 104
   VOLT         = SpeedyResponse[7]; // 9
   TEMP         = SpeedyResponse[5] - 40; //7
   IAT          = SpeedyResponse[4] - 40; // 6
   voltval      = float(VOLT)/10;
   VSS          = ((SpeedyResponse[99] << 8)| (SpeedyResponse[88])); //VSS high and low byte
  }
  if (MAP > peakboost){ // If current map value is higher than peak, store in peak memory
   peakboost = MAP; }// Store current boost in peak memory
  }

void manageAC() {   // Read the AC inputs and turn AC on/off
 if (digitalRead(pinAC)==LOW && digitalRead(pinACoff)==HIGH && (TPS)<80) {
    digitalWrite(pinACon,LOW);
    } 
 else {
    digitalWrite(pinACon,HIGH);
 }
}

void sendLights()  //read park lights state and control brightness of screens
{
  pinLightsnew = digitalRead(pinLights);

  if (pinLightsnew != pinLightsstate) {
    if (pinLightsnew == HIGH){ 
                              nexSerial.print("dim=100");
                              nexSerial.print("\xFF\xFF\xFF");
                              }
    else {
          nexSerial.print("dim=10");
          nexSerial.print("\xFF\xFF\xFF");
          }
    pinLightsstate = pinLightsnew;
  }
}

void sendCmd()  // wrapper to send commands to Nextion screen
{
  nexSerial.print("x0.val="); nexSerial.print(AFR_RAW);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n0.val="); nexSerial.print(MAP);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n1.val=");nexSerial.print(IAT,0);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("x1.val="); nexSerial.print(VOLT);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n3.val="); nexSerial.print(peakboost);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
}

void loop() {
 if (Serial1.available () > 0) {  // read bytes from serial
  SpeedyResponse[ByteNumber ++] = Serial1.read();
 }
 if (ByteNumber > (ResponseLength +1)){          // After the data from speeduino has been received so time to process it
  oldtime = millis();          // All ok. zero out timeout calculation
  ByteNumber = 0;              // zero out the byte number pointer
  processData();               // do the necessary processing for received data
  servoData();                 // send to the servo gauge
  manageAC();                  //triggers AC on/off in Speduino
  displayData();               // only required for debugging
  sendLights();                // screen brightness
  sendCmd();                   // send values to Nextion screen
  requestData();               //restart data reading
 }
 if ( (millis()-oldtime) > 500) { // timeout if for some reason reading from Speeduino fails
  oldtime = millis();
  ByteNumber = 0;             // zero out the byte number pointer
  requestData();              //restart data reading
 }
}
