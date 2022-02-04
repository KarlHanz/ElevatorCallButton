// https://kit.alexgyver.ru/tutorials/433mhz/
// https://habr.com/ru/post/388079/ - режимы сна
// SYN115, маленький чип: 1.8-3.6V

#define G433_MEDIUM       // средняя синхронизация для SYN480R при отправке ЧАЩЕ 400мс (активно по умолчанию)
//#define G433_SLOW         // длинная синхронизация для SYN480R при отправке РЕЖЕ 400мс
#define G433_SPEED 5000   // скорость 100-10000 бит/с, по умолч. 2000 бит/с

#include <Gyver433.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>



Gyver433_TX<PB4> tx; // порт PB0 для отправки данных DATA на передатчик
#define SW_PIN PB2 // порт PB2 для подключения кнопки SW
#define PWR_PIN PB3 // порт PB1 для подключения питания на передатчик

uint32_t data = 9039019901; // число для отправки //9039019901
unsigned long currentTime = 0;
volatile bool sw_pcint = false;
bool Txdata = false;

void setup() {
  pinMode(SW_PIN, INPUT_PULLUP); // кнопкa SW
  pinMode(PWR_PIN, OUTPUT); // питание передатчика TX
  digitalWrite(PWR_PIN, LOW); // вЫключаю передатчик
}

void loop() {
  if (Txdata == false) sleep(); //засыпаем!
  if (sw_pcint) { // пробуждение от прерывания
    sw_pcint = false; //меняю флаг
    currentTime = millis();
    Txdata = true;
  }
  if (Txdata) { //выполнение основной программы TX
    digitalWrite(PWR_PIN, HIGH); // включаю передатчик
    tx.sendData(data); // отправка данных любого типа
    delay(100); // отправка 3 раз в сек
  }
  if (millis() - currentTime >= 18000) { // работаем 18 сек
    digitalWrite(PWR_PIN, LOW); // вЫключаю передатчик
    Txdata = false;
  }
}



ISR(PCINT0_vect) // Вектор (функция) прерывания PCINT, срабатывает при изменении состояния порта
{
  sw_pcint = true;
}

void sleep() //30mkA ток сна
{
  GIMSK = _BV(PCIE); // Включить Pin Change прерывания
  PCMSK |= _BV(SW_PIN); // PCINT3; включить
  ADCSRA &= ~_BV(ADEN); // отключить ADC; уменьшает энергопотребление
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // установить режим сна "Power-down"
  sleep_enable(); // разрешить режим сна
  sei(); // включаем прерывания, иначе не проснемся
  sleep_cpu();  // собственно сон
  //---------------  THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP ---------------
  cli(); // // отключить прерывания; для безопасного отключения PCINT3
  sleep_disable(); // запретить режим сна
  PCMSK &= ~_BV(SW_PIN); // PCINT3; отключить
  sei(); // включить прерывания; иначе таймеры не будут работать
}
