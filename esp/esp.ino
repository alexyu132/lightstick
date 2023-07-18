#include "WiFi.h"
#include "HTTPClient.h"
#include "FastLED.h"

#define NUM_LEDS 91
#define MAX_IMAGE_COLS 100
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
#define LED_PIN 16

byte image[MAX_IMAGE_COLS][NUM_LEDS][3];
// Actual image width will be known after we finish reading stream
size_t image_width = 0; 
uint8_t curr_render_col = 0;
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[0] = CRGB::Black;
    FastLED.show();
  }

  delay(3000);
  
  HTTPClient http;
  http.begin("http://antlimain.s3.us-west-2.amazonaws.com/image_data.bin");
  
  int httpCode = http.GET();

  WiFiClient *stream = http.getStreamPtr();
  uint8_t value;

  while (http.connected() && image_width < MAX_IMAGE_COLS) {
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t pixelData[3];
        size_t numBytesRead = stream->readBytes(pixelData, 3);
        if (numBytesRead != 3) {
            // This might happen if we are at the end of the stream, or if there's a problem
            break;
        }
        image[image_width][i][0] = pixelData[0]; // R
        image[image_width][i][1] = pixelData[1]; // G
        image[image_width][i][2] = pixelData[2]; // B
    }
    if (stream->available()) {
        image_width++;
    }
  }

  leds[0] = CRGB(255, 0, 0);
  FastLED.setBrightness(35);
  FastLED.show();
}

void setLedsFromImageCol(uint8_t imageCol) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(image[imageCol][i][0], image[imageCol][i][1], image[imageCol][i][2]);
  }
  FastLED.show();
}


void loop() {
  if (curr_render_col == image_width - 1) {
    FastLED.clear();
    FastLED.show();
    delay(500);
    curr_render_col = 0;
  }
  setLedsFromImageCol(curr_render_col);
  curr_render_col++;
  delay(50);
}
