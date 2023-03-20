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
String DATA[11] = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"}; 

#define DHTPIN 4 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
unsigned long pullDHT;
float t = 0;
float h = 0;

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
bool startMotor = true;
uint16_t motorRawSpeed = 0;

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
  ledcSetup(1, 5000, 10);
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
      splitString(s);
      Serial.println(s);
      operateDataINPUI();
      responds();
    }
  }
  if(millis() - pullDHT >= 2000) {pullDHT = millis(); runTemp();}
}
// {client, equipment, io, x, y}
//    0       1       2   3   4  
 // 0  1    2    3    4     *5     *6      7    *8    *9      *10
// M1_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St_mPow_mSpeed_dir&

void operateDataINPUI() {
    ActiveMotor();
    actionLED1(DATA[5].toInt(), DATA[6].toInt());
}

void responds(){
  sendPacket(MainHost, compress(attchDHT_temp(), attchDHT_humi(), attchLDR()
  , led1pushStatus(DATA[2].toInt()) ,"X", pullStatus()
  , String(motorStatus), checkSpeed(), checkPosition()));
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

String compress(String a2, String a3, String a4, String a5, String a6, String a7, String a8, String a9, String a10) {
  String i = String(name + "_A_" + a2 + "_" + a3 + "_" + a4 + "_" 
  + a5 + "_" + a6 + "_" + a7 + "_" + a8 + "_" + a9 + "_" + a10 + "&");
  // Serial.println(i);
  return i;
}


String pullStatus() {
    return ("IP: " + myIP + " PORT: " + port);
}


String attchDHT_temp() {
  return String(t);
}


void runTemp() {
  float H = dht.readHumidity();
  float T = dht.readTemperature();
  if(!isnan(H)) {
    h = H;
  }
  if(!isnan(T)) {
    t = T;
  }
}

String attchDHT_humi() {
  return String(h);
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
    return String(map(motorSpeed,600 ,1000, 5, 100));
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

void setPosition(String data) {
  if(data == "L") {
    in1 = 0; in2 = 1;
  }
  else if(data == "R") {
    in1 = 1; in2 = 0;
  }
  else if(data == "S"){
    in1 = 0; in2 = 0;
  }
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
}
 // 0  1    2    3    4     *5     *6      7    *8    *9      *10
// M1_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St_mPow_mSpeed_dir&

void ActiveMotor() {
  motorPow = DATA[8].toInt();
  motorRawSpeed = DATA[9].toInt();
  if(motorRawSpeed != 0) {motorSpeed = map(motorRawSpeed, 0, 1000, 600, 1000);}
  else{motorSpeed = 0;}
  motorPosition = DATA[10];
  setPosition(motorPosition);
  if(motorPow == 0 || motorSpeed  == 0 || motorPosition == "S") { 
    ledcWrite(1, 0); motorStatus = 0;
    startMotor = true;
  }
  else { 
    if(startMotor == true) {
      ledcWrite(1, 1000); delay(50);
      startMotor = false;
    }
    ledcWrite(1, motorSpeed); motorStatus = 1; 
  }
}