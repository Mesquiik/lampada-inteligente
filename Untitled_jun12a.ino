#include "thingProperties.h"
#include <DHT.h>

#define PIN_BOTAO    13
#define PIN_POT      34
#define PIN_LDR      35
#define PIN_TEMP     32   
#define PIN_BUZZER   14
#define PIN_RED      33
#define PIN_GREEN    26
#define PIN_BLUE     27

#define DHTTYPE DHT22
DHT dht(PIN_TEMP, DHTTYPE);

volatile bool sistemaAtivoBotao = true;
unsigned long debounceTempo     = 0;

bool tempAtiva   = true;
bool luzAtiva    = true;
bool buzzerAtivo = true;

bool          corTemp       = false;
unsigned long corTempInicio = 0;
const unsigned long COR_DURACAO = 1000;


unsigned long ultimoSerial = 0;
const unsigned long INTERVALO_SERIAL = 2000; 


void IRAM_ATTR ISR_Botao() {
  if (millis() - debounceTempo > 250) {
    sistemaAtivoBotao = !sistemaAtivoBotao;
    debounceTempo = millis();
  }
}


void definirRGB(int r, int g, int b) {
  analogWrite(PIN_RED,   r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE,  b);
}

void desligarLED() { definirRGB(0, 0, 0); }

void desligarAtuadores() {
  desligarLED();
  noTone(PIN_BUZZER);
}

void processarCorPotenciometro(int valor) {
  if      (valor < 820)  definirRGB(255,   0,   0); // Vermelho
  else if (valor < 1640) definirRGB(255, 127,   0); // Laranja
  else if (valor < 2460) definirRGB(255, 255, 255); // Branco
  else if (valor < 3280) definirRGB(  0, 255, 255); // Azul turquesa
  else                   definirRGB(  0,   0, 255); // Azul
}

void processarComando(String cmd) {
  cmd.trim();
  Serial.print("[CMD] "); Serial.println(cmd);

  if      (cmd == "Ligar")                { sistemaAtivoBotao = true;  sistemaOn = true;  }
  else if (cmd == "Desligar")             { sistemaAtivoBotao = false; sistemaOn = false; }
  else if (cmd == "Vermelho")             { definirRGB(255, 0,   0);   corTemp = true; corTempInicio = millis(); }
  else if (cmd == "Amarelo")              { definirRGB(255, 255, 0);   corTemp = true; corTempInicio = millis(); }
  else if (cmd == "Azul")                 { definirRGB(0,   0,   255); corTemp = true; corTempInicio = millis(); }
  else if (cmd == "Ativar Temperatura")   { tempAtiva   = true;  }
  else if (cmd == "Desativar Temperatura"){ tempAtiva   = false; }
  else if (cmd == "Ativar Detector")      { luzAtiva    = true;  }
  else if (cmd == "Desativar Detector")   { luzAtiva    = false; }
  else if (cmd == "Ativar Buzzer")        { buzzerAtivo = true;  }
  else if (cmd == "Desativar Buzzer")     { buzzerAtivo = false; noTone(PIN_BUZZER); }

  comando = "";
}

void onComandoChange() {
  if (comando.length() > 0) processarComando(comando);
}

void onSistemaOnChange() {
  sistemaAtivoBotao = sistemaOn;
  if (!sistemaOn) desligarAtuadores();
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(PIN_BOTAO,  INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RED,    OUTPUT);
  pinMode(PIN_GREEN,  OUTPUT);
  pinMode(PIN_BLUE,   OUTPUT);

  sistemaAtivoBotao = true;

  dht.begin();

  attachInterrupt(digitalPinToInterrupt(PIN_BOTAO), ISR_Botao, FALLING);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  if (!sistemaAtivoBotao) {
    desligarAtuadores();
    ledAtivo = false;
    if (millis() - ultimoSerial > INTERVALO_SERIAL) {
      Serial.println("LED RGB: OFF | Sistema desativado");
      ultimoSerial = millis();
    }
    return;
  }

  float tempLida = 0.0;
  int   luzLida  = 0;
  int   valorPot = analogRead(PIN_POT);

  if (tempAtiva) {
    tempLida = dht.readTemperature();
    if (isnan(tempLida)) {
      Serial.println("[ERRO] DHT22 falhou na leitura!");
      tempLida = 0.0;
    }
  }
  if (luzAtiva) luzLida = analogRead(PIN_LDR);

  temperatura  = tempLida;
  luminosidade = luzLida;

  bool alerta = false;
  if (tempAtiva && (tempLida < 0.0 || tempLida > 25.0)) {
    alerta = true;
    if (buzzerAtivo) tone(PIN_BUZZER, 1000); 
  } else {
    noTone(PIN_BUZZER);
  }

  if (millis() - ultimoSerial > INTERVALO_SERIAL) {
    if (alerta) Serial.println("Perigo! Desligar!");
    Serial.print("LED RGB: "); Serial.print(ledAtivo ? "ON" : "OFF");
    Serial.print(" | Temp: "); Serial.print(tempLida, 1);
    Serial.print(" ºC | LDR: "); Serial.println(luzLida);
    ultimoSerial = millis();
  }

  if (corTemp) {
    if (millis() - corTempInicio >= COR_DURACAO) {
      corTemp = false;
    } else {
      ledAtivo = true;
      return;
    }
  }

  bool ambienteClaro = (luzAtiva && luzLida < 2200);

  if (alerta || ambienteClaro) {
    desligarLED();
    ledAtivo = false;
    if (ambienteClaro && !alerta)
      Serial.println("LED RGB: OFF (Ambiente Claro)");
  } else {
    processarCorPotenciometro(valorPot);
    ledAtivo = true;
  }
} 