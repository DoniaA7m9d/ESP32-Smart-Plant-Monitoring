#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME "Smart Plant"
#define BLYNK_AUTH_TOKEN ""

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

#define SOIL_PIN 34
#define BUZZER_PIN 13
#define VIRT_MOISTURE V0
#define VIRT_PLANT_STATUS V1
#define VIRT_BUZZER_MUTE V2

#define DRY_THRESHOLD 30
#define uS_TO_MIN_FACTOR 60000000
#define TIME_TO_SLEEP 10
#define REPORT_HOUR 20

char ssid[] = "";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;

#define BOTtoken ""
#define CHAT_ID ""

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;
const int daylightOffset_sec = 3600;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
BlynkTimer timer;

int soilMoisture;
int rawValue;
bool isBuzzerMuted = false;

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int dryCount = 0;
RTC_DATA_ATTR int wetCount = 0;
RTC_DATA_ATTR int maxMoisture = 0;
RTC_DATA_ATTR int minMoisture = 100;
RTC_DATA_ATTR int lastReportDay = -1;

BLYNK_WRITE(VIRT_BUZZER_MUTE) {
  isBuzzerMuted = param.asInt();
}

void readSoilMoisture() {
  rawValue = analogRead(SOIL_PIN);
  soilMoisture = map(rawValue, 3500, 1200, 0, 100);
  soilMoisture = constrain(soilMoisture, 0, 100);
  
  if(soilMoisture > maxMoisture || maxMoisture == 0) maxMoisture = soilMoisture;
  if(soilMoisture < minMoisture || minMoisture == 100) minMoisture = soilMoisture;
}

void sendAlert() {
  String alert = "WARNING: Plant is Dry\n\n";
  alert += "Current Moisture: " + String(soilMoisture) + "%\n";
  alert += "Below Threshold: " + String(DRY_THRESHOLD) + "%\n\n";
  alert += "Please water the plant now";
  bot.sendMessage(CHAT_ID, alert, "");
}

void sendDailyReport() {
  String report = "Daily Plant Report\n\n";
  report += "Total Checks Today: " + String(bootCount) + "\n";
  report += "Max Moisture: " + String(maxMoisture) + "%\n";
  report += "Min Moisture: " + String(minMoisture) + "%\n";
  report += "Dry Count: " + String(dryCount) + "\n";
  report += "Wet Count: " + String(wetCount) + "\n\n";
  
  if(minMoisture < DRY_THRESHOLD) {
    report += "Plant was dry " + String(dryCount) + " times today";
  } else {
    report += "Plant was healthy all day";
  }
  
  bot.sendMessage(CHAT_ID, report, "");
  
  bootCount = 0;
  dryCount = 0;
  wetCount = 0;
  maxMoisture = 0;
  minMoisture = 100;
}

void goToSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_MIN_FACTOR);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  bootCount++;
  
  WiFi.begin(ssid, pass);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Blynk.config(auth);
  Blynk.connect();
  
  readSoilMoisture();
  
  if(soilMoisture < DRY_THRESHOLD) {
    dryCount++;
    sendAlert();
    if(!isBuzzerMuted) {
      for(int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
    }
  } else {
    wetCount++;
  }
  
  Blynk.virtualWrite(VIRT_MOISTURE, soilMoisture);
  
  if(soilMoisture < DRY_THRESHOLD) {
    Blynk.virtualWrite(VIRT_PLANT_STATUS, "Plant is Dry");
  } else {
    Blynk.virtualWrite(VIRT_PLANT_STATUS, "Plant is Happy");
  }
  
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)) {
    if(timeinfo.tm_hour == REPORT_HOUR && timeinfo.tm_mday != lastReportDay) {
      sendDailyReport();
      lastReportDay = timeinfo.tm_mday;
    }
  }
  
  Blynk.run();
  delay(2000);
  
  goToSleep();
}

void loop() {
}