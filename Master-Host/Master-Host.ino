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

bool con_state = false;
String myIP = "";
char *M1 = "172.20.10.5";
char *M2 = "172.20.10.4";
unsigned long time_out1;
unsigned long time_out2;
unsigned long times;
bool contiState = true;
uint8_t stateM1 = 1;
uint8_t stateM2 = 1;
uint8_t MaxstateM1 = 6; //6
uint8_t MaxstateM2 = 5;  // 5
String textInRam1 = "";
String textInRam2 = "";
String DATA[5] = {"0","0","0","0","0"}; // {client, equipment, ON/OFF, x, y}
//                                                             1/0

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
  Serial.begin(9600); //คำสั่งเรียงตามนี้เท่านั้น
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pwd); 
  WiFi.config(fix_address, gateway, subnet);
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(800);
  } 
  Serial.println(""); Serial.println("Connected");
  myIP = WiFi.localIP().toString();
  Serial.println("IP:" + myIP + " Port:" + String(port));
  udp.begin(port);
  timer.setInterval(200L, myTimer); 
}


void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();

  if(millis() - times > 100) {
      times = millis();
    if (stateM1 == 1) {
      request(M1, "M1_Temp_1_0_0&"); detect_Packet("M1");
    }

    else if (stateM1 == 2) {
      request(M1, "M1_Humi_1_0_0&"); detect_Packet("M1");
    }

    else if (stateM1 == 3) {
      request(M1, "M1_Light_1_0_0&"); detect_Packet("M1");
    }

    else if (stateM1 == 4) {
      request(M1, "M1_M1LED1_" + String(M1SW1_status) + "_" + String(M1LED1_value) + "_0&"); detect_Packet("M1");
    }

    else if(stateM1 == 5) {
      request(M1, "M1_Status&"); detect_Packet("M1");
    }

    else if(stateM1 == 6) { //M1_Mor_Enable_speed_position
      request(M1, "M1_Mor_" + String(motorPow) + "_" + String(motorSpeed) + "_" + changeSW2Text(SWLeft, SWRight) + "&"); detect_Packet("M1");
    }


    if (stateM2 == 1) {
      request(M2, "M2_Temp_1_0_0&"); detect_Packet("M2");
    }
    
    else if (stateM2 == 2) {
      request(M2, "M2_Humi_1_0_0&"); detect_Packet("M2");
    }

    else if (stateM2 == 3) {
      request(M2, "M2_Light_1_0_0&"); detect_Packet("M2");
    } 

    else if (stateM2 == 4) {
      request(M2, "M2_M2LED1_" + String(M2SW1_status) + "_" + String(M2LED1_value) + "_0&"); detect_Packet("M2");
    }

    else if(stateM2 == 5) {
      request(M2, "M2_Status&"); detect_Packet("M2");
    }

    shortData();
  }

  timer.run();
  
}

BLYNK_CONNECTED(){}

void Master2Client(char *target, String text) {
  if(target == M1) {
    Blynk.virtualWrite(V13, "M1: "+ text);
  }
  else if(target == M2) {
    Blynk.virtualWrite(V14, "M2: "+ text);
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
void shortData() {
  if(DATA[0] == "M1") {
    if(DATA[1] == "Temp") {
      M1_Temp = DATA[3].toFloat();   
    }
    else if(DATA[1] == "Humi") {
      M1_Humi = DATA[3].toFloat();    
    }
    else if(DATA[1] == "Light") {
      M1_Light = DATA[3].toFloat();
    }
    else if(DATA[1] == "M1LED1") {
      M1LED1_status = DATA[2].toInt();
    }
    else if(DATA[1] == "Status") {
      M1_status = DATA[2];
    }
    else if(DATA[1] == "Mor") {
      motorPowComeIn = DATA[3].toInt();
      motorPositionComeInChange = DATA[4];
    }
  }
  else if(DATA[0] == "M2") {
    if(DATA[1] == "Temp") {
      M2_Temp = DATA[3].toFloat();   
    }
    else if(DATA[1] == "Humi") {
      M2_Humi = DATA[3].toFloat();    
    }
    else if(DATA[1] == "Light") {
      M2_Light = DATA[3].toFloat();
    }
    else if(DATA[1] == "M2LED1") {
      M2LED1_status = DATA[2].toInt();
    }
    else if(DATA[1] == "Status") {
      M2_status = DATA[2];
    }
  }
}

void detect_Packet(String type) {
  int numbuff = udp.parsePacket();
  if(numbuff > 0) {
    int len = udp.read(packetBuffer, 255);
    if(len > 0) {
      packetBuffer[len] = '\0';
      String s(packetBuffer);
      splitString(s);
      nextState(type);
      Serial.println(s);
    }
  }
}

void nextState(String type) {
  if(type == "M1") {
    stateM1 += 1; 
    if(stateM1 > MaxstateM1) stateM1 = 1;
  }
  else if(type == "M2") {
    stateM2 += 1; 
    if(stateM2 > MaxstateM2) stateM2 = 1;
  }
}

void request(char *targetHost, String text){
  if(text != textInRam1 && targetHost == M1) {
    textInRam1 = text;
    Master2Client(targetHost, "Normal State");
    sendPacket(targetHost, text); 
    time_out1 = millis();
  }
  else if(text != textInRam2 && targetHost == M2) {
    textInRam2 = text;
    Master2Client(targetHost, "Normal State");
    sendPacket(targetHost, text); 
    time_out2 = millis();
  }

  if((millis() - time_out1 >= 1500) && text == textInRam1) {
    time_out1 = millis();
    Serial.println(String(targetHost) + " Request timed out.");
    Master2Client(targetHost, " Request timed out.");
    M1_status = "Lost connection!";
    sendPacket(targetHost, text);
  }

  if((millis() - time_out2 >= 1500) && text == textInRam2) {
    time_out2 = millis();
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
