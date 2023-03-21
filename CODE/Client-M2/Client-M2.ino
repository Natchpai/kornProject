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

IPAddress fix_address(172,20,10,4);
IPAddress subnet(255,255,255,240);
IPAddress gateway(172,20,10,1);
String myIP = "";
String name = "M2";
char *MainHost = "172.20.10.6";

// 0  1    2    3    4     5     6      7    8    9      10
// M1_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St_mPow_mSpeed_dir&
// M2_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St&
String DATA[11] = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};


#define DHTPIN 4 
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
unsigned long pullDHT;
float t = 0;
float h = 0;
bool DHTDis = false;

#define statusLEDWifi 5
#define LDR1_Pin 34

#define M2LED1 2
uint8_t M2LED1_value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.config(fix_address, gateway, subnet);
  WiFi.begin(ssid, pwd);
  pinMode(statusLEDWifi, OUTPUT);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(800);
  } 
  myIP = WiFi.localIP().toString();
  Serial.println(""); Serial.println("Connected");
  Serial.println("IP:" + myIP + " Port:" + String(port));
  udp.begin(port);

  dht.begin();
  ledcSetup(0, 5000, 8);
  ledcAttachPin(M2LED1, 0);
  
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
      respond();
    }
  }
  if(millis() - pullDHT >= 2000) {
    pullDHT = millis(); runTemp();
    if(WiFi.status() != WL_CONNECTED) digitalWrite(statusLEDWifi, 0);
    else digitalWrite(statusLEDWifi, 1);
  }
}


void operateDataINPUI() {
  actionLED1(DATA[5].toInt(), DATA[6].toInt());
}

void respond(){
  sendPacket(MainHost, compress(attchDHT_temp(), attchDHT_humi(), 
  attchLDR(), led1pushStatus(DATA[5].toInt()) ,"X", pullStatus(), "X", "X", "X"));
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
  if(DHTDis == true) {
    return "DHT Connection lost!";
  }
  else{
    return ("IP: " + myIP + " PORT: " + port);
  }
}


String attchDHT_temp() {
  return String(t);
}

void runTemp() {
  float H = dht.readHumidity();
  float T = dht.readTemperature();
  if(!isnan(H)) {
    DHTDis = false;
    h = H;
  }
  if(!isnan(T)) {
    t = T;
  }
  else{
    DHTDis = true;
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
  M2LED1_value = map(value, 0, 20, 0, 255);
  Serial.println(String(st) + "/" +String(M2LED1_value));
  if(st != 0) {
    ledcWrite(0, M2LED1_value);
  }
  else{
    ledcWrite(0, 0);
  }
}

String led1pushStatus(uint8_t st) {
  if(M2LED1_value == 0 || st == 0) {
    return "0";
  }
  else{
    return "1";
  }
}
