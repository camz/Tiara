# Tiara
Neopixel Tiara w/WiFi Control UI

![](TiaraCaptivePortal/data/TiaraBanner-sm.jpg)

This project was inspired by [Becky Stern's](https://beckystern.com/) [NeoPixel Tiara](https://learn.adafruit.com/neopixel-tiara), with some changes to the hardware.
The control board is an ESP8266 [D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) combined with a [Battery Shield](https://www.wemos.cc/en/latest/d1_mini_shield/battery.html).  Software was written using the Arduino IDE.

The "gems" in the Tiara are from AliExpress; [small](https://www.aliexpress.com/item/33026835790.html) "gem" & [large](https://www.aliexpress.com/item/32835427711.html) "gem".

The software uses the [FastLED](https://github.com/FastLED/FastLED) library and borrows most of the animations from the Demo100 example project included with the library.
I've added two of my own animations customized for the tiara, as well as some minor code to change the sequence of the leds in the animations instead of their actual physical order.  This was then combined with the Captive Portal example from the ESP8266 Arduino library.  The color picker code is using [IRO.js](https://iro.js.org) Javascript code.
