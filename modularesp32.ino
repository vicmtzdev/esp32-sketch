#include <WiFi.h>
#include <HTTPClient.h>

#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "Modulares_2023B";
const char* password = "modulo.2023";

const char* servidor = "https://mern-modular-944957264f4a.herokuapp.com/api/event";

String answer;
const int requestInterval = 1000;

//Elementos del objeto
boolean machineIsHeating;
boolean machineIsWorking;
boolean machineIsTakingOut;
int referenceTemperature;

int A = 23; // Linea y neutro (siempre entran)
int B = 18; // Sentido de giro 1
int C = 5; // Sentido de giro 2
int D = 19; // Electrovalvula
int E = 32; // Pin Sensor detector de llama
int F = 35; // Pin analogico para temperatura

int x = 0;            // Lectura serial
int temperatura = 0;  // Temperatura 
int banderallama = 0; // Valor de el detector de llama


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {

  Serial.begin(115200);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, INPUT);

  // Se inicializa la configuración de la red 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado a la red: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 
}

void loop() {

    // Se iguala el string answer a la llamada de la funcion de la petición get
    answer = getRequest(servidor); 

    //Inicio: desestructuración
    char json[500]; // Se crea el buffer
    answer.toCharArray(json, 500); //Se copia el string de la respuesta al buffer

    StaticJsonDocument<200> doc; // Se crea un  JsonDocument pequeño 
    deserializeJson(doc, json); // Se hace la deserialización 
 
    machineIsHeating = doc["machineIsHeating"];
    machineIsWorking = doc["machineIsWorking"];
    machineIsTakingOut = doc["machineIsTakingOut"];
    referenceTemperature = doc["referenceTemperature"];
    //Fin: desestructuración
    
    delay(requestInterval);

    //Inicio: lectura de temperatura
    x = analogRead(F);
    //temperatura = (0.0163*x)+(28.398);
    temperatura = (0.0139*x)+(29.412);
    oled.clearDisplay();
    oled.setTextSize(3);
    oled.setTextColor(WHITE);
    oled.setCursor(35, 25);
    oled.println(temperatura);
    oled.setCursor(75, 25);
    oled.println("C");
    oled.drawCircle(96, 27, 2, WHITE);
    oled.display();
    Serial.println(temperatura);
    delay(1500);
    //Fin: lectura de temperatura

    banderallama = digitalRead(E);

    if(banderallama && machineIsHeating){ // Calentar
      
      //Serial.println("La maquina esta calentando");
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, LOW);

      //Inicio: control de temperatura
      if(temperatura >= (referenceTemperature + 5) ){
      
        digitalWrite(D,LOW);
      
      }
      
      if (temperatura <= (referenceTemperature - 5) ){
      
        digitalWrite(D,HIGH);
      
      }
      //Fin: control de temperatura
    
      } else if(banderallama && machineIsWorking){ //Trabajar

        //Serial.println("La maquina esta trabajando");
        digitalWrite(A, HIGH);
        digitalWrite(B, HIGH);
        digitalWrite(C, LOW);

        //Inicio: control de temperatura
        if(temperatura >= (referenceTemperature + 5) ){
      
          digitalWrite(D,LOW);
      
        }
        
        if (temperatura <= (referenceTemperature - 5) ){
      
          digitalWrite(D,HIGH);
      
        }
        //Fin: control de temperatura
                           
    
      } else if(machineIsTakingOut){ // Retirar

        //Serial.println("La maquina esta retirando");
        digitalWrite(A, HIGH);
        digitalWrite(B, LOW);
        digitalWrite(C, HIGH);
      
      }else if( !banderallama || ((!machineIsHeating) && (!machineIsWorking) && (!machineIsTakingOut)) ){ //Parar
    
        //Serial.println("La maquina esta detenida");
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        digitalWrite(D,LOW);
    
      }

     
}



String getRequest(const char* serverName){

  //Inicializa la conexion como cliente
  HTTPClient http;
  http.begin(serverName);

  // Enviamos petición HTTP
  int httpResponseCode = http.GET();
  String payload = "...";


  if(httpResponseCode > 0){

    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
    
  }
  else {

    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  
  }

  http.end();
  return payload;
  
}


  