void send_network_configuration_html()
{
  
  if (server.args() > 0 )  // Save Settings
  {
    //String temp = "";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "ssid") ssid =   urldecode(server.arg(i));
      if (server.argName(i) == "password") pass =    urldecode(server.arg(i));
    }
     server.send_P ( 200, "text/html", PAGE_WaitAndReload );
     //Serial.printf("SSID:%s\n", ssid);
     //Serial.printf("Pass:%s\n", pass);
     Serial.print("SSID: ");
     Serial.println(ssid);
     Serial.print("Pass: ");
     Serial.println(pass);
     WriteStringToEEPROM(64, ssid);
     WriteStringToEEPROM(96, pass);
     Serial.println("Write EEPROM OK!");
     delay(2000);
     ESP.reset();

        
  }
  else
  {
    server.send_P ( 200, "text/html", PAGE_NetworkConfiguration ); 
  }
  Serial.println(__FUNCTION__); 
}

void send_NTP_configuration_html()
{
    
  if (server.args() > 0 )  // Save Settings
  {
    isDayLightSaving = false;
    //String temp = "";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "tz") timeZone =  server.arg(i).toInt(); //http://192.168.203.249/ntp.html?tz=20
      if (server.argName(i) == "dst") isDayLightSaving = true; 
    }
     server.send_P ( 200, "text/html", PAGE_NTPConfiguration );
     EEPROM.write(17, isDayLightSaving);
     EEPROMWritelong(22, timeZone); // 4 Byte
     Serial.print("timeZone: ");
     Serial.println(timeZone);
     Serial.print("isDayLightSaving: ");
     Serial.println(isDayLightSaving);
     Serial.println("Write EEPROM OK!");  
  }
  else
  {
    server.send_P ( 200, "text/html", PAGE_NTPConfiguration ); 
  }
  Serial.println(__FUNCTION__); 
}

void send_ssid_html()
{
  server.send ( 200, "text/html", SSIDlist );
}

void send_bright_html()
{
  if (server.args() > 0 )  // Save Settings
  {
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "bri") dmd.Spacing = server.arg(i).toInt(); 
    }
     server.send ( 200, "text/html", (String("Brightness(max. 255): ") + String(dmd.Spacing)) ); //?bri=15
     EEPROM.write(10, byte(dmd.Spacing));
     EEPROM.commit();
     Serial.print("dmd.Spacing: ");
     Serial.println(dmd.Spacing);
     Serial.println("Write EEPROM OK!");  
  }
  else
  {
     server.send ( 200, "text/html", (String("Brightness(max. 255): ") + String(dmd.Spacing)) ); //?bri=15
  }
}

void reboot_device()
{
  server.send_P ( 200, "text/html", PAGE_AdminMainPage);
  ESP.reset();
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c){
  if (c >= '0' && c <= '9') {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
  char c;
  String ret = "";

  for (byte t = 0; t < input.length(); t++)
  {
    c = input[t];
    if (c == '+') c = ' ';
    if (c == '%') {


      t++;
      c = input[t];
      t++;
      c = (h2int(c) << 4) | h2int(input[t]);
    }

    ret.concat(c);
  }
  return ret;

}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void serverSetup(){
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

    // Start HTTP Server for configuration
  server.on ( "/", []() {
    Serial.println("admin.html");
    server.send_P ( 200, "text/html", PAGE_AdminMainPage);  // const char top of page
  }  );
  
  server.on ( "/favicon.ico",   []() {
    Serial.println("favicon.ico");
    server.send( 200, "text/html", "" );
  }  );
  
  server.on ( "/config.html", send_network_configuration_html );
  server.on ( "/ntp.html", send_NTP_configuration_html );
  server.on ( "/ssid", send_ssid_html );
  server.on ( "/bright.html", send_bright_html );
  server.on ( "/reboot", reboot_device );

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}
