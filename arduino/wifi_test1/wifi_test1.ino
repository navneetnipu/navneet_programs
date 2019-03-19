#include <SoftwareSerial.h>

SoftwareSerial esp8266 (11, 10);

String ssid = "NIPU1";
String password = "nipu123456";


String server = "api.virtualworld.today";

const int timeout = 2000;

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200); // Should match ESP's current baudrate
  
  setupESP8266();
  connectToWiFi();

  startTCPConnection();
  sendDataRequest();
  sendStatusRequest();
  closeTCPConnection();
}

void loop() {

  startTCPConnection();
  sendDataRequest();
  sendStatusRequest();
  closeTCPConnection();
  
}

String atCommand(String command, int timeout) {
  String response = "";
  esp8266.println(command);

  long int time = millis();

  while( (time+timeout) > millis() ) {
    while(esp8266.available()) {
      char c = esp8266.read();
      response += c;
    }
  }

  Serial.println(response);
  return response;
}

void setupESP8266() {
  atCommand("AT+RST", timeout);
  atCommand("AT+CWMODE=1", timeout);
}

void connectToWiFi() {
  String connect = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  atCommand(connect, 6000);
  atCommand("AT+CIFSR", timeout);
}

void startTCPConnection() {
  String connect = "AT+CIPSTART=\"TCP\",\"" + server + "\",80";
  atCommand(connect, timeout);
}

void closeTCPConnection() {
  atCommand("AT+CIPCLOSE", timeout);
}

String getData(){
  return "arduino"+String(millis());
}
String sendDataRequest() {
  String send_path = "/receive.php?name="+getData();

  String request = "GET " + send_path + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
  String requestLength = String(request.length());

  atCommand("AT+CIPSEND=" + requestLength, timeout);
  String response = atCommand(request, 6000);
  return response;
}
String sendStatusRequest() {
String status_path = "/status.php";

  String request = "GET " + status_path + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
  String requestLength = String(request.length());
  atCommand("AT+CIPSEND=" + requestLength, timeout);
  String response = atCommand(request, 6000);
  return response;
}
