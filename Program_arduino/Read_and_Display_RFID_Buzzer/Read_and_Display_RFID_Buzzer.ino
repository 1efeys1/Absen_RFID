#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

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

#define BUZZER_PIN 25

// Fungsi untuk mendapatkan nama berdasarkan UID
String getNameFromUID(String uid) {
  if (uid == "CF CB AE 13") return "Raditya Ahmad Arfandi";
  else if (uid == "3F 59 5C 2B") return "Ivan Antony";
  else if (uid == "04 50 3C 3A 0B 64 80") return "Naufal Kholis Arrahman";
  else if (uid == "FF 93 77 23") return "Muhammad Wafda Rizkiansyah";
  else return "Unknown User";
}

// Variabel untuk mengatur waktu tampilan
unsigned long lastUIDDisplayTime = 0;
bool showingUID = false;

// Variabel untuk buzzer menggunakan millis
bool buzzerState = false;
unsigned long lastBuzzerToggleTime = 0;
const unsigned long buzzerInterval = 2000; // Interval toggling buzzer
const unsigned long buzzerActiveTime = 500; // Durasi buzzer aktif

bool buzzerActive = false; // Status aktif buzzer
unsigned long buzzerStartTime = 0; // Waktu awal buzzer aktif

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  // Inisialisasi OLED
  konten.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  konten.clearDisplay();
  konten.setTextColor(WHITE);

  // Inisialisasi RFID
  Serial.begin(115200);
  mfrc522.PCD_Init();

  // Tampilkan pesan awal
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

  // Fungsi buzzer menggunakan millis
  if (buzzerActive) {
    if (millis() - buzzerStartTime >= buzzerActiveTime) {
      digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer setelah durasi
      buzzerActive = false;
    }
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

  // Cari nama pengguna berdasarkan UID
  String userName = getNameFromUID(uidHex);

  // Tampilkan UID dan nama pengguna di OLED
  konten.clearDisplay();
  konten.setCursor(0, 10);
  konten.setTextSize(1);
  konten.print("UID:");
  konten.setCursor(0, 20);
  konten.print(uidHex);

  konten.setCursor(0, 35);
  konten.print("User:");
  konten.setCursor(0, 45);
  konten.print(userName);

  konten.display();

  // Aktifkan buzzer satu kali
  if (!buzzerActive) {
    buzzerActive = true;
    buzzerStartTime = millis();
    digitalWrite(BUZZER_PIN, HIGH); // Nyalakan buzzer
  }

  // Set waktu terakhir UID ditampilkan
  lastUIDDisplayTime = millis();
  showingUID = true;

  // Hentikan kartu
  mfrc522.PICC_HaltA();
}
