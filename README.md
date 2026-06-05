## Sistem Pemantau Kualitas Air Bersih Otomatis (IoT)

Proyek Tugas Besar Mikrokontroler (Kelas Micro A / 2023) berupa sistem otomatisasi pemantauan dan pemblokiran air keruh skala rumah tangga menggunakan **ESP32/ESP8266**, **Fuzzy Logic Sugeno**, dan notifikasi **Telegram Bot**[cite: 1].

---

## Overview Proyek
Terinspirasi dari masalah kualitas air PDAM di Kota Samarinda yang sering keruh akibat pencemaran Sungai Mahakam, proyek ini hadir sebagai solusi deteksi dini[cite: 1]. 

Sistem ini bekerja secara *real-time* untuk mencegah air kotor masuk ke tandon rumah warga dengan cara menutup aliran air secara otomatis melalui *Solenoid Valve* jika mendeteksi tingkat kekeruhan (NTU) di luar ambang batas aman[cite: 1].

## Cara Kerja Sistem
1. **Sensing:** Sensor Turbidity Seno189 membaca tingkat kekeruhan air[cite: 1].
2. **Processing:** Mikrokontroler mengolah data menggunakan **Fuzzy Logic Sugeno** untuk menentukan status air (Jernih, Keruh, atau Sangat Keruh)[cite: 1].
3. **Actuating:** 
   * **Air Jernih (Z <= 1.5):** *Solenoid Valve* terbuka (air mengalir ke tandon)[cite: 1].
   * **Air Keruh/Sangat Keruh (Z > 1.5):** Relay memicu *Solenoid Valve* untuk menutup aliran air[cite: 1].
4. **Notifikasi:** Sistem mengirimkan laporan kondisi air (Nilai ADC, Voltase, NTU, dan Status) langsung ke Telegram Bot secara berkala[cite: 1].

---

## Spesifikasi Utama & Komponen
* **Brain:** ESP8266MOD / ESP32 + Bluino IoT Starter Kit[cite: 1].
* **Input:** Turbidity Sensor Seno189 (Range 0 - 3000 NTU)[cite: 1].
* **Output:** Solenoid Valve (12V DC / AC 220V) & Relay Module 1-Channel[cite: 1].
* **Software & Protocol:** Arduino IDE, eFLL (Embedded Fuzzy Logic Library), Universal Telegram Bot API, dan HTTPS (JSON format)[cite: 1].

---

## Hasil Pengujian & Temuan Penting
* **Akurasi Sistem:** Mencapai **75%** dalam mengklasifikasikan kondisi air berdasarkan pengujian 3 sampel (Jernih, Keruh, Sangat Keruh)[cite: 1].
* **Catatan Analisis:** Ditemukan kendala *drop* tegangan. Sensor Turbidity membutuhkan suplai ideal 5V, namun karena menggunakan output dari *microcontroller* yang tidak stabil (~3.5V), pembacaan sensor menjadi kurang maksimal pada kondisi air yang ekstrem[cite: 1].

---

## Tim Pengembang
* Vicky Ahmad Fernanda (2309106123)[cite: 1]
* Hafizal Asfa Wadhiah (2309106129)[cite: 1]
* Akmal Alvian Pratama (2309106021)[cite: 1]
* Muhammad Dzacky (2309106101)[cite: 1]

**Dosen Pengampu:** Anton Prafanto, S.T., M.T.  
**Fakultas Teknik • Universitas Mulawarman • 2025/2026**[cite: 1]
