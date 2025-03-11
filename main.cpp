#include <stdio.h>
#include <string>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/sntp.h"
#include "lwip/dns.h"

#include "lfs.h"

#include "pico_display_2.hpp"
#include "pico_graphics.hpp"
#include "st7789.hpp"
#include "rgbled.hpp"
#include "button.hpp"

using namespace pimoroni;

class MyPicoGraphics : public PicoGraphics_PenRGB332 {
    public:
        int textx, texty, twidth;
        MyPicoGraphics(uint16_t width, uint16_t height, void *frame_buffer);
        void clear();
        void writeln(const std::string_view &t);
};

MyPicoGraphics::MyPicoGraphics(uint16_t width, uint16_t height, void *frame_buffer)
    : PicoGraphics_PenRGB332(width, height, frame_buffer)    
{
    textx=10;
    texty=10;
    twidth=width-20-10;
}

void MyPicoGraphics::clear()
{
    PicoGraphics_PenRGB332::clear();
    textx=10;
    texty=10;
}

void MyPicoGraphics::writeln(const std::string_view &t)
{
    PicoGraphics_PenRGB332::text(t, pimoroni::Point(textx, texty), twidth);
    texty+=16;
}

// Display driver
ST7789 st7789(PicoDisplay2::WIDTH, PicoDisplay2::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));

// Graphics library - in RGB332 mode you get 256 colours and optional dithering for 75K RAM.
MyPicoGraphics picoscreen(st7789.width, st7789.height, nullptr);

// RGB LED
//RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);

// And each button
Button button_a(PicoDisplay2::A);
Button button_b(PicoDisplay2::B);
Button button_x(PicoDisplay2::X);
Button button_y(PicoDisplay2::Y);

PicoDisplay2 display;

// Define a datatype for RGB values
struct LEDColor {
    int r, g, b;
    LEDColor(int red, int green, int blue) : r(red), g(green), b(blue) {}
};

// Define friendly names for screen dimensions
const int WIDTH = st7789.width;
const int HEIGHT = st7789.height;

class Color {
    private:
        static inline const std::map<std::string, std::tuple<uint8_t, uint8_t, uint8_t>> color_map = {
            {"black",   {0,0,0}},
            {"white",   {255,255,255}},
            {"red",     {255,0,0}},
            {"green",   {0,255,0}},
            {"blue",    {0,0,255}},
            {"yellow",  {255,255,0}},
            {"cyan",    {0,255,255}},
            {"magenta", {255,0,255}},
            {"orange",  {255,165,0}},
            {"purple",  {128,0,128}},
            {"pink",    {255,192,203}}
            // Add more colors here if you need to
        };

    public:
        static std::tuple<uint8_t, uint8_t, uint8_t> get_rgb(const std::string& color_name) {
            auto match = color_map.find(color_name);
            if (match != color_map.end()) {
                return match->second;  
            } else {
                // If the color is not defined, we return white
                return {255, 255, 255};
            } 
        }
};

class myLED {
    private: 
        RGBLED led;
    
    public:
        // Constructor
        myLED() : led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B){} 
        // Methods
        void set_rgb(const std::string& color_name) {
            auto [r, g, b] = Color::get_rgb(color_name);
            led.set_rgb(r,g,b);
        }

        void set_rgb(uint8_t r, uint8_t g, uint8_t b){
            led.set_rgb(r, g, b);
        }
        void set_brightness(uint8_t brightness){
            led.set_brightness(brightness);
        };
};

myLED led;

int current_led_brightness = 155;
std::string current_color = "black";

void led_blink(std::string blink_color, int blinks){
    led.set_rgb(blink_color);
    for (int i = 1; i <= blinks; i++){
        led.set_brightness(255);
        sleep_ms(250);
        led.set_brightness(0);
        sleep_ms(250);
    }
    led.set_rgb(blink_color);
    led.set_brightness(current_led_brightness);
}

// This will help us know where to write text at bootup
void init_screen(){
    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(200);

    // set the Led to OFF
    led.set_rgb("black");

    // set the colour of the pen
    // parameters are red, green, blue all between 0 and 255
    picoscreen.set_pen(0, 255, 0);
    // fill the screen with the current pen colour
    picoscreen.clear();

    // draw a box to put some text in
    picoscreen.set_pen(0, 0, 0);
    Rect text_rect(10, 10, WIDTH-20, HEIGHT-20);
    picoscreen.rectangle(text_rect);
    // write some text inside the box with 10 pixels of margin
    // automatically word wrapping
    text_rect.deflate(10);
    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("SCREEN: OK");
    st7789.update(&picoscreen);
}

// Initialize WiFi chipset
void init_wifi(){
    if(cyw43_arch_init()) {
        picoscreen.set_pen(255, 0, 0);
        picoscreen.writeln("ERROR INITIALIZING WIFI CHIPSET");
    } else {
        picoscreen.set_pen(255, 255, 255);
        picoscreen.writeln("WIFI CHIPSET: OK");
    }
    st7789.update(&picoscreen);

    // Enable chipset operation
    cyw43_arch_enable_sta_mode();

    // Connect to WiFi network
    for(int attempt = 0; attempt < 3; attempt++){
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            std::string msg = "ERROR IN WIFI NETWORK #" + std::to_string(attempt+1);
            picoscreen.set_pen(255, 0, 0);
            picoscreen.writeln(msg);
            led_blink("red", 5);
            st7789.update(&picoscreen);
        } else {
            picoscreen.set_pen(255, 255, 255);
            picoscreen.writeln("WIFI NETWORK: OK");
            led_blink("green", 5);
            break;
        }
    }
    st7789.update(&picoscreen);
}

void init_sntp(std::string sntp_server) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // Poll mode for periodic updates
    sntp_setservername(0, sntp_server.c_str());   // Set NTP server
    sntp_init();  // Start SNTP service

    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("SNTP TIME SERVER: OK");
    st7789.update(&picoscreen);
}

void sntp_callback(time_t sec, suseconds_t us) {
    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("TIME SYNCHRONIZED");
    st7789.update(&picoscreen);
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

    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("SYNCRONIZING TIME...");
    st7789.update(&picoscreen);
    time_t now = 0;
    while (now < 1000000000){
        time(&now);
        sleep_ms(500);
    }

    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("TIME IS: ");
    st7789.update(&picoscreen);

    std::string time_string; 
    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(button_a.raw()) {
            // make the led glow green
            // parameters are red, green, blue all between 0 and 255
            // these are also gamma corrected
            led_blink("blue",10);
        }
        picoscreen.set_pen(0, 0, 0);
        picoscreen.text(time_string, Point(100, 200), 100);
        time_string = get_time();
        picoscreen.set_pen(255, 255, 255);
        picoscreen.text(time_string, Point(100, 200), 100);
        st7789.update(&picoscreen);
        sleep_ms(500);
    }
}