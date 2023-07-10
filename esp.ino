#include "WiFi.h"
#include "HTTPClient.h"
#include "FastLED.h"

#define NUM_LEDS 91
#define MAX_IMAGE_COLS 100
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define LED_PIN 2

uint8_t image[MAX_IMAGE_COLS][NUM_LEDS][3];
// Actual image width will be known after we finish reading stream
size_t image_width = 0; 
uint8_t curr_render_col = 0;
CRGB leds[NUM_LEDS];

void setup() {
  
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected! ");
  
  HTTPClient http;
  http.begin("http://antlimain.s3.us-west-2.amazonaws.com/image_data.bin");
  
  Serial.println("Starting image download");
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Image file download failed with HTTP Code ");
    Serial.print(httpCode);
    return;
  } else {
    Serial.println("Image download success!");
  }

  WiFiClient *stream = http.getStreamPtr();
  uint8_t value;


  while (http.connected() && (stream->available()) && image_width < MAX_IMAGE_COLS) {
    for (int i = 0; i < NUM_LEDS; i++) {
      image[image_width][i][0] = stream->read(); // R
      image[image_width][i][1] = stream->read(); // G
      image[image_width][i][2] = stream->read(); // B
      if (!stream->available()) {
        break;
      }
    }
    image_width++;
  }
  // TODO: Occasionally image_width is lower than expected? Should be 91
  Serial.print("Finished loading image with width: ");
  Serial.print(image_width);
  Serial.println("");



  leds[0] = CRGB(255, 0, 0);
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
    delay(3000);
    curr_render_col = 0;
  }
  setLedsFromImageCol(curr_render_col);
  curr_render_col++;
  delay(100);
}