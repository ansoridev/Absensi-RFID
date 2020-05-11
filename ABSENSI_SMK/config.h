
#ifndef __CONFIGFILE__
#define __CONFIGFILE__
//flag for saving data
bool shouldSaveConfig = true;

const long interval = 2000;

uint8_t MAC_array[6];
char MAC_char[18];  
WiFiClient espClient;
PubSubClient client(espClient);
                     
#define RST_PIN  5  // RST-PIN für RC522 - RFID - SPI - Modul GPIO5 - D1
#define SS_PIN  4  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO4 - D2
#define BUZZER  D3 // pcb merah (D3) (kalau hijau, biru D8) 
#define LED1 D8// pcb merah (kalau hijau,biru d4)
#define LED2 D4 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
void SaveConfigCallback ();
void ReadConfigFile();

#endif
