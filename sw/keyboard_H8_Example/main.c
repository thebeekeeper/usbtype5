//(c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*  
 * ======== main.c ========
 * Keyboard HID Demo:
 *
 * This code implements a "keyboard" using a FET target board as hardware.  Pressing
 * S1 represents pressing an ordinary key on the keyboard.  Holding down S2 while
 * you do it represents holding down the SHIFT key.  Run Notepad, or any program
 * that accepts keyboard input, and attach the FET target board.  Pressing S1 results
 * in "msp430" being entered.  Holding S2 down at the same time results in "MSP$#)".
 *
 * A standard keyboard report descriptor is set up in descriptors.c.  The polling
 * interval for this report is 48ms.  Whenever S1 is pressed, the PORT1 ISR evaluates
 * whether S2 is also held down, then populates the HID report accordingly.  It
 * then flags main() to send the report.  Immediately a blank report follows it,
 * otherwise Windows takes this as an indication of repeating the text indefinitely.
 *
 * During USB attachment, the application spends its time in LPM0, until the button
 * is pressed.  When disconnected, or if attached to a powered hub without a host
 * (ST_NOENUM_SUSPENDED), or if suspended, the corresponding event handlers
 * disable port interrupts, and the device enters LPM3.  Port interrupts are
 * re-enabled at resume or attachment with the corresponding event handlers.
 *
 +----------------------------------------------------------------------------+
 * Please refer to the MSP430 USB API Stack Programmer's Guide,located
 * in the root directory of this installation for more details.
 *----------------------------------------------------------------------------*/

#include "USB_config/descriptors.h"

#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"                           //Basic Type declarations
#include "USB_API/USB_Common/usb.h"                             //USB-specific functions

#include "F5xx_F6xx_Core_Lib/HAL_UCS.h"
#include "F5xx_F6xx_Core_Lib/HAL_PMM.h"

#include "USB_API/USB_HID_API/UsbHid.h"

#include "main.h"                                               //HID key codes stored here

VOID Init_Ports (VOID);
VOID Init_Clock (VOID);

VOID init_pads(void);

BYTE report[8];                                                 //Standard keyboard input report
BYTE buttonPressed = FALSE;

void config_timer(void);

/*  
 * ======== main ========
 */
VOID main (VOID)
{
    BYTE i;
	
    WDTCTL = WDTPW + WDTHOLD;                                   //Stop watchdog timer
	
    Init_Ports();                                               //Init ports (do first ports because clocks do change ports)
    SetVCore(3);
    Init_Clock();                                               //Init clocks

    USB_init();                                                 //Init USB

    //Configure pushbuttons
    //S1 will represent a key getting pressed; S2 will represent the SHIFT key
    //P1OUT |= (BIT6 | BIT7);                                     //Enable the integrated pullup for P1.6 and P1.7
    //P1REN |= (BIT6 | BIT7);                                     //Enable the integrated pullup for P1.6 and P1.7
    P1OUT |= BIT7;
    P1REN |= BIT7;
    P2OUT |= BIT2;
    P2REN |= BIT2;
    P1IFG = 0;                                                  //Ensure no flags
    //P1IE |= BIT6;                                               //Enable a pullup for S1 only
    P1IE |= BIT7;
    
    P4OUT = 0x00;
    // read pin
    P4SEL &= ~BIT1;
    P4DIR &= ~BIT1;
    //P4OUT &= ~BIT1;
    P4OUT |= BIT1;
    P4REN |= BIT1;
    //P4OUT |= BIT1;
    //P4REN |= BIT1;
    //P4IFG = 0;
    //P4IFE |= BIT1;
    // write pin
    P4OUT &= ~BIT2;
    
    config_timer();
    
    
#ifdef PADS
    init_pads();
#endif
    
    USB_setEnabledEvents(
        kUSB_VbusOnEvent + kUSB_VbusOffEvent + kUSB_receiveCompletedEvent
        + kUSB_UsbSuspendEvent + kUSB_UsbResumeEvent);

    //In case USB is already attached (meaning no VBUS event will
    //occur), manually start the connection
    if (USB_connectionInfo() & kUSB_vbusPresent){
        USB_handleVbusOnEvent();
    }

    __enable_interrupt();                           //Enable interrupts globally
    while (1)
    {
        switch (USB_connectionState())
        {
            case ST_USB_DISCONNECTED:
                __bis_SR_register(LPM3_bits + GIE);             //Enter LPM3 w/interrupt.  Nothing for us to do while disconnected.
                _NOP();
                break;

            case ST_USB_CONNECTED_NO_ENUM:
                break;

            case ST_ENUM_ACTIVE:
                __bis_SR_register(LPM0_bits + GIE);             //Enter LPM0 w/interrupt, until a keypress occurs

                if (buttonPressed){                             //Has a keypress really occurred?
                    //Then send the report created in the PORT1 ISR
                    USBHID_sendReport((void *)&report, HID0_INTFNUM);
                   //Send a blank report, to signal the end of the text
                   for (i = 0; i < 8; i++){
                        report[i] = 0x00;
                    }
                    USBHID_sendReport((void *)&report, HID0_INTFNUM);
                                                            
                    buttonPressed = FALSE;						//Clear the flag
                }
                break;

            case ST_ENUM_SUSPENDED:
                __bis_SR_register(LPM3_bits + GIE);             //Enter LPM3 w/interrupt.  Nothing for us to do while
                break;                                          //suspended.  (Remote wakeup isn't enabled in this example.)

            case ST_ENUM_IN_PROGRESS:
                break;

            case ST_NOENUM_SUSPENDED:
                __bis_SR_register(LPM3_bits + GIE);
                break;

            case ST_ERROR:
                _NOP();
                break;

            default:;
        }
    }  //while(1)
} //main()

void init_pads(void)
{
  P1OUT |= (BIT1 | BIT2 | BIT3 | BIT4 | BIT5); 
  P2REN |= (BIT1 | BIT2 | BIT3 | BIT4 | BIT5); 
  P1IE |= (BIT1 | BIT2 | BIT3 | BIT4 | BIT5); 
}

/*  
 * ======== Init_Clock ========
 */
VOID Init_Clock (VOID)
{
    //Initialization of clock module
    if (USB_PLL_XT == 2){
		#if defined (__MSP430F552x) || defined (__MSP430F550x)
			P5SEL |= 0x0C;                                      //enable XT2 pins for F5529
		#elif defined (__MSP430F563x_F663x)
			P7SEL |= 0x0C;
		#endif

        //use REFO for FLL and ACLK
        UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

        //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
        Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);   //Start the FLL, at the freq indicated by the config
                                                                        //constant USB_MCLK_FREQ
        XT2_Start(XT2DRIVE_0);                                          //Start the "USB crystal"
    } 
	else {
		#if defined (__MSP430F552x) || defined (__MSP430F550x)
			P5SEL |= 0x10;                                      //enable XT1 pins
		#endif
        //Use the REFO oscillator to source the FLL and ACLK
        UCSCTL3 = SELREF__REFOCLK;
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

        //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
        Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);   //set FLL (DCOCLK)

        XT1_Start(XT1DRIVE_0);                                          //Start the "USB crystal"
    }
}

/*  
 * ======== Init_Ports ========
 */
VOID Init_Ports (VOID)
{
    //Initialization of ports (all unused pins as outputs with low-level
    P1OUT = 0x00;
    P1DIR = 0xFF;
	P2OUT = 0x00;
    P2DIR = 0xFF;
    P3OUT = 0x00;
    P3DIR = 0xFF;
    P4OUT = 0x00;
    P4DIR = 0xFF;
    P5OUT = 0x00;
    P5DIR = 0xFF;
    P6OUT = 0x00;
    P6DIR = 0xFF;
	P7OUT = 0x00;
    P7DIR = 0xFF;
    P8OUT = 0x00;
    P8DIR = 0xFF;
    #if defined (__MSP430F563x_F663x)
		P9OUT = 0x00;
		P9DIR = 0xFF;
    #endif
}

/*  
 * ======== UNMI_ISR ========
 */
#pragma vector = UNMI_VECTOR
__interrupt VOID UNMI_ISR (VOID)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG); //Clear OSC flaut Flags fault flags
            SFRIFG1 &= ~OFIFG;                          //Clear OFIFG fault flag
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
                                                    //If bus error occured - the cleaning of flag and re-initializing of USB is
                                                    //required.
            SYSBERRIV = 0;                          //clear bus error flag
            USB_disable();                          //Disable
    }
}

/*  
 * ======== Port1_ISR ========
 */
#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR (void)
{
    WORD i;
        
    if(~P4IN & BIT1) {
      
      report[0] = 0x00;
      report[1] = 0x00;
      report[2] = usbUsageEnter;
      report[3] = 0x00;
      report[4] = 0x00;
      report[5] = 0x00;
      report[6] = 0x00;
      report[7] = 0x00;
      buttonPressed = TRUE;
      __bic_SR_register_on_exit(LPM3_bits);
      P1IFG = 0;
    }

    if(P1IFG & BIT7) {
        for (i = 0x23FF; i > 0; i--){               //Cheap debounce.
        }
        if (P1IN & BIT7){
            //if ((P2IN & BIT2)){
                report[0] = 0x02;                   //This byte is a bitfield for modifier keys.  If S2 is pressed,
            //} else {                                //signal the "left shift" key; else no modifier keys
            //    report[0] = 0x00;
            //}
            report[1] = 0x00;                       //Reserved
            //report[2] = usbUsageM;                  //The remaining six bytes are for entered text.  It allows up
            //report[3] = usbUsageS;                  //to six in a single report.  Since we don't have the hardware
            //report[4] = usbUsageP;                  //for a true keyboard, let's just fill it with a six-letter word.
            //report[5] = usbUsage4;                  //A real keyboard app would put whatever keys had been pressed
            //report[6] = usbUsage3;                  //since the last report.
            //report[7] = usbUsage0;
            report[2] = usbUsageL;
            report[3] = usbUsageA;
            report[4] = usbUsageU;
            report[5] = usbUsageR;
            report[6] = usbUsageA;
            report[7] = usbUsageEnter;

            //Set flag to wake main.  We can't call the USB API functions out of an interrupt context
            buttonPressed = TRUE;
            __bic_SR_register_on_exit(LPM3_bits);   //Wake main from LPMx
        }
        P1IFG = 0;
    }
#ifdef PADS
    if(P1IFG & (BIT1 | BIT2 | BIT3 | BIT4 | BIT5))
    {
      report[0] = 0x00;
      report[1] = 0x00;
      if(P1IFG & BIT1) {
        report[2] = usbUsageL;
      }
      if(P1IFG & BIT2) {
        report[2] = usbUsageA;
      }
      if(P1IFG & BIT3) {
        report[2] = usbUsageU;
      }
      if(P1IFG & BIT4) {
        report[2] = usbUsageR;
      }
      if(P1IFG & BIT5) {
        report[2] = usbUsageA;
      }
      report[3] = 0x00;
      report[4] = 0x00;
      report[5] = 0x00;
      report[6] = 0x00;
      report[7] = 0x00;
      buttonPressed = TRUE;
      __bic_SR_register_on_exit(LPM3_bits);   //Wake main from LPMx      
    }
    P1IFG = 0;
#endif
}

void config_timer2(void)
{
//  TA1CCR0 = 14;
//  TA1CTL = CCIE;
//  TA1CCR0 = TASSEL_1 + ID_2 + MC_1 + TACLR;  // use ACLK, div 4, up mode, clear
  
   //SFRIFG1 &= ~WDTIFG;
    //WDTCTL = WDTPW + WDTSSEL_1 + WDTTMSEL + WDTCNTCL + WDTIS_5;
    //SFRIE1 |= WDTIE;
    
        TA1CTL = 0;

    /* Run the timer of the ACLK. */
    TA1CTL = TASSEL1 | ID0;     // use ACLK as 32768;

    /* Clear everything to start with. */
    TA1CTL |= TACLR;

    /* Set the compare match value according to the tick rate we want. */
//    TA1CCR0 = (portACLK_FREQUENCY_HZ / configTICK_RATE_HZ) - 1;
      TA1CCR0 = TASSEL_1 + ID_2 + MC_1 + TACLR;  // use ACLK, div 4, up mode, clear

    /* Enable the interrupts. */
    TA1CCTL0 = CCIE;

    /* Start up clean. */
    TA1CTL |= TACLR;

    /* Up mode. */
    TA1CTL |= TASSEL1 | MC1 | ID0;
}

void config_timer(void)
{
  TA1CCR0 = 320;
  TA1CCTL0 = CCIE;
  TA1CTL = TASSEL_1 + MC_1 + TACLR;
}

// timer isr
//#pragma vector=WDT_VECTOR
//__interrupt void WDT_ISR(void)
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
  if(~P4IN & BIT1) {     
    report[0] = 0x00;
    report[1] = 0x00;
    report[2] = usbUsageEnter;
    report[3] = 0x00;
    report[4] = 0x00;
    report[5] = 0x00;
    report[6] = 0x00;
    report[7] = 0x00;
    buttonPressed = TRUE;
    __bic_SR_register_on_exit(LPM3_bits);
  }
}

