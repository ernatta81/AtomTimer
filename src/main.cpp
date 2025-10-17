#include <M5Atom.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

/*
  Behavior:
  - On power-up enter deep sleep waiting only for button press (EXT0 wake on GPIO39, active LOW).
  - When button pressed: set RELAY_GPIO HIGH, enable gpio_hold so it stays HIGH during deep sleep,
    set a timer wakeup for 10800 seconds (3 hours) and enter deep sleep.
  - When woken by the timer: disable gpio_hold, set RELAY_GPIO LOW, and return to waiting deep sleep for the button.
  - No Serial output (as requested).
*/

constexpr gpio_num_t RELAY_GPIO = GPIO_NUM_26;
constexpr gpio_num_t BUTTON_RTC_GPIO = GPIO_NUM_39; // M5 button (RTC GPIO)
constexpr uint64_t HOLD_WAKE_SECONDS = 10800ull; // 3 hours

void enterWaitForButton();
void onButtonPressedThenSleep();

void setup() {
  // Minimal M5 init for button
  M5.begin(false, false, false);

  // Configure relay pin and ensure no hold active
  pinMode((int)RELAY_GPIO, OUTPUT);
  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);

  // Check wakeup reason
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER) {
    // Woke from timer: disable hold, turn relay off and go back to waiting for button
    gpio_hold_dis(RELAY_GPIO);
    digitalWrite((int)RELAY_GPIO, LOW);
    delay(50);
    enterWaitForButton();

  } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
    // Woke from button press: handle action
    onButtonPressedThenSleep();

  } else {
    // Fresh boot or other wake: go straight to wait-for-button
    delay(20);
    enterWaitForButton();
  }
}

void loop() {
  // Nothing to do: all handled in setup and deep sleep cycles
}

void enterWaitForButton() {
  // Ensure relay is off and hold disabled
  gpio_hold_dis(RELAY_GPIO);
  digitalWrite((int)RELAY_GPIO, LOW);

  // Configure EXT0 wake on the RTC pin for button (wake on LOW)
  esp_sleep_enable_ext0_wakeup(BUTTON_RTC_GPIO, 0);

  // Enter deep sleep waiting only for the button
  esp_deep_sleep_start();
}

void onButtonPressedThenSleep() {
  // Turn relay ON and enable gpio hold so it remains during deep sleep
  digitalWrite((int)RELAY_GPIO, HIGH);
  delay(100);
  gpio_hold_en(RELAY_GPIO);

  // Configure timer wake for HOLD_WAKE_SECONDS and enter deep sleep
  const uint64_t sleepTimeUs = HOLD_WAKE_SECONDS * 1000000ull;
  esp_sleep_enable_timer_wakeup(sleepTimeUs);
  esp_deep_sleep_start();
}
