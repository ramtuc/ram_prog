
#include <Arduino.h>
#include <M5UnitGLASS2.h>
#include <M5Unified.h>
#include <WiFi.h>
#include "time.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp32-hal-timer.h"

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?id=1858067&APPID=";
const String key = "75ef8309145b74ca04800e97ed4288f3";
const char* ssid = "aterm-4c6fba-g";
const char* password = "8343afa25010e";
const char* ntpServer = "jp.pool.ntp.org";
const long gmtOffset_sec = 3600 * 9;  // 日本はUTC+9
const int daylightOffset_sec = 0;     // 日本は夏時間を採用していないため0
const int buttonPin = 41;
int time_flag = 0, btn_flag = 0;
int mode = 0;
unsigned long previousMillis = 0;  // 前回の同期時刻を保持
const long interval = 43200000;    // 24時間 = 86,400,000ミリ秒

void IRAM_ATTR handleButtonPress() {
  if (btn_flag == 0) {
    if (mode == 0) {
      mode = 1;
    } else {
      mode = 0;
    }
    btn_flag = 1;
  }
  Serial.println(mode);
}

void IRAM_ATTR timerCallback() {
  // ここにタイマー割り込みで実行したい処理を記述
  time_flag = 1;
  Serial.println("5分経過しました！");
}

double kelvinToCelsius(double kelvin) {
  return kelvin - 273.15;
}

const char* convertWeatherToReadable(const char* weather) {
  if (strcmp(weather, "Clear") == 0) return "Sunny";
  else if (strcmp(weather, "Few Clouds") == 0) return "Mostly Clear";
  else if (strcmp(weather, "Clouds") == 0) return "Cloud";
  else if (strcmp(weather, "Scattered Clouds") == 0) return "Partly Cloud";
  else if (strcmp(weather, "Broken Clouds") == 0) return "Mostly Cloud";
  else if (strcmp(weather, "Shower Rain") == 0) return "Shower Rain";
  else if (strcmp(weather, "Rain") == 0) return "Rain";
  else if (strcmp(weather, "Thunderstorm") == 0) return "Thunderstorm";
  else if (strcmp(weather, "Snow") == 0) return "Snow";
  else if (strcmp(weather, "Mist") == 0) return "Mist";
  else return "Unknown";  // 未知の天気
}

void fetchData() {
  HTTPClient http;
  http.begin(endpoint + key);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);

    parseWeatherData(payload);
  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();
}

void parseWeatherData(const String& json) {
  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("deserializeJson() failed");
    Serial.println(error.c_str());
    return;
  }

  const char* weather = doc["weather"][0]["main"];
  const double temp = kelvinToCelsius(doc["main"]["temp"].as<double>());
  const double temp_max = kelvinToCelsius(doc["main"]["temp_max"].as<double>());
  const double temp_min = kelvinToCelsius(doc["main"]["temp_min"].as<double>());
  const double humidity = doc["main"]["humidity"].as<double>();

  Serial.print("Weather: ");
  Serial.println(weather);
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Max Temperature: ");
  Serial.println(temp_max);
  Serial.print("Min Temperature: ");
  Serial.println(temp_min);
  Serial.print("Humidity: ");
  Serial.println(humidity);


  M5.Displays(1).setTextSize(1);
  M5.Displays(1).setCursor(2, 5);
  M5.Displays(1).print("Weather: " + String(convertWeatherToReadable(weather)));
  M5.Displays(1).setCursor(2, 15);
  M5.Displays(1).print("Temp:    " + String(temp) + "C");
  M5.Displays(1).setCursor(2, 25);
  M5.Displays(1).print("Humi:    " + String(humidity) + "%");
  M5.Displays(1).setCursor(2, 35);
  M5.Displays(1).print("MaxTemp: " + String(temp_max) + "C");
  M5.Displays(1).setCursor(2, 45);
  M5.Displays(1).print("MinTemp: " + String(temp_min) + "C");
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}


void printLocalTimedisp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  M5.Displays(1).setTextSize(2);
  M5.Displays(1).setCursor(2, 0);
  M5.Displays(1).print(&timeinfo, "%Y-%m-%d");
  M5.Displays(1).setCursor(2, 25);
  M5.Displays(1).setTextSize(3);
  M5.Displays(1).print(&timeinfo, "%H:%M");
  M5.Displays(1).setTextSize(2);
  M5.Displays(1).setCursor(92, 32);
  M5.Displays(1).print(&timeinfo, ":%S");
}

void setup(void) {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  fetchData();
  // NTPサーバーの設定
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // 時刻の取得
  printLocalTime();

  auto cfg = M5.config();
  cfg.external_display.unit_glass2 = true;  // default=true. use UnitGLASS2
  // begin M5Unified.
  M5.begin(cfg);

  // Get the number of available displays
  int display_count = M5.getDisplayCount();

  for (int i = 0; i < display_count; ++i) {
    // All displays are available in M5.Displays.
    // ※ Note that the order of which displays are numbered is the order in which they are detected, so the order may change.

    int textsize = M5.Displays(i).height() / 60;
    if (textsize == 0) { textsize = 1; }
    M5.Displays(i).setTextSize(textsize);
    M5.Displays(i).printf("No.%d\n", i);
  }


  // If an external display is to be used as the main display, it can be listed in order of priority.
  M5.setPrimaryDisplayType({
    m5::board_t::board_M5UnitGLASS2,
  });


  // The primary display can be used with M5.Display.
  M5.Display.print("primary display\n");
  int index_unit_glass2 = M5.getDisplayIndex(m5::board_t::board_M5UnitGLASS2);
  if (index_unit_glass2 >= 0) {
    M5.Displays(index_unit_glass2).print("This is Unit GLASS2\n");
  }
  hw_timer_t* timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerCallback, true);
  timerAlarmWrite(timer, 300000000, true);
  timerAlarmEnable(timer);
  attachInterrupt(buttonPin, handleButtonPress, FALLING);
  M5.delay(5000);
  M5.Displays(index_unit_glass2).clear();
}

void loop(void) {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // 24時間が経過したら時刻同期を行う
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    previousMillis = currentMillis;  // 最後の同期時刻を更新
    Serial.println("Time synchronized with NTP server.");
  }
  if (btn_flag == 1) {
    btn_flag = 0;
    M5.Displays(1).clear();
    M5.Displays(0).clear();
  }
  M5.delay(1);
  if (mode == 0) {
    time_flag = 1;
    printLocalTimedisp();
    M5.Displays(0).setTextSize(3);
    M5.Displays(0).setCursor(20, 52);
    M5.Displays(0).print("Clock");
  } else {
    M5.Displays(0).setTextSize(3);
    M5.Displays(0).setCursor(0, 52);
    M5.Displays(0).print("Whether");
    if (time_flag == 1) {
      time_flag = 0;
      fetchData();
    }
  }
  delay(100);
}
