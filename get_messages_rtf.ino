void GetRTFMessages() {
  
  // Використовуємо клас WiFiClient щоб створити TCP з'єднання
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("work.rtf.kpi.ua", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.println("GET /web/api/get?esp HTTP/1.1");
  client.println("Host: work.rtf.kpi.ua");
  client.println("Connection: close\r\n\r\n");
  client.println();

  t.update();
 
  //Smart Delay
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 3) {
    delay(500);
    Serial.print(".");
    repeatCounter++;
  }
  t.update();
  String line; 
  // Читаємо всі рядки відповіді від сервера і друкуємо їх в Serial
  while(client.available()){
    client.readStringUntil('{'); 
    line = "{" + client.readString(); 
  }
  Serial.println();
  Serial.print(line);
  Serial.println();
  Serial.println("closing connection");
  client.stop();
  
  t.update();

  DynamicJsonBuffer jsonBuffer;                        // Динамічний JSON буфер
  JsonObject& root = jsonBuffer.parseObject(line);     // парсимо отримані дані
  if (!root.success()) {
    Serial.print("RTF messages: ");
    Serial.println("parseObject() failed");
    return;    
  }
  Serial.println("GetRTFMessages OK!");

  t.update();
  
  messageGreeting = root["0"]["text"].as<String>(); 
  messageGreetingAllow = 1;
  if((root["0"]["text"].as<String>())==0){
    messageGreetingAllow = 0;
  }
  for (int i = 0; i<10; i++){
    if ((root[String(i+1)]["text"].as<String>())==0){
      numberOfMessages = i;
      break;
    }
    message[i] = root[String(i+1)]["text"].as<String>();  
    Serial.println(message[i]);
  }   
}
