#include <stdio.h>
#include <string>
#include <time.h>
#include <cstdint>
#include <hardware/sync.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/sntp.h"
#include "lwip/dns.h"

#include "pico_display_2.hpp"
#include "pico_graphics.hpp"
#include "st7789.hpp"
#include "rgbled.hpp"
#include "button.hpp"

#include "project_libraries/color.h"
#include "project_libraries/myscreen.h"
#include "project_libraries/myled.h"

#include "lfs.h"



#include "project_libraries/bootsel.h"


bool __no_inline_not_in_flash_func(get_bootsel_button)();

using namespace pimoroni;

// Instantiate Screen
myScreen screen;

// And each button
Button button_a(PicoDisplay2::A);
Button button_b(PicoDisplay2::B);
Button button_x(PicoDisplay2::X);
Button button_y(PicoDisplay2::Y);

class myButton {
    // TO-DO
};

// Instantiate LED
myLED led;

void init_screen(){
    screen.set_brightness(200);
    screen.set_pen("light blue");
    screen.clear();
    screen.update();
    sleep_ms(1000);
    screen.set_pen("dark blue");
    screen.rectangle(10,10,screen.get_width()-20, screen.get_height()-20);
    screen.update(); 
    screen.writeln("SCREEN: OK","white");
}

// Initialize WiFi chipset
void init_wifi(){
    if(cyw43_arch_init()) {
        screen.writeln("ERROR INITIALIZING WIFI CHIPSET", "red");
    } else {
        screen.writeln("WIFI CHIPSET: OK", "white");
    }

    // Enable chipset operation
    cyw43_arch_enable_sta_mode();

    // Connect to WiFi network
    for(int attempt = 0; attempt < 3; attempt++){
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            std::string msg = "ERROR IN WIFI NETWORK #" + std::to_string(attempt+1);
            screen.writeln(msg, "red");
        } else {
            screen.writeln("WIFI NETWORK: OK", "white");
            break;
        }
    }
}

void init_sntp(std::string sntp_server) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // Poll mode for periodic updates
    sntp_setservername(0, sntp_server.c_str());   // Set NTP server
    sntp_init();  // Start SNTP service

    screen.writeln("SNTP SERVER FOUND: OK", "pink");
    led.set_rgb("pink");
}

bool time_synched = false;
void sntp_callback(time_t sec, suseconds_t us) {
    time_synched = true;
    led.set_rgb("orange");
    struct timeval tv = {sec, us};
    settimeofday(&tv, NULL);
}

// Get formatted current time as string
std::string get_time() {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    char buffer[9]; // "hh:mm:ss" format 9 chars (8+null)
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    return std::string(buffer);
} 


int main() {

    // Initialize Pico
    stdio_init_all();

    // Initialize screen 
    init_screen();
 
    // Mount or format LittleFS partition
    int pico_mount(bool format);
    
    init_wifi();
    init_sntp("pool.ntp.org");

    screen.writeln("SYNCRONIZING TIME","white");
    while (!time_synched) {
        sleep_ms(50); // wait for sntp to sync clock
    }
    screen.writeln("TIME SYCHRONIZED","green");
    screen.writeln(); // empty line

    screen.writeln("TIME IS: ", "orange");

    std::string time_string;
    led.set_rgb("magenta");
    time_string = get_time();
    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(button_a.raw() && !button_y.raw()) {
            led.new_blink(5,500,"blue");
        } 
        if(button_a.raw() && button_y.raw()) {
            reset_usb_boot(0, 0);
        } 

        if(get_bootsel_button()) {
            led.new_blink(5,500,"green");
        }

        led.blink_update();
        if(get_time() != time_string){
            screen.writexy(120,150,time_string, "dark blue");
            time_string = get_time();
            screen.writexy(120,150,time_string, "white");
        };
    } 

}