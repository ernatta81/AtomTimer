#include <M5Atom.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

void deepStart();

void setup(){
  M5.begin(false, false, false);
  pinMode(26, OUTPUT);         
  //pinMode(32, OUTPUT);
  gpio_hold_dis(GPIO_NUM_26);
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER) {
    digitalWrite(26, LOW);
    } else {
        digitalWrite(26, LOW);
      }
    
}

void loop(void){
  deepStart();
}

 void deepStart(){
   M5.update();
   if (M5.Btn.wasPressed()){
    unsigned long start = millis();
    digitalWrite(26, HIGH);
    delay(100);
    gpio_hold_en(GPIO_NUM_26);
// wakeup 10 sec
    const uint64_t sleepTimeUs = 900ull * 1000ull * 1000ull; // 10 seconds
    esp_sleep_enable_timer_wakeup(sleepTimeUs);
    esp_deep_sleep_start();
  }
  delay(20);
}