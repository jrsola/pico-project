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
RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);

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

// Define friendly names for colors
const LEDColor RED(255,0,0);
const LEDColor GREEN(0,255,0);
const LEDColor BLUE(0,0,255);
const LEDColor BLACK(0,0,0);
const LEDColor WHITE(255,255,255);

int current_led_brightness = 155;
LEDColor current_led_color = BLACK;

void led_blink(LEDColor blink_color, int blinks){
    led.set_rgb(blink_color.r,blink_color.g,blink_color.b);
    for (int i = 1; i <= blinks; i++){
        led.set_brightness(255);
        sleep_ms(250);
        led.set_brightness(0);
        sleep_ms(250);
    }
    led.set_rgb(current_led_color.r,current_led_color.g,current_led_color.b);
    led.set_brightness(current_led_brightness);
}

// This will help us know where to write text at bootup
void init_screen(){
    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(200);
    led.set_rgb(0, 0, 0);
    // set the Led to OFF
    LEDColor current_led_color(BLACK);
    led.set_rgb(current_led_color.r,current_led_color.g,current_led_color.b);

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

void init_wifi(){

    // Initialize WiFi chipset
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
            std::string msg = "ERROR CONNECTING TO WIFI\n ATTEMPT: " + std::to_string(attempt+1);
            picoscreen.set_pen(255, 0, 0);
            picoscreen.writeln(msg);
            picoscreen.writeln(" "); // texty+=32;
            led_blink(RED, 5);
            st7789.update(&picoscreen);
        } else {
            picoscreen.set_pen(255, 255, 255);
            picoscreen.writeln("WIFI NETWORK: OK");
            led_blink(GREEN, 5);
            break;
        }
    }
    st7789.update(&picoscreen);
}

void init_sntp() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // Poll mode for periodic updates
    sntp_setservername(0, "time.cloudflare.com");   // Set NTP server
    sntp_init();  // Start SNTP service

    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("SNTP TIME SERVER: OK");
    st7789.update(&picoscreen);
}

void sntp_callback(uint32_t sec) {

    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("I've been called");
    picoscreen.writeln(std::to_string(sec));
    st7789.update(&picoscreen);
}
/*     // Set up the timeval structure to set the system time
    struct timeval tv;
    tv.tv_sec = timestamp;  // Set seconds from SNTP timestamp
    tv.tv_usec = 0;         // Microseconds set to 0

    // Set the system time
    if (settimeofday(&tv, NULL) == 0) {
        // Successfully set the time, retrieve the updated time
        struct tm *timeinfo = localtime(&tv.tv_sec); // Use tv_sec for local time

        // Prepare the time string in "hh:mm:ss" format
        char time_string[9]; // "hh:mm:ss" format (8 characters + null terminator)
        snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d",
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

        // Display the time on the screen
        picoscreen.set_pen(255, 255, 255);
        picoscreen.text(time_string, Point(textx, texty), twidth);
    } else {
        // Display error message if setting time fails
        picoscreen.set_pen(255, 0, 0); // Set pen color for error message
        picoscreen.text("Error setting NTP time", Point(textx, texty), twidth);
    } */


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
    init_sntp();

    sleep_ms(10000);
    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("TIME IS: ");
    picoscreen.writeln(get_time());
    st7789.update(&picoscreen);

    std::string time_string; 
    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(button_a.raw()) {
            // make the led glow green
            // parameters are red, green, blue all between 0 and 255
            // these are also gamma corrected
            led_blink(BLUE,10);
        }
         picoscreen.set_pen(0, 0, 0);
/*         picoscreen.text(time_string, Point(0, 0), 100);
        time_string = get_time();
        picoscreen.set_pen(255, 255, 255);
        picoscreen.text(time_string, Point(textx, texty), twidth);
        st7789.update(&picoscreen);
        sleep_ms(500); */
    }
}