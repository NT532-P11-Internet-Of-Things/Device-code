#include <Adafruit_NeoPixel.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

// Thông tin mạng Wi-Fi
#define WIFI_SSID "KALYS 4342"
#define WIFI_PASSWORD "87654321"

// Thông tin Firebase
#define FIREBASE_HOST "smart-traffic-light-03-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "YOUR_FIREBASE_AUTH"

// Pin cho dải WS2812
#define LED_PIN D1
#define NUM_LEDS 12

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

FirebaseData firebaseData;

// Các biến toàn cục
int greenTime[5] = {0, 10, 10, 10, 10}; // Thời gian đèn xanh (Lane 1 đến 4)
int remainingTime[5] = {0, 10, 10, 10, 10};
bool isGreen[5] = {false, true, false, true, false}; // Trạng thái đèn xanh (Lane 1 đến 4)

// Hàm khởi tạo
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Cài đặt WS2812
  strip.begin();
  strip.show(); // Tắt tất cả đèn
  syncWithFirebase(); // Đồng bộ ngay khi khởi động
}

// Vòng lặp chính
void loop() {
  handleTrafficLight();
}

void syncWithFirebase() {
  for (int lane = 1; lane <= 4; lane++) {
    String path = "/traffic_system/intersections/main_intersection/lanes/" + String(lane);
    if (Firebase.getJSON(firebaseData, path)) {
      String jsonStr = firebaseData.jsonString();
      FirebaseJson json;
      json.setJsonData(jsonStr);

      // Lấy giá trị từ Firebase
      json.get(greenTime[lane], "green_time");
      json.get(isGreen[lane], "is_green");
      json.get(remainingTime[lane], "remaining_time");

      Serial.printf("Lane %d synced with Firebase\n", lane);
    } else {
      Serial.printf("Failed to sync Lane %d with Firebase\n", lane);
    }
  }
}

void handleTrafficLight() {
  for (int lane = 1; lane <= 4; lane++) {
    if (lane == 3) continue; // Lane 3 đồng bộ với Lane 1
    if (lane == 4) continue; // Lane 4 đồng bộ với Lane 2

    if (isGreen[lane]) {
      // Đèn xanh
      setTrafficLight(lane, "GREEN");
      remainingTime[lane]--;

      if (remainingTime[lane] <= 0) {
        isGreen[lane] = false; // Chuyển sang đèn vàng + đỏ
        remainingTime[lane] = greenTime[lane] + 3;
      }
    } else {
      if (remainingTime[lane] > greenTime[lane]) {
        // Đèn vàng
        setTrafficLight(lane, "YELLOW");
        remainingTime[lane]--;
      } else if (remainingTime[lane] > 0) {
        // Đèn đỏ
        setTrafficLight(lane, "RED");
        remainingTime[lane]--;
      } else {
        // Kết thúc chu kỳ, cập nhật Firebase và chuyển đèn xanh
        updateFirebaseState(lane);
        isGreen[lane] = true;
        remainingTime[lane] = greenTime[lane];
      }
    }
    syncPairLane(lane); // Đồng bộ với lane đối diện
  }
  delay(1000);
}

void syncPairLane(int lane) {
  if (lane == 1) {
    isGreen[3] = isGreen[1];
    remainingTime[3] = remainingTime[1];
    setTrafficLight(3, getLightColor(isGreen[3], remainingTime[3], greenTime[3]));
  } else if (lane == 2) {
    isGreen[4] = isGreen[2];
    remainingTime[4] = remainingTime[2];
    setTrafficLight(4, getLightColor(isGreen[4], remainingTime[4], greenTime[4]));
  }
}

void updateFirebaseState(int lane) {
  FirebaseJson json;
  json.set("is_green", isGreen[lane]);
  json.set("remaining_time", remainingTime[lane]);

  String path = "/traffic_system/intersections/main_intersection/lanes/" + String(lane);
  if (Firebase.updateNode(firebaseData, path, json)) {
    Serial.printf("Lane %d updated Firebase state\n", lane);
  } else {
    Serial.printf("Failed to update Firebase state for Lane %d\n", lane);
  }
}

String getLightColor(bool isGreen, int remainingTime, int greenTime) {
  if (isGreen) return "GREEN";
  if (remainingTime > greenTime) return "YELLOW";
  return "RED";
}

void setTrafficLight(int lane, String color) {
  int offset = (lane - 1) * 3;
  if (color == "RED") {
    strip.setPixelColor(offset, strip.Color(255, 0, 0));    // Bật đèn đỏ
    strip.setPixelColor(offset + 1, strip.Color(0, 0, 0));  // Tắt đèn vàng
    strip.setPixelColor(offset + 2, strip.Color(0, 0, 0));  // Tắt đèn xanh
  } else if (color == "YELLOW") {
    strip.setPixelColor(offset, strip.Color(0, 0, 0));      // Tắt đèn đỏ
    strip.setPixelColor(offset + 1, strip.Color(255, 255, 0)); // Bật đèn vàng
    strip.setPixelColor(offset + 2, strip.Color(0, 0, 0));  // Tắt đèn xanh
  } else if (color == "GREEN") {
    strip.setPixelColor(offset, strip.Color(0, 0, 0));      // Tắt đèn đỏ
    strip.setPixelColor(offset + 1, strip.Color(0, 0, 0));  // Tắt đèn vàng
    strip.setPixelColor(offset + 2, strip.Color(0, 255, 0)); // Bật đèn xanh
  }
  strip.show();
}
