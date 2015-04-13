# node-button-barfer
A node.js server that barfs /r/thebutton data to clients. (For an Arduino project of mine.)

### Hardware

I'm using [this 2.42" OLED screen](http://www.ebay.ca/itm/131332928598) connected to an old [Arduino Pro 5V 16MHz with ATmega328](http://arduino.cc/en/Guide/ArduinoPro) with an [Ethernet Shield](http://arduino.cc/en/Main/ArduinoEthernetShield) attached.  The SDA and SCL pins of the display connect to the A4 and A5 pins on the Arduino board, respectively.  VCC and GND of the display connect to the obvious places.

A common-cathode tri-colour LED has its red, green, and blue anodes connected to pins 6, 5, and 3 of the Arduino.  These pins are PWM-capable (as are pins 9, 10 and 11, however the Ethernet Shield uses pins 10 - 13 for SPI, so 10 and 11 are not available to us here.)

### Software

I set out to do everything natively on the Arduino, but after a couple of frustrating hours of messing with various websocket and JSON parsing libraries, I decided to go another route (I wanted to have something working before the timer runs out.) 

The Arduino connects to a custom server and listens for data.  The server barfs out ten lines of data at a time, which are:

- Participants (how many people have pressed the button)
- The current timer value (seconds remaining)
- Non-pressers currently active in the subreddit
- Purple pressers currently active in the subreddit
- Blue pressers currently active in the subreddit
- Green pressers currently active in the subreddit
- Yellow pressers currently active in the subreddit
- Orange pressers currently active in the subreddit
- Red pressers currently active in the subreddit
- Can't-pressers currently active in the subreddit

The Arduino reads these ten lines, then updates its display.  This happens roughly once per second, so updating the timer takes care of itself.

The *button.ino* file contains the Arduino sketch, and depends on [u8glib](http://code.google.com/p/u8glib/).  It would be fairly simple to replace u8glib with a display library of your choosing, and/or to adapt *button.ino* to whatever display hardware you have available.  In the unlikely event that you have the exact same OLED display as me, it should just work as it is.

The server component was written for node.js.  It maintains a connection to the button's websocket service and receives updates from there.  When an update is received, the server parses the data and then spits out the lines described above to all connected clients.

The 'active user' data is not included in the websocket feed, so at a (configurable) interval the server reloads the [/r/thebutton](http://www.reddit.com/r/thebutton/) front page and then parses that info out of the sidebar.  Variables are updated and included in the next broadcast to clients.  This HTML-scraping method is a bit slow, so it happens in a background process thanks to [backgrounder](http://jolira.github.io/backgrounder/).  The HTML parsing is done via [jsdom](https://github.com/tmpvar/jsdom/tree/3.x) and [jQuery](https://jquery.com/).

### Server Installation

I've only tested this on Ubuntu 14.04, but I expect it will work on various other linuces, unices, and OS X.  If you want to use it on Windows, a few small changes may be necessary (let me know and I can point them out.  I don't expect *rconsole* to work on Windows.)

Assuming you've got [node.js](https://nodejs.org/), [npm](https://www.npmjs.com/), and [git](http://git-scm.com/downloads) installed, the following should do the trick:

```sh
git clone https://github.com/echicken/node-button-barfer.git
cd node-button-barfer
npm install
```

Running the server is just a matter of:
```sh
node index.js
```

### Client (Arduino) Installation

As previously mentioned, *button.ino* is the Arduino sketch.  You'll need to make a few changes to this file to suit your configuration.  There are some configuration variables on lines 11-18 that you will need to modify to set your IP address and the address of the server to connect to.

Line 6 of *button.ino* is important, and tells [u8glib](http://code.google.com/p/u8glib/) what type of display you're using. [Here's a list](https://code.google.com/p/u8glib/wiki/device) of displays.  You'll likely want to replace *U8GLIB_SSD1306_128X64* on line 6 with an entry from the *C++ Constructor* column from the list as applies to your hardware.  Additionally the *U8G_I2C_OPT_NO_ACK* argument to *u8g()* probably isn't needed for most displays.

The calls to *drawStr()* in the *cycleDisplay()* function specify *x* and *y* coordinates for where each item should be drawn.  Mine is a 128x64 display.  You may need to adjust the locations of stuff and things in order to fit them on your screen, and possibly drop certain items altogether.