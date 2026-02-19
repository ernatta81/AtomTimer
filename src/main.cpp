#include <M5Atom.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

constexpr gpio_num_t RELAY_GPIO = GPIO_NUM_26;
constexpr gpio_num_t BUTTON_RTC_GPIO = GPIO_NUM_39; // M5 button (RTC GPIO)
constexpr uint64_t HOLD_WAKE_SECONDS = 10800ull; // 3 hours

RTC_DATA_ATTR uint8_t rtcPaletteIndex = 0; // 0.. 255 palette colori

void enterWaitForButton();
void onButtonPressedThenSleep();
void animatedLED();
void setPixels(const uint8_t *indices, size_t count, uint32_t color);
uint32_t Wheel(uint8_t pos);
uint32_t rgbToGRB(uint32_t rgb);

void setup(){
  M5.begin(false, false, true);
  pinMode((int)RELAY_GPIO, OUTPUT);
  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER){
    animatedLED();
    gpio_hold_dis(RELAY_GPIO);
    digitalWrite((int)RELAY_GPIO, LOW);
    delay(50);
    enterWaitForButton();
  } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0){
    onButtonPressedThenSleep();
  } else{
    animatedLED();
    delay(20);
    enterWaitForButton();
  }
}

void loop(){
}

void enterWaitForButton(){
  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);
  esp_sleep_enable_ext0_wakeup(BUTTON_RTC_GPIO, 0);
  const uint64_t sleepTimeStandBy = 1ull * 1000000ull; // microsecondi
  esp_sleep_enable_timer_wakeup(sleepTimeStandBy);
  esp_deep_sleep_start();
}

void onButtonPressedThenSleep(){
  digitalWrite((int)RELAY_GPIO, HIGH);
  delay(100);
  gpio_hold_en(RELAY_GPIO);
  esp_deep_sleep_start();
}

void animatedLED(){
  static const uint8_t groupA[] = {12};
  static const uint8_t groupB[] = {6,7,8,11,13,16,17,18};
  static const uint8_t groupC[] = {0,1,2,3,4,5,9,10,14,15,19,20,21,22,23,24};

  uint8_t paletteIndex = rtcPaletteIndex; // carica valore persistente
  uint32_t colorRGB = Wheel(paletteIndex);
  uint32_t colorDevice = rgbToGRB(colorRGB);
  const uint32_t OFF = 0x000000;
  setPixels(groupA, sizeof(groupA)/sizeof(groupA[0]), colorDevice);
  delay(100);
  setPixels(groupB, sizeof(groupB)/sizeof(groupB[0]), colorDevice);
  delay(100);
  setPixels(groupC, sizeof(groupC)/sizeof(groupC[0]), colorDevice);
  delay(10);
  setPixels(groupA, sizeof(groupA)/sizeof(groupA[0]), OFF);
  delay(100);
  setPixels(groupB, sizeof(groupB)/sizeof(groupB[0]), OFF);
  delay(100);
  setPixels(groupC, sizeof(groupC)/sizeof(groupC[0]), OFF);
  // Avanza la palette e salva in RTC
  paletteIndex = (uint8_t)(paletteIndex + 1); // wrap automatico 0..255
  rtcPaletteIndex = paletteIndex;
}

void setPixels(const uint8_t *indices, size_t count, uint32_t color){
  for (size_t i = 0; i < count; ++i){
    M5.dis.drawpix(indices[i], color);
  }
}

uint32_t Wheel(uint8_t pos) { //Ruota colori
  if (pos < 85) {
    uint8_t r = pos * 3;
    uint8_t g = 255 - pos * 3;
    uint8_t b = 0;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  } else if (pos < 170) {
    pos -= 85;
    uint8_t r = 255 - pos * 3;
    uint8_t g = 0;
    uint8_t b = pos * 3;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  } else {
    pos -= 170;
    uint8_t r = 0;
    uint8_t g = pos * 3;
    uint8_t b = 255 - pos * 3;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }
}

uint32_t rgbToGRB(uint32_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}
