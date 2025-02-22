#include <stdio.h>
#include <string>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lfs.h"

#include "pico_display_2.hpp"
#include "pico_graphics.hpp"
#include "st7789.hpp"
#include "rgbled.hpp"
#include "button.hpp"

using namespace pimoroni;

// Display driver
ST7789 st7789(PicoDisplay2::WIDTH, PicoDisplay2::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));

// Graphics library - in RGB332 mode you get 256 colours and optional dithering for 75K RAM.
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

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

// Define friendly names for colors
const LEDColor RED(255,0,0);
const LEDColor GREEN(0,255,0);
const LEDColor BLUE(0,0,255);

void led_blink(RGBLED led, LEDColor current_color, LEDColor blink_color, int blinks){
    for (int i = 1; i <= blinks; i++){
        led.set_rgb(blink_color.r,blink_color.g,blink_color.b);
        sleep_ms(500);
        led.set_rgb(current_color.r,current_color.g,current_color.b);
        sleep_ms(500);
    }
}

void init_screen(){

}

void init_wifi(){

}

int main() {

    // Initialize PICO
    stdio_init_all();

    // Mount or format LittleFS partition
    int pico_mount(bool format);

    // set the Led to OFF
    LEDColor current_led_color(0,0,0);
    led.set_rgb(current_led_color.r,current_led_color.g,current_led_color.b);
    
    // Initialize the WiFi chipset
    if(cyw43_arch_init()) {
        printf("Wi-Fi init failed"); // s'hauria de canviar per un error a la pantalla
        return -1;
    }

    // Initialize WiFi chipset
    cyw43_arch_enable_sta_mode();
    
    // Connect to WiFi
    int ret = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);

    // Check if the connection was successful
    if (ret == 0) {
        led_blink(led, current_led_color, BLUE, 5);
    } else {
        led.set_rgb(255, 0, 0); // led red
    }

    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(200);
    //led.set_rgb(0, 0, 0);

    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(button_a.raw()) {
            // make the led glow green
            // parameters are red, green, blue all between 0 and 255
            // these are also gamma corrected
            led.set_rgb(255, 0, 0);
        }

        // set the colour of the pen
        // parameters are red, green, blue all between 0 and 255
        graphics.set_pen(0, 255, 0);

        // fill the screen with the current pen colour
        graphics.clear();

        // draw a box to put some text in
        graphics.set_pen(0, 0, 255);
        Rect text_rect(10, 10, 150, 150);
        graphics.rectangle(text_rect);

        // write some text inside the box with 10 pixels of margin
        // automatically word wrapping
        text_rect.deflate(10);
        graphics.set_pen(255, 255, 255);
        graphics.text("Los Amiga Rulez!", Point(text_rect.x, text_rect.y), text_rect.w);


        // Clock
        graphics.set_pen(50, 50, 50);
        Rect clock_rect(170, 10, 140, 150);
        graphics.rectangle(clock_rect);
        clock_rect.deflate(10);
        graphics.set_pen(255, 255, 255);
        graphics.text("11:00 am",Point(clock_rect.x, clock_rect.y), clock_rect.w);

        // now we've done our drawing let's update the screen
        st7789.update(&graphics);

    }
}