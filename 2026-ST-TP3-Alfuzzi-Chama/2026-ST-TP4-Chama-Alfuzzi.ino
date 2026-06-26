//==================================================
// TP EEPROM - Grupo 8
// Valentin Alfuzzi - Tobias Chama
//==================================================

#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <Preferences.h>

#define DHTPIN 23
#define DHTTYPE DHT11

#define SW1 34
#define SW2 35

DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C pantalla(U8G2_R0, U8X8_PIN_NONE);

Preferences preferencias;

//================ MAQUINA DE ESTADOS ================

enum Estado
{
  PANTALLA1,
  PANTALLA2
};

Estado estado = PANTALLA1;

//====================================================

int temperatura = 0;
int umbral = 25;

bool sw1Actual = false;
bool sw2Actual = false;

bool sw1Anterior = false;
bool sw2Anterior = false;

unsigned long tiempoSW1 = 0;
unsigned long tiempoSW2 = 0;

unsigned long tiempoLectura = 0;

bool guardado = false;

//====================================================

void leerTemperatura();
void mostrarPantalla1();
void mostrarPantalla2();

//====================================================

void setup()
{
  Serial.begin(115200);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);

  dht.begin();

  pantalla.begin();
  pantalla.setFont(u8g2_font_ncenB08_tr);

  preferencias.begin("TP", false);

  umbral = preferencias.getInt("umbral", 25);

  pantalla.clearBuffer();
  pantalla.drawStr(20, 30, "Iniciando...");
  pantalla.sendBuffer();

  tiempoLectura = millis();
}
void loop()
{
  if (millis() - tiempoLectura >= 1000)
  {
    tiempoLectura = millis();
    leerTemperatura();
  }

  sw1Actual = (digitalRead(SW1) == LOW);
  sw2Actual = (digitalRead(SW2) == LOW);

  switch (estado)
  {
    //==================== PANTALLA 1 ====================

    case PANTALLA1:

      mostrarPantalla1();

      // Mantener SW1 5 segundos para entrar a configuración

      if (sw1Actual && !sw1Anterior)
      {
        tiempoSW1 = millis();
      }

      if (!sw1Actual && sw1Anterior)
      {
        tiempoSW1 = 0;
      }

      if (sw1Actual && tiempoSW1 != 0)
      {
        if (millis() - tiempoSW1 >= 5000)
        {
          estado = PANTALLA2;
          tiempoSW1 = 0;
        }
      }

    break;

    //==================== PANTALLA 2 ====================

    case PANTALLA2:

      mostrarPantalla2();

      //---------------- SW1 ----------------

      // Guarda el instante en que se presiona

      if (sw1Actual && !sw1Anterior)
      {
        tiempoSW1 = millis();
      }

      // Al soltar, si fue una pulsación corta, aumenta

      if (!sw1Actual && sw1Anterior)
      {
        if (millis() - tiempoSW1 < 5000)
        {
          umbral++;

          if (umbral > 99)
            umbral = 99;
        }
      }

      //---------------- SW2 ----------------

      // Guarda el instante en que se presiona

      if (sw2Actual && !sw2Anterior)
      {
        tiempoSW2 = millis();
        guardado = false;
      }

      // Si permanece presionado 5 s, guarda y vuelve

      if (sw2Actual && !guardado)
      {
        if (millis() - tiempoSW2 >= 5000)
        {
          preferencias.putInt("umbral", umbral);

          guardado = true;

          estado = PANTALLA1;
        }
      }

      // Al soltar, si NO se guardó, resta

      if (!sw2Actual && sw2Anterior)
      {
        if (!guardado)
        {
          umbral--;

          if (umbral < 0)
            umbral = 0;
        }

        guardado = false;
      }

    break;
  }

  sw1Anterior = sw1Actual;
  sw2Anterior = sw2Actual;
}//--------------------------------------------------
// Lee la temperatura del DHT11
//--------------------------------------------------

void leerTemperatura()
{
  float t = dht.readTemperature();

  if (!isnan(t))
  {
    temperatura = (int)t;
  }
}

//--------------------------------------------------
// Pantalla principal
//--------------------------------------------------

void mostrarPantalla1()
{
  pantalla.clearBuffer();

  pantalla.setCursor(0, 18);
  pantalla.print("Temperatura");

  pantalla.setCursor(0, 38);
  pantalla.print("VA: ");
  pantalla.print(temperatura);
  pantalla.print(" C");

  pantalla.setCursor(0, 58);
  pantalla.print("VU: ");
  pantalla.print(umbral);
  pantalla.print(" C");

  pantalla.sendBuffer();
}

//--------------------------------------------------
// Pantalla de configuracion
//--------------------------------------------------

void mostrarPantalla2()
{
  pantalla.clearBuffer();

  pantalla.setCursor(12, 18);
  pantalla.print("Configuracion");

  pantalla.setCursor(0, 40);
  pantalla.print("Umbral:");

  pantalla.setCursor(70, 40);
  pantalla.print(umbral);
  pantalla.print(" C");

  pantalla.setCursor(0, 62);
  pantalla.print("SW1:+  SW2:-");

  pantalla.sendBuffer();
}