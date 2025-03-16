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

// And each button
Button button_a(PicoDisplay2::A);
Button button_b(PicoDisplay2::B);
Button button_x(PicoDisplay2::X);
Button button_y(PicoDisplay2::Y);

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

//Screen class
class myScreen {
    private:
        const uint16_t WIDTH = 320;
        const uint16_t HEIGHT = 240;
        uint8_t backlight = 255; // default screen backlight
        std::tuple<uint8_t, uint8_t, uint8_t> pen_color;
        ST7789 st7789; // screen driver
        std::vector<uint8_t> frame_buffer;
        pimoroni::PicoGraphics_PenRGB332 screen; // order is important, this goes after frame_buffer
        int textx, texty, twidth;

        public:
        // Constructor
        myScreen(): frame_buffer(WIDTH*HEIGHT),
                    screen(WIDTH, HEIGHT, frame_buffer.data()), 
                    st7789(WIDTH, HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT))
            {
                this->set_brightness(this->backlight);
                this->set_pen("white");
                this->clear();
                this->update();
                sleep_ms(2000);
            }
        
        // Methods to return screen dimensions
        uint16_t get_width(){
            return WIDTH;
        }
        uint16_t get_height(){
            return HEIGHT;
        }

        // Methods for screen brightness
        void set_brightness(uint8_t backlight){
            this->backlight = backlight;
            st7789.set_backlight(this->backlight);
        }
        uint8_t get_brightness(){
            return this->backlight;
        }

        // Methods for pen color
        void set_pen(const std::string& color_name) {
            auto [r, g, b] = Color::get_rgb(color_name);
            this->set_pen(r,g,b);
        };
        void set_pen(std::tuple<uint8_t, uint8_t, uint8_t> rgb_tuple){
            auto [r, g, b] = rgb_tuple;
            this->set_pen(r,g,b);
        };
        void set_pen(uint8_t r, uint8_t g, uint8_t b){
            this->pen_color = {r, g, b};
            screen.set_pen(r,g,b);
        };

        std::tuple<uint8_t, uint8_t, uint8_t> get_pen(){
            return this->pen_color;
        };

        void pixel(const Point &p){
            screen.pixel(p);
        }
        void clear(){
            this->textx=10;
            this->texty=10;
            this->twidth=WIDTH-20-10; 
            screen.clear();
        }
        void update(){
            st7789.update(&screen);
        }
        void rectangle(int x, int y, int width, int height) {
            screen.rectangle(Rect(x,y,width,height));
        }
        void writeln(const std::string_view &t)
        {
            this->write(t);
            this->texty+=16;
        }
        void write(const std::string_view &t)
        {
            screen.text(t, pimoroni::Point(this->textx+5, this->texty+2), this->twidth);
            this->update();
        }

};
// Instantiate Screen
myScreen screen;


class myButton {
    // TO-DO
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
// Instantiate LED
myLED led;

void init_screen(){
    screen.set_brightness(200);
    screen.set_pen("green");
    screen.clear();
    screen.update();
    sleep_ms(1000);
    screen.set_pen("black");
    screen.rectangle(10,10,screen.get_width()-20, screen.get_height()-20);
    screen.update(); 
    screen.set_pen("white");
    screen.writeln("SCREEN: OK");
}

// Initialize WiFi chipset
void init_wifi(){
    if(cyw43_arch_init()) {
        screen.set_pen("red");
        screen.writeln("ERROR INITIALIZING WIFI CHIPSET");
    } else {
        screen.set_pen("white");
        screen.writeln("WIFI CHIPSET: OK");
    }

    // Enable chipset operation
    cyw43_arch_enable_sta_mode();

    // Connect to WiFi network
    for(int attempt = 0; attempt < 3; attempt++){
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            std::string msg = "ERROR IN WIFI NETWORK #" + std::to_string(attempt+1);
            screen.set_pen("red");
            screen.writeln(msg);
        } else {
            screen.set_pen("white");
            screen.writeln("WIFI NETWORK: OK");
            break;
        }
    }
}

void init_sntp(std::string sntp_server) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // Poll mode for periodic updates
    sntp_setservername(0, sntp_server.c_str());   // Set NTP server
    sntp_init();  // Start SNTP service

    screen.set_pen("white");
    screen.writeln("SNTP TIME SERVER: OK");
    led.set_rgb("pink");
}

void sntp_callback(time_t sec, suseconds_t us) {
    screen.set_pen("white");
    screen.writeln("TIME SYNCHRONIZED: OK");
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

    screen.set_pen("white");
    screen.writeln("SYNCRONIZING TIME...");
    time_t now = 0;
    while (now < 1000000000){
        time(&now);
        sleep_ms(500);
    }

    screen.set_pen("white");
    screen.writeln("TIME IS: ");

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
        screen.set_pen("black");
        screen.write(time_string);
        time_string = get_time();
        screen.set_pen("white");
        screen.write(time_string);
        sleep_ms(50);
    } 
}