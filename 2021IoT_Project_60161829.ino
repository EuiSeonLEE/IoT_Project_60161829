#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "AnotherIFTTTWebhook.h"
#define DHT11PIN 14


HTTPClient myClient;

void setup() {
  Serial.begin(115200);
 //와이파이 설정
  WiFi.begin("MJ_E4 502","dmltjs123@");
  while(1){
    if(WiFi.status()==WL_CONNECTED)
      break;
    delay(500);
  }
  Serial.printf("Connected!\r\n");
  Serial.printf("Please contact IP Addr...");
  Serial.println(WiFi.localIP());

}

void loop() {
 
    int readTemp;
  
    //readDHT11(&readTemp); 센서데이터 함수
    
    char TSBuffer[200];
    char IFBuffer[80];
    snprintf(TSBuffer,sizeof(TSBuffer),"http://api.thingspeak.com/update?api_key=W9A28S50RN1LC3BV&field1=%ld&field2=%ld",readTemp);
    snprintf(IFBuffer,sizeof(IFBuffer),"%ld",readTemp);
    
    myClient.begin(TSBuffer);
    myClient.GET();
    myClient.getString();
    myClient.end();
    
    send_webhook("DHT11","dA1d3k1GerkMWbdvmYeSAt",IFBuffer,"","");
    Serial.printf("Temp:%d\r\n", readTemp);
      
    delay(200000);
}
