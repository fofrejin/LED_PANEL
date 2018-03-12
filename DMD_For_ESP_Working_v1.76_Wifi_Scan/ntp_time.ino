void displayTime() {
   //ntp_time    = t_cur + t_correct;
   uint16_t s = ( ntp_time )%60;
   uint16_t m = ( ntp_time/60 )%60;
   uint16_t h = ( ntp_time/3600 )%24;
   Serial.print("Time: ");
   Serial.print(h);
   Serial.print(":");
   Serial.print(m);
   if(DEBUG){
      Serial.print(":");
      Serial.print(s);
   }
   Serial.println();
   itoa(h, inData, 10);
   if(h<10){
      inData[1] = inData[0];
      inData[0] = '0';
      inData[2] = '\0';
   }
   dmd.clearScreen( true );
   dmd.drawString(  ((32*DISPLAYS_ACROSS/2)-20),  0, inData, strlen(inData) );
   if(DEBUG){
      dmd.drawString(  ((32*DISPLAYS_ACROSS)-20)-44,  0, inData, strlen(inData) ); //Clock in the end of pannel
      dmd.drawString(  ((32*DISPLAYS_ACROSS)-20)-26,  0, ":", 1 );
   }
   if(points && getNTPTimeSuccessFlag){
      dmd.drawString(  18+((32*DISPLAYS_ACROSS/2)-20),  0, ":", 1 );
   }
   itoa(m, inData, 10);
   if(m<10){
      inData[1] = inData[0];
      inData[0] = '0';
      inData[2] = '\0';
   }
   dmd.drawString(  22+((32*DISPLAYS_ACROSS/2)-20),  0, inData, strlen(inData) );
   if(DEBUG){
      dmd.drawString(  (32*DISPLAYS_ACROSS)-42,  0, inData, strlen(inData) ); //Clock in the end of pannel
      dmd.drawString(  (32*DISPLAYS_ACROSS)-22,  0, ":", 1 );
      itoa(s, inData, 10);
      dmd.drawString(  (32*DISPLAYS_ACROSS)-18,  0, inData, strlen(inData) ); //Clock in the end of pannel
   }
   points = !points;
}


//Посилаємо запит NTP серверу і парсимо відповідь
bool GetNTP(void) {
   WiFi.hostByName(ntpServerName, timeServerIP); 
   t_cur = millis()/1000;
   sendNTPpacket(timeServerIP); 
   delay(300);
  
   int cb = udp.parsePacket();
   if (cb == 0) {
      Serial.println("No packet yet");
      if(ErrGetTimeCount > 2){
        ErrGetTimeCount = 3;
        getNTPTimeSuccessFlag = 0;
      }
      ErrGetTimeCount++;
      return false;
   }
   else {
      ErrGetTimeCount = 0;
      getNTPTimeSuccessFlag = 1;
      Serial.print("packet received, length=");
      Serial.println(cb);
      // Читаемо пакет в буфер    
      udp.read(packetBuffer, NTP_PACKET_SIZE); 
      // 4 байти починаючи з 40-го містять таймстамп часу - число секунд від 01.01.1900   
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // Конвертуємо два слова в змінну long
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      // Конвертуємо в UNIX-таймстамп (число секунд від 01.01.1970
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      // Робимо поправку на часовий пояс
      ntp_time = adjustTimeZone(epoch, timeZone, isDayLightSaving); 
      Serial.print("Unix time = ");
      Serial.println(ntp_time);
      t_correct = ntp_time - t_cur;
      ntp_time    = (millis()/1000) + t_correct;
      Serial.print("Correct time = ");
      Serial.println(ntp_time);
   }
   return true;
}


//Посилаємо запит NTP серверу на задану адресу
unsigned long sendNTPpacket(IPAddress& address) {
   Serial.println("sending NTP packet...");
   // Очистка буферу в 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Формуємо рядок запиту NTP серверу
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode // Індикатор корекції, Версія, Режим
   packetBuffer[1] = 0;     // Stratum, or type of clock // Часовий шар
   packetBuffer[2] = 6;     // Polling Interval // Інтервал запиту
   packetBuffer[3] = 0xEC;  // Peer Clock Precision // Точність системного годинника 
   // 8 bytes of zero for Root Delay & Root Dispersion // 8 байт нулів для Затримки і Дисперсії
   // Ідентифікатор джерела 4 байти
   packetBuffer[12]  = 49; 
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;
   // Посилаємо запит на NTP сервер (123 порт)
   udp.beginPacket(address, 123); 
   udp.write(packetBuffer, NTP_PACKET_SIZE);
   udp.endPacket();
}


unsigned long adjustTimeZone(unsigned long _timeStamp, int _timeZone, bool _isDayLightSavingSaving) {
  _timeStamp += _timeZone *  360; // Поправка на часовий пояс (360 це не помилка!)
  if (_isDayLightSavingSaving) _timeStamp += 3600; // Поправка на літній час
  return _timeStamp;
}

void ISRsecondTick(){
  ntp_time++;
}

