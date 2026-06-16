
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h"

const char DEVICE_LOGIN_NAME[]  = "35f803a8-14e7-4047-a8b2-e949bd52f057";

const char SSID[]               = SECRET_SSID;    
const char PASS[]               = SECRET_OPTIONAL_PASS;    
const char DEVICE_KEY[]  = SECRET_DEVICE_KEY;    

void onComandoChange();
void onSistemaOnChange();

String comando;
float temperatura;
int luminosidade;
bool ledAtivo;
bool sistemaOn;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(comando, READWRITE, ON_CHANGE, onComandoChange);
  ArduinoCloud.addProperty(temperatura, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(luminosidade, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(ledAtivo, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sistemaOn, READWRITE, ON_CHANGE, onSistemaOnChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
