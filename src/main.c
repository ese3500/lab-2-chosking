#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "header.h"
#include "uart.h"

char String[32];
volatile uint16_t start;
volatile uint16_t end;
volatile int overflow;
volatile int index;
int message[5];

void q1(){
    DDRB |= (1 << DDB1);
    DDRB |= (1 << DDB2);
    DDRB |= (1 << DDB3);
    DDRB |= (1 << DDB4);

    PORTB |= (1 << DDB1);
    PORTB |= (1 << DDB2);
    PORTB |= (1 << DDB3);
    PORTB |= (1 << DDB4);
}

void q2(){
    DDRB |= (1 << DDB1);
    DDRD &= ~(1 << DDD7);

    PORTB &= ~(1 << DDB1);

    while(1){
        if (PIND & (1 << PIND7)) {
            PORTB |= (1 << DDB1);
        } else {
            PORTB &= ~(1 << DDB1);
        }
    }
}

void q3(){
    //Enable i/o pins
    DDRB |= (1 << DDB1);
    DDRB |= (1 << DDB2);
    DDRB |= (1 << DDB3);
    DDRB |= (1 << DDB4);
    DDRD &= ~(1 << DDD7);

    PORTB |= (1 << DDB1);
    PORTB &= ~(1 << DDB2);
    PORTB &= ~(1 << DDB3);
    PORTB &= ~(1 << DDB4);

    int ready_for_next = 0;
    int state = 1;

    while(1){
        if (PIND & (1 << PIND7)){
            if (ready_for_next){
                if (state == 1){
                    PORTB |= (1 << DDB2);
                    PORTB &= ~(1 << DDB1);
                    state = 2;
                } else if (state == 2){
                    PORTB |= (1 << DDB3);
                    PORTB &= ~(1 << DDB2);
                    state = 3;
                } else if (state == 3){
                    PORTB |= (1 << DDB4);
                    PORTB &= ~(1 << DDB3);
                    state = 4;
                } else {
                    PORTB |= (1 << DDB1);
                    PORTB &= ~(1 << DDB4);
                    state = 1;
                }
                ready_for_next = 0;
                _delay_ms(300);
            }
        } else {
            ready_for_next = 1;
        }
    }
}

void morse_code(){
    UART_init(BAUD_PRESCALER);

    //Enable i/o pins
    DDRB |= (1 << DDB1);
    DDRB |= (1 << DDB2);
    DDRB &= ~(1 << DDB0);

    PORTB &= ~(1 << DDB1);
    PORTB &= ~(1 << DDB2);

    cli(); //Disable global interrupts

    //-------------------- Timer 1 --------------------
    //Set timer 1 to normal
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B &= ~(1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

    //Prescale timer 1 by 256, 62.5kHz
    TCCR1B |= (1 << CS12);
    TCCR1B &= ~(1 << CS11);
    TCCR1B &= ~(1 << CS10);

    //Look for rising edge at start
    TCCR1B |= (1 << ICES1);

    //Enable input capture and overflow and compare match interrupts
    TIMSK1 |= (1 << ICIE1);
    TIMSK1 |= (1 << TOIE1);
    TIMSK1 |= (1 << OCIE1A);

    OCR1A = 50000;

    //Manually clear the capture flag
    TIFR1 |= (1 << ICF1);

    sei(); //Enable global interrupts

    //Init variables
    start = 0;
    end = 0;
    overflow = 0;
    index = 0;

    while(1){
    }

}

void decode(){ //30ms = 1875 ticks, 200ms = 12500 ticks, 400ms = 25000
    int ticks_passed = end - start;
    if (overflow > 1) { //Button held down for a long time, no behavior
        return;
    } else if (overflow == 1) {
        ticks_passed = 65535 - start + end;
    }

    if (ticks_passed < 12500 && ticks_passed >= 1875) { //DOT
        sprintf(String,"DOT\n");
        //UART_putstring(String);
        PORTB |= (1 << DDB1);

        if (index >= 5) {
            sprintf(String,"---Error: Invalid Char ---\n");
            UART_putstring(String);
            index = 0;
        } else {
            message[index] = 0;
            index += 1;
        }
    } else if (ticks_passed < 25000) { //DASH
        sprintf(String,"DASH\n", ICR1);
        //UART_putstring(String);
        PORTB |= (1 << DDB2);

        if (index >= 5) {
            sprintf(String,"---Error: Invalid Char ---\n");
            UART_putstring(String);
            index = 0;
        } else {
            message[index] = 1;
            index += 1;
        }
    }
}

void print_char() {
    switch (index) {
        case 0:
            break;
        case 1:
            if (message[0] == 0) {
                sprintf(String,"E\n");
                UART_putstring(String);
            } else {
                sprintf(String,"T\n");
                UART_putstring(String);
            }
            break;
        case 2:
            if (message[0] == 0) {
                if (message[1] == 0) {
                    sprintf(String,"I\n");
                    UART_putstring(String);
                } else {
                    sprintf(String,"A\n");
                    UART_putstring(String);
                }
            } else {
                if (message[1] == 0) {
                    sprintf(String,"N\n");
                    UART_putstring(String);
                } else {
                    sprintf(String,"M\n");
                    UART_putstring(String);
                }
            }
            break;
        case 3:
            if (message[0] == 0) {
                if (message[1] == 0) {
                    if (message[2] == 0) {
                        sprintf(String,"S\n");
                        UART_putstring(String);
                    } else {
                        sprintf(String,"U\n");
                        UART_putstring(String);
                    }
                } else {
                    if (message[2] == 0) {
                        sprintf(String,"R\n");
                        UART_putstring(String);
                    } else {
                        sprintf(String,"W\n");
                        UART_putstring(String);
                    }
                }
            } else {
                if (message[1] == 0) {
                    if (message[2] == 0) {
                        sprintf(String,"D\n");
                        UART_putstring(String);
                    } else {
                        sprintf(String,"K\n");
                        UART_putstring(String);
                    }
                } else {
                    if (message[2] == 0) {
                        sprintf(String,"G\n");
                        UART_putstring(String);
                    } else {
                        sprintf(String,"O\n");
                        UART_putstring(String);
                    }
                }
            }
            break;
        case 4:
            if (message[0] == 0) {
                if (message[1] == 0) {
                    if (message[2] == 0) {
                        if (message[3] == 0) {
                            sprintf(String,"H\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String,"V\n");
                            UART_putstring(String);
                        }
                    } else {
                        if (message[3] == 0) {
                            sprintf(String,"F\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String,"---Error: Invalid Char ---\n");
                            UART_putstring(String);
                        }
                    }
                } else {
                    if (message[2] == 0) {
                        if (message[3] == 0) {
                            sprintf(String,"L\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String, "---Error: Invalid Char ---\n");
                            UART_putstring(String);
                        }
                    } else {
                        if (message[3] == 0) {
                            sprintf(String,"P\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String,"J\n");
                            UART_putstring(String);
                        }
                    }
                }
            } else {
                if (message[1] == 0) {
                    if (message[2] == 0) {
                        if (message[3] == 0) {
                            sprintf(String,"B\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String,"X\n");
                            UART_putstring(String);
                        }
                    } else {
                        if (message[3] == 0) {
                            sprintf(String,"C\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String,"Y\n");
                            UART_putstring(String);
                        }
                    }
                } else {
                    if (message[2] == 0) {
                        if (message[3] == 0) {
                            sprintf(String,"Z\n");
                            UART_putstring(String);
                        } else {
                            sprintf(String, "Q\n");
                            UART_putstring(String);
                        }
                    } else {
                        sprintf(String, "---Error: Invalid Char ---\n");
                        UART_putstring(String);
                    }
                }
            }
            break;
        case 5:
            if (message[0] && message[1] && message[2] && message[3] && message[4]) {
                sprintf(String, "0\n");
                UART_putstring(String);
            } else if (message[0] && message[1] && message[2] && message[3] && !message[4]) {
                sprintf(String, "9\n");
                UART_putstring(String);
            } else if (message[0] && message[1] && message[2] && !message[3] && !message[4]) {
                sprintf(String, "8\n");
                UART_putstring(String);
            } else if (message[0] && message[1] && !message[2] && !message[3] && !message[4]) {
                sprintf(String, "7\n");
                UART_putstring(String);
            } else if (message[0] && !message[1] && !message[2] && !message[3] && !message[4]) {
                sprintf(String, "6\n");
                UART_putstring(String);
            } else if (!message[0] && !message[1] && !message[2] && !message[3] && !message[4]) {
                sprintf(String, "5\n");
                UART_putstring(String);
            } else if (!message[0] && !message[1] && !message[2] && !message[3] && message[4]) {
                sprintf(String, "4\n");
                UART_putstring(String);
            } else if (!message[0] && !message[1] && !message[2] && message[3] && message[4]) {
                sprintf(String, "3\n");
                UART_putstring(String);
            } else if (!message[0] && !message[1] && message[2] && message[3] && message[4]) {
                sprintf(String, "2\n");
                UART_putstring(String);
            } else if (!message[0] && message[1] && message[2] && message[3] && message[4]) {
                sprintf(String, "1\n");
                UART_putstring(String);
            } else {
                sprintf(String, "---Error: Invalid Char ---\n");
                UART_putstring(String);
            }
            break;
    }
}


ISR(TIMER1_COMPA_vect) {
    if (TCCR1B & (1 << ICES1)){
        print_char();
        index = 0;
    }
    sprintf(String,"SPACE\n", ICR1);
    //UART_putstring(String);

    OCR1A = (TCNT1 + 50000) % 65536;
}

ISR(TIMER1_CAPT_vect) {
    sprintf(String,"BOOM\n");
    //UART_putstring(String);
    PORTB &= ~(1 << DDB1);
    PORTB &= ~(1 << DDB2);

    if (TCCR1B & (1 << ICES1)) {
        start = ICR1;
        TIMSK1 &= ~(1 << OCIE1A); //Disable compare interrupt for SPACE checking
        overflow = 0;
    } else {
        end = ICR1;
        decode();
        OCR1A = (TCNT1 + 50000) % 65536;
        TIFR1 |= (1 << OCF1A); //Clear any SPACE interrupts queued
        TIMSK1 |= (1 << OCIE1A); //Enable compare interrupt for SPACE checking
    }
    //Toggle what edge we are looking for
    TCCR1B ^= (1 << ICES1);
    //Clear capture bit
    TIFR1 |= (1 << ICF1);
}

ISR(TIMER1_OVF_vect) {
    overflow += 1;
}

int main() {
    //q1();
    //q2();
    //q3();
    //partb();
    morse_code();
}