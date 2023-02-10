#include <avr/interrupt.h>

void partb(){
    //Enable i/o pins
    DDRB |= (1 << DDB5);
    DDRB &= ~(1 << DDB0);

    cli(); //Disable global interrupts

    //Set timer 1 to normal
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B &= ~(1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

    //Look for rising edge at start
    TCCR1B |= (1 << ICES1);

    //Enable input capture interrupt
    TIMSK1 |= (1 << ICIE1);

    //Manually clear the capture flag
    TIFR1 |= (1 << ICF1);

    sei(); //Enable global interrupts

    while(1){
    }
}

//Uncomment to run part b from main.c
/*
ISR(TIMER1_CAPT_vect) {
    //Toggle LED
    PORTB ^= (1 << DDB5);
    //Toggle what edge we are looking for
    TCCR1B ^= (1 << ICES1);
}*/
