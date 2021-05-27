#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
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

int readDHT11(int *readTemp, int *readHumid){
  int dt[82] = {0,};
  
  digitalWrite(DHT11PIN, 0);
  pinMode(DHT11PIN, OUTPUT);
  delay(20);
  pinMode(DHT11PIN, INPUT);
  //Phase 1
  while(1)
    if(digitalRead(DHT11PIN) == 1) break;
  while(1)
    if(digitalRead(DHT11PIN) == 0) break;
  
  for(int cnt = 0; cnt < 41; cnt++){
    //Phase 2
    dt[cnt*2] = micros();//이전시간
    while(1)
      if(digitalRead(DHT11PIN) == 1) break;
    dt[cnt*2] = micros() - dt[cnt*2]; //현재시간

    
    //Phase 3
    dt[cnt*2+1] = micros(); //이전시간
    while(1)
      if(digitalRead(DHT11PIN) == 0) break;
    dt[cnt*2+1] = micros() - dt[cnt*2+1]; //현재시간
  }
  *readHumid = 0;
  for(int cnt = 1; cnt <9; cnt++){//Humid
    *readHumid = *readHumid << 1;
    if(dt[cnt*2+1] > 49){
      *readHumid = *readHumid + 1;
    }
    else{
      *readHumid = *readHumid + 0;
    }
  }
  *readTemp = 0;
  for(int cnt = 17; cnt <25; cnt++){//Temp
    *readTemp = *readTemp << 1;
    if(dt[cnt*2+1] > 49){
      *readTemp = *readTemp + 1;
    }
    else{
      *readTemp = *readTemp + 0;
    }
    
  }
  for(int cnt = 0; cnt < 41; cnt++){
  Serial.printf("cnt: %d, dt = [%d, %d]\r\n",cnt, dt[cnt*2], dt[cnt*2+1]);
  }
  Serial.printf("Humidty : %d\r\n", *readHumid);
  Serial.printf("Temp : %d\r\n", *readTemp);
  return 1;
}

unsigned long long prevMillis = 0;

void loop() {
 
    int readTemp, readHumid;
  
    readDHT11(&readTemp,&readHumid);
    Serial.printf("Temp:%d, Humid:%d\r\n", readTemp, readHumid);
        
    char tempBuffer[200];
    
    snprintf(tempBuffer,sizeof(tempBuffer),"http://api.thingspeak.com/update?api_key=W9A28S50RN1LC3BV&field1=%ld&field2=%ld",readTemp,readHumid);
    myClient.begin(tempBuffer);
    myClient.GET();
    myClient.getString();
    myClient.end();
      
    delay(60000);
}
