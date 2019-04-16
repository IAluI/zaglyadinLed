#include <tiny2313a.h>
#include <interrupt.h>
#include <stdint.h>

uint16_t velocity = 0;
uint8_t velocityCounter = 0;
uint8_t inverse = 0;
uint8_t velocityChangePeriod = 366;
int16_t a = 5000;
void acceleration() {
  int16_t diff = velocity + a;
  if (diff >= 0xffff) {
    inverse = !inverse;
    a = -a;
    velocity = velocity + a;
  } else if (diff <= 0) {
    inverse = !inverse;
    a = -a;
    velocity = -diff;
  } else {
    velocity = diff;
  }
}

typedef struct pinAddresses_t {
    uint8_t *port;
    uint8_t pin;
} pinAddresses;
pinAddresses ledAddresses[12];
// ������ ������� ������� ������
uint8_t ledAddressesLen = sizeof(ledAddresses) / sizeof(pinAddresses);
// ��������� ������������ ����� ������� �������� ������������ ������� �������
uint8_t ledAddressShift = 0;

//������ �������� �����������
uint8_t ledBrightness[ledAddressesLen] = {
    0b11111111,
    0b10000000,
    0b00011001,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};
uint8_t brightnessMask = 0b00000001;

// ��������������� ������� ��� ���������� ���������
void setBrightness(pinAddresses *ledAdr, uint8_t brightnessIndex) {
  if (brightness[brightnessIndex] & brightnessMask) {
    *ledAdr->port |= ledAdr->pin;
  } else {
    *ledAdr->port &= ~ledAdr->pin;
  }
}

void main(void) {
  // �������������� ������ �������� ������� ����� ����� � ����� ��� ����������� ������� ���� ������� ����� � 1
  ledAddresses[0].port = &PORTD;
  ledAddresses[0].pin = (1 << 2);
  ledAddresses[1].port = &PORTA;
  ledAddresses[1].pin = (1 << 0);
  ledAddresses[2].port = &PORTA;
  ledAddresses[2].pin = (1 << 1);
  ledAddresses[3].port = &PORTD;
  ledAddresses[3].pin = (1 << 6);
  ledAddresses[4].port = &PORTB;
  ledAddresses[4].pin = (1 << 0);
  ledAddresses[5].port = &PORTB;
  ledAddresses[5].pin = (1 << 1);
  ledAddresses[6].port = &PORTB;
  ledAddresses[6].pin = (1 << 2);
  ledAddresses[7].port = &PORTB;
  ledAddresses[7].pin = (1 << 3);
  ledAddresses[8].port = &PORTB;
  ledAddresses[8].pin = (1 << 4);
  ledAddresses[9].port = &PORTB;
  ledAddresses[9].pin = (1 << 5);
  ledAddresses[10].port = &PORTB;
  ledAddresses[10].pin = (1 << 6);
  ledAddresses[11].port = &PORTB;
  ledAddresses[11].pin = (1 << 7);

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
  TCNT0 = ~brightnessMask;
  // ��������� �������1
  TCCR1A = 0b00000000; // �������� �������
  //TCCR1B = 0b00000000;
  TCNT1 = velocity;

  TCCR0B = 0b00000100; // ������ �������0 � ��������� 256

  //TCCR1B = 0b00000010; // ������ �������1 � ��������� 8
  //TCCR1B = 0b00000100; // ������ �������1 � ��������� 256
  TCCR1B = 0b00000011; // ������ �������1 � ��������� 64
}

ISR(TIM0_OVF) {
  uint8_t i;
  // ���������
  brightnessMask = (brightnessMask >> 1) | (brightnessMask << 7);
  TCNT0 = ~brightnessMask;
  // �������� �����
  uint8_t bShft = ledAddressesLen - ledAddressShift;
  for (i = 0; i < ledAddressShift; i++) {
    setBrightness(&ledAddresses[i], ledBrightness[ledAddressesLen * inverse + (1 - 2 * inverse) * (i + bShft)]);
  }
  for (i = ledAddressShift; i < ledAddressesLen; i++) {
    setBrightness(&ledAddresses[i], ledBrightness[ledAddressesLen * inverse + (1 - 2 * inverse) * (i - ledAddressShift)]);
  }

  if (brightnessMask == 0b00000001) {
    if (++velocityCounter == velocityChangePeriod) {
      acceleration();
    }
  }
}

ISR(TIM1_OVF) {
  // �������� �������
  if (++ledAddressShift == ledAddressesLen) {
    ledAddressShift = 0;
  }
}