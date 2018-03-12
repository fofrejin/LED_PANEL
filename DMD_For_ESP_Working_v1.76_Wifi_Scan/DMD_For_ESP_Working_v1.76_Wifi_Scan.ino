#include <SPI.h>
#include <DMD.h>
#include <Ticker.h>
#include "Timer.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h> 
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
//#include "SystemFont5x7.h"
#include "Arial_black_16.h"

#define DEBUG 0

//DMD
#define DISPLAYS_ACROSS 7
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

//Web-сервер
ESP8266WebServer server(80);
#include "Page_Admin.h"
#include "Page_NetworkConfiguration.h"
#include "Page_NTPSettings.h"

char inData[500] = "test";// Буфер для відображення на панелі

//Повідомлення РТФ
String  message[9];
String  messageGreeting;
bool messageGreetingAllow = 0;

//Ticker
Ticker DMDTicker;
Ticker scrollTicker;
Ticker ntpTimeTicker;

Timer t;

//WiFi    
String ssid; // ім'я WiFi мережі (SSID)
String pass; // пароль WiFi мережі

//NTP time
long timeZone;
bool isDayLightSaving;
unsigned int  localPort = 2390;      // локальний порт для прийому UDP пакетів
unsigned long ntp_time = 0;
long  t_correct        = 0;
unsigned long cur_ms   = 0;
unsigned long ms1      = 0;
unsigned long ms2      = 10000000UL;
unsigned long t_cur    = 0;
bool          points   = true;
unsigned int err_count = 0;
bool getNTPTimeSuccessFlag = 0;
byte ErrGetTimeCount = 0;

IPAddress timeServerIP; 
const char* ntpServerName = "0.ua.pool.ntp.org"; // NTP сервер

const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[ NTP_PACKET_SIZE]; 
WiFiUDP udp;

// Wheather
const char* host = "api.openweathermap.org"; // сервер погоди
String line; 

String ipstring;
String SSIDlist;


void ScanDMD(){ 
  dmd.scanDisplayBySPI();
}

void setup(void){
  Serial.begin(115200);

  // Зчитування конфігурації з EEPROM
  EEPROM.begin(512);
  ssid = ReadStringFromEEPROM(64);
  Serial.println();
  Serial.print("EEPROM contents: SSID:");
  Serial.println(ssid);
  pass = ReadStringFromEEPROM(96);
  Serial.println();
  Serial.print("EEPROM contents: Pass:");
  Serial.println(pass);
  
  timeZone = EEPROMReadlong(22);
  Serial.println();
  Serial.print("EEPROM contents: timeZone:");
  Serial.println(timeZone);
  isDayLightSaving = EEPROM.read(17);
  Serial.println();
  Serial.print("EEPROM contents: isDayLightSaving:");
  Serial.println(isDayLightSaving);
  
  dmd.Spacing = EEPROM.read(10);
  Serial.println();
  Serial.print("EEPROM contents: Brightness:");
  Serial.println(dmd.Spacing);
  
  analogWriteFreq(400);
  DMDTicker.attach_ms(1, ScanDMD); // Ініціалізуємо таймер виклику функції відрисовки панелі

  //dmd.Spacing = 11; //Керування яскравістю
  dmd.clearScreen( true );   //true нормальний режим (всі пікселі вимкнені), false інверсія (всі пікселі ввімнені)
  dmd.selectFont(Arial_Black_16);
  //goto label;
  
  // З'єднання з WiFi   
   if( !ConnectWiFi(ssid.c_str(),pass.c_str()) ){

      Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    SSIDlist = "0";
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      SSIDlist += ((i+1) + ". " + String(WiFi.SSID(i) + " "));
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
  
      //label: // Мітка
      Serial.println("Setting AP mode");
      WiFi.mode(WIFI_AP);  
      WiFi.softAP("DMD Pannel Wifi");
      Serial.println();
      Serial.print("Wifi ip: ");
      //Serial.println("192.168.4.1");
      Serial.println(WiFi.softAPIP());
      ipstring = (
      String(WiFi.softAPIP()[0]) + "." +
      String(WiFi.softAPIP()[1]) + "." +
      String(WiFi.softAPIP()[2]) + "." +
      String(WiFi.softAPIP()[3])
      );
      dmd.clearScreen( true );
      dmd.drawString(  (DISPLAYS_ACROSS*32)-78,  ((DISPLAYS_DOWN*16)/2)-8, ipstring.c_str(), strlen(ipstring.c_str()) ); //(x, y, string, sizeofstring)
      serverSetup(); // Сервер для web інтерфейсу
      while(1){
        server.handleClient();
      }
      //Serial.println("Reset ESP8266 ...");
      //ESP.reset();
   }
   ipstring = (
   String(WiFi.localIP()[0]) + "." +
   String(WiFi.localIP()[1]) + "." +
   String(WiFi.localIP()[2]) + "." +
   String(WiFi.localIP()[3])
   );
   dmd.clearScreen( true );
   dmd.drawMarquee(ipstring.c_str(), strlen(ipstring.c_str()), (DISPLAYS_ACROSS*32)-98,  ((DISPLAYS_DOWN*16)/2)-8);
   delay(1000);
   scrollingRoutine(-1,0,15,-1); // (x,y,delay,amount) amount=-1 якщо скролимо горизонтально
   
   
// Ініціалізація UDP з'єднання з NTP сервером
   Serial.println("Starting UDP");
   udp.begin(localPort);
   Serial.print("Local port: ");
   Serial.println(udp.localPort());
   
   serverSetup(); // Сервер для web інтерфейсу
   
   ArduinoOTA.setHostname("DMD-Pannel"); // Задаємо ім'я мережевого порту для прошивки через WiFi
   ArduinoOTA.setPassword((const char *)"radio2017"); // Задаємо пароль доступу
   ArduinoOTA.begin(); // Ініціалізуємо OTA

   // Отримуємо час
   //cur_ms       = millis();
   //t_cur        = cur_ms/1000;
   //t_cur        = millis()/1000;
   GetNTP();
   //t_correct = ntp_time - t_cur;
   // Повідомлення РТФ
   GetRTFMessages();
   // Погоду
   GetWeather();

   t.every(1000, ISRsecondTick);
}

int disp = 0;
int getOrder = 0;
bool Scroll = 0;
bool EnableTimeDisplay = 1;
long current_ms = 0;
long previous_ms1 = 0;
long previous_ms2 = 0;
long previous_ms3 = 0;
long previous_ms4 = 0;

int messageNumber = 0;
int numberOfMessages = 0;

//Weather
float tempC;
char tempCstr[8];
float pressure;
int humidity;


void loop(){
   ArduinoOTA.handle();

   server.handleClient();

   t.update();
   
   if(!Scroll){
      current_ms       = millis();
      // Відображення погоди і повідомленнь
      if((current_ms < previous_ms2 || current_ms > (previous_ms2 + 30000))){
         if(disp >= 3) disp=0;
         if(disp == 0){
            String Text = "Темп. " + (String)tempCstr + "°C " + "Волог. " + (String)humidity + "% " + "Тиск " + (String)pressure + "мм рт.ст.";
            displayString(Text);
            Scroll = 1;
            disp++;
            previous_ms2 = millis();
         }
         /*
         else if (disp == 1){
            String Text = "Волог. " + (String)humidity + "%";
            displayString(Text);
            Scroll = 1;
            disp++;
            previous_ms2 = millis();
         }
         else if (disp == 2){
            String Text = "Тиск " + (String)pressure + "мм рт.ст.";
            displayString(Text);
            Scroll = 1;
            disp++;
            previous_ms2 = millis();
         }*/
        
         else if (disp == 1){
            displayString(message[messageNumber]);
            Scroll = 1;
            messageNumber++;
            if (messageNumber == numberOfMessages) {
               messageNumber = 0;
               disp++;
            }
            previous_ms2 = millis();
         }
         else if (disp == 2){
            if(messageGreetingAllow){
               Serial.println("messageGreeting: ");
               displayString(messageGreeting);
               Scroll = 1;
               previous_ms2 = millis();
            }
            disp++;
         }
      }
      // Відображення часу
      else if((current_ms < previous_ms2 || current_ms > (previous_ms1 + 1000)) && EnableTimeDisplay){
          //t_cur        = current_ms/1000;
          displayTime();
          Serial.println(ntp_time);
          previous_ms1 = millis();
      }

   }
   // Скролінг
   else{
      current_ms       = millis();
      if((current_ms < previous_ms3 || current_ms > (previous_ms3 + 20))){
         Scroll = !dmd.stepMarquee(-1, 0);
         previous_ms3 = millis();
        // Отримання даних
        if(!Scroll){
        if(getOrder >= 3) getOrder=0;
        if (getOrder == 0){
          t_cur        = millis()/1000;
          GetNTP();
          t_correct = ntp_time - t_cur;
          getOrder++;
          Serial.print("ErrGetTimeCount ");
          Serial.println(ErrGetTimeCount);
          Serial.print("getNTPTimeSuccessFlag ");
          Serial.println(getNTPTimeSuccessFlag);
          //previous_ms4 = millis();
        }
        else if (getOrder == 1){
           GetRTFMessages();
           getOrder++;
           //previous_ms4 = millis();
        }
        else if (getOrder == 2){
           GetWeather();
           getOrder++;
           //previous_ms4 = millis();
        }
      }
      }
   }
}

void displayString(String buf){
          Serial.print("displayString: ");
          Serial.println(buf);
          strncpy(inData, buf.c_str(), sizeof(inData));
          inData[sizeof(inData) - 1] = '\0'; 
          utf8ascii(inData);
          Serial.print("inData: ");
          Serial.println(inData);
          dmd.clearScreen( true );
          dmd.drawMarquee(inData, strlen(inData),(DISPLAYS_ACROSS*32),0); //strlen(inData)
          Serial.println(strlen(inData));
          //Serial.println(buf.length());
}

void scrollingRoutine(int x, int y, byte del, int amount){ // Based on delay
  bool ret = 0;
  while(!ret){
    amount--;
    ret = dmd.stepMarquee(x, y);
    if(amount==0){
      break;
    }
    delay(del);
  }
}


// З'єднання з WiFi
bool ConnectWiFi(const char *ssid, const char *pass) {
   WiFi.mode(WIFI_STA); 
// Три спроби з'єднання з WiFi мережею
   for( int i=0; i<2; i++){
      Serial.print("\nConnecting to: "); 
      Serial.println(ssid); 
      WiFi.begin(ssid,pass);
      delay(1000);
// Максиммум 6 разів перевірка з'єднання
      Serial.print("WiFi.status: ");      
      for( int j=0; j<6; j++ ){
          if (WiFi.status() == WL_CONNECTED) { 
              Serial.print("\nWiFi connect true: ");
              Serial.print(WiFi.localIP());
              Serial.print("/");
              Serial.print(WiFi.subnetMask());
              Serial.print("/");
              Serial.println(WiFi.gatewayIP());
              return true; 
          } 
          delay(1000);
          Serial.print(WiFi.status()); 
      }
   }   
   Serial.println("\nConnect WiFi failed ...");
   return false;
} 
