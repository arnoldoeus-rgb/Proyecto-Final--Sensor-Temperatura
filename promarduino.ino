#include <DHT.h>

#define DHTPIN 7          // Pin donde conectas DATA del DHT11
#define DHTTYPE DHT11     // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

#define LED_VERDE 3
#define LED_ROJO 4
#define BOTON 2

bool activo = false;
unsigned long ultimoRebote = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  pinMode(BOTON, INPUT_PULLUP);
}

void loop() {

  // Leer botón físico
  bool estadoBoton = !digitalRead(BOTON);

  if (estadoBoton && millis() - ultimoRebote > 300) {
    ultimoRebote = millis();
    activo = !activo;
  }

  // Activar / desactivar por comando serial (desde C++)
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') activo = true;
    if (cmd == '0') activo = false;
  }

  // LEDs
  digitalWrite(LED_VERDE, activo);
  digitalWrite(LED_ROJO, !activo);

  // ENVÍO DE ESTADO
  Serial.print("A:");
  Serial.println(activo ? 1 : 0);

  // Si está desactivado, no mide
  if (!activo) {
    delay(200);
    return;
  }

  // ---------- LECTURA DEL SENSOR DHT11 ----------
  float temperatura = dht.readTemperature();

  // Si falla la lectura
  if (isnan(temperatura)) {
    Serial.println("T:-1");  // indicar error
    delay(500);
    return;
  }

  // ENVIAR TEMPERATURA A VISUAL STUDIO
  Serial.print("T:");
  Serial.println(temperatura);

  delay(500);
}





