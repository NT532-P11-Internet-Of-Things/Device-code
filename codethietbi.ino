#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define NUM_LEDS 8
#define STRIP1_PIN D1
#define STRIP2_PIN D2
#define STRIP3_PIN D3
#define STRIP4_PIN D4

// Tạo đối tượng dải LED cho mỗi strip
Adafruit_NeoPixel strip1(NUM_LEDS, STRIP1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, STRIP2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUM_LEDS, STRIP3_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4(NUM_LEDS, STRIP4_PIN, NEO_GRB + NEO_KHZ800);

String currentColor1 = "Green";
String currentColor2 = "Green";

const char *ssid = "PX";
const char *password = "12345678";

ESP8266WebServer server(5000);

void getColorValues(const char *color, int &r, int &g, int &b)
{
  if (strcmp(color, "Red") == 0)
  {
    r = 255;
    g = 0;
    b = 0;
  }
  else if (strcmp(color, "Green") == 0)
  {
    r = 0;
    g = 255;
    b = 0;
  }
  else if (strcmp(color, "Yellow") == 0)
  {
    r = 255;
    g = 255;
    b = 0;
  }
  else
  {
    r = 255;
    g = 255;
    b = 255;
  }
}

void handlePost()
{
  if (server.hasArg("plain") == false)
  {
    server.send(400, "text/plain", "Body not received");
    return;
  }

  String body = server.arg("plain");
  Serial.println("Received JSON data:");
  Serial.println(body);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char *light_1 = doc["light_1"];
  const char *light_2 = doc["light_2"];

  int r1, g1, b1;
  int r2, g2, b2;

  getColorValues(light_1, r1, g1, b1);
  getColorValues(light_2, r2, g2, b2);

if (strcmp(light_1, currentColor1.c_str()) != 0)
  {
    // Clear previous colors
    strip1.clear();
    strip3.clear();

    if (strcmp(light_1, "Red") == 0)
    {
      strip1.setPixelColor(0, r1, g1, b1);
      strip1.setPixelColor(1, r1, g1, b1);
      strip3.setPixelColor(0, r1, g1, b1);
      strip3.setPixelColor(1, r1, g1, b1);
    }
    else if (strcmp(light_1, "Yellow") == 0)
    {
      strip1.setPixelColor(2, r1, g1, b1);
      strip1.setPixelColor(3, r1, g1, b1);
      strip3.setPixelColor(2, r1, g1, b1);
      strip3.setPixelColor(3, r1, g1, b1);
    }
    else if (strcmp(light_1, "Green") == 0)
    {
      strip1.setPixelColor(4, r1, g1, b1);
      strip1.setPixelColor(5, r1, g1, b1);
      strip3.setPixelColor(4, r1, g1, b1);
      strip3.setPixelColor(5, r1, g1, b1);
    }

    strip1.show();
    strip3.show();
    currentColor1 = light_1;
    Serial.println("Change color 1 to " + currentColor1);
  }

  if (strcmp(light_2, currentColor2.c_str()) != 0)
  {
    // Clear previous colors
    strip2.clear();
    strip4.clear();

    if (strcmp(light_2, "Red") == 0)
    {
      strip2.setPixelColor(0, r2, g2, b2);
      strip2.setPixelColor(1, r2, g2, b2);
      strip4.setPixelColor(0, r2, g2, b2);
      strip4.setPixelColor(1, r2, g2, b2);
    }
    else if (strcmp(light_2, "Yellow") == 0)
    {
      strip2.setPixelColor(2, r2, g2, b2);
      strip2.setPixelColor(3, r2, g2, b2);
      strip4.setPixelColor(2, r2, g2, b2);
      strip4.setPixelColor(3, r2, g2, b2);
    }
    else if (strcmp(light_2, "Green") == 0)
    {
      strip2.setPixelColor(4, r2, g2, b2);
      strip2.setPixelColor(5, r2, g2, b2);
      strip4.setPixelColor(4, r2, g2, b2);
      strip4.setPixelColor(5, r2, g2, b2);
    }

    strip2.show();
    strip4.show();
    currentColor2 = light_2;
    Serial.println("Change color 2 to " + currentColor2);
  }


  server.send(200, "application/json", "{\"status\":\"received\"}");
  Serial.println("/////////////////////////////");
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/postjson", HTTP_POST, handlePost);
  server.begin();
  Serial.println("Server started");

  strip1.begin();
  strip2.begin();
  strip3.begin();
  strip4.begin();
  strip1.show(); 
  strip2.show();
  strip3.show();
  strip4.show(); 
}

void loop()
{
  server.handleClient();
}