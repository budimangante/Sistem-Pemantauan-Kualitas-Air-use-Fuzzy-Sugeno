#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Fuzzy.h>
const char* ssid     = "Electrical Workshop";
const char* password = "RESISTOR";
const char* botToken = "8638563351:AAF49tzTc9PGC07unlHJVu8xQJt0AQIDZEU"; 
const char* chatId   = "7585338767";   

// --- Konfigurasi Pin sensor dan relay ---
const int PinTurbidity = A0;
const int PinRelay     = D1; 

int adcRaw;
float vrata2;
float vhasil;
float kekeruhan;

unsigned long waktuSBLM = 0;
const unsigned long intervalTelegram = 30000;

// --- Inisialisasi Objek Fuzzy ---
Fuzzy *fuzzy = new Fuzzy();

// --- Fungsi Kirim Pesan ke Telegram ---
void kirimTelegram(String pesan) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); 
    
    Serial.println("Menghubungkan ke server Telegram...");
    if (client.connect("api.telegram.org", 443)) {
      
      String payload = "{\"chat_id\":\"" + String(chatId) + "\",\"text\":\"" + pesan + "\"}";
      
      client.println("POST /bot" + String(botToken) + "/sendMessage HTTP/1.1");
      client.println("Host: api.telegram.org");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(payload.length());
      client.println();
      client.print(payload);
      
      Serial.println("Pesan berhasil terkirim ke Telegram!");
    } else {
      Serial.println("Gagal terhubung ke api.telegram.org");
    }
    client.stop();
  } else {
    Serial.println("Gagal kirim Telegram, Wi-Fi terputus!");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PinRelay, OUTPUT);
  digitalWrite(PinRelay, HIGH);

  Serial.println();
  Serial.print("Menghubungkan ke Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ====================================================
  // CONFIGURATION FUZZY (Sesuai model Python Colab)
  // ====================================================
  FuzzyInput *kekeruhanInput = new FuzzyInput(1); 
  FuzzySet *jernih = new FuzzySet(0, 0, 25, 50);
  kekeruhanInput->addFuzzySet(jernih);
  FuzzySet *keruh = new FuzzySet(25, 100, 200, 400);
  kekeruhanInput->addFuzzySet(keruh);
  FuzzySet *kotor = new FuzzySet(200, 400, 3000, 3000);
  kekeruhanInput->addFuzzySet(kotor);
  fuzzy->addFuzzyInput(kekeruhanInput);

  FuzzyOutput *statusOutput = new FuzzyOutput(1); 
  FuzzySet *outJernih = new FuzzySet(1, 1, 1, 1); 
  statusOutput->addFuzzySet(outJernih);
  FuzzySet *outKeruh = new FuzzySet(2, 2, 2, 2);  
  statusOutput->addFuzzySet(outKeruh);
  FuzzySet *outKotor = new FuzzySet(3, 3, 3, 3);  
  statusOutput->addFuzzySet(outKotor);
  fuzzy->addFuzzyOutput(statusOutput);

  // Rule 1
  FuzzyRuleAntecedent *ifJernih = new FuzzyRuleAntecedent();
  ifJernih->joinSingle(jernih);
  FuzzyRuleConsequent *thenOutJernih = new FuzzyRuleConsequent();
  thenOutJernih->addOutput(outJernih);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifJernih, thenOutJernih);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // Rule 2
  FuzzyRuleAntecedent *ifKeruh = new FuzzyRuleAntecedent();
  ifKeruh->joinSingle(keruh);
  FuzzyRuleConsequent *thenOutKeruh = new FuzzyRuleConsequent();
  thenOutKeruh->addOutput(outKeruh);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, ifKeruh, thenOutKeruh);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // Rule 3
  FuzzyRuleAntecedent *ifKotor = new FuzzyRuleAntecedent();
  ifKotor->joinSingle(kotor);
  FuzzyRuleConsequent *thenOutKotor = new FuzzyRuleConsequent();
  thenOutKotor->addOutput(outKotor);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, ifKotor, thenOutKotor);
  fuzzy->addFuzzyRule(fuzzyRule3);

  Serial.println("SYSTEM READY - NODEMCU FUZZY INITIALIZED");
  delay(2000);
}

void loop() {
  // --- 1. Baca Sensor Turbidity (Sampling 800x) ---
  long adcSum = 0;
  for (int i = 0; i < 800; i++) {
    adcSum += analogRead(PinTurbidity);
  }
  
  adcRaw = adcSum / 800;
  vrata2 = (adcRaw * 5.0) / 1023.0;
  vhasil = roundf(vrata2 * 10.0f) / 10.0f;

  if (vhasil < 1.5) {
    kekeruhan = 3000;
  } else {
    kekeruhan = -1120.4 * sq(vhasil) + 5742.3 * vhasil - 4353.8;
  }
  
  if (kekeruhan < 0)    kekeruhan = 0;
  if (kekeruhan > 3000) kekeruhan = 3000;

  // --- 2. PROSES LOGIKA FUZZY DENGAN LIBRARY ---
  fuzzy->setInput(1, kekeruhan);
  fuzzy->fuzzify();
  float z_akhir = fuzzy->defuzzify(1);

  // --- 3. KONTROL RELAY (LOGIKA AKTIF LOW) ---
  String statusAir = "";

  if (z_akhir <= 1.5) {
    statusAir = "AIR JERNIH";
    digitalWrite(PinRelay, LOW);  
  } 
  else if (z_akhir > 1.5 && z_akhir <= 2.5) {
    statusAir = "AIR KERUH";
    digitalWrite(PinRelay, HIGH); 
  } 
  else {
    statusAir = "AIR SANGAT KERUH";
    digitalWrite(PinRelay, HIGH); 
  }

  // --- 4. Monitoring via Serial Monitor ---
  Serial.print("ADC Mentah: ");  Serial.print(adcRaw);
  Serial.print("\t Volt Virtual: "); Serial.print(vhasil, 1); Serial.print("V");
  Serial.print("\t NTU: ");       Serial.print(kekeruhan, 1);
  Serial.print("\t Fuzzy Z: ");  Serial.print(z_akhir, 2);
  Serial.print("\t Status: ");   Serial.println(statusAir);

  // --- 5. Logika Pengiriman Data ke Telegram Berkala ---
  unsigned long waktuSekarang = millis();
  if (waktuSekarang - waktuSBLM >= intervalTelegram) {
    waktuSBLM = waktuSekarang;
    String teksLaporan = "LAPORAN MONITORING AIR\\n\\n";
    teksLaporan += "ADC Mentah: " + String(adcRaw) + "\\n";
    teksLaporan += "Volt Virtual: " + String(vhasil, 1) + " V\\n";
    teksLaporan += "Kekeruhan: " + String(kekeruhan, 1) + " NTU\\n";
    teksLaporan += "Fuzzy Output Z: " + String(z_akhir, 2) + "\\n";
    teksLaporan += "Status: " + statusAir;
    
    kirimTelegram(teksLaporan);
  }

  delay(1000);
}