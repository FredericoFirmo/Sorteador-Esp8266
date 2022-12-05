#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <CTBot.h>

LiquidCrystal_I2C lcd(0x27,16,2);

//definição da senha do Wi-Fi + IP do Servidor
const char *ssid = ""; //nome da rede wifi
const char *password = ""; //senha da rede wifi
const char *LocalSelecionado = "";
const char *LinkURL = "";
String Link = "http://127.0.01/Restaurant"; //URL do endpoint na API
String token = ""; //Token bot Telegram
int cont = 0;

WiFiClient wifiClient;
HTTPClient http;
CTBot myBot;

void setup()
{
  //serial
  Serial.begin(9600);
  //Inicialização do LCD com o conversor I2C
  lcd.init();
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Iniciando...");

  pinMode(D0,INPUT);  //Pino 0 entrada digital para o botão
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    lcd.setCursor(2, 0);
    lcd.print("Start Wi-Fi");
    lcd.clear();
  }

  //conectando o bot do telegram
  myBot.wifiConnect(ssid, password);
  myBot.setTelegramToken(token);
  
  //Apresentação do APP 1
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("100-LUGARES");
  delay (3000);
  lcd.clear();
  
  //Apresentação do APP 2
  lcd.setCursor(0, 0);
  lcd.print("Nos escolhemos");
  lcd.setCursor(4, 1);
  lcd.print("para vc!");
  delay(2000);
  lcd.clear();

}

void loop()
{
  TBMessage msg;

  //Verifica se chegou alguma mensagem
  if(myBot.getNewMessage(msg))
  {
    //Verifica se foi recebida a mensagem
    if(msg.text.equalsIgnoreCase("Restaurante"))
    {
      http.begin(wifiClient,Link);
      int httpCode = http.GET();
      delay(250);
      String payload = http.getString();
      http.end();
      //Fim do request HTTP

      //Obtendo local selecionado a partir do JSON entregue pela API
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      LocalSelecionado = doc["LocalSelecionado"];
      LinkURL = doc["LinkURL"];

      String Place = String(LocalSelecionado);
      int length = Place.length();

      if(length!=0)
      {
        String reply;
        reply = (String)"Ola " + msg.sender.firstName + (String)", o local selecionado foi: " + LocalSelecionado + (String) ". \nClique no link para visitar o site: " + LinkURL ;
        myBot.sendMessage(msg.sender.id,reply);
        delay(500);
      }
      else
      {
        String reply;
        reply = (String) "O servidor está offline.";
        myBot.sendMessage(msg.sender.id,reply);
      }

    }
    else
    {
      String reply;
      reply = (String)"Ola " + msg.sender.firstName + (String)", Digite Restaurante para descobrir onde você vai hoje!";
      myBot.sendMessage(msg.sender.id, reply);
    }
  }

  if(digitalRead(D0)==0)
  {
    if(cont == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Clique no botao!");
      delay(1000);
      cont = cont + 1;
    }
  }
  
  if(digitalRead(D0)==1) //Se o botão for pressionado
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Buscando...");
    
    cont = 0;
    http.begin(wifiClient,Link);
    int httpCode = http.GET();
    delay(250);
    String payload = http.getString();
    http.end();
    //Fim do request HTTP

    //Obtendo local selecionado a partir do JSON entregue pela API
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    LocalSelecionado = doc["LocalSelecionado"];
    LinkURL = doc["LinkURL"];

    for (int i = 0; i <= 15; i++)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Carregando...");
      lcd.setCursor(i, 1);
      lcd.print("*");
      delay (200);
      lcd.clear();
  	}
    String Place = String(LocalSelecionado);
    int length = Place.length();
    Serial.print("Tamanho da string:");
    Serial.println(length);

    if(length == 0)
    {
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.status());
      lcd.print("API offline");
      delay(5000);
      lcd.clear();
    }
    else
    {
      lcd.print(LocalSelecionado);
      delay(5000);
      lcd.clear();
    }
  }
  else
  {
    return;
  }
}