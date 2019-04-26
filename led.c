#include <tiny2313a.h>
#include <interrupt.h>
#include <stdint.h>

uint16_t velocity = 40000;
uint8_t velocityCounter = 0;
uint8_t inverse = 0;
uint8_t velocityChangePeriod = 100;
uint16_t a = 5000;
void acceleration() {
  if(0xffff - velocity - a <= 1024 && !inverse) {
    inverse = !inverse;
    velocity = 0xfbff;
  } else if(velocity <= a && inverse) {
    inverse = !inverse;
    velocity = 0b0;
  } else {
    velocity = (-2 * inverse + 1) * a + velocity;
  }
}

typedef struct pinAddresses_t {
    uint8_t *port;
    uint8_t pin;
} pinAddresses;
pinAddresses ledAddresses[12];
// Размер массива адресов диодов
uint8_t ledAddressesLen = sizeof(ledAddresses) / sizeof(pinAddresses);
// Константа определяющая сдвиг массива яркостей относительно массива адресов
uint8_t ledAddressShift = 0;

//Массив яркостей светодиодов
uint8_t ledBrightnessRef[ledAddressesLen] = {
    0b00011001,
    0b10000000,
    0b11111111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b00000000,
    0b00000000,
    0b00000000
};
uint8_t ledBrightness[ledAddressesLen] = {
    0b00011001,
    0b10000000,
    0b11111111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b00000000,
    0b00000000,
    0b00000000
};
uint8_t brightnessMask = 0b00000001;
// 0 - не изменять, 1 - увеличивать, 2 - уменьщать
uint8_t brBrightDir[ledAddressesLen] = {
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	1,
	0,
	0,
	0
};
uint8_t brChangePeriod = 100;
uint8_t brCounter = 0;
uint8_t brChangeSteps = 17;
uint8_t brChangeStep = 15; // 255 / brChangeSteps
uint8_t brChangeStepI = 0;

void brChange() {
  uint8_t i;
  uint16_t curStepVal;
  for (i = 0; i < ledAddressesLen; i++) {
    if (brBrightDir[i]) {
      curStepVal = ledBrightnessRef[i] + brChangeStepI * brChangeStep;
      if (brBrightDir[i] == 1) {
        if (curStepVal <= 255) {
          ledBrightness[i] = curStepVal;
        } else if (curStepVal <= 510) {
          ledBrightness[i] = 510 - curStepVal;
        } else if (curStepVal <= 510 + ledBrightnessRef[i]) {
          ledBrightness[i] = curStepVal - 510;
        }
      } else {
        if (curStepVal <= 2 * ledBrightnessRef[i]) {
          ledBrightness[i] = 2 * ledBrightnessRef[i] - curStepVal;
        } else if (curStepVal <= 255 + 2 * ledBrightnessRef[i]) {
          ledBrightness[i] = curStepVal - 2 * ledBrightnessRef[i];
        } else if (curStepVal <= 510 + ledBrightnessRef[i]) {
          ledBrightness[i] = 510 + 2 * ledBrightnessRef[i] - curStepVal;
        }
      }
    }
  }

  if (++brChangeStepI == 2 * brChangeSteps) {
    brChangeStepI = 0;
  }
}

// Вспомогательная функция для реализации модуляции
void setBrightness(pinAddresses *ledAdr, uint8_t brightness) {
  if (brightness & brightnessMask) {
    *ledAdr->port |= ledAdr->pin;
  } else {
    *ledAdr->port &= ~ledAdr->pin;
  }
}

void main(void) {
  // Инициализируем массив структур ханящих адрес порта и маску для выставление нужного бита данного порта в 1
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

  // Конфигурируем и инициализируем порты ввода вывода
  DDRA = 0b011;
  PORTA = 0b100;
  DDRB = 0b11111111;
  PORTB = 0b00000000;
  DDRD = 0b1000111;
  PORTD = 0b0111000;

  // Настройка таймеров
  TIFR = 0x0; // сброс регистра флагов
  TIMSK = 0b10000010; // Разраешаем прерывания по переполнению таймера0 и по переполнению таймера1
  sei();
  // Настройка таймера0
  TCCR0A = 0b00000000; // настрока таймера
  //TCCR0B = 0b00000000;
  //TCNT0 = 0x0; // установка начального значения счетчика
  TCNT0 = 0;
  // Настройка таймера1
  TCCR1A = 0b00000000; // настрока таймера
  //TCCR1B = 0b00000000;
  TCNT1 = velocity;

  //TCCR0B = 0b00000100; // Запуск таймера0 с делителем 256
  TCCR0B = 0b00000011; // Запуск таймера0 с делителем 64

  //TCCR1B = 0b00000010; // Запуск таймера1 с делителем 8
  //TCCR1B = 0b00000100; // Запуск таймера1 с делителем 256
  TCCR1B = 0b00000011; // Запуск таймера1 с делителем 64
}

ISR(TIM0_OVF) {
  uint8_t i;
  uint8_t adrShift = ledAddressesLen * inverse + (1 - 2 * inverse) * ledAddressShift;
  uint8_t bShift = ledAddressesLen - adrShift;
  // Зажигаем диоды
  for (i = 0; i < adrShift; i++) {
    setBrightness(&ledAddresses[i], ledBrightness[(ledAddressesLen - 1) * inverse + (1 - 2 * inverse) * (i + bShift)]);
  }
  for (i = adrShift; i < ledAddressesLen; i++) {
    setBrightness(&ledAddresses[i], ledBrightness[(ledAddressesLen - 1) * inverse + (1 - 2 * inverse) * (i - adrShift)]);
  }
  // Изменяем скорость и яркость
  if (brightnessMask == 0b00000001) {
    if (++velocityCounter == velocityChangePeriod) {
      velocityCounter = 0;
      acceleration();
    }
	  if (++brCounter == brChangePeriod) {
      brCounter = 0;
      brChange();
	  }
	}
  // Модуляция
  TCNT0 = ~brightnessMask;
  brightnessMask = (brightnessMask >> 1) | (brightnessMask << 7);
}

ISR(TIM1_OVF) {
  TCNT1 = velocity;
  // Обнуляем счетчик
  if (++ledAddressShift == ledAddressesLen) {
    ledAddressShift = 0;
  }
}
