Модифіковані бібліотеки і остання версія прошивки для ESP.

Достуний один шрифт з кирилицею з підтримкою UTF-8 Arial Black 16.
В шрифті SystemFont5x7 є кирилиця, але в кодуванні windows-1251.
Максимальна довжина тексту для відображення на панелі 498 символів (можна змінити).
Максимальна кількість повідомлень: 10 + вітання з Днем народження (можна змінити).
Регулювання яскравості здійснюється через dmd.Spacing = рівень від 1 до 255;
Для ArduinoOTA краще задати пароль, щоб ніхто несанкціоновано не прошив панель по WiFi. Для цього треба розкоментувати рядок "ArduinoOTA.setPassword((const char *)"radio2017");" і поставити надійний пароль замість "radio2017".
Для зміни кількості світлодіодних матриць використовуються рядки: 
#define DISPLAYS_ACROSS 6  // Матриць по горизонталі
#define DISPLAYS_DOWN 1    // Матриць по вертикалі
Якщо протібно під'єднати більше ніж 7 світлодіодних матриць, то в біліотеці DMD, файл DMD.cpp, портібно збільшити швидкість SPI, рядок "SPISettings settings(2000000, MSBFIRST, SPI_MODE0);" бажано ставити не більше 10 000 000 через можливі артефакти на екрані.
Зміна швидкості скролінгу тексту встановлється в:
   // Скролінг
   else{
      current_ms       = millis();
      if((current_ms < previous_ms3 || current_ms > (previous_ms3 + 20))){
, де 20 це затримка в мілісекундах між викликами функції зміщення тексту на один піксель.
Тобто більше значення - повільніший скролінг, менше - швидший.
