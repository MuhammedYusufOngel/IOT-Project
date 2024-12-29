#include <Servo.h>

#define BLYNK_TEMPLATE_ID "TMPL6or6aH2wJ"
#define BLYNK_TEMPLATE_NAME "FinalBoss"
#define BLYNK_AUTH_TOKEN "0pW67jQBXLVey7WePtby3S9V_nKOX37Q"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = D2; //mcu > HX711 dout pin
const int HX711_sck = D1; //mcu > HX711 sck pin
const int servoPin = D0;

WiFiUDP ntpUdp;
const long utcOffsetInSeconds = 10800;
NTPClient timeClient(ntpUdp, "asia.pool.ntp.org", utcOffsetInSeconds);

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

char ssid[] = "realme8";
char pass[] = "1234567890";

const int calVal_eepromAdress = 0;
unsigned long t = 0;
float dizi[] = {0.00 , 0.00};
int a = 0;

int startTime = 0;
int esitle = 0;
double toplamGram = 0;
double gecicitoplamGram = 0;
bool isOkay = false;
double gerekenGram = 0;
Servo s;



BLYNK_WRITE(V0) {
  startTime = param.asInt();

  if(startTime == 0)
    startTime = 99999;

  isOkay = false;
  Serial.print("Start Time: ");
  Serial.println(startTime);
  Serial.print("End Time: ");
  Serial.println(startTime + 10);
}

BLYNK_WRITE(V2)
{
  int servoMotor = param.asInt();

  s.write(servoMotor);
}

BLYNK_WRITE(V3)
{
  gerekenGram = param.asDouble();
}


void setup() {
  s.attach(servoPin);
  s.write(0);

  Serial.begin(115200); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
}

void loop() {

  float i;
  static boolean newDataReady = 0;
  const int serialPrintInterval = 1000; //increase value to slow down serial print activity
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;
  
  LoadCell.setCalFactor(2500);
  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      timeClient.update();

      int HH = timeClient.getHours();
      int MM = timeClient.getMinutes();
      int SS = timeClient.getSeconds();

      int serverTime = 3600 * HH + 60 * MM + SS;
      Serial.print("Live Time: ");
      Serial.println(serverTime);
      
      if(startTime == serverTime) {
        s.write(0);
        
        if(!isOkay){
          gecicitoplamGram = i * 100;
          isOkay = true;
        }

        if(gecicitoplamGram - toplamGram >= gerekenGram){
          s.write(90);
        }
        
      }

      i = LoadCell.getData();
      toplamGram = i * 100;
      Blynk.virtualWrite(V1, toplamGram);
      Serial.print("Load_cell output val: ");
      Serial.println(i + esitle);
      /*dizi[a] = i;
      a++;
      Serial.println(a);
      
      if(a == 2){
        float b = dizi[1] - dizi[0];
        Serial.print("Gerçek ağırlık: ");
        Serial.println(b * 300);
        a = 1;
      }*/

      newDataReady = 0;
      

      t = millis();
    }
  }

  Blynk.run();
}