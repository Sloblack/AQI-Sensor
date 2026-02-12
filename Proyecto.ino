#include <SDS011.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
//#include <WiFi.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
HardwareSerial SerialPort(2);
TFT_eSPI tft = TFT_eSPI();
SDS011 sds;
float p10, p25;
int led_Red = 13;
int led_Yellow = 12;
int led_Green = 14;
int motor = 25;
int intentos = 0;
float aqi_Total = 0;
bool active = true;
void setup() {
  Serial.begin(9600);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17);
  sds.begin(&SerialPort);
  SerialBT.begin("ESP32_BT"); // Nombre del dispositivo Bluetooth
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(led_Red, OUTPUT);
  pinMode(led_Green, OUTPUT);
  pinMode(led_Yellow, OUTPUT);
  pinMode(motor, OUTPUT);
  
  delay(1000);
}

void loop() {
  int error = sds.read(&p25, &p10);

  if (aqiPm10(p10) > aqiPm25(p25)){
    aqi_Total = aqiPm10(p10);
  }
  else {
    aqi_Total = aqiPm25(p25);
  }

  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(2, 2);
  tft.setTextColor(TFT_WHITE);
  tft.println("Pm2.5: ");
  tft.setCursor(2, 22);
  tft.setTextColor(TFT_GREEN);
  tft.println(p25);
  tft.setCursor(82, 2);
  tft.setTextColor(TFT_WHITE);
  tft.println("Pm10: ");
  tft.setCursor(82, 22);
  tft.setTextColor(TFT_GREEN);
  tft.println(p10);
  tft.setCursor(2, 45);
  tft.setTextColor(TFT_WHITE);
  tft.println("AQI: ");
  tft.setCursor(2, 65);
  tft.setTextColor(TFT_GREEN);
  tft.println(aqi_Total);
  tft.setCursor(2, 85);
  tft.setTextColor(TFT_WHITE);
  tft.println("Calidad:");
  tft.setCursor(2, 105);
  if (aqi_Total < 50){
    digitalWrite(led_Green, HIGH);
    digitalWrite(led_Yellow, LOW);
    digitalWrite(led_Red, LOW);
    tft.setTextColor(TFT_GREEN);
    tft.println("Buena");
    if (active == true){
      digitalWrite(motor, LOW);
    }
  }
  else if (aqi_Total < 150){
    digitalWrite(led_Yellow, HIGH);
    digitalWrite(led_Green, LOW);
    digitalWrite(led_Red, LOW);
    tft.setTextColor(TFT_YELLOW);
    tft.println("Moderada");
    if (active == true){
      digitalWrite(motor, LOW);
    }
  }
  else{
    digitalWrite(led_Red, HIGH);
    digitalWrite(led_Green, LOW);
    digitalWrite(led_Yellow, LOW);
    tft.setTextColor(TFT_RED);
    tft.println("Dañina");
    if (active == true){
      digitalWrite(motor, HIGH);
    }
  }  

  //Mandar los datos a traves de Bluetooth
  SerialBT.print(String(p10) + " " + String(p25) + " " + String(aqi_Total));
  
  if(SerialBT.available()){
    char dato = SerialBT.read();
    if (dato == '1'){
      digitalWrite(motor, HIGH);
      active = false;
    }
    else if (dato == '0'){
      digitalWrite(motor, LOW);
      active = false;
    }
    else if(dato == '2'){
      active = true;
    }
  }
  delay(500);
}

float aqiPm10(float pm10){
  float ih, il, ch, cl;
  if (pm10 <= 55){
    ih = 50;
    il = 0;
    ch = 55;
    cl = 0;
  }
  else if (pm10 <= 155){
    ih = 100;
    il = 50;
    ch = 155;
    cl = 55;
  }
  else if (pm10 <= 255){
    ih = 150;
    il = 100;
    ch = 255;
    cl = 155;
  }
  else if (pm10 <= 355){
    ih = 200;
    il = 150;
    ch = 355;
    cl = 255;
  }
  else if (pm10 <= 425){
    ih = 300;
    il = 200;
    ch = 425;
    cl = 355;
  }
  else if (pm10 <= 505){
    ih = 400;
    il = 300;
    ch = 505;
    cl = 425;
  }
  else if (pm10 <= 605){
    ih = 500;
    il = 400;
    ch = 605;
    cl = 505;
  }

  float aqi_total = aqi(ih, il, ch, cl, pm10);
  return aqi_total;
}

float aqiPm25(float pm25){
  float ih, il, ch, cl;
  if (pm25 <= 12){
    ih = 50;
    il = 0;
    ch = 12;
    cl = 0;
  }
  else if (pm25 <= 35.5){
    ih = 100;
    il = 50;
    ch = 35.5;
    cl = 12;
  }
  else if (pm25 <= 55.5){
    ih = 150;
    il = 100;
    ch = 55.5;
    cl = 35.5;
  }
  else if (pm25 <= 150.5){
    ih = 200;
    il = 150;
    ch = 150.5;
    cl = 55.5;
  }
  else if (pm25 <= 250.5){
    ih = 300;
    il = 200;
    ch = 250.5;
    cl = 150.5;
  }
  else if (pm25 <= 350.5){
    ih = 400;
    il = 300;
    ch = 350.5;
    cl = 250.5;
  }
  else if (pm25 <= 500.5){
    ih = 500;
    il = 400;
    ch = 500.5;
    cl = 350.5;
  }
  
  float aqi_total = aqi(ih, il, ch, cl, pm25);
  return aqi_total;
}

float aqi(float ih, float il, float ch, float cl, float cp){
  return ((ih - il)/(ch-cl)) * (cp - cl) + il;
}

