#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseArduino.h>
#include <DHT.h>
#include <time.h>

// Config Firebase
#define FIREBASE_HOST "nodemcu-dht11-5aabf.firebaseio.com"
#define FIREBASE_AUTH "FIbDoTk42oPrRI9P6Yv3sjzPwbwbHdhCKBOeB4tl"

// Config connect WiFi
#define WIFI_SSID "nattois" // Put here your Wi-Fi SSID
#define WIFI_PASSWORD "ggongchisson" // Put here your Wi-Fi password

// Config DHT
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiUDP udp;
NTPClient timeClient(udp, "kr.pool.ntp.org", 32400, 3600000);

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);  // Wi-Fi 초기화
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting......");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // NTP 설정(네트워크 시간 설정)
  timeClient.begin();

  // Firebase 실시간 데이터베이스 인증
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  delay(5000);

  // Read temp & Humidity for DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return;
  }

  timeClient.update();

  //  int hour = timeClient.getHours();
  //  int minute = timeClient.getMinutes();
  //  int second = timeClient.getSeconds();

  unsigned long epochTime = timeClient.getEpochTime();

  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int currentMonth = ptm->tm_mon + 1;
  int monthDay = ptm->tm_mday;
  int currentYear = ptm->tm_year + 1900;
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  String currentTime = timeClient.getFormattedTime();

  String currentDateTime = currentDate + " " + currentTime;

  //  Serial.println(timeClient.getFormattedTime());
  //  Serial.println(currentDate);
  //
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("   Temperature: ");
  Serial.print(t);
  Serial.print("   Time: ");
  Serial.print(currentDateTime);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = t;
  root["humidity"] = h;
  //root["time"] = hour;
  root["time"] = currentDateTime;

  // append a new value to /logDHT
  String name = Firebase.push("logDHT", root);
  // handle error
  if (Firebase.failed()) {
    Serial.println("pushing /logs failed:");
    Serial.println(Firebase.error());
    return;
  }
  Serial.println("");
  Serial.print("   pushed: /logDHT/");
  Serial.println(name + "\n");
  delay(5000);
}
