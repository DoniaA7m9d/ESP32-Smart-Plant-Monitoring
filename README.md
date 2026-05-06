 🌱 IoT Smart Plant Monitoring System | ESP32 + Blynk + Telegram

Low-power ESP32-based soil moisture monitoring with automated Telegram alerts, Blynk IoT dashboard, and Deep Sleep mode for 95% power savings.

 Technical Features
- Ultra Low Power: ESP32 Deep Sleep mode - wakes every 10 min, reduces power by 95%
- Smart Alerting: Telegram bot sends instant alerts only when moisture < 30%
- Daily Analytics: Automated 8 PM report: max/min moisture, drought frequency, total checks
- Remote Control: Mute buzzer via Blynk app using Virtual Pin V2
- Data Persistence: RTC memory tracks daily statistics across sleep cycles
- NTP Time Sync: Accurate time-based reporting with GMT+2 timezone

 Hardware
- ESP32 DevKit V1
- Capacitive Soil Moisture Sensor
- Active Buzzer Module

 Setup
1. Replace credentials with your own in the code
2. Install libraries: `Blynk`, `UniversalTelegramBot`
3. Upload via Arduino IDE or Arduino Droid

 Future Work
Integrating relay + water pump for closed-loop automated irrigation

 How It Works
| Time | Action |
| --- | --- |
| Every 10 min | Wake, read sensor, sleep |
| If Dry < 30% | Telegram alert + Buzzer |
| 8:00 PM | Daily summary report |
