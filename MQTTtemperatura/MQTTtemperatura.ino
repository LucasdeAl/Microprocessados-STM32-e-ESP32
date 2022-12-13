
#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define BOTtoken "5857061289:AAGLybCDIk3-TwwTvQBECan52huu6OoinnE"
#define CHAT_ID "-838506591"
#define LED 13
#define RX_pin 16
#define TX_pin 17

// dados da rede wifi e do broker MQTT
const char* ssid = "Seu SSID";
const char* password = "SUA SENHA";
const char* mqtt_server = "broker.emqx.io";
String tempI,tempD,umI,umD;

WiFiClient espClient;
PubSubClient client(espClient);
HardwareSerial SerialPort(2); // inicializa a UART0

unsigned long ultimoEnvio = 0;

#define TOPICO_STATUS_DESEJADO_LED  "lucas/ligarLed"
#define TOPICO_STATUS_ATUAL_LED     "lucas/statusLed"
#define TOPICO_TEMPERATURA          "lucas/temperatura"
#define TOPICO_UMIDADE              "lucas/umidade"

WiFiClientSecure SecureClient;
UniversalTelegramBot bot(BOTtoken, SecureClient);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages,String temperatura, String umidade) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Bem vindo, " + from_name + "!\n";
      welcome += "Use os seguintes comandos para acessar o status da estufa.\n\n";
      welcome += "/temperatura para acessar a temperatura \n";
      welcome += "/umidade para acessar a umidade  \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/temperatura") {
      bot.sendMessage(chat_id, "Temperatura: " + temperatura, "") ;
    }
    
    if (text == "/umidade") {
      bot.sendMessage(chat_id, "Umidade: " + umidade, "");
    }
  }
}

/***************************************************************************
 * conectaWiFi()
 */
void conectaWiFi() {
  // faz a conexão com a rede WiFi
  Serial.println();
  Serial.print("Conectando a rede: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);        // Wi-Fi no modo station
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectada");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

/***************************************************************************
 * conectaMQTT()
 */
void conectaMQTT(){
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  if (WiFi.status() == WL_CONNECTED){
    // loop até conseguir conexão com o MQTT
    while (!client.connected()){
      Serial.println("Tentativa de conexão MQTT...");
      // cria um client ID randômico
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      // tentantiva de conexão
      if (client.connect(clientId.c_str())){
        Serial.println("MQTT conectado!");
        // se conectado, anuncia isso com um tópico especifico desse dispositivo
        String topicoStatusConexao = "lucas/conexao/";
        topicoStatusConexao.concat(WiFi.macAddress());
        client.publish(topicoStatusConexao.c_str(), "1");
        // ... e se increve nos tópicos de interesse, no caso aqui apenas um
        client.subscribe(TOPICO_STATUS_DESEJADO_LED);
      }
      else{
        Serial.println("Conexão falhou");
        Serial.println("tentando novamente em 5 segundos");
        delay(5000);
      }
    }
  }
}

/***************************************************************************
 * callback()
 */
void callback(char* topic, byte* payload, unsigned int length){
  String receivedTopic = String(topic);
  
  Serial.println("recebeu mensagem");


  if(receivedTopic.equals(TOPICO_STATUS_DESEJADO_LED)){
    if(*payload == '1'){
      digitalWrite(LED, HIGH);      // liga o led
      client.publish(TOPICO_STATUS_ATUAL_LED, "1");  // publica o status
    }
    if(*payload == '0'){
      digitalWrite(LED, LOW);     // desliga o led
      client.publish(TOPICO_STATUS_ATUAL_LED, "0");
    }
  }
}

/***************************************************************************
 * setup()
 */
void setup() {
  pinMode(LED, OUTPUT);      
  Serial.begin(115200);
  conectaWiFi();
  SecureClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  conectaMQTT();  
  Serial2.begin(9600,SERIAL_8N1, RX_pin, TX_pin); 

}

void loop(){
  // fica atento aos dados MQTT
  client.loop();
  
  
while(Serial2.available()){
  tempI = String(Serial2.read());
  tempD = String(Serial2.read());
  umI = String(Serial2.read());
  umD = String(Serial2.read());
}
  // se conexão MQTT caiu, reconecta
  if (!client.connected()){ 
    conectaMQTT();
  }

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages, String(tempI+"."+tempD), String(umI+"."+umD));
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // envia temperatura e humidade via MQTT a cada 3 segundos
  if (millis() - ultimoEnvio > 3000){
    ultimoEnvio = millis();
   
     if(client.connected()){
      Serial.println("enviando dados sensor via MQTT");
      client.publish(TOPICO_TEMPERATURA, String(tempI+"."+tempD).c_str());
      client.publish(TOPICO_UMIDADE, String(umI+"."+umD).c_str());
    }
  }
   
  }
