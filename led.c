/*
 * led.c
 *
 * Created: 23.03.2019 16:04:06
 * Author: main
 */

#include <tiny2313a.h>
#include <interrupt.h>
#include <stdint.h>

#ifndef _BV
#define _BV(n) (1<<n)
#endif

#define F_CPU 8000000
#define SPEED 300000    //Скорость бегущих огней

uint8_t brightness[3] = {
    0b10000000,
    0b00010000,
    0b00000010
};
uint8_t brightnessMask = 0b00000001;

typedef struct pinAdreses_t {
    uint8_t* port;
    uint8_t pin;
}pinAdreses;
pinAdreses ledAdreses[12];

uint8_t ledAdresesLen = sizeof(ledAdreses) / sizeof(pinAdreses);

uint8_t getInputValue (uint8_t port) {
    return ( port & 0b00111000 ) >> 3;
    //return ( port & 0b00001000 ) >> 3;
}

void setBrightness (pinAdreses *ledAdr, uint8_t brightnessIndex) {
    if(brightness[brightnessIndex] & brightnessMask) {
		*ledAdr->port |= ledAdr->pin;
	} else {
        *ledAdr->port &= ~ledAdr->pin;
	}
}

uint8_t counter = 0;

void main(void) {
	// Инициализируем массив структур ханящих адрес порта и маску для выставление нужного бита данного порта в 1
    ledAdreses[0].port = &PORTD;
    ledAdreses[0].pin = (1 << 2);
    ledAdreses[1].port = &PORTA;
    ledAdreses[1].pin = (1 << 0);
    ledAdreses[2].port = &PORTA;
    ledAdreses[2].pin = (1 << 1);
    ledAdreses[3].port = &PORTD;
    ledAdreses[3].pin = (1 << 6);
    ledAdreses[4].port = &PORTB;
    ledAdreses[4].pin = (1 << 0);
    ledAdreses[5].port = &PORTB;
    ledAdreses[5].pin = (1 << 1);
    ledAdreses[6].port = &PORTB;
    ledAdreses[6].pin = (1 << 2);
    ledAdreses[7].port = &PORTB;
    ledAdreses[7].pin = (1 << 3);
    ledAdreses[8].port = &PORTB;
    ledAdreses[8].pin = (1 << 4);
    ledAdreses[9].port = &PORTB;
    ledAdreses[9].pin = (1 << 5);
    ledAdreses[10].port = &PORTB;
    ledAdreses[10].pin = (1 << 6);
    ledAdreses[11].port = &PORTB;
    ledAdreses[11].pin = (1 << 7);

	DDRA = 0b011;
    DDRB = 0b11111111;
    DDRD = 0b1000111;

    // Настройка таймеров
    TIFR = 0x0; // сброс регистра флагов
    TIMSK = 0b10000010; // Разраешаем прерывания по переполнению таймера0 и по переполнению таймера1
    sei();
    // Настройка таймера0
    TCCR0A = 0b00000000; // настрока таймера
    //TCCR0B = 0b00000000;
    //TCNT0 = 0x0; // установка начального значения счетчика
    TCNT0 = ~brightnessMask;
    // Настройка таймера1
    TCCR1A = 0b00000000; // настрока таймера
    //TCCR1B = 0b00000000;
    TCNT1 = 0b0;

    TCCR0B = 0b00000100; // Запуск таймера0 с делителем 256
	//TCCR1B = 0b00000010; // Запуск таймера1 с делителем 8
	TCCR1B = 0b00000100; // Запуск таймера1 с делителем 256
}

ISR(TIM0_OVF) {
    //uint8_t i;
	uint8_t buff = counter;

	brightnessMask = (brightnessMask >> 1) | (brightnessMask << 7);
	TCNT0 = ~brightnessMask;

    for (i = 0; i < ledAdresesLen; i++) {
        setBrightness(&ledAdreses[i], (i + buff) % 3);
    }
	/*setBrightness(&ledAdreses[(buff + 1) % 12], 0);
	setBrightness(&ledAdreses[(buff) % 12], 1);
	setBrightness(&ledAdreses[(buff - 1) % 12], 2);*/
}

ISR(TIM1_OVF) {
    TCNT1 = 0b0;
	if (++counter == (sizeof(ledAdreses) / sizeof(pinAdreses) + 1)) {
		counter = 0;
	}
}
