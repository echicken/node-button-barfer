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

// LED pins
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
  "0,000,000",
  "00",
  "0000",
  "0000",
  "0000",
  "0000",
  "0000",
  "0000",
  "0000",
  "0000"
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

void setFlairColour() {
  int red;
  int green;
  int blue;
  int c = atoi(values[1]);
  if(c >= 52) {
    red = 0x82;
    green = 0x00;
    blue = 0x80;
  } else if(c >= 42) {
    red = 0x00;
    green = 0x83;
    blue = 0xC7;
  } else if(c >= 32) {
    red = 0x02;
    green = 0xBE;
    blue = 0x01;
  } else if(c >= 22) {
    red = 0xE5;
    green = 0xD9;
    blue = 0x00;
  } else if(c >= 12) {
    red = 0xE5;
    green = 0x95;
    blue = 0x00;
  } else if(c >= 1) {
    red = 0xE5;
    green = 0x00;
    blue = 0x00;
  }
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void setup() {

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  u8g.setColorIndex(1); // Set up a h'wite mono display, IIRC
  u8g.setFontPosTop(); // Not sure when or how often to call this

  // Configure our internets
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(1000); // Ugh.
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
    setFlairColour();

  }

}

