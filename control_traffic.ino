#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Thông tin mạng Wi-Fi
#define WIFI_SSID "KALYS 4342"
#define WIFI_PASSWORD "87654321"

// Thông tin Firebase
#define FIREBASE_HOST ".firebaseio.com"
#define FIREBASE_AUTH ""

// GPIO cho đèn giao thông
#define LANE_1_RED D1
#define LANE_1_YELLOW D2
#define LANE_1_GREEN D3
#define LANE_2_RED D4
#define LANE_2_YELLOW D5
#define LANE_2_GREEN D6
#define LANE_3_RED D7
#define LANE_3_YELLOW D8
#define LANE_3_GREEN D9
#define LANE_4_RED D10
#define LANE_4_YELLOW D11
#define LANE_4_GREEN D12

FirebaseData firebaseData;

// Các biến toàn cục
int remainingTimeLane1 = 10; // Thời gian mặc định
int remainingTimeLane2 = 10;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 5000; // Kiểm tra Firebase mỗi 5 giây

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  Serial.println("WiFi connected");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Cài đặt GPIO
  pinMode(LANE_1_RED, OUTPUT);
  pinMode(LANE_1_YELLOW, OUTPUT);
  pinMode(LANE_1_GREEN, OUTPUT);
  pinMode(LANE_2_RED, OUTPUT);
  pinMode(LANE_2_YELLOW, OUTPUT);
  pinMode(LANE_2_GREEN, OUTPUT);
  pinMode(LANE_3_RED, OUTPUT);
  pinMode(LANE_3_YELLOW, OUTPUT);
  pinMode(LANE_3_GREEN, OUTPUT);
  pinMode(LANE_4_RED, OUTPUT);
  pinMode(LANE_4_YELLOW, OUTPUT);
  pinMode(LANE_4_GREEN, OUTPUT);
}

void loop() {
  if (millis() - lastCheckTime > checkInterval) {
    lastCheckTime = millis();
    if (!Firebase.connected()) {
      Serial.println("Lost Firebase connection, switching to default mode");
      defaultMode();
    } else {
      fetchRemainingTime();
    }
  }

  // Điều khiển đèn giao thông
  controlTrafficLights();
}

void fetchRemainingTime() {
  if (Firebase.getInt(firebaseData, "/lane1/remainingTime")) {
    remainingTimeLane1 = firebaseData.intData();
    remainingTimeLane2 = remainingTimeLane1; // Cặp làn 1-3, 2-4 giống nhau
    Serial.println("Fetched remaining time from Firebase");
  } else {
    Serial.println("Failed to fetch remaining time, switching to default mode");
    defaultMode();
  }
}

void defaultMode() {
  remainingTimeLane1 = 10; // Thời gian mặc định
  remainingTimeLane2 = 10;
}

void controlTrafficLights() {
  // Làn 1-3: Đèn xanh
  digitalWrite(LANE_1_GREEN, HIGH);
  digitalWrite(LANE_1_RED, LOW);
  digitalWrite(LANE_1_YELLOW, LOW);
  digitalWrite(LANE_3_GREEN, HIGH);
  digitalWrite(LANE_3_RED, LOW);
  digitalWrite(LANE_3_YELLOW, LOW);

  delay(remainingTimeLane1 * 1000); // Thời gian đèn xanh

  // Làn 1-3: Đèn vàng
  digitalWrite(LANE_1_GREEN, LOW);
  digitalWrite(LANE_1_YELLOW, HIGH);
  digitalWrite(LANE_3_GREEN, LOW);
  digitalWrite(LANE_3_YELLOW, HIGH);
  delay(3000); // 3 giây đèn vàng

  // Làn 1-3: Đèn đỏ, Làn 2-4: Đèn xanh
  digitalWrite(LANE_1_YELLOW, LOW);
  digitalWrite(LANE_1_RED, HIGH);
  digitalWrite(LANE_3_YELLOW, LOW);
  digitalWrite(LANE_3_RED, HIGH);

  digitalWrite(LANE_2_GREEN, HIGH);
  digitalWrite(LANE_2_RED, LOW);
  digitalWrite(LANE_2_YELLOW, LOW);
  digitalWrite(LANE_4_GREEN, HIGH);
  digitalWrite(LANE_4_RED, LOW);
  digitalWrite(LANE_4_YELLOW, LOW);

  delay(remainingTimeLane2 * 1000); // Thời gian đèn xanh

  // Làn 2-4: Đèn vàng
  digitalWrite(LANE_2_GREEN, LOW);
  digitalWrite(LANE_2_YELLOW, HIGH);
  digitalWrite(LANE_4_GREEN, LOW);
  digitalWrite(LANE_4_YELLOW, HIGH);
  delay(3000); // 3 giây đèn vàng

  // Làn 2-4: Đèn đỏ
  digitalWrite(LANE_2_YELLOW, LOW);
  digitalWrite(LANE_2_RED, HIGH);
  digitalWrite(LANE_4_YELLOW, LOW);
  digitalWrite(LANE_4_RED, HIGH);
}
