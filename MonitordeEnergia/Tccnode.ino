#include <ESP8266WiFi.h>
#include "EmonLib.h"
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
#include <SD.h>
#define CURRENT_CAL 87.2
#define BLYNK_PRINT Serial


char auth[] = "462f8fa8cf094302a2ddc8d2529aff61";
EnergyMonitor emon1; //cria instancias
int pinSCT = A0;
int desliga = D3;
int pinsd = D8;
String apiWritekey = "88NJ35H87NSAC8G1"; // replace with your THINGSPEAK WRITEAPI key here
const char* ssid = "EDISON"; // your wifi SSID name
const char* password = "01024539" ;// wifi pasword
const char* server = "api.thingspeak.com";
double Irms = 0;


File myFile;
WiFiClient client;
 
  
void setup() {                            
  Serial.begin(115200);
  emon1.current(pinSCT, CURRENT_CAL); 
    
   // verifica se o cartão SD está presente e se pode ser inicializado
  if (!SD.begin(pinsd)) {
    Serial.println("Falha, verifique se o cartão está presente.");
    //programa encerrrado
    return;
  }
   
  //se chegou aqui é porque o cartão foi inicializado corretamente  
  Serial.println("Cartão inicializado.");
 
  WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, password);
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
  Serial.println(ssid);
  Serial.println();
  Blynk.begin(auth, ssid, password);
  Blynk.run();
  Blynk.tweet("Sistema online e operante, qualquer mudança o notificaremos via tweet");   
}


void loop() {
int i = 0;
double soma = 0;
double media = 0;
double Irms = emon1.calcIrms(1480);
  while(i<1000){
    soma = soma + Irms;
    i++;
    }
    media = soma /i;
    
  float tenSensor = 117;
  double potencia = tenSensor * Irms;
  
  if (client.connect(server,80))
  {  
    String tsData = apiWritekey;
           tsData +="&field1=";
           tsData += String(media);
           tsData +="&field2=";
           tsData += String(tenSensor);
           tsData += "\r\n\r\n";
           tsData +="&field3=";
           tsData += String(potencia);
           tsData += "\r\n\r\n";
           tsData +="&field4=";
           tsData += String(potencia);
           tsData += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiWritekey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(tsData.length());
     client.print("\n\n");  // the 2 carriage returns indicate closing of Header fields & starting of data
     client.print(tsData);
 
     Serial.print("Corrente: ");
     Serial.println(media);
     Serial.print("Tensão: ");
     Serial.println(tenSensor);
     Serial.print("Potencia: ");
     Serial.println(potencia);
     Serial.println("uploaded to Thingspeak server....");
     Blynk.virtualWrite(V1,media);
     Blynk.virtualWrite(V2,tenSensor);
     Blynk.virtualWrite(V0,tenSensor*Irms);
       
  }
  client.stop();
 
  Serial.println("Waiting to upload next reading...");
  Serial.println();

  File dataFile = SD.open("log.txt", FILE_WRITE);
  // se o arquivo foi aberto corretamente, escreve os dados nele
  if (dataFile) {
    Serial.println("O arquivo foi aberto com sucesso.");
      //formatação no arquivo: linha a linha >> UMIDADE | TEMPERATURA
      dataFile.println(media);
      dataFile.print(" | ");
      dataFile.println(tenSensor);
      dataFile.print(" | ");
      dataFile.println(tenSensor*media);
 
      //fecha o arquivo após usá-lo
      dataFile.close();
  }
  // se o arquivo não pôde ser aberto os dados não serão gravados.
  else {
    Serial.println("Falha ao abrir o arquivo LOG.txt");
  }
  
  // thingspeak needs minimum 15 sec delay between updates
  delay(15000);
}
