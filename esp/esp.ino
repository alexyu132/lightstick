#include "WiFi.h"
#include "FastLED.h"
#include "ESPAsyncWebSrv.h"
#include "SPIFFS.h"

#define NUM_LEDS 91
#define MAX_IMAGE_COLS 200
#define WIFI_SSID "lightstick"
#define LED_PIN 16

#define BYTES_PER_COLUMN NUM_LEDS*3

uint8_t image[MAX_IMAGE_COLS * NUM_LEDS * 3];

// Actual image width will be known after we finish reading stream
size_t image_width = 0;
size_t dataArrayIndex = 0;
unsigned int curr_render_col = 0;
CRGB leds[NUM_LEDS];

AsyncWebServer server(80);

void setup() {
  memset(image, 0, sizeof(image));

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  WiFi.softAP(WIFI_SSID);

  SPIFFS.begin(true);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/upload", HTTP_POST, handleInitialResponse, nullptr, handleBinaryUpload);

  server.begin();
}

void handleBinaryUpload(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
  if (!index) {
    // Handle the start of the upload
    // You can create/open a file here to save the uploaded data
    dataArrayIndex = 0;  // Reset the array index
  }

  if (len > 0) {
    // Handle incoming binary data
    // data is the data buffer
    // len is the size of the current chunk
    if (dataArrayIndex + len <= sizeof(image)) {
      // Copy the incoming data into the array
      memcpy(image + dataArrayIndex, data, len);
      dataArrayIndex += len;
    } else {
    }
  }

  if (index >= total) {
    // Handle the end of the upload
    // Close the file and perform any necessary processing
    request->send(200, "text/plain", "File upload successful");
    curr_render_col = 0;
  }
}

void handleInitialResponse(AsyncWebServerRequest *request) {  
  request->send(200);
}


void setLedsFromImageCol(unsigned int col) {
  unsigned int offset = col * BYTES_PER_COLUMN;

  // Store the pixel number separately to avoid a division in loop
  uint8_t pixel = 0;

  // Each group of 3 bytes represents R,G,B values for a pixel
  for (int i = 0; i < NUM_LEDS * 3; i += 3) {
    leds[pixel] = CRGB(image[offset + i], image[offset + i + 1], image[offset + i + 2]);
    pixel++;
  }
  FastLED.show();
}


void loop() {
  if (curr_render_col == MAX_IMAGE_COLS - 1) {
    FastLED.clear();
    FastLED.show();
    delay(200);
    curr_render_col = 0;
  }

  setLedsFromImageCol(curr_render_col);

  curr_render_col++;
  delay(50);
}
