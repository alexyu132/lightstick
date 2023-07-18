#include "WiFi.h"
#include "HTTPClient.h"
#include "FastLED.h"

#define NUM_LEDS 91
#define MAX_IMAGE_COLS 200
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
#define LED_PIN 16

#define BYTES_PER_COLUMN NUM_LEDS*3

uint8_t image[MAX_IMAGE_COLS * NUM_LEDS * 3];

// Actual image width will be known after we finish reading stream
size_t image_width = 0;
unsigned int curr_render_col = 0;
CRGB leds[NUM_LEDS];

void setup() {
  memset(image, 0, sizeof(image));

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
  http.begin("http://antlimain.s3.us-west-2.amazonaws.com/neon.bin");

  int httpCode = http.GET();

  WiFiClient *stream = http.getStreamPtr();

  // Dump the raw bytes for the image into the image array
  // image_width stores num bytes read after this function call
  image_width = stream->readBytes(image, sizeof(image));

  // Get ceiling of bytes divided by bytes per column to get true image width
  image_width = (image_width + BYTES_PER_COLUMN - 1) / (BYTES_PER_COLUMN);

  leds[0] = CRGB(255, 0, 0);
  FastLED.setBrightness(35);
  FastLED.show();
}

void setLedsFromImageCol(unsigned int col) {
  unsigned int offset = col * BYTES_PER_COLUMN;

  // Store the pixel number separately to avoid a division in loop
  uint8_t pixel = 0;

  // Each group of 3 bytes represents R,G,B values for a pixel
  for (int i = 0; i < NUM_LEDS * 3; i+=3) {
    leds[pixel] = CRGB(image[offset+i],image[offset+i+1],image[offset+i+2]);
    pixel++;
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
