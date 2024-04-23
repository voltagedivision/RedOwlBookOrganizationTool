// Configuration - SET THESE!
//#define OUTPUT_PIN  (0x40)  // Set to whatever UCB0SIMO is on your processor (Px.7 here)
#define NUM_LEDS    (49)    // NUMBER OF LEDS IN YOUR STRIP

// Useful typedefs
typedef unsigned char u_char;   // 8 bit
typedef unsigned int u_int;     // 16 bit

// Transmit codes
#define HIGH_CODE   (0x0F)      // b11110000
#define LOW_CODE    (0x0C)      // b11000000

// Configure processor to output to data strip
void initStrip(void);
void initStriptwo(void);

// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(void);
void showStriptwo(void);

// Set the color of a certain LED
void setLEDColor(u_int p, u_char r, u_char g, u_char b);
void setLEDColortwo(u_int p, u_char r, u_char g, u_char b);
// Clear the color of all LEDs (make them black/off)
void clearStrip(void);
void clearStriptwo(void);
// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b);
void fillStriptwo(u_char r, u_char g, u_char b);

