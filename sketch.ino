#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <IRremote.hpp>
#include <math.h>

// Configuración DHT22
#define DHTPIN1 2
#define DHTPIN2 11
#define DHTTYPE DHT22
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

//IR 
#define IR_PIN 12
int setpointTemp = 23;
bool sistemaActivo = true;


LiquidCrystal_I2C lcd(0x27, 16, 2);

//Sensor de luz
const int LDR_AO = A0;
const float GAMMA = 0.7;
const float RL10 = 50;

//Potenciómetros
const int POT_WIND = A1;
const int POT_AIR = A2;

//LEDs calidad del aire
const int LED_R = 5;
const int LED_G = 6;
const int LED_B = 7;

//LEDs indicadores 
const int LED_LUZ = 10;
const int LED_FRIO = 8;
const int LED_CALOR = 9;

//Pulsador
const int BUTTON_PIN = 4;

//Estados
bool frioActivo = false;
bool calorActivo = false;
bool luzActiva = false;
bool mostrarDatos = false;
unsigned long ultimaLectura = 0;
unsigned long tiempoMostrar = 10000;
bool modoNocheManual = false;
bool modoNocheAuto = false;
bool modoNoche = false;
bool sensorPrincipal = true;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  dht1.begin();
  dht2.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Sistema iniciado");
  lcd.setCursor(0, 1);
  lcd.print("correctamente");
  delay(3000);
  lcd.clear();
  lcd.noBacklight();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_LUZ, OUTPUT);
  pinMode(LED_FRIO, OUTPUT);
  pinMode(LED_CALOR, OUTPUT);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  Serial.println("Sistema iniciado correctamente");
}

void setLEDColor(String calidad) {
  if (calidad == "buena") {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
  } else if (calidad == "regular") {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
  } else {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
  }
}

void loop() {
  // --- Comandos IR ---
  if (IrReceiver.decode()) {
    uint8_t comando = IrReceiver.decodedIRData.command;
    Serial.print("Comando IR recibido: ");
    Serial.println(comando);

    switch (comando) {
      case 2:
        setpointTemp++;
        break;
      case 152:
        setpointTemp--;
        break;
      case 162:
        sistemaActivo = !sistemaActivo;
        break;
      case 168:
        mostrarDatos = true;
        ultimaLectura = millis();
        lcd.backlight();
        break;
      case 104:
        modoNocheManual = !modoNocheManual;
        Serial.print("Modo noche manual: ");
        Serial.println(modoNocheManual ? "activado" : "desactivado");
        digitalWrite(LED_LUZ, modoNocheManual); // Encender o apagar luz artificial
        break;
      case 48:
        digitalWrite(LED_LUZ, HIGH);
        lcd.backlight(); lcd.clear(); lcd.setCursor(0, 0); lcd.print("Luz manual ON");
        delay(5000);
        digitalWrite(LED_LUZ, LOW);
        lcd.noBacklight(); lcd.clear();
        break;
      case 24:
        lcd.backlight(); lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp:"); lcd.print(dht1.readTemperature(), 1); lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Hum:"); lcd.print(dht1.readHumidity(), 0); lcd.print("%");
        delay(4000);
        lcd.noBacklight(); lcd.clear();
        break;
      case 176:
        setpointTemp = 23;
        sistemaActivo = true;
        modoNocheManual = false;
        modoNocheAuto = false;
        mostrarDatos = false;
        lcd.clear();
        lcd.noBacklight();
        Serial.println("Sistema reiniciado (por C)");
        break;
      default:
        Serial.println("Comando no programado");
        break;
    }

    IrReceiver.resume();
  }

  //Temperatura y humedad
  float temp = dht1.readTemperature();
  float hum = dht1.readHumidity();
  sensorPrincipal = !(isnan(temp) || isnan(hum));
  if (!sensorPrincipal) {
    temp = dht2.readTemperature();
    hum = dht2.readHumidity();
  }

  //Luz ambiente
  int analogValue = analogRead(LDR_AO);
  float voltage = analogValue / 1024.0 * 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1000 * pow(10, GAMMA) / resistance, 1 / GAMMA);

  // Control automático de modo noche
  if (!modoNocheManual) {
    if (lux < 100) {
      modoNocheAuto = true;
    } else if (lux > 300) {
      modoNocheAuto = false;
    }
  }

  // Estado total de modo noche
  modoNoche = modoNocheManual || modoNocheAuto;

  int setpointActual = setpointTemp + (modoNoche ? -2 : 0);

  //Potenciómetros
  int potWind = analogRead(POT_WIND);
  float windSpeed = map(potWind, 0, 1023, 0, 200);
  int airValue = analogRead(POT_AIR);
  int airQuality = map(airValue, 0, 1023, 0, 100);
  String calidadTexto = airQuality < 33 ? "mala" : (airQuality < 66 ? "regular" : "buena");
  setLEDColor(calidadTexto);

  //Control de clima y luz
  bool nuevoFrio = temp < setpointActual - 5 && sistemaActivo;
  bool nuevoCalor = temp > setpointActual + 5 && sistemaActivo;
  bool nuevaLuz = lux < 300;

  if (nuevoFrio && !frioActivo) {
    lcd.backlight(); lcd.clear(); lcd.setCursor(0, 0); lcd.print("Calefaccion");
    lcd.setCursor(0, 1); lcd.print("encendida");
    delay(3000); lcd.noBacklight(); lcd.clear();
  }
  if (nuevoCalor && !calorActivo) {
    lcd.backlight(); lcd.clear(); lcd.setCursor(0, 0); lcd.print("Ventilador");
    lcd.setCursor(0, 1); lcd.print("encendido");
    delay(3000); lcd.noBacklight(); lcd.clear();
  }
  if (nuevaLuz && !luzActiva) {
    lcd.backlight(); lcd.clear(); lcd.setCursor(0, 0); lcd.print("Luz encendida");
    delay(3000); lcd.noBacklight(); lcd.clear();
  }

  frioActivo = nuevoFrio;
  calorActivo = nuevoCalor;
  luzActiva = nuevaLuz;

  digitalWrite(LED_FRIO, frioActivo);
  digitalWrite(LED_CALOR, calorActivo);
  digitalWrite(LED_LUZ, luzActiva || modoNocheManual);

  //Pulsador físico
  if (digitalRead(BUTTON_PIN) == LOW) {
    mostrarDatos = true;
    ultimaLectura = millis();
    lcd.backlight();
  }

  //Mostrar datos completos (si se solicita)
  if (mostrarDatos) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp:"); lcd.print(temp, 1); lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum:"); lcd.print(hum, 0); lcd.print("%");
    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Luz:"); lcd.print((int)lux); lcd.print(" lux");
    lcd.setCursor(0, 1);
    lcd.print("Viento:"); lcd.print((int)windSpeed); lcd.print("km/h");
    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calidad del Aire:");
    lcd.setCursor(0, 1);
    lcd.print(calidadTexto);
    delay(3000);

    if (!sensorPrincipal) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor1 fallo");
      lcd.setCursor(0, 1);
      lcd.print("usando backup");
      delay(3000);
    }

    mostrarDatos = false;
    lcd.clear();
    lcd.noBacklight();
  }

  // Mostrar en consola si hay cambios
  static float ultimaTemp = -100;
  static bool ultimoModoNoche = false;
  static int ultimoSetpoint = -1;

  if (abs(temp - ultimaTemp) > 0.2 || modoNoche != ultimoModoNoche || setpointTemp != ultimoSetpoint) {
    Serial.print("Temperatura medida: ");
    Serial.print(temp);
    Serial.print(" °C | Setpoint: ");
    Serial.print(setpointTemp);
    Serial.print(" °C | Modo noche: ");
    Serial.println(modoNoche ? "Sí" : "No");

    ultimaTemp = temp;
    ultimoModoNoche = modoNoche;
    ultimoSetpoint = setpointTemp;
  }

  delay(100);
}



























