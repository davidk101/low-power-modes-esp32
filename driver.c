/*
 * 
 * Description: Implementing light sleep and deep sleep low power modes in the ESP32 
 * Author: David Kumar 
 * 
 */

/*In Light-sleep mode, the digital peripherals, most of the RAM, and CPUs are clock-gated and their supply voltage is reduced. 
Upon exit from Light-sleep, the digital peripherals, RAM, and CPUs resume operation and their internal states are preserved.

In Deep-sleep mode, the CPUs, most of the RAM, and all digital peripherals that are clocked are powered off.*/

const byte led_red = 16; // red LED
const byte led_green = 17; // green LED
const byte interrupt_pin = 0; // interrupt pin to detect button depression 

volatile byte state = 0;
// 'volatile' allows this variable to be to not be cached in-memory but rather be derived from the main memory directly due to the value being altered by interrupts  

RTC_DATA_ATTR byte rtc_data_attr_state = 0;          // state stored in long-term memory and can be recovered after deep sleep
RTC_DATA_ATTR bool isLightSleeping = 0;              // stored in long-term memory and can be recovered after deep sleep
RTC_DATA_ATTR bool isDeepSleeping = 0;               // stored in long-term memory and can be recovered after deep sleep

void isPressed() { 
  state = rtc_data_attr_state;
  state += 1;
}

void setup() {
  
  Serial.begin(115200); // baud rate 
  
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(interrupt_pin, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), isPressed, FALLING); // attaching interrupt 
}

void loop() {

  // Move from volative memory to sleep-friendly memory.
  rtc_data_attr_state = state;
  
  if(rtc_data_attr_state == 0){ // off state 
    
      digitalWrite(led_green, HIGH); // LED is off
      digitalWrite(led_red, HIGH);   // LED is off
      
  }
  
  else if(rtc_data_attr_state == 1){ // on state 
    
    digitalWrite(led_green, LOW); // green LED is on
    digitalWrite(led_red, HIGH);  // LED is off
  }
  
  else if(rtc_data_attr_state == 2){ // flashing state 
    
    digitalWrite(led_green, HIGH); // green LED is off

    while(state == 2){
      
      digitalWrite(led_red, LOW); // red LED is on
      delay(50);
      digitalWrite(led_red, HIGH); // red LED is off
      delay(950);
    }
  }
  
  else if (rtc_data_attr_state == 3 && !isLightSleeping){ // light sleep state

    Serial.println("Currently in light sleep mode.");
    isLightSleeping = 1;
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0); // wakes up in LOW mode from GPIO0

    delay(100);
    esp_light_sleep_start(); // begin light sleep 
    Serial.println("Currently woken up from light sleep mode.");
  }
  
  else if (rtc_data_attr_state == 4 && !isDeepSleeping){ // deep sleep state 

    Serial.println("Currently in deep sleep mode.");
    isDeepSleeping = 1;
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0); // wakes up in LOW mode from GPIO0

    delay(100);
    esp_deep_sleep_start(); // begin deep sleep 
    Serial.println("This statement will not print.");
  }
  
  else if(rtc_data_attr_state > 4){ // resetting state of interrupt button depression and thus, restoring back to off from the deep sleep state 
    
    state = 0;
    rtc_data_attr_state = 0;
    isDeepSleeping = 0;
    isLightSleeping = 1;
  }
  
  delay(100);
}
