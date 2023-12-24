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

char ssid[] = "TP-Link_93C4";
char pwd[] = "47299238";
const uint16_t port = 2001;
char packetBuffer[255];

IPAddress fix_address(172,20,10,6);
IPAddress subnet(255,255,255,240);
IPAddress gateway(172,20,10,1);

bool con_state = false;
String myIP = "";
char *M1 = "172.20.10.5";
char *M2 = "172.20.10.4";
unsigned long time_out1;
unsigned long time_out2;
unsigned long times;
bool contiState = true;
uint8_t Mstate = 1;
uint8_t Maxstate = 2;
String textInRam1 = "";
String textInRam2 = "";
String MastertoC1 = "";
String MastertoC2 = "";
bool responseM1 = true;
bool responseM2 = true;

#define statusLEDM1 19
#define statusLEDM2 18
#define statusLEDWifi 5

// 0  1    2    3    4     5     6      7    8    9      10
// M1_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St_mPow_mSpeed_dir&
// M2_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St&
String DATA[11] = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};                                 

float M1_Temp; float M1_Humi; float M1_Light;
uint8_t M1LED1_status = 0;
uint8_t M1LED1_value = 0;
uint8_t M1SW1_status = 0;
String M1_status = "%NA";

float M2_Temp; float M2_Humi; float M2_Light;
uint8_t M2LED1_status = 0;
uint8_t M2LED1_value = 0;
uint8_t M2SW1_status = 0;
String M2_status = "%NA";

uint8_t motorPow = 0;
uint8_t SWLeft = 0;
uint8_t SWRight = 0;
uint16_t motorSpeed = 0;
String motorPosition = "S";
String motorPositionComeIn = "S";
String motorPositionComeInChange = "STOP";
uint8_t motorPowComeIn = 0;

BlynkTimer timer; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //คำสั่งเรียงตามนี้เท่านั้น
  pinMode(statusLEDM1, OUTPUT);
  pinMode(statusLEDM2, OUTPUT);
  pinMode(statusLEDWifi, OUTPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pwd); 
  WiFi.config(fix_address, gateway, subnet);
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED) {
    delay(800);
  } 
  Serial.println(""); Serial.println("Connected");
  myIP = WiFi.localIP().toString();
  Serial.println("IP:" + myIP + " Port:" + String(port));
  udp.begin(port);
  timer.setInterval(1000L, myTimer); 
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  // ss();
  
 if(millis() - times > 500) {//LEDsw_LEDval_M1St_mPow_mSpeed_dir&

    if(WiFi.status() != WL_CONNECTED) digitalWrite(statusLEDWifi, 0);
    else digitalWrite(statusLEDWifi, 1);

    times = millis();
    if(Mstate == 1) {
      request(M1, "M1_A_T_H_L_" + String(M1SW1_status) + "_" + String(M1LED1_value) + "_Status_"
      + String(motorPow) + "_" + String(motorSpeed) + "_" + changeSW2Text(SWLeft, SWRight) + "&" ); 
      nextState();
    }
    else if(Mstate == 2) {
      request(M2, "M2_A_T_H_L_" + String(M2SW1_status) + "_" + String(M2LED1_value) + "_Status&"); 
      nextState();
    }
   
 }
  detect_Packet();
  timer.run();
}

BLYNK_CONNECTED(){}

void Master2Client(char *target, String text) {
  if(target == M1) {
    MastertoC1 = "M1: " + text;
  }
  else if(target == M2) {
    MastertoC2 = "M2: " + text;
  }
}

void myTimer() {
    
  // Blynk.virtualWrite(V0, "CUNNY");
  Blynk.virtualWrite(V1, M1_Temp);
  Blynk.virtualWrite(V2, M1_Humi);
  Blynk.virtualWrite(V3, M1_Light);
  Blynk.virtualWrite(V5, M1LED1_status);

  Blynk.virtualWrite(V7, M2_Temp);
  Blynk.virtualWrite(V8, M2_Humi);
  Blynk.virtualWrite(V9, M2_Light);
  Blynk.virtualWrite(V11, M2LED1_status);

  Blynk.virtualWrite(V15, M1_status);  
  Blynk.virtualWrite(V16, M2_status);

  Blynk.virtualWrite(V17, motorPowComeIn);
  Blynk.virtualWrite(V22, changeText(motorPositionComeInChange));

  Blynk.virtualWrite(V13, MastertoC1);
  Blynk.virtualWrite(V14, MastertoC2);
}

BLYNK_WRITE(V4) {
  M1SW1_status = param.asInt();
}

BLYNK_WRITE(V6) {
  M1LED1_value = param.asInt();
}

BLYNK_WRITE(V10) {
  M2SW1_status = param.asInt();
}

BLYNK_WRITE(V12) {
  M2LED1_value = param.asInt();
}

BLYNK_WRITE(V21) {
  motorPow = param.asInt();
}

BLYNK_WRITE(V19) {
  SWLeft = param.asInt();
}

BLYNK_WRITE(V20) {
  SWRight = param.asInt();
}

BLYNK_WRITE(V18) {
  motorSpeed = param.asInt();
}

String changeSW2Text(uint8_t x, uint8_t y) {
  if(x == 0 && y == 1) {return "R";}
  else if(x == 1 && y == 0) {return "L";}
  else{return "S";}
}

String changeText(String data) {
  if(data == "L") {
    return "Left Rotation";
  }
 else if(data == "R") {
    return "Right Rotation";
  }
  else if(data == "S") {
    return "STOP";
  }
}
// 0  1    2    3    4     5     6      7    8    9      10
// M1_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St_mPow_mSpeed_dir&
// M2_Auto_Temp_Humi_Light_LEDsw_LEDval_M1St&
void shortData() {
  if(DATA[0] == "M1") {

    responseM1 = true;

    M1_Temp = DATA[2].toFloat();   

    M1_Humi = DATA[3].toFloat();    

    M1_Light = DATA[4].toFloat();
    
    if(DATA[5] != "X") M1LED1_status = DATA[5].toInt();

    M1_status = DATA[7];
    
    if(DATA[8] != "X") motorPowComeIn = DATA[9].toInt();
    if(DATA[10] != "X") motorPositionComeInChange = DATA[10];
    
  }
  else if(DATA[0] == "M2") {

    responseM2 = true;

    M2_Temp = DATA[2].toFloat();   

    M2_Humi = DATA[3].toFloat();    
    
    M2_Light = DATA[4].toFloat();
    
    if(DATA[5] != "X") M2LED1_status = DATA[5].toInt();
    
    M2_status = DATA[7];
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
      shortData();
    }
  }
}

void nextState() {
  Mstate += 1; 
  if(Mstate > Maxstate) Mstate = 1;
}

void request(char *targetHost, String text){
  if(targetHost == M1 && responseM1 == true) {
    Master2Client(targetHost, "Normal State");
    sendPacket(targetHost, text); 
    time_out1 = millis();
    digitalWrite(statusLEDM1, 1);
    responseM1 = false;
    if(M1_status == "Lost connection!") M1_status = "Connected to Master";
  }
  else if((millis() - time_out1 >= 5000)) {
    time_out1 = millis();
    digitalWrite(statusLEDM1, 0);
    Serial.println(String(targetHost) + " Request timed out.");
    Master2Client(targetHost, " Request timed out.");
    M1_status = "Lost connection!";
    sendPacket(targetHost, text);
  }

  if(targetHost == M2 && responseM2 == true) {
    Master2Client(targetHost, "Normal State");
    sendPacket(targetHost, text); 
    time_out2 = millis();
    digitalWrite(statusLEDM2, 1);
    responseM2 = false;
    if(M2_status == "Lost connection!") M2_status = "Connected to Master";
  }
  else if((millis() - time_out2 >= 5000)) {
    time_out2 = millis();
    digitalWrite(statusLEDM2, 0);
    Serial.println(String(targetHost) + " Request timed out.");
    Master2Client(targetHost, " Request timed out.");
    M2_status = "Lost connection!";
    sendPacket(targetHost, text);;
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

