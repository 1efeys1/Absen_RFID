#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Konfigurasi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 konten(128, 64, &Wire, OLED_RESET);

// Konfigurasi RFID
MFRC522DriverPinSimple ss_pin(5);
MFRC522DriverPinSimple rst_pin(17);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

// Konfigurasi WiFi
const char* ssid = "efeys";            // Ganti dengan SSID WiFi Anda
const char* password = "arpan004";    // Ganti dengan password WiFi Anda

// ID Google Apps Script
String GOOGLE_SCRIPT_ID = "AKfycbz9yFlsjDf9z2T-wVcbpLdV40rqPkaaX8Kz39bJ3sJmLtxvLk_Y4E9jyhtD3noXgoe35w"; // Ganti dengan ID Google Apps Script

// Pin untuk buzzer
#define BUZZER_PIN 25

// Variabel untuk mengatur waktu tampilan
unsigned long lastUIDDisplayTime = 0;
bool showingUID = false;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  // Inisialisasi OLED
  konten.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  konten.clearDisplay();
  konten.setTextColor(WHITE);

  // Inisialisasi RFID
  Serial.begin(115200);
  mfrc522.PCD_Init();

  // Inisialisasi WiFi
  WiFi.begin(ssid, password);
  konten.setCursor(10, 20);
  konten.setTextSize(1);
  konten.print("Connecting WiFi");
  konten.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    konten.print(".");
    konten.display();
  }
  konten.clearDisplay();
  konten.setCursor(10, 20);
  konten.print("WiFi Connected");
  konten.display();
  delay(1000);

  // Pesan awal
  konten.clearDisplay();
  konten.setCursor(10, 20);
  konten.setTextSize(1);
  konten.print("Scan your card");
  konten.display();
}

void loop() {
  // Cek apakah sedang menampilkan UID
  if (showingUID && (millis() - lastUIDDisplayTime >= 5000)) {
    // Kembali ke tampilan awal setelah 5 detik
    konten.clearDisplay();
    konten.setCursor(10, 20);
    konten.setTextSize(1);
    konten.print("Scan your card");
    konten.display();
    showingUID = false;
  }

  // Jika tidak ada kartu baru, lanjutkan loop
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Mengonversi UID menjadi format hex string
  String uidHex = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidHex += "0";
    }
    uidHex += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) uidHex += " ";
  }
  uidHex.toUpperCase();

  // Tampilkan UID di OLED
  konten.clearDisplay();
  konten.setCursor(0, 10);
  konten.setTextSize(1);
  konten.print("UID:");
  konten.setCursor(0, 20);
  konten.print(uidHex);
  konten.display();

  // Aktifkan buzzer
  digitalWrite(BUZZER_PIN, HIGH); // Nyalakan buzzer
  delay(300);                     // Tunggu 300ms
  digitalWrite(BUZZER_PIN, LOW);  // Matikan buzzer

  // Kirim data ke Google Sheets
  sendDataToGoogleSheets(uidHex);

  // Set waktu terakhir UID ditampilkan
  lastUIDDisplayTime = millis();
  showingUID = true;

  // Hentikan kartu
  mfrc522.PICC_HaltA();
}

String urlEncode(const String &str) {
  String encoded = "";
  char c;
  for (size_t i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      encoded += '%' + String(c, HEX);
    }
  }
  return encoded;
}

void sendDataToGoogleSheets(const String& uid) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Format URL dengan parameter
    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?tag=" + urlEncode(uid);
    Serial.println("Requesting URL: " + url);

    http.begin(url);  // Inisialisasi koneksi HTTP
    int httpResponseCode = http.GET();  // Kirim permintaan GET

    if (httpResponseCode > 0) {
      String response = http.getString();  // Respons dari server
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error in HTTP request");
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
    }
    
    http.end();  // Mengakhiri koneksi HTTP
  } else {
    Serial.println("WiFi Disconnected!");
  }
}
