/* Blink Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "soc/rtc.h"
#include "sdkconfig.h"
#include <Arduino.h>

#define CALIBRATE_ONE(cali_clk) calibrate_one(cali_clk, #cali_clk)

static uint32_t calibrate_one(rtc_cal_sel_t cal_clk, const char *name)
{

    const uint32_t cal_count = 1000;
    const float factor = (1 << 19) * 1000.0f;
    uint32_t cali_val;
    printf("%s:\n", name);
    for (int i = 0; i < 5; ++i)
    {
        printf("calibrate (%d): ", i);
        cali_val = rtc_clk_cal(cal_clk, cal_count);
        printf("%.3f kHz\n", factor / (float)cali_val);
    }
    return cali_val;
}

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5           /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;


void setup()
{

    Serial.begin(115200);

    rtc_clk_32k_bootstrap(512);
    rtc_clk_32k_bootstrap(512);
    rtc_clk_32k_enable(true);
    rtc_clk_32k_enable_external();

    uint32_t cal_32k = CALIBRATE_ONE(RTC_CAL_32K_XTAL);
    rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);

    if (cal_32k == 0)
    {
        printf("32K XTAL OSC has not started up");
    }
    else
    {
        printf("done\n");
    }

    if (rtc_clk_32k_enabled())
    {
        Serial.println("OSC Enabled");
    }
    bootCount++;
    Serial.println("Bootcount: " + String(bootCount));

    delay(5000);

    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                   " Seconds");

    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

void loop()
{
    Serial.println("Hello!");
    delay(1000);
}