#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

/*Put your SSID & Password*/
const char* ssid = "S23 Ultra de Joshua";   // Enter SSID here
const char* password = "";  // Enter Password here

WebServer server(80);

uint8_t LED1pin = 2;
bool LED1status = LOW;
bool p1status = LOW;

int counter = 0;

#define I2CSlaveAddress1 0x18
#define I2C_SDA 21
#define I2C_SCL 22

uint8_t error = 0;

typedef enum{
  I2C_ERROR_OK = 0, 
  I2C_ERROR_DEV,
  I2C_ERROR_ACK,
  I2C_ERROR_TIMEOUT,
  I2C_ERROR_BUS,
  I2C_ERROR_BUSY,
  I2C_ERROR_MEMORY,
  I2C_ERROR_CONTINUE,
  I2C_ERROR_NO_BEGIN
}i2c_err_t;

uint8_t estadoByte = 0;
bool p[8] = {0,0,0,0,0,0,0,0};


void setup() {
  Serial.begin(115200);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  pinMode(23, INPUT_PULLUP);
  pinMode(LED1pin, OUTPUT);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  //server.on("/", handle_OnConnect);
  server.on("/", handle_OnConnect);
  server.on("/actualizar", handle_actualizar);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);

  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);

  Wire.begin(I2C_SDA, I2C_SCL);
  i2cScanner();


}

void loop() {
  server.handleClient();
  leerSensoresI2C();
  p1status = digitalRead(23);
  if(LED1status){
    digitalWrite(LED1pin, HIGH);
  }else{
    digitalWrite(LED1pin, LOW);
  }

  

}

void i2cScanner(){
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.print("I2C device found at address 0x");
      if(address<16){
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }else if(error == 4){
      Serial.print("Unknow error at address 0x");
      if(address<16){
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found\n");
  }else{
    Serial.println("done\n");
  }
}


void handle_OnConnect() {
  counter++;
  LED1status = LOW;
  Serial.println("GPIO status: OFF");
  Serial.println(p1status);
  server.send(200, "text/html", SendHTML(LED1status));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void handle_actualizar(){
  Serial.println("Actualizar pagina");
  Serial.println(p1status);
  server.send(200, "text/html", SendHTML2(p1status));
}

void handle_led1on(){
  LED1status = HIGH;
  Serial.println("GPIO status: ON");
  server.send(200, "text/html", SendHTML(LED1status));
}

void handle_led1off(){
  LED1status = LOW;
  Serial.println("GPIO status: OFF");
  server.send(200, "text/html", SendHTML(LED1status));
}

void leerSensoresI2C() {

  // Pedir datos a STM sensores
  Wire.beginTransmission(0x18);
  Wire.write('S');
  Wire.endTransmission();

  // Leer respuesta
  Wire.requestFrom(0x18, 1);

  if (Wire.available()) {

    estadoByte = Wire.read();

    Serial.print("Sensores: ");
    Serial.println(estadoByte, BIN);

    // Actualizar estados web
    for (int i = 0; i < 8; i++) {
      p[i] = (estadoByte >> i) & 1;
    }

    // Enviar a STM pantalla
    Wire.beginTransmission(0x20);
    Wire.write(estadoByte);
    Wire.endTransmission();

  } else {
    Serial.println("No data");
  }
}

String SendHTML(uint8_t LED1stat) {


  String str = "<!DOCTYPE html><html lang='es'>";
  str += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  
  
  str += "<style>";
  str += "body {font-family: Arial; text-align: center; background-color: #f4f4f4;}";
  str += ".contenedor {display: flex; justify-content: center; margin-top: 40px; gap: 100px;}";
  str += ".lado {display: flex; flex-direction: column; gap: 20px;}";
  str += ".parqueo {width: 100px; height: 60px; border: 2px solid #333; display: flex; align-items: center; justify-content: center; font-weight: bold; border-radius: 8px;}";
  str += ".disponible {background-color: #4CAF50; color: white;}";
  str += ".ocupado {background-color: #f44336; color: white;}";
  str += ".button {padding: 10px 20px; font-size: 18px; margin: 10px; border-radius: 8px;}";
  str += ".on {background-color: green; color: white;}";
  str += ".off {background-color: red; color: white;}";
  str += "</style>";
  str += "<script>";

  str += "setTimeout(function(){ location.reload(); }, 1000);";
  str += "</script>";
  str += "</head><body>";

  // 🔷 Título
  str += "<h1>PARQUEO</h1>";

  // 🔷 Estado del LED
  if (LED1stat)
    str += "<p><b>LED: ENCENDIDO</b></p>";
  else
    str += "<p><b>LED: APAGADO</b></p>";

  // 🔷 Botones LED
  str += "<a href='/led1on'><button class='button on'>ENCENDER LED</button></a>";
  str += "<a href='/led1off'><button class='button off'>APAGAR LED</button></a>";

  // 🔷 Parqueos
  str += "<div class='contenedor'>";

  // Lado izquierdo (P1-P4)
  str += "<div class='lado'>";
  for(int i=0;i<4;i++){
    str += "<div class='parqueo ";
    str += (p[i] ? "ocupado" : "disponible");
    str += "'>P" + String(i+1) + "</div>";
  }
  str += "</div>";

  // Lado derecho (P5-P8)
  str += "<div class='lado'>";
  for(int i=4;i<8;i++){
    str += "<div class='parqueo ";
    str += (p[i] ? "ocupado" : "disponible");
    str += "'>P" + String(i+1) + "</div>";
  }
  str += "</div>";

  str += "</div>";

  str += "</body></html>";

  return str;
}
String SendHTML2(uint8_t p1stat) {
  String str = "<!DOCTYPE html> <html>";
  str += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  str += "<style>";
  str += "body {font-family: Arial, sans-serif; color: #444; text-align: center;}";
  str += ".title {font-size: 30px; font-weight: bold; letter-spacing: 2px; margin: 80px 0 55px;}";
  str += ".counter {font-size: 80px; font-weight: 300; line-height: 1; margin: 0px; color: #4285f4;}";
  str += "</style>";
  str += "</head>";
  str += "<body>";
  str += "<h1 class=\"title\">VISITOR COUNTER</h1>";
  str += "<div class=\"counter\">";
  str += counter;
  str += "</div>";
  str += "</body>";
  str += "</html>";
  return str;
}
