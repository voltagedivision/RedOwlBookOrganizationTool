#include "ws2812.h"
#include "msp430.h"
#include <msp430fr6989.h>
#include <stdio.h>

#define FLAGS UCA1IFG  // Contains the transmit & receive flags
#define RXFLAG UCRXIFG  // Receive flag
#define TXFLAG UCTXIFG  // Transmit flag
#define TXBUFFER UCA1TXBUF  // Transmit buffer
#define RXBUFFER UCA1RXBUF  // Receive buffer
#define SYSTEM_CLOCK 16000000  // 16 MHz system clock
#define UART_TARGET_BAUD 9600  // Desired UART baud rate
#define LED_COUNT 49 // Define the total number of LEDs

volatile unsigned int timer_ticks[32] = { 0 };
//volatile unsigned char led_groups_chars[32] = {'0', '1', '2', '3', '4', '5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31'};
unsigned int led_trigger[32] = { 0 };

// Define the arrays for each group of 3 LEDs
// Define the arrays for each group of 3 LEDs
int led_index_0[] = {1, 2, 3};
int led_index_1[] = {4, 5, 6};
int led_index_2[] = {7, 8, 9};
int led_index_3[] = {10, 11, 12};
int led_index_4[] = {13, 14, 15};
int led_index_5[] = {16, 17, 18};
int led_index_6[] = {19, 20, 21};
int led_index_7[] = {22, 23, 24};
int led_index_8[] = {25, 26, 27};
int led_index_9[] = {28, 29, 30};
int led_index_10[] = {31, 32, 33};
int led_index_11[] = {34, 35, 36};
int led_index_12[] = {37, 38, 39};
int led_index_13[] = {40, 41, 42};
int led_index_14[] = {43, 44, 45};
int led_index_15[] = {46, 47, 48};

int led_index_16[] = {1, 2, 3};
int led_index_17[] = {4, 5, 6};
int led_index_18[] = {7, 8, 9};
int led_index_19[] = {10, 11, 12};
int led_index_20[] = {13, 14, 15};
int led_index_21[] = {16, 17, 18};
int led_index_22[] = {19, 20, 21};
int led_index_23[] = {22, 23, 24};
int led_index_24[] = {25, 26, 27};
int led_index_25[] = {28, 29, 30};
int led_index_26[] = {31, 32, 33};
int led_index_27[] = {34, 35, 36};
int led_index_28[] = {37, 38, 39};
int led_index_29[] = {40, 41, 42};
int led_index_30[] = {43, 44, 45};
int led_index_31[] = {46, 47, 48};


void turnOnLEDGroup(int group[], int groupIndex)
{
 //   printf("RECEIVED GROUP TO TURN ON: %d %d %d\n", group[0], group[1],group[2]);
    timer_ticks[groupIndex] = 0;  // Start timer for this group
    led_trigger[groupIndex] = 1;  // Set trigger for this group
    setLEDColor(group[0], 0xFF, 0xFF, 0xFF);
    setLEDColor(group[1], 0xFF, 0xFF, 0xFF);  // Turn on LED
    setLEDColor(group[2], 0xFF, 0xFF, 0xFF);
    showStrip();

}

void turnOffLEDGroup(int group[], int groupIndex)
{
//    printf("RECEIVED GROUP TO TURN OFF: %d %d %d\n", group[0], group[1],group[2]);
    //led_trigger[groupIndex] = 0;
    setLEDColor(group[0], 0x00, 0x00, 0x00);
    setLEDColor(group[1], 0x00, 0x00, 0x00);
    setLEDColor(group[2], 0x00, 0x00, 0x00);
    showStrip();
}

void turnOnLEDGrouptwo(int group[], int groupIndex)
{
 //   printf("RECEIVED GROUP TO TURN ON: %d %d %d\n", group[0], group[1],group[2]);
    timer_ticks[groupIndex] = 0;  // Start timer for this group
    led_trigger[groupIndex] = 1;  // Set trigger for this group
    setLEDColortwo(group[0], 0xFF, 0xFF, 0xFF);
    setLEDColortwo(group[1], 0xFF, 0xFF, 0xFF);  // Turn on LED
    setLEDColortwo(group[2], 0xFF, 0xFF, 0xFF);
    showStriptwo();

}

void turnOffLEDGrouptwo(int group[], int groupIndex)
{
//    printf("RECEIVED GROUP TO TURN OFF: %d %d %d\n", group[0], group[1],group[2]);
    //led_trigger[groupIndex] = 0;
    setLEDColortwo(group[0], 0x00, 0x00, 0x00);
    setLEDColortwo(group[1], 0x00, 0x00, 0x00);
    setLEDColortwo(group[2], 0x00, 0x00, 0x00);
    showStriptwo();
}

void Initialize_UART(void);
void uart_write_char(unsigned char ch);
unsigned char uart_read_char(void);
void configureTimer(void);
void turnOnLED(char led);
void turnOffLED(int group);
int main(void)
{
    char group;

    // Stop the Watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    // Enable the GPIO pins
    PM5CTL0 &= ~LOCKLPM5;

    // Configure clock
    CSCTL0 = CSKEY;
    FRCTL0 = FRCTLPW | NWAITS_1;
    CSCTL1 &= ~DCOFSEL_7;
    CSCTL1 |= DCOFSEL_4;
    CSCTL1 |= DCORSEL;
    CSCTL3 &= ~(DIVM2 | DIVM1 | DIVM0 | DIVS2 | DIVS1 | DIVS0);
    CSCTL0_H = 0;

    Initialize_UART();  // Initialize UART settings
    initStrip();
    initStriptwo();
    configureTimer();

    for (;;)
    {
   //     printf("Starting main for loop again...\n");
        // Read character from UART
        group = uart_read_char();
        // If '1' to 'q' is received, turn on corresponding LED
        if (group >= '0' && group <= 'v')
        {
       //     printf("char = %c\n", group);
            turnOnLED(group);
        }
    }

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  //  printf("Starting interrupt...\n");
    int i;
    for (i = 0; i < 32; i++) {
            if (led_trigger[i]) {      // Only count time for active groups
                timer_ticks[i]++;
                if (timer_ticks[i] >= 3000) {
                    turnOffLED(i);     // Turn off the group
                    timer_ticks[i] = 0;      // Reset timer
                    led_trigger[i] = 0;      // Reset trigger
                }
            }
        }

  //  printf("Exiting interrupt...\n");
}

void Initialize_UART(void)
{
    //Configure the pins to UART functionality
    P3SEL1 &= ~(BIT4 | BIT5);
    P3SEL0 |= (BIT4 | BIT5);

    // Main configuration register
    UCA1CTLW0 = UCSWRST; // Engage reset; change all the fields to zero
    // Most fields in this register, when set to zero, correspond to the
    // popular configuration
    UCA1CTLW0 |= UCSSEL_2; // Set clock to SMCLK
    // Configure the clock dividers and modulators (and enable oversampling)
    UCA1BRW = 104; // divider
    // Modulators: UCBRF = 8 = 1000 --> UCBRF3 (bit #3)
    // UCBRS = 0x20 = 0010 0000 = UCBRS5 (bit #5)
    UCA1MCTLW = UCBRF1 | UCOS16 | UCBRS6;
    // Exit the reset state
    UCA1CTLW0 &= ~UCSWRST;
}

void uart_write_char(unsigned char ch)
{
    // Wait for any ongoing transmission to complete
    while ((FLAGS & TXFLAG) == 0)
    {
    }
    // Copy the byte to the transmit buffer
    TXBUFFER = ch; // Tx flag goes to 0 and Tx begins!
}

unsigned char uart_read_char(void)
{
    // Return null character (ASCII=0) if no byte was received
    if ((FLAGS & RXFLAG) == 0)
        return 0;
    // Otherwise, copy the received byte (this clears the flag) and return it
    return RXBUFFER;
}

void configureTimer(void) {
    TA0CCTL0 = CCIE;                 // Enable interrupt for CCR0
    TA0CCR0 = 60000 - 1;            // Set CCR0 to generate interrupt every 16000 cycles (1 second at 16 MHz)
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK, Up mode, clear timer
}

void turnOnLED(char led)
{
 //   printf("Turning on group %c...\n", led);
    switch (led)
    {
    case '0':
        turnOnLEDGroup(led_index_0, 0);
        break;
    case '1':
        turnOnLEDGroup(led_index_1, 1);
        break;
    case '2':
        turnOnLEDGroup(led_index_2, 2);
        break;
    case '3':
        turnOnLEDGroup(led_index_3, 3);
        break;
    case '4':
        turnOnLEDGroup(led_index_4, 4);
        break;
    case '5':
        turnOnLEDGroup(led_index_5, 5);
        break;
    case '6':
        turnOnLEDGroup(led_index_6, 6);
        break;
    case '7':
        turnOnLEDGroup(led_index_7, 7);
        break;
    case '8':
        turnOnLEDGroup(led_index_8, 8);
        break;
    case '9':
        turnOnLEDGroup(led_index_9, 9);
        break;
    case 'a':
        turnOnLEDGroup(led_index_10, 10);
        break;
    case 'b':
        turnOnLEDGroup(led_index_11, 11);
        break;
    case 'c':
        turnOnLEDGroup(led_index_12, 12);
        break;
    case 'd':
        turnOnLEDGroup(led_index_13, 13);
        break;
    case 'e':
        turnOnLEDGroup(led_index_14, 14);
        break;
    case 'f':
        turnOnLEDGroup(led_index_15, 15);
        break;
    case 'g':
        turnOnLEDGrouptwo(led_index_16, 16);
        break;
    case 'h':
        turnOnLEDGrouptwo(led_index_17, 17);
        break;
    case 'i':
        turnOnLEDGrouptwo(led_index_18, 18);
        break;
    case 'j':
        turnOnLEDGrouptwo(led_index_19, 19);
        break;
    case 'k':
        turnOnLEDGrouptwo(led_index_20, 20);
        break;
    case 'l':
        turnOnLEDGrouptwo(led_index_21, 21);
        break;
    case 'm':
        turnOnLEDGrouptwo(led_index_22, 22);
        break;
    case 'n':
        turnOnLEDGrouptwo(led_index_23, 23);
        break;
    case 'o':
        turnOnLEDGrouptwo(led_index_24, 24);
        break;
    case 'p':
        turnOnLEDGrouptwo(led_index_25, 25);
        break;
    case 'q':
        turnOnLEDGrouptwo(led_index_26, 26);
        break;
    case 'r':
        turnOnLEDGrouptwo(led_index_27, 27);
        break;
    case 's':
        turnOnLEDGrouptwo(led_index_28, 28);
        break;
    case 't':
        turnOnLEDGrouptwo(led_index_29, 29);
        break;
    case 'u':
        turnOnLEDGrouptwo(led_index_30, 30);
        break;
    case 'v':
        turnOnLEDGrouptwo(led_index_31, 31);
        break;

    }
}

void turnOffLED(int group)
{
//   printf("Turning off group %d...\n", group);
    switch (group)
    {
    case 0:
        turnOffLEDGroup(led_index_0, 0);
        break;
    case 1:
        turnOffLEDGroup(led_index_1, 1);
        break;
    case 2:
        turnOffLEDGroup(led_index_2, 2);
        break;
    case 3:
        turnOffLEDGroup(led_index_3, 3);
        break;
    case 4:
        turnOffLEDGroup(led_index_4, 4);
        break;
    case 5:
        turnOffLEDGroup(led_index_5, 5);
        break;
    case 6:
        turnOffLEDGroup(led_index_6, 6);
        break;
    case 7:
        turnOffLEDGroup(led_index_7, 7);
        break;
    case 8:
        turnOffLEDGroup(led_index_8, 8);
        break;
    case 9:
        turnOffLEDGroup(led_index_9, 9);
        break;
    case 10:
        turnOffLEDGroup(led_index_10, 10);
        break;
    case 11:
        turnOffLEDGroup(led_index_11, 11);
        break;
    case 12:
        turnOffLEDGroup(led_index_12, 12);
        break;
    case 13:
        turnOffLEDGroup(led_index_13, 13);
        break;
    case 14:
        turnOffLEDGroup(led_index_14, 14);
        break;
    case 15:
        turnOffLEDGroup(led_index_15, 15);
        break;
    case 16:
        turnOffLEDGrouptwo(led_index_16, 16);
        break;
    case 17:
        turnOffLEDGrouptwo(led_index_17, 17);
        break;
    case 18:
        turnOffLEDGrouptwo(led_index_18, 18);
        break;
    case 19:
        turnOffLEDGrouptwo(led_index_19, 19);
        break;
    case 20:
        turnOffLEDGrouptwo(led_index_20, 20);
        break;
    case 21:
        turnOffLEDGrouptwo(led_index_21, 21);
        break;
    case 22:
        turnOffLEDGrouptwo(led_index_22, 22);
        break;
    case 23:
        turnOffLEDGrouptwo(led_index_23, 23);
        break;
    case 24:
        turnOffLEDGrouptwo(led_index_24, 24);
        break;
    case 25:
        turnOffLEDGrouptwo(led_index_25, 25);
        break;
    case 26:
        turnOffLEDGrouptwo(led_index_26, 26);
        break;
    case 27:
        turnOffLEDGrouptwo(led_index_27, 27);
        break;
    case 28:
        turnOffLEDGrouptwo(led_index_28, 28);
        break;
    case 29:
        turnOffLEDGrouptwo(led_index_29, 29);
        break;
    case 30:
        turnOffLEDGrouptwo(led_index_30, 30);
        break;
    case 31:
        turnOffLEDGrouptwo(led_index_31, 31);
        break;

    }
}

