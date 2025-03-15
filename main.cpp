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


// And each button
Button button_a(PicoDisplay2::A);
Button button_b(PicoDisplay2::B);
Button button_x(PicoDisplay2::X);
Button button_y(PicoDisplay2::Y);

PicoDisplay2 display;

// Define friendly names for screen dimensions
const int WIDTH = st7789.width;
const int HEIGHT = st7789.height;

//Screen class
class myScreen {
    private:
        pimoroni::PicoGraphics_PenRGB332 screen;
    public:
        // Constructor
        myScreen(uint16_t width, uint16_t height, void *frame_buffer) :
            screen(WIDTH, HEIGHT, frame_buffer) {
        }
        
        // Methods (pel Sergi TODO)

};

class myButton {

};


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
        static std::string get_color_name(const std::tuple<uint8_t, uint8_t, uint8_t>& rgb){
            auto [r,g,b] = rgb;
            for (const auto& [name, color] : color_map) {
                auto [cr, cg, cb] = color;
                if (r == cr && g == cg && b == cb){
                    return name;
                }
            }
            return "";
        }
};

class myLED {
    private: 
        pimoroni::RGBLED led;
        bool led_state = false;  // true = on, false = off
        uint8_t led_brightness = 155;
        std::tuple<uint8_t, uint8_t, uint8_t> led_color;

        bool is_blinking = false; // status about blinker
        std::tuple<uint8_t, uint8_t, uint8_t> blink_precolor; // previous color before blink
        uint8_t n_blinks = 0; // number of blinks left
        uint16_t blink_delay_ms = 0; // delay between blinks
        uint32_t last_blink_time = 0; // last time blink happened

    public:
        // Constructor
        myLED() : led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B){
            this->led_color = {0,0,0};
            this->set_rgb(led_color);
        }; 
        
        // LED On/Off methods
        void set_state(bool state){
            if (state){
                set_brightness(this->led_brightness);
            } else {
                set_brightness(0);
            };
        };

        bool get_state(){
            if(this->led_brightness){
                return true;
            }
            return false;
        }

        // LED Brightness methods
        void set_brightness(uint8_t value){
            if (value <=255) {
                this->led_brightness = value;
                led.set_brightness(value);
            } 
        };

        uint8_t get_brightness(){
            return this->led_brightness;
        }

        // LED Color methods
        void set_rgb(const std::string& color_name) {
            auto [r, g, b] = Color::get_rgb(color_name);
            this->set_rgb(r,g,b);
        };

        void set_rgb(std::tuple<uint8_t, uint8_t, uint8_t> rgb_tuple){
            auto [r, g, b] = rgb_tuple;
            this->set_rgb(r,g,b);
        };

        void set_rgb(uint8_t r, uint8_t g, uint8_t b){
            this->led_color = {r, g, b}; 
            led.set_rgb(r,g,b);
        };

        std::tuple<uint8_t, uint8_t, uint8_t> get_rgb(){
            return this->led_color;
        };

        // LED Blinking methods
        void set_blink_on() {
            // Fade-in
            for (uint8_t i = 0; i <= this->led_brightness; i++) {
                led.set_brightness(i);
                sleep_ms(2);  // Fade effect
            }
            this->n_blinks--;
        };

        // LED off with fade out
        void set_blink_off() {
            // Fade out
            for (uint8_t i = this->led_brightness; i > 0; i--) {
                led.set_brightness(i);
                sleep_ms(2);  // Fade effect
            }
            // led.set_brightness(0);  // Turn LED off completely
            this->n_blinks--;  
        };

        // Starting a new blink
        int new_blink(uint8_t blinks, uint16_t delay_ms, const std::string& blink_color) {
            if (is_blinking) {
                return -1;  // If it's already blinking then just return 
            }
            this->blink_precolor=this->led_color;
            set_rgb(blink_color); 
            is_blinking = true;          // Blinking task started
            n_blinks = blinks * 2;       // Each blink is 2 calls (on and off)
            blink_delay_ms = delay_ms;   // Time between blinks
            last_blink_time = millis();  // Initialize timer
            return 0;                    // 0 to indicate good initialization
        };

        // Async blinking
        int blink_update() {
            if (!is_blinking) {
                return 0;  // If no blinking happening, just return
            }

            if (millis() - this->last_blink_time >= this->blink_delay_ms) {
                if (this->get_state()) {
                    this->set_blink_off();  
                } else {
                    this->set_blink_on(); 
                }
                this->last_blink_time = millis();  // Update timer
                if (this->n_blinks == 0) {
                    this->is_blinking = false;
                    set_rgb(this->blink_precolor);
                    set_brightness(this->led_brightness);
                }
            }

            return n_blinks;  // blinks left
        };
};

// Initialize the LED
myLED led;

// This will help us know where to write text at bootup
void init_screen(){
    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(200);

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
            led.set_rgb("red");
            st7789.update(&picoscreen);
        } else {
            picoscreen.set_pen(255, 255, 255);
            picoscreen.writeln("WIFI NETWORK: OK");
            led.set_rgb("blue");
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
    led.set_rgb("pink");
    st7789.update(&picoscreen);
}

void sntp_callback(time_t sec, suseconds_t us) {
    picoscreen.set_pen(255, 255, 255);
    picoscreen.writeln("TIME SYNCHRONIZED");
    led.set_rgb("orange");
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
    led.set_rgb("magenta");
    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(button_a.raw()) {
            // make the led glow green
            // parameters are red, green, blue all between 0 and 255
            // these are also gamma corrected
            //led_blink("blue",10);
            led.new_blink(5,500,"blue");
        }
        led.blink_update();
        picoscreen.set_pen(0, 0, 0);
        picoscreen.text(time_string, Point(100, 200), 100);
        time_string = get_time();
        picoscreen.set_pen(255, 255, 255);
        picoscreen.text(time_string, Point(100, 200), 100);
        st7789.update(&picoscreen);
        sleep_ms(50);
    }
}