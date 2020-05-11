/*  
 *  Penambahan Kirim Web
 *  http://presensi.smkm2kalirejo.smkn3metro.sch.id/api/teacher/attend?card_id=1234589&mac=xxxxx5454
 *  versi 3
*/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "MFRC522.h"
#include <PubSubClient.h>
#include "config.h"

int tag[4];
//=========================================== Setup ======================
void setup() {
  //ResetSettings();
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(BUZZER, LOW);
                for( int i = 0; i < 2;  ++i ){
                  digitalWrite(BUZZER, HIGH);
                  delay(500);
                  digitalWrite(BUZZER, LOW);
                  delay(500);
                } 
  Serial.begin(115200);


  Serial.println(F("Booting....   Versi 3"));

  //ReadConfigFile(); // Read Config File if it is exist
  setup_wifi();
  //SaveConfigFile();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  delay(100);
  SPI.begin();
  // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  Serial.println(F("Ready! Versi 3"));
  Serial.println(F("======================================================"));
  Serial.println(F("Scan for Card and print UID:"));
}

void setup_wifi() {

  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array) - 1; ++i) {
    sprintf(MAC_char, "%s%02x:", MAC_char, MAC_array[i]);
  }
  sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[sizeof(MAC_array) - 1]);

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  String(mqtt_port).toCharArray(smqtt_port, 5);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", smqtt_port, 5);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);
  WiFiManagerParameter custom_mqtt_keywords1("keyword1", "mqtt keyword1", mqtt_keywords1, 40);
  WiFiManagerParameter custom_mqtt_keywords2("keyword2", "mqtt keyword2", mqtt_keywords2, 40);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.addParameter(&custom_mqtt_keywords1);
  wifiManager.addParameter(&custom_mqtt_keywords2);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(MAC_char, "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //WiFi.begin(wifi_ssid, wifi_password);

  /* while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
    }*/

  Serial.println("");
  Serial.println("WiFi connected To:");
  Serial.println(WiFi.SSID());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String sTopic = topic;

  if (sTopic == mqtt_keywords2)
  {
    Serial.println("Sense Control Command");
    if ((char)payload[1] == '1')
    {
      ResetSettings();
    }

    if ((char)payload[0] == '1')
    {
      delay(2000);
      ESP.reset();
      delay(5000);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    digitalWrite(LED1, LOW);
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    delay(1500);
    Serial.println(mqtt_server);
    // Attempt to connect
    if (client.connect(MAC_char, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      digitalWrite(LED1, HIGH);
      // Once connected, publish an announcement...
      client.publish(mqtt_keywords1, MAC_char);
      client.subscribe(mqtt_keywords2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if (client.state() == 4) ESP.restart();
      else {
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
}

unsigned long previousMillis = 0;
bool delayMeas = false;
int value = 0;

void loop() {
  digitalWrite(BUZZER, LOW);

  unsigned long currentMillis = millis();

  if (!client.connected()) {
    digitalWrite(BUZZER, LOW);
    reconnect();
  }
  client.loop();

  if (delayMeas)
  {
    if ((currentMillis - previousMillis) >= interval)
    {
      previousMillis = currentMillis;
      delayMeas = false;
      delay(1800);

    }
  }
  else
  {

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      digitalWrite(BUZZER, LOW);
      delay(550);
      return;
    }
    digitalWrite(BUZZER, HIGH);

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      digitalWrite(BUZZER, LOW);
      delay(900);
      return;
    }
    char msg[150];
    char msg1[150];
    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    msg[0] = char(0);
    msg1[0] = char(0);
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      sprintf(msg, "%s0x%02x ", msg, mfrc522.uid.uidByte[i]);
      tag[i] = mfrc522.uid.uidByte[i];
    }
    sprintf(msg1, "{\"mac\":\"%s\",\"rf_id\":\"%s\"}", MAC_char, msg);
    Serial.println();

    Serial.print("Publish message: ");
    Serial.println(msg1);

    client.publish(mqtt_keywords1, msg1);

    msg[0] = char(0);
    delayMeas = true;
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED1, HIGH);
    delay(220);
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED1, HIGH);
    delay(200);
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED1, HIGH);
    delay(100);
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED1, HIGH);
    digitalWrite(BUZZER, LOW);
    kirimWeb();
  }
}

//--------------------------------------------------------- olah Hexa -----
void byte_to_str(char* buff, uint8_t val) {  // convert an 8-bit byte to a string of 2 hexadecimal characters
  buff[0] = nibble_to_hex(val >> 4);
  buff[1] = nibble_to_hex(val);
}

char nibble_to_hex(uint8_t nibble) {  // convert a 4-bit nibble to a hexadecimal character
  nibble &= 0xF;
  return nibble > 9 ? nibble - 10 + 'A' : nibble + '0';
}

//======================================  void kirim web ===========
void kirimWeb(){
        HTTPClient http;
         Serial.println(WiFi.SSID());
         Serial.println(WiFi.localIP());
        Serial.print("[HTTP] begin...\n");
        String url = "http://presensi.smkm2kalirejo.smkn3metro.sch.id/api/teacher/attend?card_id=";         //1234589&mac=xxxxx5454"
      char rgbTxt[9];
//      rgbTxt[0] = '#';
      byte_to_str(&rgbTxt[0], tag[0]);
      byte_to_str(&rgbTxt[2], tag[1]);
      byte_to_str(&rgbTxt[4], tag[2]);
      byte_to_str(&rgbTxt[6], tag[3]);
      rgbTxt[8] = '\0';
      Serial.println(rgbTxt); // prints '#1000FF'
      Serial.println("Kirim Data");
        url+=rgbTxt;
        url+="&mac=";
        url+=MAC_char;

//        Serial.println(url);       
        http.begin(url);
        int httpCode = http.GET();                                              // start connection and send HTTP header
        if(httpCode > 0) {                                                      // httpCode will be negative on error
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);            // HTTP header has been send and Server response header has been handled
            if(httpCode == HTTP_CODE_OK) {                                      // file found at server
                String payload = http.getString();
                Serial.println(payload);

                for( int i = 0; i < 3;  ++i ){
                  digitalWrite(BUZZER, HIGH);
                  digitalWrite(LED1, LOW);
                  delay(80);
                  digitalWrite(BUZZER, LOW);
                  digitalWrite(LED1, HIGH); // nyalakan kembali led indikator
                  delay(50);
                } 
                
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();    
}
