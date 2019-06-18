#include <tiny2313a.h>
#include <interrupt.h>
#include <stdint.h>

//#define F_CPU 8000000

// ������� ��� ����� �������, 2 ������� ���� ����� ��������
uint8_t settings = 0b000;
uint8_t settingsCodes[8] = {
    0b0000,
    0b1000,
    0b0001,
    0b0101,
    0b1001,
    0b0010,
    0b0110,
    0b1010
};
// ������ ���������
uint16_t speed[4] = {
    //0x3333,
    0x0000,
    0x8000,
    0xA666
};
// ������ ��������
uint8_t brightnessShift = 0;
uint8_t brightness[9] = {
    0b11111111,
    0b10000000,
    0b00011001,

    0b11111111,
    0b10011001,
    0b00110011,

    0b11111111,
    0b10110011,
    0b01001101
};
uint8_t brightnessMask = 0b00000001;

typedef struct pinAdreses_t {
    uint8_t *port;
    uint8_t pin;
} pinAdreses;
pinAdreses ledAdreses[11];
// ������ ������� ������� ������
uint8_t ledAdresesLen = sizeof(ledAdreses) / sizeof(pinAdreses);
pinAdreses centralLed;

// ������� ������ ��������� ������
uint8_t getInputValue(uint8_t port) {
  return (port & 0b00111000) >> 3;
}

// ��������������� ������� ��� ���������� ���������
void setBrightness(pinAdreses *ledAdr, uint8_t brightness) {
  if (brightness & brightnessMask) {
    *ledAdr->port |= ledAdr->pin;
  } else {
    *ledAdr->port &= ~ledAdr->pin;
  }
}

uint8_t counter = 0;

void main(void) {
  // �������������� ������ �������� ������� ����� ����� � ����� ��� ����������� ������� ���� ������� ����� � 1
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
  /*ledAdreses[11].port = &PORTB;
  ledAdreses[11].pin = (1 << 7);*/

  centralLed.port = &PORTB;
  centralLed.pin = (1 << 7);

  // ������������� � �������������� ����� ����� ������
  DDRA = 0b011;
  PORTA = 0b100;
  DDRB = 0b11111111;
  PORTB = 0b00000000;
  DDRD = 0b1000111;
  PORTD = 0b0111000;

  // ��������� ��������
  TIFR = 0x0; // ����� �������� ������
  TIMSK = 0b10000010; // ���������� ���������� �� ������������ �������0 � �� ������������ �������1
  sei();
  // ��������� �������0
  TCCR0A = 0b00000000; // �������� �������
  //TCCR0B = 0b00000000;
  //TCNT0 = 0x0; // ��������� ���������� �������� ��������
  TCNT0 = 0;
  // ��������� �������1
  TCCR1A = 0b00000000; // �������� �������
  //TCCR1B = 0b00000000;
  TCNT1 = 0b0;

  TCCR0B = 0b00000100; // ������ �������0 � ��������� 256

  //TCCR1B = 0b00000010; // ������ �������1 � ��������� 8
  //TCCR1B = 0b00000100; // ������ �������1 � ��������� 256
  TCCR1B = 0b00000011; // ������ �������1 � ��������� 64
}

ISR(TIM0_OVF) {
  uint8_t i;
  // ��������� ������� ��������� �������� � �����
  uint8_t buff = counter;

  // �������� �����
  for (i = 0; i < 3; i++) {
    setBrightness(&ledAdreses[(buff + 3 - i) % ledAdresesLen], brightness[i + brightnessShift * 3]);
  }
  setBrightness(&centralLed, 0xf0);

  // ���������
  TCNT0 = ~brightnessMask;
  brightnessMask = (brightnessMask >> 1) | (brightnessMask << 7);
}

ISR(TIM1_OVF) {
  // ��������� ��������� ������
  settings = getInputValue(PIND);
  // ���������� ��������� �������� �������� � ����������� � ��������� ���������
  TCNT1 = speed[settingsCodes[settings] & 0b011];
  // ��������� ��������� �������
  brightnessShift = settingsCodes[settings] >> 2;

  // �������� �������
  if(++counter == ledAdresesLen) {
    counter = 0;
  }
  // ����� ���� ������ �� ��������� �������
  *(ledAdreses[counter % ledAdresesLen].port) &= ~ledAdreses[counter].pin;
}
