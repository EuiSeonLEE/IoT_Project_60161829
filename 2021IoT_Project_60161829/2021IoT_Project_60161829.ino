#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "AnotherIFTTTWebhook.h"

#define trig 12    // D6 : GPIO12
#define echo 13    // D7 : GPIO13
#define ANALOGPIN A0   // MQ-135

HTTPClient myClient;
LiquidCrystal_I2C lcd(0x27,16,2);
const int led2 = 2;
int BTT_result, BTT_chose;
int mqdata = 0;

void inputpw(String *password){
  int value = 0;
  int value2 = 0;
  char wifipw[12] = {0,};
  int cnt = 0;
  lcd.clear();
  lcd.print("['{' = Retyping]");
  lcd.setCursor(0, 1);
  lcd.print("PW: ");
 
  while(1){
    if(cnt >= 12){ //비밀번호 12자리 까지
      Serial.printf("pw = %s\r\n",wifipw);
      *password = (String)wifipw;
      Serial.println(*password);
      break;
    }
    delay(100);
    BTT_chose = digitalRead(13);
    BTT_result = digitalRead(14); //스위치 읽기
    
    value +=  1 * BTT_chose; //스위치 읽기

    wifipw[cnt] = (value+31)%124;
    if(wifipw[cnt] == 31){
      wifipw[cnt] = 0;
    }
    
    delay(50);
    lcd.setCursor(cnt+3, 1);
    lcd.print((String)wifipw[cnt]);
    Serial.printf("pw[%d] = %c, btt = %d\r\n",cnt,wifipw[cnt],BTT_result);
    if(BTT_result){ 
      if(wifipw[cnt] == 123){ //아스키코드 123 입력시 다시입력
      lcd.clear();
      lcd.print("['{' = Retyping]");
      lcd.setCursor(0, 1);
      lcd.print("PW:");
      cnt = 0;
      delay(500); 
      }
      else{ //아니면 다음 문자 선택
      cnt++;
      delay(500);
      }
    }
  }
}

void setup() {
 Serial.begin(115200);
 delay(1000);
 
 lcd.init();
 lcd.backlight();
 lcd.print("Looking for");
 lcd.setCursor(0, 1);
 lcd.print("WiFi...");
 WiFi.mode(WIFI_AP_STA);
 
 Serial.println(WiFi.getMode());
 int foundN = WiFi.scanNetworks(0,1);//찾아낸 WiFi 갯수

 Serial.printf("I found ssid n= %d \r\n",foundN);
 int i=0;
 int mywifi;
 String id;
 String pw;
 int32_t rssi = WiFi.RSSI(0); //처음의 WiFi의 RSSI
  
 for(i=1;i<foundN;i++){
  Serial.printf("SSID: %s, WIFIStrength :%d\r \n",
    WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    if(rssi < WiFi.RSSI(i)){ //다음 WiFi의 RSSI와 비교
      rssi = WiFi.RSSI(i);
      id = WiFi.SSID(i);
      mywifi = i;
    }
 }
  if(WiFi.encryptionType(mywifi) == ENC_TYPE_NONE){
      pw = "";
  }
  else{
     //패스워드 입력 함수(스위치로 아스키코드 숫자 올라감)
     //pw = "dmltjs123@";
     inputpw(&pw);
  }
      
 Serial.printf("id = %s, ssid = %d, pw = %s\r\n",id.c_str(),rssi,pw.c_str());
 WiFi.begin(id,pw);
 WiFi.setAutoReconnect(1);
 while(1)
 {
  lcd.clear();
  lcd.print("WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  int ConnResult = WiFi.status();
  if(ConnResult == WL_CONNECTED){
     lcd.clear();
    lcd.print("Connect Success!!");
    Serial.printf("Connection OK!\r\n");
  break;
  }
 else{
  Serial.printf("Connextion Failed, reason:%d\r\n",ConnResult);
    if(ConnResult == 4){
      lcd.clear();
      lcd.print("Connect Fail!!");
      lcd.setCursor(0, 1);
      lcd.print("[Push the Reset]");
      delay(500);
    }
  delay(500);
  }
   
 }

 Serial.printf("MY Mac Address : %s\r\n",WiFi.macAddress().c_str());
 Serial.printf("MY IP Address : %s\r\n",WiFi.localIP().toString().c_str());

 pinMode(led2, OUTPUT);

 pinMode(trig, OUTPUT);    // trig 와 연결된 핀(D6)을 출력핀으로 사용
 pinMode(echo, INPUT);     // echo 와 연결된 핀(D7)을 입력핀으로 사용
 Serial.println("측정 시작!!!");
}


void loop() {
 
    int readTemp;
  
    delay(200000);
}
