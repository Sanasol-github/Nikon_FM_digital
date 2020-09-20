//Copyright (c) Sanasol@asanonasa_net
//special thanks ano研 https://anoken.jimdofree.com/
//special thanks mgo-tec https://www.mgo-tec.com/
//You can freely modify and publish your 3D data and code as long as you adhere to the following guidelines.
//Any modifications or changes you make based on this project must be disclosed to the public.
//When you publish your modification, you must write "Nikon_FM_digital".
//"https://www.youtube.com/watch?v=vqoochG0a4w&t=4s"
//Please describe both the

#include "esp_camera.h"
#include <WiFi.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>

const char* ssid = "ex.Sanasol";//ANDROID's Tethering SSID
const char* passwd = "ex.asanonasa_net";//ANDROID's Tethering password
const char* host = "notify-api.line.me";
const char* token = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";//LINE TOKEN

#define CAMERA_MODEL_M5STACK_WIDE
#include "camera_pins.h"

const int cameraCAP = 40;//Brightness threshold at the time of image acquisition
const int cameraRST = cameraCAP - 1;

RTC_DATA_ATTR int bootCount = 0;

#define NOTE_DS8 493.883
const int channel = 15;
const int melody1 = NOTE_DS8;
const int duration = 100;

void setup() {
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  pinMode(4, OUTPUT);
  ledcSetup(channel, 12000, 8);
  ledcAttachPin(4, channel);
  setup_camera_VGA_JPEG();
  Serial.begin(115200);
  Serial.println();

  if ( bootCount == 0 ) {
    delay(1000);
  }

  bootCount++;
  Serial.printf("起動回数: %d ", bootCount);

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0      : Serial.printf("外部割り込み(RTC_IO)で起動\n"); break;
    case ESP_SLEEP_WAKEUP_EXT1      : Serial.printf("外部割り込み(RTC_CNTL)で起動 IO=%llX\n", esp_sleep_get_ext1_wakeup_status()); break;
    case ESP_SLEEP_WAKEUP_TIMER     : Serial.printf("タイマー割り込みで起動\n"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD  : Serial.printf("タッチ割り込みで起動 PAD=%d\n", esp_sleep_get_touchpad_wakeup_status()); break;
    case ESP_SLEEP_WAKEUP_ULP       : Serial.printf("ULPプログラムで起動\n"); break;
    case ESP_SLEEP_WAKEUP_GPIO      : Serial.printf("ライトスリープからGPIO割り込みで起動\n"); break;
    case ESP_SLEEP_WAKEUP_UART      : Serial.printf("ライトスリープからUART割り込みで起動\n"); break;
    default                         : Serial.printf("スリープ以外からの起動\n"); break;
  }

  pinMode(GPIO_NUM_13, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, LOW);

  setup_wifi();

  delay(1000);

}

void loop() {

  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  uint8_t * test_buf = NULL;
  test_buf = fb->buf;

  uint8_t grayvalue = 0;

  grayvalue = *(test_buf);
  //  Serial.println(grayvalue);
  if (grayvalue > cameraCAP) {
    const char * path = "/test.jpg";
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_SVGA);
    s->set_pixformat(s, PIXFORMAT_JPEG);

    capture_camera();

    s->set_framesize(s, FRAMESIZE_QQVGA);
    s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

    Serial.println("cameraRST");
  }
  boolean Ledval = digitalRead(GPIO_NUM_13);
  if (Ledval == true) {
    ledcWriteTone(channel, melody1);
    delay(duration);
    ledcWriteTone(channel, 0);
    delay(duration);
    ledcWriteTone(channel, melody1);
    delay(duration);
    ledcWriteTone(channel, 0);
    delay(duration);
    ledcWriteTone(channel, melody1);
    delay(duration);
    ledcWriteTone(channel, 0);
    delay(duration);
    ledcWriteTone(channel, melody1);
    delay(duration);
    ledcWriteTone(channel, 0);

    esp_deep_sleep_start();
  }

  Serial.println();

}

void setup_wifi() {     // WiFi network
  int COUNTER = 0;
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    COUNTER = ++COUNTER;
    Serial.print(".");
    if (COUNTER > 20) {
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);
      delay(duration);
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);
      delay(duration);
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);
      delay(duration);
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);
      delay(duration);
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);
      delay(duration);
      ledcWriteTone(channel, melody1);
      delay(duration);
      ledcWriteTone(channel, 0);

      esp_deep_sleep_start();
    }

  }
  if (WiFi.status() == WL_CONNECTED) {
    ledcWriteTone(channel, melody1);
    delay(duration);
    ledcWriteTone(channel, 0);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

//Line
void sendLineNotify(uint8_t* image_data, size_t image_sz) {
  WiFiClientSecure client;
  if (!client.connect(host, 443))   return;
  int httpCode = 404;
  size_t image_size = image_sz;
  String boundary = "----purin_alert--";
  String body = "--" + boundary + "\r\n";
  String message = "photo shoot!";
  body += "Content-Disposition: form-data; name=\"message\"\r\n\r\n" + message + " \r\n";
  if (image_data != NULL && image_sz > 0 ) {
    image_size = image_sz;
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"image.jpg\"\r\n";
    body += "Content-Type: image/jpeg\r\n\r\n";
  }
  String body_end = "--" + boundary + "--\r\n";
  size_t body_length = body.length() + image_size + body_end.length();
  String header = "POST /api/notify HTTP/1.1\r\n";
  header += "Host: notify-api.line.me\r\n";
  header += "Authorization: Bearer " + String(token) + "\r\n";
  header += "User-Agent: " + String("M5Stack") + "\r\n";
  header += "Connection: close\r\n";
  header += "Cache-Control: no-cache\r\n";
  header += "Content-Length: " + String(body_length) + "\r\n";
  header += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n\r\n";
  client.print( header + body);
  Serial.print(header + body);

  bool Success_h = false;
  uint8_t line_try = 3;
  while (!Success_h && line_try-- > 0) {
    if (image_size > 0) {
      size_t BUF_SIZE = 1024;
      if ( image_data != NULL) {
        uint8_t *p = image_data;
        size_t sz = image_size;
        while ( p != NULL && sz) {
          if ( sz >= BUF_SIZE) {
            client.write( p, BUF_SIZE);
            p += BUF_SIZE; sz -= BUF_SIZE;
          } else {
            client.write( p, sz);
            p += sz; sz = 0;
          }
        }
      }
      client.print("\r\n" + body_end);
      Serial.print("\r\n" + body_end);

      while ( client.connected() && !client.available()) delay(10);
      if ( client.connected() && client.available() ) {
        String resp = client.readStringUntil('\n');
        httpCode    = resp.substring(resp.indexOf(" ") + 1, resp.indexOf(" ", resp.indexOf(" ") + 1)).toInt();
        Success_h   = (httpCode == 200);
        Serial.println(resp);
      }
      delay(10);
    }
  }
  client.stop();
}



void capture_camera() {

  uint32_t data_len = 0;
  camera_fb_t * fb = esp_camera_fb_get();
  Serial.printf("image size%d[byte]:width%d,height%d,format%d\r\n", fb->len, fb->width, fb->height, fb->format);
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  sensor_t * s = esp_camera_sensor_get();

  auto st = s->status;
  printf(" JPEG Quality: %u\n", st.quality);
  printf(" Contrast: %d\n", st.contrast);
  printf(" Brightness: %d\n", st.brightness);
  printf(" Saturation: %d\n", st.saturation);
  printf(" Vertical Flip: %d\n", (st.vflip));
  printf(" Horizontal Mirror: %d\n", (st.hmirror));
  printf(" Special Effect: %u\n", st.special_effect);
  printf(" White Balance Mode: %u\n", st.wb_mode);
  printf(" Auto White Balance: %u\n", st.awb);
  printf(" Auto White Balance Gain: %u\n", st.awb_gain);
  printf(" Auto Exposure Control: %u\n", st.aec);
  printf(" Auto Exposure Control 2: %u\n", st.aec2);
  printf(" Auto Exposure Level: %d\n", st.ae_level);
  printf(" Auto Exposure Value: %u\n", st.aec_value);
  printf(" AGC: %u\n", st.agc);
  printf(" AGC Gain: %u\n", st.agc_gain);
  printf(" Gain Ceiling: %u\n", st.gainceiling);
  printf(" BPC: %u\n", st.bpc);
  printf(" WPC: %u\n", st.wpc);
  printf(" RAW_GMA: %u\n", st.raw_gma);
  printf(" Lens Correction: %u\n", st.lenc);
  printf(" DCW: %u\n", st.dcw);
  printf(" Test Pattern: %d\n", (st.colorbar));

  ledcWriteTone(channel, melody1);
  delay(duration);
  ledcWriteTone(channel, 0);
  delay(duration);
  ledcWriteTone(channel, melody1);
  delay(duration);
  ledcWriteTone(channel, 0);

  sendLineNotify(fb->buf, fb->len);      //LINE

  delay(1000);
  ledcWriteTone(channel, melody1);
  delay(1000);
  ledcWriteTone(channel, 0);

  esp_camera_fb_return(fb);

  delay(100);
}




void setup_camera_VGA_JPEG() {             //M5Cameraの設定
  camera_config_t config;

  //  M5Camera GPIO
  config.ledc_channel = LEDC_CHANNEL_0; /*!< LEDC channel generating XCLK  */
  config.ledc_timer = LEDC_TIMER_0;     /*!< LEDC timer generating XCLK  */
  config.pin_d0 = Y2_GPIO_NUM;          /*!< GPIO pin D0 line */
  config.pin_d1 = Y3_GPIO_NUM;          /*!< GPIO pin D1 line */
  config.pin_d2 = Y4_GPIO_NUM;          /*!< GPIO pin D2 line */
  config.pin_d3 = Y5_GPIO_NUM;          /*!< GPIO pin D3 line */
  config.pin_d4 = Y6_GPIO_NUM;          /*!< GPIO pin D4 line */
  config.pin_d5 = Y7_GPIO_NUM;          /*!< GPIO pin D5 line */
  config.pin_d6 = Y8_GPIO_NUM;          /*!< GPIO pin D6 line */
  config.pin_d7 = Y9_GPIO_NUM;          /*!< GPIO pin D7 line */
  config.pin_xclk = XCLK_GPIO_NUM;      /*!< GPIO pin XCLK line */
  config.pin_pclk = PCLK_GPIO_NUM;      /*!< GPIO pin PCLK line */
  config.pin_vsync = VSYNC_GPIO_NUM;    /*!< GPIO pin VSYNC line */
  config.pin_href = HREF_GPIO_NUM;      /*!< GPIO pin HREF line */
  config.pin_sscb_sda = SIOD_GPIO_NUM;  /*!< GPIO pin SDA line */
  config.pin_sscb_scl = SIOC_GPIO_NUM;  /*!< GPIO pin SCL line */
  config.pin_pwdn = PWDN_GPIO_NUM;      /*!< GPIO pin power down line */
  config.pin_reset = RESET_GPIO_NUM;    /*!< GPIO pin reset line */
  config.xclk_freq_hz = 20000000;       /*!< Frequency of XCLK signal*/

  //  M5Camera Image Format
  config.pixel_format = PIXFORMAT_JPEG; /*!< Format of the pixel data*/
  config.frame_size = FRAMESIZE_VGA;    /*!<imageSize*/
  config.jpeg_quality = 6;              /*!< JPEG Quality range of 0-63 */
  config.fb_count = 1;                  /*!< Number of frame buffers to be allocated.  */

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QQVGA);
  s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
  s->set_whitebal(s, 0); //white balance
  s->set_wb_mode(s, 3); //White Balance Mode 0: sunny   1: cloudy   2: office   3: hom
  s->set_awb_gain (s, 0); //Auto White Balance Gain
  s->set_exposure_ctrl(s, 1);
  s->set_aec_value(s, 0);
  s->set_saturation(s, -1); //Saturation

}
