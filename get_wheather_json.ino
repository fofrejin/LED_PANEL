void GetWeather() {
  
  // Використовуємо клас WiFiClient щоб створити TCP з'єднання
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
                                         // (703448) код м. Києва
    client.println("GET /data/2.5/weather?id=703448&appid=21de3ed563e0e36a5fac77f85242682b HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
    
  t.update();
  delay(500);
  t.update();
  String line; 
  // Читаємо всі рядки відповіді від сервера і друкуємо їх в Serial
  while(client.available()){
    line = client.readStringUntil('\r'); 
  }
  Serial.println();
  Serial.print(line);
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  t.update();

   StaticJsonBuffer<2000> jsonBuffer;                   
   JsonObject& root = jsonBuffer.parseObject(line);   
   if (!root.success()) {
      Serial.print("Weather: ");
      Serial.println("parseObject() failed");           
      return;                                      
   }
   else
     Serial.println("GetWheather OK!");  
  
  t.update();
                          

  String name = root["name"];                    
  Serial.print("name:");
  Serial.println(name);
  
  float tempK = root["main"]["temp"];               
  tempC = tempK - 273.15;                     
  dtostrf(tempC,2,1,tempCstr);
  Serial.print("temp: ");
  Serial.print(tempC);                                
  Serial.println("°C");
  
  /*float tempKmin = root["main"]["temp_min"];         
  float tempCmin = tempKmin - 273.15;
  Serial.print("temp min: ");
  Serial.print(tempCmin);
  Serial.println(" C");

  float tempKmax = root["main"]["temp_max"];
  float tempCmax = tempKmax - 273.15;
  Serial.print("temp max: ");
  Serial.print(tempCmax);
  Serial.println(" C");
  */
  int pressurehPa = root["main"]["pressure"]; 
  pressure = pressurehPa/1.333;
  Serial.print("pressure: ");
  Serial.print(pressure);
  Serial.println(" mmHc");

  humidity = root["main"]["humidity"]; 
  Serial.print("humidity: ");
  Serial.print(humidity);  
  Serial.println(" %");

/*
  float windspeed = root["wind"]["speed"]; 
  Serial.print("wind speed: ");
  Serial.print(windspeed);  
  Serial.println(" m/s");

  /*int winddeg = root["wind"]["deg"]; 
  Serial.print("wind deg :");
  Serial.println(winddeg); 
  */
}


