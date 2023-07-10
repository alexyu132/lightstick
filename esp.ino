#include "WiFi.h"
#include "HTTPClient.h"

#define NUM_LEDS 91
#define MAX_IMAGE_COLS 100
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

uint8_t image[MAX_IMAGE_COLS][NUM_LEDS][3];
// Actual image width will be known after we finish reading stream
size_t image_width = 0; 

void setup() {
  Serial.begin(9600);
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
}

void loop() {
  delay(100);
}