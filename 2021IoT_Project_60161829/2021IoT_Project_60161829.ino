#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "AnotherIFTTTWebhook.h"

#define trig 12    // D6 : GPIO12
#define echo 14    // D5 : GPIO14
#define ANALOGPIN A0   // MQ-135

HTTPClient myClient;
LiquidCrystal_I2C lcd(0x27, 16, 2);
int BTT_result, BTT_chose;
int mqdata = 0;
String temp = "";
void inputpw(String *password) {
  int value = 0;
  int value2 = 0;
  char wifipw[12] = {0,};
  int cnt = 0;
  lcd.clear();
  lcd.print("['{' = Retyping]");
  lcd.setCursor(0, 1);
  lcd.print("PW: ");

  while (1) {
    if (cnt >= 12) { //비밀번호 12자리 까지
      Serial.printf("pw = %s\r\n", wifipw);
      *password = (String)wifipw;
      Serial.println(*password);
      break;
    }
    delay(100);
    BTT_chose = digitalRead(13);
    BTT_result = digitalRead(15); //스위치 읽기

    value +=  1 * BTT_chose; //스위치 읽기

    wifipw[cnt] = (value + 31) % 124;
    if (wifipw[cnt] <= 31) {
      wifipw[cnt] = 0;
    }

    delay(50);
    lcd.setCursor(cnt + 3, 1);
    lcd.print((String)wifipw[cnt]);
    Serial.printf("pw[%d] = %c, btt = %d\r\n", cnt, wifipw[cnt], BTT_result);
    if (BTT_result) {
      if (wifipw[cnt] == 123) { //아스키코드 123 입력시 다시입력
        lcd.clear();
        lcd.print("['{' = Retyping]");
        lcd.setCursor(0, 1);
        lcd.print("PW:");
        cnt = 0;
        delay(500);
      }
      else { //아니면 다음 문자 선택
        cnt++;
        delay(500);
      }
    }
  }
}
void inc_insertion_sort(int list[], String list2[], int list3[], int first, int last, int gap) {
  int i, j, key, key3;
  String key2;
  for (i = first + gap; i <= last; i = i + gap) {
    key = list[i];
    key2 = list2[i];
    key3 = list3[i];
    for (j = i - gap; j >= first && key < list[j]; j = j - gap) {
      list[j + gap] = list[j];
      list2[j + gap] = list2[j];
      list3[j + gap] = list3[j];
    }
    list[j + gap] = key;
    list2[j + gap] = key2;
    list3[j + gap] = key3;
  }
}
void shell_sort(int list[], String list2[], int list3[], int n) {
  int i, gap;
  for (gap = n / 2; gap > 0; gap = gap / 2) {
    if ((gap % 2) == 0) gap++;
    for (i = 0; i < gap; i++) {
      inc_insertion_sort(list, list2, list3, i, n - 1, gap);
    }
  }
}
String split(String str, String b) {
  int index1 = 0, index2 = 0, index3 = 0;
  String sub = str;
  index2 = sub.length();

  while (index2 != NULL) //sub문자열이 NULL이 될 때까지
  {
    index1 = sub.indexOf(b); //":"가 있는 문자 갯수
    index3 = sub.indexOf(")"); // ")"가 있는 문자 갯수
    if ((b == ":") && (index1 != -1) && (index3 != -1)) { // ":",")" 둘다 존재 한다면
      temp = sub.substring(0, index1); // 0번째 부터 ":"가 있는 문자 갯수만큼 String복사
      sub = sub.substring(index1 + 1); // ":"다음 문자열부터 문자열 끝까지 String 복사
    }
    else if ((b == ":") && index3 != -1) { //")"만 존재한다면
      temp = sub.substring(0, index3); //0번째 부터 ")"가 있는 문자 갯수만큼 String 복사
      sub = sub.substring(index3 + 1); // NULL이 되게 만듦
    }
    else {
      break;//while문에서 exit
    }
    if ((b == ",") && (index1 != -1)) {
      temp = sub.substring(0, index1);
      sub = sub.substring(index1 + 1);
    }
    else  { //전부 존재하지 않는다면
      //Serial.println(sub);
      return sub;
    }
    //Serial.println(sub);
    return sub;
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(15, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.print("Looking for");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  WiFi.mode(WIFI_AP_STA);

  Serial.println(WiFi.getMode());
  int foundN = WiFi.scanNetworks(0, 1); //찾아낸 WiFi 갯수

  Serial.printf("I found ssid n= %d \r\n", foundN);
  int i = 0;
  int mywifi[foundN];
  String id[foundN];
  int pw[foundN];
  int32_t rssi[foundN]; //처음의 WiFi의 RSSI

  for (i = 0; i < foundN; i++) {
    rssi[i] = WiFi.RSSI(i);
    id[i] = WiFi.SSID(i);
    mywifi[i] = i;
    Serial.printf("(before)SSID: %s, WIFIStrength :%d, num :%d\r \n",
                  id[i].c_str(), rssi[i], mywifi[i]);
    /*if (rssi < WiFi.RSSI(i)) { //다음 WiFi의 RSSI와 비교
      rssi = WiFi.RSSI(i);
      id = WiFi.SSID(i);
      mywifi = i;
      }*/
  }
  shell_sort(rssi, id, mywifi, foundN);
  for (i = 0; i < foundN; i++) {
    if (WiFi.encryptionType(mywifi[i]) == ENC_TYPE_NONE) {
      pw[i] = 0;
    }
    else pw[i] = 1;
    Serial.printf("(after)SSID: %s, WIFIStrength :%d, num :%d, pw :%d\r \n",
                  id[i].c_str(), rssi[i], mywifi[i], pw[i]);
  }
  //nodemcu.write(String(foundN));
  String WiFi_id = "", WiFi_pw = "";
  bool c = false;
  while (1) {
    String ch;
    while (nodemcu.available() > 0) {
      ch = nodemcu.readString();

      Serial.println(String(ch).toInt());
      if (String(ch).toInt() == 0) {
        nodemcu.write(String("(1" + id[foundN - 1] + ":" + id[foundN - 2] + ":" + id[foundN - 3] + ":" + id[foundN - 4] + ":" + id[foundN - 5] + ")"));
      }
      else if (String(ch).toInt() == 1) {
        nodemcu.write(String("(2" + pw[foundN - 1] + ":" + pw[foundN - 2] + ":" + pw[foundN - 3] + ":" + pw[foundN - 4] + ":" + pw[foundN - 5] + ")"));
      }
      else  {
        ch = split(ch, ":");
        WiFi_id = temp;
        ch = split(ch, ":");
        WiFi_pw = temp;
        c = true;
      }

      //String velue = velue1 + velue2 + velue3 + velue4;
      //Serial.write(String(ch).toInt());
      //Serial.println(API_DATA[String(ch).toInt()]);
      //ch = NULL;
    }
    if (c) break;
  }
  /*if (WiFi.encryptionType(mywifi) == ENC_TYPE_NONE) {
    pw = "";
    }
    else {
    //패스워드 입력 함수(스위치로 아스키코드 숫자 올라감)
    //pw = "dmltjs123@";
    inputpw(&pw);
    }

    Serial.printf("id = %s, ssid = %d, pw = %s\r\n", id.c_str(), rssi, pw.c_str());
    WiFi.begin(WiFi_id, WiFi_pw);
    WiFi.setAutoReconnect(1);*/
  while (1)
  {
    lcd.clear();
    lcd.print("WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Connecting...");

    int ConnResult = WiFi.status();
    if (ConnResult == WL_CONNECTED) {
      lcd.clear();
      lcd.print("Connect Success!!");
      Serial.printf("Connection OK!\r\n");
      break;
    }
    else {
      Serial.printf("Connextion Failed, reason:%d\r\n", ConnResult);
      if (ConnResult == 4) {
        lcd.clear();
        lcd.print("Connect Fail!!");
        lcd.setCursor(0, 1);
        lcd.print("[Push the Reset]");
        delay(500);
      }
      delay(500);
    }
  }

  Serial.printf("MY Mac Address : %s\r\n", WiFi.macAddress().c_str());
  Serial.printf("MY IP Address : %s\r\n", WiFi.localIP().toString().c_str());

  pinMode(trig, OUTPUT);    // trig 와 연결된 핀(D6)을 출력핀으로 사용
  pinMode(echo, INPUT);     // echo 와 연결된 핀(D5)을 입력핀으로 사용
  Serial.println("측정 시작!!!");

}

void loop() {
  ////////////냄새 측정!!//////////////
  mqdata = analogRead(ANALOGPIN);
  delay(1000);
  Serial.print("CO2 ppm value : ");
  Serial.println(mqdata);

  ////////////물체 감지!!//////////////
  int distance, measure;
  // 트리거 핀으로 10us 동안 펄스 출력

  digitalWrite(trig, LOW);   // Trig 핀 강제 Low 상태 셋팅
  delayMicroseconds(2);      // 2us 동안 Low 상태 유지

  digitalWrite(trig, HIGH);  // High 상태로 셋팅
  delayMicroseconds(10);     // 10us 동안 High 상태 유지
  digitalWrite(trig, LOW);   // Low 상태로 셋팅

  measure = pulseIn(echo, HIGH);  // pulseIn은 지정된 포트에서 펄스를 읽음 (High / Low)

  distance = measure / 58;

  Serial.print("측정된 거리: ");
  Serial.print(distance);
  Serial.println("cm");      // cm단위로 측정 후 계산된 값을 시리얼 통신으로 전송

  lcd.clear();

  lcd.print("DT: ");
  lcd.print((String)distance);


  if (distance <= 20 && mqdata >= 390)
  {
    lcd.setCursor(0, 1);
    lcd.print("MQ: ");
    lcd.print((String)mqdata);
    char TSBuffer[200];
    snprintf(TSBuffer, sizeof(TSBuffer), "http://api.thingspeak.com/update?api_key=W9A28S50RN1LC3BV&field1=%ld", mqdata);
    myClient.begin(TSBuffer);
    myClient.GET();
    myClient.getString();
    myClient.end();
    send_webhook("PET", "hzh9j5sWsqYGPXQBkeMFC", "", "", ""); // 연결된 IFTTT앱으로 알람
  }
  delay(1000);
}
