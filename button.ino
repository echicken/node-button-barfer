#include "U8glib.h" // http://code.google.com/p/u8glib/
#include <SPI.h>
#include <Ethernet.h>

// This makes ebay item #131332928598 work with u8glib
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

// Your MAC address
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Your IP configuration
IPAddress ip(192,168,1,11);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Server info
IPAddress server(192,168,1,10);
int port = 8000;

/*  The anodes of our common-cathode tri-colour LED must be
    connected to PWM-capable pins.  Meanwhile, the Ethernet
    Shield uses pins 10-13 for SPI, meaning we can't use
    pins 10 & 11 for PWM on the Arduino Pro.  So we'll use
    pins 3, 5, and 6 for our LED.  Adjust these as needed
    for your Arduino board. */
int redPin = 6;
int greenPin = 5;
int bluePin = 3;

// Labels which map to values (below)
char names[10][13] = {
  "Presses:",
  "/r/thebutton",
  "Greys:",
  "Purps:",
  "Blues:", 
  "Greens:",
  "Yellows:",
  "Oranges:",
  "Reds:",
  "Noobs:"
};

// This will be populated with data from the server
char values[10][10] = {
  "0,000,000", // Presses
  "00", // Timer
  "0000", // Greys
  "0000", // Purps
  "0000", // Blues
  "0000", // Greens
  "0000", // Yellows
  "0000", // Oranges
  "0000", // Reds
  "0000" // Noobs
};

// Our colours, as { Red, Green, Blue } values each from 0 to 255
int colours[6][3] = {
  { 80, 0, 80 }, // Purple
  { 0, 0, 80 }, // Blue
  { 0, 80, 0 }, // Green
  { 80, 60, 0 }, // Yellow
  { 80, 10, 0 }, // Orange
  { 80, 0, 0 } // Red
};

EthernetClient ethernetClient;

void cycleDisplay() {

  // The u8glib wiki has info on this 'picture loop' concept
  u8g.firstPage();
  do {

    // Draw 'participants' (pressers) label and count
    u8g.setFont(u8g_font_04b_03);
    u8g.drawStr(85, 55, names[0]);
    u8g.drawStr(85, 63, values[0]);

    // Draw the timer label and value
    u8g.drawStr(70, 5, names[1]);
    u8g.setFont(u8g_font_helvB24);
    u8g.drawStr(85, 40, values[1]);

    // Display the 'active users' data
    u8g.setFont(u8g_font_04b_03);
    u8g.drawStr(2, 5, "Active users:");
    for(int i = 2; i < 10; i++) {
      u8g.drawStr(2, (i*7), names[i]); 
      u8g.drawStr(40, (i*7), values[i]);
    }

  } 
  while(u8g.nextPage());

}

void setColour(int* colour) {
  analogWrite(redPin, colour[0]);
  analogWrite(greenPin, colour[1]);
  analogWrite(bluePin, colour[2]);
}

void setFlairColour() {

  int c = atoi(values[1]);

  if(c >= 52) {
    setColour(colours[0]);
  } else if(c >= 42) {
    setColour(colours[1]);
  } else if(c >= 32) {
    setColour(colours[2]);
  } else if(c >= 22) {
    setColour(colours[3]);
  } else if(c >= 12) {
    setColour(colours[4]);
  } else if(c >= 1) {
    setColour(colours[5]);
  }

}

void setup() {

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  u8g.setColorIndex(1); // Set up a h'wite mono display, IIRC
  u8g.setFontPosTop(); // Not sure when or how often to call this

  // Configure our internets
  Ethernet.begin(mac, ip, gateway, subnet);
  
  /*  Most Ethernet Shield examples have a delay here.
      We'll cycle through our flair colours to make it
      more interesting.  */
  for(int i = 0; i < 6; i++) {
    setColour(colours[i]);
    delay(250);
  }
  
  // Now we'll connect to the node-button-barfer server
  ethernetClient.connect(server, port);

}

void loop() {

  /*  We will naively assume that if any data is available from
   the server, that a complete set is waiting to be read.
   This is not great, but so far it hasn't borked on me.  */
  if(ethernetClient.available()) {

    /*  I could probably do all of these in one loop, but for
     now I'll grab differently-sized values separately. */

    // Read the participants count
    for(int n = 0; n < 10; n++) {
      char c = ethernetClient.read();
      // Stop at a newline
      if(c == '\n') {
        // Overwrite any remaining chars
        while(n < 10) {
          values[0][n] = 32;
          n++;
        }
        break;
      }
      // If we're still here, append the char to this value
      values[0][n] = c;
    }

    // Read the timer value
    for(int n = 0; n < 3; n++) {
      char c = ethernetClient.read();
      if(c == '\n') {
        while(n < 3) {
          values[1][n] = 32;
          n++;
        }
        break;
      }
      values[1][n] = c;
    }

    // Read each line of 'active user' data
    for(int i = 2; i < 10; i++) {
      for(int n = 0; n < 5; n++) {
        char c = ethernetClient.read();
        if(c == '\n') {
          while(n < 5) {
            values[i][n] = 32;
            n++;
          }
          break;
        }
        values[i][n] = c;
      }
    }

    // Redraw the screen   
    cycleDisplay();

    // Update the tri-colour LED
    setFlairColour();

  }

}


