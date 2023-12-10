#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 4
#define RELAY 21  // Pin for the relay

const char *SSID = "BRAIN";
const char *PASSW = "Brain2023";
bool ACCESSO = false;
String lastUID= "";


WebServer server(80);

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);

    SPI.begin();
    mfrc522.PCD_Init();

    pinMode(RELAY, OUTPUT);

    WiFi.softAP(SSID, PASSW);
    Serial.println("Access point avviato");
    Serial.println("Ip: ");
    Serial.println(WiFi.softAPIP());

  server.on("/",HTTP_GET,handleRoot);
  server.on("/accept",HTTP_GET,handleAccept);
  server.on("/reject",HTTP_GET,handleReject);
  server.on("/getUID",HTTP_GET,UIDFound);
  server.begin();
}

void handleRoot(){
        String htmlPage = "<!DOCTYPE html><html><head><title>ESP32 NFC Reader</title></head><body>";
        htmlPage += "<h1>Avvicina la tua card NFC</h1>";
        htmlPage += "<div id='cardUID' style='display:none;'>";
        htmlPage += "<p>UID: <span id='uid'></span></p>";
        htmlPage += "<button onclick='onAccept()'>Accetta</button>";
        htmlPage += "<button onclick='onReject()'>Rifiuta</button>";
        htmlPage += "</div>";
        htmlPage += "<script>";
        htmlPage += "function updateUI(data) {";
        htmlPage += "  var cardUID = document.getElementById('cardUID');";
        htmlPage += "  var uidSpan = document.getElementById('uid');";
        htmlPage += "  if(data !== '') {";
        htmlPage += "    uidSpan.textContent = data;";
        htmlPage += "    cardUID.style.display = 'block';";
        htmlPage += "  } else {";
        htmlPage += "    uidSpan.textContent = '';";
        htmlPage += "    cardUID.style.display = 'none';";
        htmlPage += "  }";
        htmlPage += "}";
        htmlPage += "function pollUID() {";
        htmlPage += "  fetch('/getUID').then(response => response.text()).then(updateUI);";
        htmlPage += "  setTimeout(pollUID, 1000);";  // Aggiorna ogni secondo
        htmlPage += "}";
        htmlPage += "pollUID();";  // Inizia il polling
        htmlPage += "function sendCommand(command) {";
        htmlPage += "  var xhr = new XMLHttpRequest();";
        htmlPage += "  xhr.open('GET', '/' + command, true);";
        htmlPage += "  xhr.send();";
        htmlPage += "}";
        htmlPage += "function onAccept() { sendCommand('accept'); }";
        htmlPage += "function onReject() { sendCommand('reject'); }";
        htmlPage += "</script>";
        htmlPage += "</body></html>";
        server.send(200, "text/html", htmlPage);
}

void handleAccept(){
    Serial.println("Card Accettata");
    lastUID="";
    server.send(200, "text/plain", "Accesso Consentito");

    digitalWrite(RELAY,HIGH);
}

void handleReject(){
    Serial.println("Card Rifiutata");
    lastUID="";
    server.send(200, "text/plain", "Accesso Negato");
}

void UIDFound(){
    server.send(200,"text/plain", lastUID);
}

void loop(){
  server.handleClient();

  if ( ! mfrc522.PICC_IsNewCardPresent()) 
    return;

  if ( ! mfrc522.PICC_ReadCardSerial()) 
    return;

  Serial.print("UID tag :");

  lastUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
      lastUID += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      lastUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  lastUID.toUpperCase();

  Serial.println(lastUID);


  delay(500);
}