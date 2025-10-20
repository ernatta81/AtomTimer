#include <M5Atom.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

constexpr gpio_num_t RELAY_GPIO = GPIO_NUM_26;
constexpr gpio_num_t BUTTON_RTC_GPIO = GPIO_NUM_39; // M5 button (RTC GPIO)
constexpr uint64_t HOLD_WAKE_SECONDS = 10800ull; // 3 hours

void enterWaitForButton();
void onButtonPressedThenSleep();

void setup() {
  
  M5.begin(false, false, false);

  pinMode((int)RELAY_GPIO, OUTPUT);
  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);

  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER) {
  
    gpio_hold_dis(RELAY_GPIO);
    digitalWrite((int)RELAY_GPIO, LOW);
    delay(50);
    enterWaitForButton();

  } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
  
    onButtonPressedThenSleep();

  } else {
  
    delay(20);
    enterWaitForButton();
  }
}

void loop() {
  
}

void enterWaitForButton() {

  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);

  esp_sleep_enable_ext0_wakeup(BUTTON_RTC_GPIO, 0);

  esp_deep_sleep_start();
}

void onButtonPressedThenSleep() {

  digitalWrite((int)RELAY_GPIO, HIGH);
  delay(100);
  gpio_hold_en(RELAY_GPIO);

  const uint64_t sleepTimeUs = HOLD_WAKE_SECONDS * 1000000ull;
  esp_sleep_enable_timer_wakeup(sleepTimeUs);
  esp_deep_sleep_start();
}
