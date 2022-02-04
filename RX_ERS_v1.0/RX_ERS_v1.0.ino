// https://kit.alexgyver.ru/tutorials/433mhz/
// https://habr.com/ru/post/388079/ - режимы сна
// SYN480R, крупный чип: 3.3-5.5V
//использовать BOD 2.7V
//РАБОЧАЯ ВЕРСИЯ! BOD = 2.9V фактически

// дефайны перед подключением библиотеки
#define G433_SPEED 5000   // скорость 100-10000 бит/с, по умолч. 2000 бит/с

#include <Gyver433.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define PWR_PIN PB0 // PB0 для управления питанием приемника
#define SW_PIN PB3 // PB3 для оптона
#define SLEEP_PERIOD WDTO_8S //засыпаем раз в 8 сек

uint32_t dataComm = 9039019901; // команда для приема
uint32_t data; // число для приема
unsigned long currentTime = 0;

//Gyver433_RX<пин, буфер> rx; // буфер: размер приёмного буфера в байтах. По умолч. 64
Gyver433_RX<PB2, 20> rx; //конфигурация для приема данных с приёмника //прерывания INT0 здесь

void setup() {
  // Настройка портов

  pinMode(SW_PIN, OUTPUT); // выход на оптон (управление кнопкой)
  pinMode(PWR_PIN, OUTPUT); // управление приемником

  bitWrite(PORTB, SW_PIN, LOW); // выключаю оптон
  bitWrite(PORTB, PWR_PIN, LOW); // выключаю приемник

  attachInterrupt(0, isr, CHANGE);   // взводим прерывания по CHANGE для приёма информации

}

// тикер вызывается в прерывании
void isr() {
  rx.tickISR();
}

void loop() {
  bitWrite(PORTB, PWR_PIN, HIGH); // включаю приемник
  if (rx.gotData()) { // gotData() вернёт количество удачно принятых байт
    if (rx.readData(data)) {  // если данные совпадают по размеру - ок
      if (data == dataComm) {  // если данные совпадают по СОДЕРЖАНИЮ
        bitWrite(PORTB, SW_PIN, HIGH); // включаю оптон // Ток 8mA
      }
    }
  }
  if (millis() - currentTime >= 1200) {
    bitWrite(PORTB, SW_PIN, LOW); // вЫключаю оптон
    bitWrite(PORTB, PWR_PIN, LOW); // вЫключаю приемник
    sleep(); //засыпаем!
  }
}

void sleep()
{
  ADCSRA &= ~_BV(ADEN); // отключить ADC; уменьшает энергопотребление
  wdt_enable(SLEEP_PERIOD); // установить таймер
  WDTCR |= _BV(WDIE); // включить прерывания от таймера; фикс для ATtiny85
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // установить режим сна "Power-down"
  sleep_enable(); // разрешить режим сна
  sei(); // включаем прерывания, иначе не проснемся
  sleep_cpu();  // собственно сон
  //---------------  THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP ---------------
  cli(); // запретить прерывания
  sleep_disable(); // запретить режим сна
  ADCSRA |= _BV(ADEN); // включить ADC
  sei(); // включить прерывания; иначе таймеры не будут работать
  currentTime = millis();
}
