#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include "DM_G_I2C.h"

#include "DHT.h"
#define DHTPIN 4        // SDA 핀의 설정
#define DHTTYPE DHT22   // DHT22 (AM2302) 센서종류 설정

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27,20,4);

#define SSID      "U+NetBB59"    // your wifi network SSID
#define KEY       "1C2B003866"   // your wifi network password
#define AUTH       "WEP"    // your wifi network security (NONE, WEP, WPA, WPA2)

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.0.22"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.0.1"
#endif

#define SERVER_PORT    80
#define PROTOCOL       "TCP"

int heat_status = 0;
int humid_status = 0;
int fan_status = 0;
int led_status = 0;

const int HeatPIN =  13;
const int HumidPIN = 12;
const int FanPIN = 11;
const int LedPIN = 10;

float temperature = 0;
int humidity =0;

int Automatic=0;

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);


void Auto(float temp, float rh){

  if(temp<20)
  digitalWrite(HeatPIN,LOW);
  else if(temp>27) 
  digitalWrite(HeatPIN,HIGH);
  if(rh < 50)
  digitalWrite(HumidPIN,LOW);
  else if(rh > 70)
  digitalWrite(HumidPIN,HIGH);
}
 

void setup() {
  char c;

  Wire.begin();
  dht.begin();

  lcd.init();
  lcd.backlight();    
  lcd.clear();     
  G_I2C_Scanner();

  pinMode(HeatPIN, OUTPUT);
  pinMode(HumidPIN,OUTPUT);
  pinMode(FanPIN,OUTPUT);
  pinMode(LedPIN,OUTPUT);
  
  mySerial.begin(9600);
  Serial.begin(9600);

  Serial.println("--------- JSN270 Android Example --------");

  // wait for initilization of JSN270
  delay(5000);
  //JSN270.reset();
  delay(1000);

  //JSN270.prompt();
  JSN270.sendCommand("at+ver\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);

#if USE_DHCP_IP
  JSN270.dynamicIP();
#else
  JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
  if (JSN270.join(SSID, KEY, AUTH)) {
    Serial.println("WiFi connect to " SSID);
  }
  else {
    Serial.println("Failed WiFi connect to " SSID);
    Serial.println("Restart System");

    return;
  }    
  delay(1000);

  JSN270.sendCommand("at+wstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);        

  JSN270.sendCommand("at+nstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);
}

void loop() {
  
  temperature = dht.readTemperature();
  humidity=dht.readHumidity();

 if (!JSN270.server(SERVER_PORT, PROTOCOL)) {
    Serial.println(F("Failed connect "));
    Serial.println(F("Restart System"));
  } else {
    Serial.println(F("Waiting for connection..."));
  }
      
    
  String currentLine = "";                // make a String to hold incoming data from the client
  int get_http_request = 0;

  while (1) {
    if (mySerial.overflow()) {
      Serial.println("SoftwareSerial overflow!");
    }
   
    if (JSN270.available() > 0) {
      char c = JSN270.read();
      Serial.print(c);

      if (c == '\n') {                    // if the byte is a newline character
        if (currentLine.length() == 0) {
          if (get_http_request) {
            Serial.println("new client");
            //Serial.println("HTTP RESPONSE");
            // Enter data mode
            JSN270.sendCommand("at+exit\r");
            delay(100);
            
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
           JSN270.println("HTTP/1.1 200 OK");
           JSN270.println("Content-type:text/html");
            JSN270.println();

            // the content of the HTTP response follows the header:
      
            JSN270.print(heat_status);
            JSN270.print(" ");
            JSN270.print(humid_status);
            JSN270.print(" ");
            JSN270.print(fan_status);
            JSN270.print(" ");
            JSN270.print(led_status);
            JSN270.print(" ");
            JSN270.print(Automatic);
            JSN270.print(" ");
            
            JSN270.print(temperature);
            JSN270.print(" ");
            JSN270.print(humidity);
          
            // The HTTP response ends with another blank line:
            JSN270.println();

            // wait until all http response data sent:
            delay(1000);

            // Enter command mode:
            JSN270.print("+++");
            delay(100);
            
            // break out of the while loop:
            break;
          }
        }
        // if you got a newline, then clear currentLine:
        else {                
          // Check the client request:
          if (currentLine.startsWith("GET / HTTP")) {
            Serial.println("HTTP REQUEST");
            get_http_request = 1;
          }
          else {
            get_http_request = 1;
            switch(currentLine[5]){
            case 'a':heat_status = 1;break;
            case 'b':heat_status = 0;break;
            case 'c':humid_status = 1;break;
            case 'd':humid_status = 0;break;
            case 'e':fan_status = 1;break;
            case 'f':fan_status = 0;break;
            case 'g':led_status = 1;break;
            case 'h':led_status = 0;break;
            case 'i':break;
            case 'j':Automatic=1;break;
            case 'k':Automatic=0;break;
           
            }
            }
           
          currentLine = "";
        }
       
      }
      else if (c != '\r') {    // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      }
    }
  }
if(Automatic)
{
    if (isnan(temperature) || isnan(humidity)) {
    //값 읽기 실패시 시리얼 모니터 출력
    lcd.setCursor(0,0);
    lcd.println("Failed to read from DHT");
  } else {
    lcd.clear();
    //온도, 습도 표시 시리얼 모니터 출력
    lcd.setCursor(0,0);
    lcd.print("Temp: "); 
    lcd.print(temperature);
    lcd.print(" C");
    
    lcd.setCursor(0,1);
    lcd.print("RH  : "); 
    lcd.print(humidity);
    lcd.print(" %");

    Auto(temperature,humidity);
   Serial.println("&&&&&Automatic : "+Automatic);
  
 delay(2000);
}
}
 else
   {     digitalWrite(HeatPIN, heat_status);
         digitalWrite(HumidPIN, humid_status);
         digitalWrite(FanPIN, fan_status);
         digitalWrite(LedPIN, led_status);
         
  }
  
            
  // close the connection
  JSN270.sendCommand("at+nclose\r");
  Serial.println("client disonnected");
    
}
