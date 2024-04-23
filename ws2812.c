#include <msp430.h>
#include "ws2812.h"

// WS2812 takes GRB format
typedef struct
{
    u_char green;
    u_char red;
    u_char blue;
} LED;

static LED leds[NUM_LEDS] = { { 0, 0, 0 } };
static LED ledstwo[NUM_LEDS] = { { 0, 0, 0 } };

// Initializes everything needed to use this library. This clears the strip.
void initStrip()
{

    // Configure UCB0SIMO/P1.6 pin to SIMO
    /* P1DIR = x , P1SEL1.x = 0 , P1SEL0.x = 1 , LCDSz = 0 */
    P1SEL1 &= ~BIT6;
    P1SEL0 |= BIT6;

    // Put eUSCI in reset state and set all fields in the register to 0
    UCB0CTLW0 = UCSWRST;

    // Fields that need to be nonzero are changed below
    // Set clock phase to "capture on 1st edge, change on following edge = 1"
    UCB0CTLW0 |= UCCKPH;

    // Set data order to "transmit MSB first"
    UCB0CTLW0 |= UCMSB;

    // Set MCU to "SPI master"
    UCB0CTLW0 |= UCMST;

    // Set module to synchronous mode
    UCB0CTLW0 |= UCSYNC;

    // Set clock to SMCLK
    UCB0CTLW0 |= UCSSEL_2;

    //  Set clock polarity to "inactive low=0"
    UCB0CTLW0 &= ~UCCKPL;

    // Set data size to 8-bit
    UCB0CTLW0 &= ~UC7BIT;

    //  Set SPI to "3-pin SPI" (we won't use eUSCI's chip select)
    UCB0CTLW0 |= UCMODE_0;

    // Configure the clock divider
    UCB0BRW = 3;
    // UCB0BR0 = 3;            // 16 MHz / 3 = .1875 us per bit
    //  UCB0BR1 = 0;

    // Exit the reset state at the end of the configuration
    UCB0CTLW0 &= ~UCSWRST;

    clearStrip();           // clear the strip
    showStrip();
}

// Sets the color of a certain LED (0 indexed)
void setLEDColor(u_int p, u_char r, u_char g, u_char b)
{
    leds[p].green = g;
    leds[p].red = r;
    leds[p].blue = b;
}
void setLEDColortwo(u_int p, u_char r, u_char g, u_char b)
{
    ledstwo[p].green = g;
    ledstwo[p].red = r;
    ledstwo[p].blue = b;
}
// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip()
{
    __bic_SR_register(GIE);  // disable interrupts

    // send RGB color for every LED
    unsigned int i, j;
    for (i = 0; i < NUM_LEDS; i++)
    {
        u_char *rgb = (u_char*) &leds[i]; // get GRB color for this LED

        // send green, then red, then blue
        for (j = 0; j < 3; j++)
        {
            u_char mask = 0x80;    // b1000000

            // check each of the 8 bits
            while (mask != 0)
            {
                // Wait as long as the module is busy
                //while ((UCB0STATW & UCBUSY)); // DOUBLE CHECK
                while (!(UCB0IFG & UCTXIFG))
                    ; // DOUBLE CHECK
                if (rgb[j] & mask)
                {        // most significant bit first
                    UCB0TXBUF = HIGH_CODE;  // send 1
                }
                else
                {
                    UCB0TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }
        }
    }

    // send RES code for at least 50 us (800 cycles at 16 MHz)
    _delay_cycles(800);

    __bis_SR_register(GIE);    // enable interrupts
}

// Clear the color of all LEDs (make them black/off)
void clearStrip()
{
    fillStrip(0x00, 0x00, 0x00);  // black
}

void clearStriptwo()
{
    fillStriptwo(0x00, 0x00, 0x00);  // black
}
// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b)
{
    int i;
    for (i = 0; i < NUM_LEDS; i++)
    {
        setLEDColor(i, r, g, b);  // set all LEDs to specified color
    }
    showStrip();  // refresh strip

}

void fillStriptwo(u_char r, u_char g, u_char b)
{
    int i;
    for (i = 0; i < NUM_LEDS; i++)
    {
        setLEDColortwo(i, r, g, b);  // set all LEDs to specified color
    }
    showStriptwo();  // refresh strip

}

void initStriptwo()
{

    // Configure UCB1SIMO/P3.1 pin to SIMO
    /* P1DIR = x , P1SEL1.x = 0 , P1SEL0.x = 1 , LCDSz = 0 */
    P3SEL1 &= ~BIT1;  // Clear bit 1 in P3SEL1
    P3SEL0 |= BIT1;   // Set bit 1 in P3SEL0

    // Put eUSCI in reset state and set all fields in the register to 0
    UCB1CTLW0 = UCSWRST;

    // Fields that need to be nonzero are changed below
    // Set clock phase to "capture on 1st edge, change on following edge = 1"
    UCB1CTLW0 |= UCCKPH;

    // Set data order to "transmit MSB first"
    UCB1CTLW0 |= UCMSB;

    // Set MCU to "SPI master"
    UCB1CTLW0 |= UCMST;

    // Set module to synchronous mode
    UCB1CTLW0 |= UCSYNC;

    // Set clock to SMCLK
    UCB1CTLW0 |= UCSSEL_2;

    // Set clock polarity to "inactive low=0"
    UCB1CTLW0 &= ~UCCKPL;

    // Set data size to 8-bit
    UCB1CTLW0 &= ~UC7BIT;

    // Set SPI to "3-pin SPI" (we won't use eUSCI's chip select)
    UCB1CTLW0 |= UCMODE_0;

    // Configure the clock divider
    UCB1BRW = 3;

    // Exit the reset state at the end of the configuration
    UCB1CTLW0 &= ~UCSWRST;

    clearStriptwo(); // clear the strip
    showStriptwo();
}

void showStriptwo()
{
    __bic_SR_register(GIE);  // disable interrupts

    // send RGB color for every LED
    unsigned int i, j;
    for (i = 0; i < NUM_LEDS; i++)
    {
        u_char *rgb = (u_char*) &ledstwo[i]; // get GRB color for this LED

        // send green, then red, then blue
        for (j = 0; j < 3; j++)
        {
            u_char mask = 0x80;    // b1000000

            // check each of the 8 bits
            while (mask != 0)
            {
                // Wait as long as the module is busy
                //while ((UCB0STATW & UCBUSY)); // DOUBLE CHECK
                while (!(UCB1IFG & UCTXIFG))
                    ; // DOUBLE CHECK
                if (rgb[j] & mask)
                {        // most significant bit first
                    UCB1TXBUF = HIGH_CODE;  // send 1
                }
                else
                {
                    UCB1TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }
        }
    }

    // send RES code for at least 50 us (800 cycles at 16 MHz)
    _delay_cycles(800);

    __bis_SR_register(GIE);    // enable interrupts
}


