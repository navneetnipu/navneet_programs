// Thingspeak 
String canalID1="744923";
String canalID2="744927";
String statusChWriteKey ="EZ8HC0E068628VGS";  // Status Channel id: 743109
int lpg_sensor_status;
int spare;
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#define HARDWARE_RESET 8
#define FREEZE_LED 13
#define lpg_gas_sensor 10
#define servomotor 11
#define alarm 12
#include <stdlib.h>
// Variables to be used with timers
long writeTimingSeconds = 17; // ==> Define Sample time in seconds to send data
long readTimingSeconds = 10; // ==> Define Sample time in seconds to receive data
long startReadTiming = 0;
long elapsedReadTime = 0;
long startWriteTiming = 0;
long elapsedWriteTime = 0;
boolean knob;
boolean Alarm;
boolean error;
void setup()
{
  Serial.begin(9600);
  pinMode(HARDWARE_RESET,OUTPUT);
  pinMode(lpg_gas_sensor,INPUT);
  pinMode(servomotor,OUTPUT);
  pinMode(FREEZE_LED,OUTPUT);
  pinMode(alarm,OUTPUT);
  digitalWrite(HARDWARE_RESET, HIGH);
  digitalWrite(FREEZE_LED, LOW);
  EspSerial.begin(9600); // Comunicate with  WiFi module
  EspHardwareReset(); //Reset WiFi module
  startReadTiming = millis();
  startWriteTiming = millis(); // starting the "program clock"
}
void loop()
{
  start: //label 
  error=0;
  
  elapsedWriteTime = millis()-startWriteTiming; 
  elapsedReadTime = millis()-startReadTiming; 
  if (elapsedReadTime >(readTimingSeconds*1000)) 
  {
    ESPcheck();
    int command = readThingSpeak(canalID1); 
    Serial.print(command);
    if(command != 9){
      knob = command;
    } 
    delay (1000); 
    command = readThingSpeak(canalID2); 
    Serial.print(command+"\n");
    if (command != 9){
      Alarm = command; 
    }
    takeActions();
    startReadTiming = millis();   
  }
  
  if (elapsedWriteTime >(writeTimingSeconds*1000)) 
  {
    ESPcheck();
    readSensors();
    writeThingSpeak();
    startWriteTiming = millis();   
  }
  
  if (error==1) //Resend if transmission is not completed 
  {       
    Serial.println(" <<<< ERROR >>>>\n"); 
    digitalWrite(FREEZE_LED, HIGH);
    delay (2000); 
    goto start; //go to label "start"
  }
}
/********* Read Sensors value *************/
void readSensors(void)
{ 
  lpg_sensor_status=digitalRead(lpg_gas_sensor);
}
void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += statusChWriteKey;
  getStr +="&field1=";
  getStr += String(lpg_sensor_status);
  getStr +="&field2=";
  getStr += String(knob);
  getStr +="&field3=";
  getStr += String(Alarm);
  getStr += "\r\n\r\n";
  sendThingSpeakGetCmd(getStr); 
}
/********* Reset ESP *************/
void EspHardwareReset(void)
{
  Serial.println("Reseting......."); 
  digitalWrite(HARDWARE_RESET, LOW); 
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(8000);
  Serial.println("RESET"); 
}
/********* Start communication with ThingSpeak*************/
void startThingSpeakCmd(void)
{
  EspSerial.flush();
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149";
  cmd += "\",80";
  EspSerial.println(cmd);
  Serial.print("Start cmd: ");
  Serial.println(cmd);
  if(EspSerial.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}
/********* send a GET cmd to ThingSpeak *************/
String sendThingSpeakGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  EspSerial.println(cmd);
  Serial.print("lenght cmd: ");
  Serial.println(cmd);
  if(EspSerial.find((char *)">"))
  {
    EspSerial.print(getStr);
    Serial.print("getStr: ");
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (EspSerial.available()) 
    {
      String line = EspSerial.readStringUntil('\n');
      if (line.length() == 1) 
      { //actual content starts after empty line (that has length 1)
        messageBody = EspSerial.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    EspSerial.println("AT+CIPCLOSE");     // alert user
    Serial.println("ESP8266 CIPSEND ERROR: RESENDING"); //Resend...
    spare = spare + 1;
    error=1;
    return "error";
  } 
} 
int readThingSpeak(String channelID)
{
  startThingSpeakCmd();
  int command;
  // preparacao da string GET
  String getStr = "GET /channels/";
  getStr += channelID;
  getStr +="/fields/1/last";
  getStr += "\r\n";

  String messageDown = sendThingSpeakGetCmd(getStr);
  if (messageDown[5] == 49)
  {
    command = messageDown[7]-48; 
    Serial.print("Command received: ");
    Serial.println(command);
  }
  else command = 9;
  return command;
}
void takeActions(void)
{
  Serial.print("lpg_sensor_status is ");
  Serial.println(lpg_sensor_status);
  Serial.print("knob: ");
  Serial.println(knob);
  Serial.print("alarm: ");
  Serial.println(Alarm);
  if (knob == 1) digitalWrite(servomotor, HIGH);
  else digitalWrite(servomotor, LOW);
  if (Alarm == 1) digitalWrite(alarm, HIGH);
  else digitalWrite(alarm,LOW);
}
boolean ESPcheck(void)
{
  EspSerial.println("AT"); // Send "AT+" command to module
   
  if (echoFind("OK")) 
  {
    //Serial.println("ESP ok");
    digitalWrite(FREEZE_LED, LOW);
    return true; 
  }
  else //Freeze ou Busy
  {
    Serial.println("ESP Freeze *********************************************************************");
    digitalWrite(FREEZE_LED, HIGH);
    EspHardwareReset();
    return false;  
  }
}
boolean echoFind(String keyword)
{
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + 5000;
 while(millis() < deadline){
  if (EspSerial.available()){
    char ch = EspSerial.read();
    Serial.write(ch);
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
       Serial.println();
       return true;
    }
   }
  }
 return false;
}
