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
#include "lwip/netif.h"
#include "lwip/ip_addr.h"

#include "pico/unique_id.h"
#include "hardware/adc.h"

#include "button.hpp"

#include "project_libraries/color.h"
#include "project_libraries/myscreen.h"
#include "project_libraries/myled.h"
#include "project_libraries/fileops.h"
#include "project_libraries/bootsel.h"

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

void init_lfs(){
    int err_code = pico_mount(false);
    while (err_code != LFS_ERR_OK){
        screen.writeln("FILESYSTEM ERROR", "red");
        screen.writeln("FORMATTING FILESYSTEM", "yellow");
        int err_code = pico_mount(true);
    }
    screen.writeln("FILESYSTEM OK", "white");
    pico_unmount();
    return;
}

std::string info_board(){
    char id_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(id_str, sizeof(id_str));
    return std::string(id_str);
}

std::string info_voltage(){
    adc_init();
    adc_select_input(3);

    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / (1 << 12) * 3.0f;
    return std::to_string(voltage);
}

void start_access_point(const std::string& board_id){
    size_t len = board_id.length();
    std::string password = "KKKKKKKK"; //board_id.substr(len - 8, 4);
    std::string suffix = board_id.substr(len - 4, 4);
    std::string ssid = "pico_" + suffix;

    screen.writeln("SSID: " + ssid, "white");
    screen.writeln("PASS: " + password, "white");

    cyw43_arch_enable_ap_mode(ssid.c_str(), password.c_str(), CYW43_AUTH_WPA2_AES_PSK);
    screen.writeln("AP CREATED", "white");
}

int main() {

    // Initialize Pico
    stdio_init_all();

    // Initialize screen 
    init_screen();
 
    // Mount or format LittleFS partition
    init_lfs();
    std::string boot_count = readfile("boot_count.cfg");
    if (boot_count == "") boot_count = "0";
    boot_count = std::to_string(std::stoi(boot_count) + 1);
    write2file("boot_count.cfg",boot_count);
    //screen.writeln("NUMBER OF BOOTUPS: " + boot_count,"white");
    std::string board_id = info_board();
    screen.writeln("BOARD ID: " + board_id,"pink");
    std::string voltage_id = info_voltage();
    //screen.writeln("VOLTAGE: " + voltage_id + "V","pink");



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
    led.set_rgb("light blue");
    time_string = get_time();

    start_access_point(board_id);

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