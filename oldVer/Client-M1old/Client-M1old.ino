//EQUIPMENT
#include <DHT.h>

// NETWORKS
#include <WiFi.h>
#include <WiFiUdp.h>
WiFiUDP udp;
char *ssid = "Natchpai";
char *pwd = "powerpay4";
const uint16_t port = 2001;
char packetBuffer[255];

IPAddress fix_address(172,20,10,5);
IPAddress subnet(255,255,255,240);
IPAddress gateway(172,20,10,1);
String name = "M1";
String myIP = "";
char *MainHost = "172.20.10.6";
String DATA[5] = {"0","0","0","0","0","0","0","0","0","0","0"}; // {client, equipment, ON/OFF, x, y}
//                                                             1/0

#define DHTPIN 4 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define LDR1_Pin 34

#define M1LED1 2
// uint8_t LED1_status = 0;
uint8_t M1LED1_value = 0;

#define PWM_Motor 3
#define IN1 19
#define IN2 18
uint8_t motorPow = 0;
uint16_t motorSpeed = 0;
String motorPosition = "S";
bool in1 = 0;
bool in2 = 0;  
uint8_t motorStatus = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.config(fix_address, gateway, subnet);
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(800);
  } 
  myIP = WiFi.localIP().toString();
  Serial.println(""); Serial.println("Connected");
  Serial.println("IP:" + myIP + " Port:" + String(port));
  
  udp.begin(port);

  // DHT 22 Pin 4
  dht.begin();
  // pinMode(LED1, OUTPUT);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(M1LED1, 0);
  //MOTOR
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  ledcSetup(0, 5000, 10);
  ledcAttachPin(PWM_Motor, 1);

}

void loop() {
  // put your main code here, to run repeatedly:
  int numbuff = udp.parsePacket();
  if(numbuff > 0) {
    int len = udp.read(packetBuffer, 255);
    if(len > 0) {
      packetBuffer[len] = '\0';
      String s(packetBuffer);
      // Serial.println(s);
      splitString(s);
      if (DATA[1] == "Temp" && DATA[2] == "1") {
        sendPacket(MainHost, compress(DATA[1],"1",attchDHT_temp(), "0"));
      }
      else if (DATA[1] == "Humi" && DATA[2] == "1") {
        sendPacket(MainHost, compress(DATA[1],"1",attchDHT_humi(), "0"));
      }
      else if (DATA[1] == "Light" && DATA[2] == "1") {
        sendPacket(MainHost, compress(DATA[1],"1",attchLDR(), "0"));
      }
      else if (DATA[1] == "M1LED1") {
        actionLED1(DATA[2].toInt(), DATA[3].toInt());
        sendPacket(MainHost, compress(DATA[1], led1pushStatus(DATA[2].toInt()), "0", "0"));
      }
      else if (DATA[1] == "Status") {
        sendPacket(MainHost, compress(DATA[1], pullStatus(), "0", "0"));
      }                                 
      else if (DATA[1] == "Mor") {   // M1_Mor_Enable_speed_position
        ActiveMotor();
        sendPacket(MainHost, compress(DATA[1], String(motorStatus), checkSpeed(), checkPosition()));
      }
    }
  }
}

void splitString(String str){
  int index = 0;
  String s = "";
  for(int i = 0;i<str.length();i++){
    if(str[i] != '_' && str[i] != '&'){
      s += str[i];      
    }else{
      DATA[index] = s;
      index++;
      s = "";
    }
  }
}

void sendPacket(char *targetHost, String str_mess) {
  udp.beginPacket(targetHost, port);
  char char_mess[255];
  sprintf(char_mess, "%s", str_mess.c_str());
  udp.printf(char_mess);
  udp.flush();
  udp.endPacket();
}

String compress(String eq, String IO, String data1, String data2) {
  String i = String(name + "_" + eq + "_" + IO + "_" + data1 + "_" + data2 + "&");
  // Serial.println(i);
  return i;
}




uint8_t count = 0;
String pullStatus() {
   if(count == 0) {
     count++;
     return ("IP: " + myIP + " PORT: " + port);
   }
   else if(count == 1) {
     count = 0;
     return "Normal State";
   }
}


String attchDHT_temp() {
  float t = dht.readTemperature();
  if(isnan(t)) {
    return "0";
  }else{
    return String(t);
  }
}

String attchDHT_humi() {
  float h = dht.readHumidity();
  if(isnan(h)) {
    return "0";
  }else{
    return String(h);
  }
}

String attchLDR() {
  float a = map(analogRead(LDR1_Pin), 0, 4096, 100, 0);
  return String(a);
}

void actionLED1(uint8_t st, uint8_t value) {
  M1LED1_value = map(value, 0, 20, 0, 255);
  if(st != 0) {
    ledcWrite(0, M1LED1_value);
  }
  else{
    ledcWrite(0, 0);
  }
}

String led1pushStatus(uint8_t st) {
  if(M1LED1_value == 0 || st == 0) {
    return "0";
  }
  else{
    return "1";
  }
}

String checkSpeed() {
  if(motorStatus == 1) {
    return String(map(motorSpeed,0 ,1000, 0, 100));
  }
  else{
    return "0";
  }
}

String checkPosition() {
  if(motorStatus == 1) {
    return motorPosition;
  }
  else{
    return "S";
  }
}

String setPosition(String data) {
  if(data == "L") {
    in1 = 0; in2 = 1;
  }
  else if(data == "R") {
    in1 = 1; in2 = 0;
  }
  else if(data == "S"){
    in1 = 0; in2 = 0;
  }
}

void ActiveMotor() {
  motorPow = DATA[2].toInt();
  motorSpeed = DATA[3].toInt();
  motorPosition = DATA[4];
  setPosition(motorPosition);
  if(motorPow == 0 || motorSpeed  == 0 || motorPosition == "S") { ledcWrite(1, 0); motorStatus = 0;}
  else { ledcWrite(1, motorSpeed); motorStatus = 1; }
}