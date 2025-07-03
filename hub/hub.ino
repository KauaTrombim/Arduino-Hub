#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <dht_nonblocking.h>

const int LCD_ADDRESS = 0x3F;
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

#define DHT_SENSOR_PIN 10
#define DHT_SENSOR_TYPE DHT_TYPE_22
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);


RTC_DS3231 rtc;

//Caractere personalizado para grau °
byte degreeSymbol[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

//Caractere personalizado para gota
byte drop[] = {
  B00100,
  B00100,
  B01110,
  B01110,
  B11001,
  B11101,
  B11111,
  B01110
};

float temperature = 0.0;
float humidity = 0.0;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando Relogio...");

  lcd.init();
  lcd.backlight();
  lcd.clear();

  if (!rtc.begin()) {
    Serial.println("RTC nao encontrado :(");
    lcd.print("RTC Error");
    while (1);
  }

  // Cria os caracteres customizados de grau e gota no LCD
  lcd.createChar(0, degreeSymbol);
  lcd.createChar(1, drop);

  // Verifica se o RTC precisa ser ajustado
  if (rtc.lostPower()) {
    Serial.println("RTC inconsistente, ajuste o relógio");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Essa linha configura o RTC pro horário do computador
  }
}

void loop() {
  // Tenta ler o sensor DHT e imprime na serial
  if (poll_dht_sensor()) {
    Serial.print("Nova leitura do DHT -> ");
    Serial.print("T: ");
    Serial.print(temperature);
    Serial.print(" C, H: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // Atualiza o display a cada 1 segundo (1000 ms)
  if (millis() - previousMillis >= 1000) {
    previousMillis = millis(); // Marca o novo tempo de referência

    DateTime now = rtc.now(); //Recebe os dados atualizados do RTC

    lcdPrint(now); //Imprime as informações no display
  }
}

static bool poll_dht_sensor() {
  // A função 'measure' da dht_nonblocking atualiza as variáveis globais
  // e só retorna 'true' quando uma nova medição válida está pronta.
  if (dht_sensor.measure(&temperature, &humidity)) {
    return true;
  }
  return false;
}


void lcdPrint(DateTime now) {
  char line1[17]; // Buffer para a primeira linha
  char line2[17]; // Buffer para a segunda linha
  char tempStr[6]; // Buffer para a string da temperatura (ex: "25.5")

  // --- Linha 1: Hora e Temperatura ---
  // Converte a temperatura (float) para uma string com 1 casa decimal
  dtostrf(temperature, 4, 1, tempStr); // (variável, tamanho total, casas decimais, buffer)

  // Monta a string completa da primeira linha
  snprintf(line1, sizeof(line1), "%02d:%02d:%02d  %s", now.hour(), now.minute(), now.second(), tempStr);
  
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.write(byte(0)); // Imprime o caractere customizado de grau °
  lcd.write('C'); //Imprime o C para indicar a unidade Celsius
  

  // --- Linha 2: Data e Umidade ---
  snprintf(line2, sizeof(line2), "%02d/%02d/%02d    %d%%", now.day(), now.month(), now.year() % 100, (int)round(humidity));
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.write(byte(1)); // Imprime o caractere customizado de gota
}