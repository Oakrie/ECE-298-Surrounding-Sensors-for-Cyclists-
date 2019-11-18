#include "driverlib/driverlib.h"
#include "hal_LCD.h"
#include <stdlib.h>
#include <msp430fr4133.h>
#include "driverlib/timer_a.h"
#include "main.h"
#include <msp430.h>
#include <stdio.h>

char KeyPadLayout[NUM_ROWS][NUM_COLS] = {{'1','2','3'},
                                         {'4','5','6'},
                                         {'7','8','9'},
                                         {'A','0','P'}};
char chars[3] = {'0', '0', '0'};
volatile unsigned int mode;
char setup_interrupted = 0;

void init_interrupt(){
    GPIO_setAsInputPinWithPullUpResistor(PB1_PORT, PB1_PIN);
    GPIO_enableInterrupt(PB1_PORT, PB1_PIN);
    GPIO_selectInterruptEdge(PB1_PORT, PB1_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(PB1_PORT, PB1_PIN);

    GPIO_setAsInputPinWithPullUpResistor(PB2_PORT, PB2_PIN);
    GPIO_enableInterrupt(PB2_PORT, PB2_PIN);
    GPIO_selectInterruptEdge(PB2_PORT, PB2_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(PB2_PORT, PB2_PIN);
}

void init_timer(){
    Timer_A_initContinuousModeParam timer_param_0;
    timer_param_0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_param_0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_param_0.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    timer_param_0.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_initContinuousMode(TIMER_A0_BASE, &timer_param_0);

//    Timer_A_initContinuousModeParam timer_param_1;
//    timer_param_1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
//    timer_param_1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
//    timer_param_1.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
//    timer_param_1.timerClear = TIMER_A_DO_CLEAR;
//    Timer_A_initContinuousMode(TIMER_A1_BASE, &timer_param_1);
}

void Init_GPIO(void){
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    GPIO_setAsInputPinWithPullDownResistor(KEYPAD_COL1_PORT, KEYPAD_COL1_PIN); //Set COL1 as input
    GPIO_setAsInputPinWithPullDownResistor(KEYPAD_COL2_PORT, KEYPAD_COL2_PIN); //Set COL2 as input
    GPIO_setAsInputPinWithPullDownResistor(KEYPAD_COL3_PORT, KEYPAD_COL3_PIN); //Set COL3 as input

    GPIO_setAsOutputPin(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN); //Set ROW1 as output
    GPIO_setAsOutputPin(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN); //Set ROW2 as output
    GPIO_setAsOutputPin(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN); //Set ROW3 as output
    GPIO_setAsOutputPin(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN); //Set ROW4 as output

    GPIO_setAsInputPin(USB_ECHO_PORT, USB_ECHO_PIN); //Set COL2 as input
    GPIO_setAsInputPin(USF_ECHO_PORT, USF_ECHO_PIN); //Set COL3 as input
    GPIO_setAsOutputPin(USB_TRIG_PORT, USB_TRIG_PIN); //Set ROW1 as output
    GPIO_setAsOutputPin(USF_TRIG_PORT, USF_TRIG_PIN); //Set ROW2 as output

}

void displayIntLCD(unsigned int numDisp){
    char charDisp[3];
    charDisp[0] = numDisp%10;
    numDisp /= 10;
    charDisp[1] = numDisp%10;
    numDisp /= 10;
    charDisp[2] = numDisp%10;
    numDisp /= 10;

    showChar('0'+(charDisp[0]), pos6);
    showChar('0'+(charDisp[1]), pos5);
    showChar('0'+(charDisp[2]), pos4);

}

void pulse_output_back(void){
    unsigned int test=0;
    test = 98;

    GPIO_setOutputHighOnPin(USB_TRIG_PORT, USB_TRIG_PIN);
    GPIO_setOutputLowOnPin(USB_TRIG_PORT, USB_TRIG_PIN);

    Timer_A_clear(TIMER_A0_BASE);
    while(GPIO_getInputPinValue(USB_ECHO_PORT, USB_ECHO_PIN) == 0 ){
        ;
    }
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
    while(GPIO_getInputPinValue(USB_ECHO_PORT, USB_ECHO_PIN) != 0 ){
        ;
    }
    Timer_A_stop(TIMER_A0_BASE);
    test =  Timer_A_getCounterValue(TIMER_A0_BASE)/58;

    displayIntLCD(test);

    __delay_cycles(16000 );
}

void pulse_output_front(void){
    unsigned int test=0;
    test = 98;

    GPIO_setOutputHighOnPin(USF_TRIG_PORT, USF_TRIG_PIN);
    GPIO_setOutputLowOnPin(USF_TRIG_PORT, USF_TRIG_PIN);

    Timer_A_clear(TIMER_A0_BASE);
    while(GPIO_getInputPinValue(USF_ECHO_PORT, USF_ECHO_PIN) == 0 ){
        ;
    }
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
    while(GPIO_getInputPinValue(USF_ECHO_PORT, USF_ECHO_PIN) != 0 ){
        ;
    }
    Timer_A_stop(TIMER_A0_BASE);
    test =  Timer_A_getCounterValue(TIMER_A0_BASE)/58;

    displayIntLCD(test);

    __delay_cycles(16000);
}

unsigned int get_distance(unsigned int timer, unsigned int port, unsigned int pin){
    Timer_A_clear(timer);
    //displayScrollText("PLZ");
    while(GPIO_getInputPinValue(port, pin) == 0 ){
        ;
    }
    //displayScrollText("HELL");
    Timer_A_startCounter(timer, TIMER_A_CONTINUOUS_MODE);
    while(GPIO_getInputPinValue(port, pin) != 0 ){
        ;
    }
    Timer_A_stop(timer);
    return Timer_A_getCounterValue(timer)/58;
}

int scan_cols(void){
    if(GPIO_getInputPinValue(KEYPAD_COL1_PORT, KEYPAD_COL1_PIN) == GPIO_INPUT_PIN_HIGH){
        return 1;
    }
    else if(GPIO_getInputPinValue(KEYPAD_COL2_PORT, KEYPAD_COL2_PIN) == GPIO_INPUT_PIN_HIGH){
        return 2;
    }
    else if(GPIO_getInputPinValue(KEYPAD_COL3_PORT, KEYPAD_COL3_PIN) == GPIO_INPUT_PIN_HIGH){
        return 3;
    }
    return 0;
}

int keypad(void){
    showChar('C', pos5);
    showChar('M', pos6);
    int counted = 0;
    int key;
    int row;
    int column;
    int num = 0;
    int power = 1;

    while(1){
        while(1){
            if(setup_interrupted){
                clearLCD();
                return -2;
            }
            GPIO_setOutputHighOnPin(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN);
            row = 1;
            column = scan_cols();
            if(column != 0){
                GPIO_setOutputLowOnPin(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN);
                break;
            }
            GPIO_setOutputLowOnPin(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN);

            GPIO_setOutputHighOnPin(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN);
            row = 2;

            column = scan_cols();
            if(column != 0){
                GPIO_setOutputLowOnPin(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN);
                break;
            }
            GPIO_setOutputLowOnPin(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN);

            GPIO_setOutputHighOnPin(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN);
            row = 3;

            column = scan_cols();
            if(column != 0){
                GPIO_setOutputLowOnPin(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN);
                break;
            }
            GPIO_setOutputLowOnPin(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN);

            GPIO_setOutputHighOnPin(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN);
            row = 4;

            column = scan_cols();
            if(column != 0){
                GPIO_setOutputLowOnPin(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN);
                break;
            }
            GPIO_setOutputLowOnPin(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN);

        }

        __delay_cycles(250000);

        key = KeyPadLayout[row - 1][column - 1];

        if(key == 'P'){
            clearLCD();
            power /= 10;
            int i;
            for(i = 0; i < counted; i++){
                num += (chars[i]-48)*power;
                power /= 10;
            }
            return num;
        }
        if(key == 'A'){
            if(counted == 0){
                return -1;
            }
            clearLCD();
            counted = 0;
            power = 1;
            showChar('C', pos5);
            showChar('M', pos6);
            continue;
        }
        if(counted == 3){
            continue;
        }

        chars[counted] = key;
        showChar(chars[counted], pos4);
        if(counted == 1){
            showChar(chars[counted - 1], pos3);
        }
        if(counted == 2){
            showChar(chars[counted - 1], pos3);
            showChar(chars[counted - 2], pos2);
        }
        counted++;
        power *= 10;
    }

}

void setup_front(int *front_values){
    if(setup_interrupted){
        return -2;
    }
    int beepTwice;
    int beepTwiceFlag = 1;
    int beepFrice;
    int beepFriceFlag = 0;

    while(1){
        if(beepTwiceFlag){
            displayScrollText("2 BEEPS");
        }
        while(beepTwiceFlag){
            beepTwice = keypad();
            if(beepTwice == -2){
                return;
            }
            if(beepTwice > 400 || beepTwice < 2){
                displayScrollText("MUST BE BETWEEN 2 AND 400");
                continue;
            }
            beepTwiceFlag = 0;
            beepFriceFlag = 1;
            break;
        }
        if(beepFriceFlag){
            displayScrollText("4 BEEPS");
        }
        while(beepFriceFlag){
            beepFrice = keypad();
            if(beepFrice == -1){
                beepTwiceFlag = 1;
                beepFriceFlag = 0;
                break;
            }
            if(beepFrice == -2){
                return;
            }
            if(beepFrice >= beepTwice){
                displayScrollText("MUST BE LESS THAN MIN 2 BEEPS");
                continue;
            }
            beepFriceFlag = 0;
            break;
        }
        if(beepFriceFlag == 0 && beepTwiceFlag == 0){
            break;
        }
    }
    front_values[0] = beepTwice;
    front_values[1] = beepFrice;
}

void setup_back(int *back_values){
    int green = -1;
    int greenFlag = 1;
    int yellowFlag = 0;
    int yellow = -1;
    int orangeFlag = 0;
    int orange = -1;

    while(1){
        if(greenFlag){
            displayScrollText("GREEN");
        }
        while(greenFlag){
            green = keypad();
            if(green == -2){
                return;
            }
            if(green > 400 || green < 3){
                displayScrollText("MUST BE BETWEEN 3 AND 400");
                continue;
            }
            greenFlag = 0;
            yellowFlag = 1;
        }

        if(yellowFlag){
            displayScrollText("YELLOW");
        }
        while(yellowFlag){
            yellow = keypad();
            if(yellow == -1){
                greenFlag = 1;
                yellowFlag = 0;
                break;
            }
            if(yellow == -2){
                return;
            }
            else if(yellow >= green || yellow < 2){
                displayScrollText("MUST BE BETWEEN 2 AND GREEN");
                continue;
            }
            yellowFlag = 0;
            orangeFlag = 1;
        }

        if(orangeFlag){
            displayScrollText("ORANGE");
        }
        while(orangeFlag){
            orange = keypad();
            if(orange == -1){
                  yellowFlag = 1;
                  orangeFlag = 0;
                  break;
            }
            if(orange == -2){
                return;
            }
            else if(orange >= yellow || orange < 1){
                displayScrollText("MUST BE BETWEEN 1 AND YELLOW");
                continue;
            }
            orangeFlag = 0;
            break;
        }
        if(greenFlag == 0 && yellowFlag == 0 && orangeFlag == 0){
            break;
        }
    }

    back_values[0] = green;
    back_values[1] = yellow;
    back_values[2] = orange;
}

int main(void)
{
    int front_vals[2];
    int rear_vals[3];

//    int index;
//    int USBVals[10];
//    int USFVals[10];


    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    //Stop watchdog timer unless you plan on using it
    WDT_A_hold(WDT_A_BASE);

    __disable_interrupt();

    // Initializations - see functions for more detail
    Init_GPIO();    //Sets all pins to output low as a default
    Init_LCD();     //Sets up the LaunchPad LCD display
    init_timer();
    init_interrupt();
    PMM_unlockLPM5();

    //setup_back(rear_vals);
    //setup_front(front_vals);

    __enable_interrupt();
    GPIO_clearInterrupt(PB1_PORT, PB1_PIN);
    GPIO_clearInterrupt(PB2_PORT, PB2_PIN);

    mode = USER_MODE;
    clearLCD();

    while(1){
        while(mode == SETUP_MODE){
            setup_back(rear_vals);
            setup_front(front_vals);
            setup_interrupted = 0;
        }
        while(mode == USER_MODE){
            //displayScrollText("USER");
            //pulse_output_back();
            pulse_output_front();
        }

    }
	return 0;
}



//PORT1 interrupt vector service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT1_VECTOR)))
#endif

void P1_ISR(void) {
    __disable_interrupt();
    if(mode == SETUP_MODE){
        setup_interrupted = 1;
    }
    mode = !mode;
    GPIO_clearInterrupt(PB1_PORT, PB1_PIN);
    __enable_interrupt();
}

//PORT2 interrupt vector service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT2_VECTOR)))
#endif

void P2_ISR(void) {
    __disable_interrupt();
    __delay_cycles(10000);
    //Start timer on rising edge, stop on falling edge, print counter value
    mode = 0;

//    GPIO_selectInterruptEdge(PB1_PORT, PB1_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(PB2_PORT, PB2_PIN);
    __enable_interrupt();
}

