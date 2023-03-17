#include <Blynk.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define BLYNK_TEMPLATE_ID "TMPL6BMsKseO"
#define BLYNK_TEMPLATE_NAME "SmartFC"
#define BLYNK_AUTH_TOKEN "-3Kwi8I1xxF5xU97xNr5q3JzXjWvOvP_"
#define BLYNK_PRINT Serial

WiFiUDP udp;

char ssid[] = "Natchpai";
char pwd[] = "powerpay4";
const uint16_t port = 2001;
char packetBuffer[255];

IPAddress fix_address(172,20,10,6);
IPAddress subnet(255,255,255,240);
IPAddress gateway(172,20,10,1);

char *M1 = "172.20.10.5";
char *M2 = "172.20.10.4";
unsigned long time_out;
unsigned long times;
bool contiState = true;
uint8_t state = 1;
uint8_t Maxstate = 3;
String textInRam = "";
String DATA[5] = {"0","0","0","0","0"}; // {client, equipment, I/O, x, y}
//                                                             1/0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //คำสั่งเรียงตามนี้เท่านั้น
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pwd); 
  WiFi.config(fix_address, gateway, subnet);
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(800);
  } 
  Serial.println(""); Serial.println("Connected");
  Serial.println("IP:" + WiFi.localIP().toString() + " Port:" + String(port));
  udp.begin(port);
}


void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  if(millis() - times > 100) {
    times = millis();
    if (state == 1) {
      request(M1, "M1_Temp_0_0&"); detect_Packet();
    }
    else if (state == 2) {
      request(M1, "M1_Humi_0_0&"); detect_Packet();
    }
    else if (state == 3) {
      request(M1, "A3_0_0_0&"); detect_Packet();
    }
  }
  pushToBlynk();
}

BLYNK_CONNECTED(){}

void pushToBlynk() {
  if(DATA[1] == "Temp") {
    Blynk.virtualWrite(V0, "CUNNY");
    Blynk.virtualWrite(V1, DATA[3].toFloat());
    nextState();
  }
  else if(DATA[1] == "Humi") {
    Blynk.virtualWrite(V2, DATA[3].toFloat());
    nextState();
  }
}



void detect_Packet() {
  int numbuff = udp.parsePacket();
  if(numbuff > 0) {
    int len = udp.read(packetBuffer, 255);
    if(len > 0) {
      packetBuffer[len] = '\0';
      String s(packetBuffer);
      Serial.println(s);
      splitString(s);
    }
  }
}

void nextState() {state += 1; if(state > Maxstate) state = 1;}

void request(char *targetHost, String text){
  if(text != textInRam) {
    textInRam = text;
    sendPacket(targetHost, text); 
    time_out = millis();
  }

  else if((millis() - time_out >= 1500) && text == textInRam) {
    time_out = millis();
    Serial.println(String(targetHost) + " Request timed out.");
    sendPacket(targetHost, text);
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
