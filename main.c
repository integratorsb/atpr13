/*
 * atpr13.c
 *
 * Created: 13.01.2016 9:30:03
 * Author : Integrator.sb
 */ 
/*
Двухрежимный регулятор мощности.
Мощность регулируется от 0 до практически максимальной с шагом 10%.
По выбору пользователя регулировка осуществляется одиним из двух методов:	
	1) метод фазовый - мощность регулируется смещением фазы открытия симмистора
	2) метод числоимпульсный - мощность регулируется пропусканием периодов сетевого напряжения
	
	текущее значение мощности показывает двухцветный светодиод
	значения
	не горит				- 0% выключен
	зеленый мигающий		- 10%
	зеленый					- 20%
	желтый/зеленый			- 30%
	желтый быстро мигающий	- 40%
	желтый мигающий			- 50%
	желтый					- 60%
	желтый/красный			- 70%
	красный быстро мигающий	- 80%
	красный мигающий		- 90%
	красный					- 100%


Управление.
1. Включение / выключение. Одинарное долгое нажатие кнопки - вкл или выкл нагрузки,
в момент выключения текущее значение мощности запоминается в EEPROM
2. Установка мощности. Короткое одинарное нажатие кнопки - установка мощности.
При каждом нажатии выходная мощность будет увеличиватся на 10%, 
если значение мощности максимальное то отсчет пойдет заново начиная с 10%.
3. Выбор режима работы. Сначала следует перевести устройство в состояние выключено(см. п.1).
Далее двойным нажатием на кнопку устройство переводится в сервисный режим, 
при этом загорается светодиод, его цвет показывает текущий режим работы:
зеленый - фазовый
красный - числоимпульсный
Выбор режима работы устройсва осущесвляется одинарным нажатием на кнопку.
Для выхода из сервисного режима и сохранении настроек необходимо повторное двойное нажатие на кнопку, 
при этом светодиод гаснет и устройсво переходит в режим выключено.
Во время работы устройсва в сервисном режиме нагрузка остается отключенной.
*/
#define F_CPU 4800000UL 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>



#define PER_DELAY  1//время перехода

#define P_BTN PORTB2	// кнопка
#define P_VCOMP PORTB1	// вход компаратора напряжения
#define P_TOUT PORTB0	// вывод управления симмистором

#define powerOn()	(PORTB &= ~(1<<P_TOUT)) // активный уровень лог. 0
#define powerOff()	(PORTB |= 1<<P_TOUT)


const uint8_t PROGMEM rmsCVal[] = {
	//0, 52, 75, 94, 111, 128, 144, 161, 180, 203, 255
	255, 203, 180, 161, 144, 128, 111, 94, 75, 52, 0
};

//const uint8_t PROGMEM ledPerVal[][2] = {
	//{0b00000000,	0b00000000},
	//{0b00000000, 0b00001111},
	//{0b00000000, 0b11111111},
	//{0b00001111, 0b11111111},
	//{0b00000101, 0b00000101},
	//{0b00001111,	0b00001111},
	//{0b00001111,	0b00001111},
	//{0b11111111, 0b00001111},
	//{0b00000101, 0b00000000},
	//{0b00001111, 0b00000000},
	//{0b11111111, 0b00000000}
//};

uint8_t EEMEM eSaveVal, eMode;

uint8_t cnt, v, div, tmr;
uint8_t mode = 0;


#define LEDP PORTB
#define LEDP_RED (1<<PORTB3)
#define LEDP_GREEN (1<<PORTB4)

#define ledRedOn() LEDP |= LEDP_RED
#define ledRedOff() LEDP &= ~LEDP_RED
#define ledGreenOn() LEDP |= LEDP_GREEN
#define ledGreenOff() LEDP &= ~LEDP_GREEN

#define  LED_PERIOD 0xff
//uint8_t  ledC;


/**
 * \brief Функция обработки светодиодов
 * 
 * \param st состояние светодиодов
 * \param t	счетчик периода, максимальное значение определяется LED_PERIOD
 * 
 * \return void
 */
void ledSM(uint8_t st, uint8_t t)
{
	
	ledGreenOff();
	ledRedOff();
	
	switch(st)
	{	
		//case 0:
		//{
			//ledRedOff();
			//ledGreenOff();
			//break;
		//}
		case 1:
		{
			//ledRedOff();
			if(t < LED_PERIOD/2)
				ledGreenOn();
			//else
				//ledGreenOff();
			break;
		}
		case 2:
		{
			//ledRedOff();
			ledGreenOn();
			break;
		}
		case 3:
		{
			ledGreenOn();
			if(t < LED_PERIOD/2)
				ledRedOn();
			//else
				//ledRedOff();
			break;
		}
		case 4:
		{
			if(((t < LED_PERIOD/2) && (t > LED_PERIOD*3/8)) || ((t < LED_PERIOD/4) && (t > LED_PERIOD/8)))
			{
				ledRedOn();
				ledGreenOn();
			}
			//else
			//{
				//ledRedOff();
				//ledGreenOff();
			//}
			break;
		}
		case 5:
		{

			if(t < LED_PERIOD/2)
			{
				ledRedOn();
				ledGreenOn();
			}
			//else
			//{
				//ledRedOff();
				//ledGreenOff();
			//}
			break;
		}
		case 6:
		{
			ledRedOn();
			ledGreenOn();
			break;
		}
		case 7:
		{
			ledRedOn();
			if(t < LED_PERIOD/2)
				ledGreenOn();
			//else
				//ledGreenOff();
			break;
		}
		case 8:
		{
			//ledGreenOff();
			if(((t < LED_PERIOD/2) && (t > LED_PERIOD*3/8)) || ((t < LED_PERIOD/4) && (t > LED_PERIOD/8)))
			{
				ledRedOn();
			}
			//else
			//{
				//ledRedOff();
			//}
			break;
		}
		case 9:
		{
			//ledGreenOff();
			if(t < LED_PERIOD/2)
				ledRedOn();
			//else
				//ledRedOff();
			break;
		}
		case 10:
		{
			//ledGreenOff();
			ledRedOn();
			break;
		}
	};
	
}

//void ledSM2(uint8_t st, uint8_t t)
//{
	//if(ledC == 0)
		//ledC = 1;
	//
	//// оптимизированная ledSM
	//ledGreenOff();
	//ledRedOff();
	//
	 //if(t & pgm_read_byte(&ledPerVal[v][0]))
		//ledRedOn();
	 //if(t & pgm_read_byte(&ledPerVal[v][1]))
		//ledGreenOn();
	//ledC <<=1;
//}

#define  BTN_PIN PINB
#define  BTN_PORT P_BTN

#define BTN_DELAY1 5	// минимальное время нажатия кнопки
#define BTN_DELAY2 100	// время долгого нажатия
#define BTN_DELAY3 50	// время между двойным нажатием
// коды состояний кнопок
#define BTN1_CLICK 1	//обычное одинарное нажатие
#define BTN1_DCLICK 2	//двойное нажатие
#define BTN1_LCLICK 3	//долгое нажатие

#define btnCheck() !(BTN_PIN & (1<<BTN_PORT))

enum
{
	BTN_CH1,
	BTN_DL1,
	BTN_CH2,
	BTN_CH3
	};


uint8_t btnTmr, btnSt, btnCnt;

/**
 * \brief Функция обработки кнопки, обрабатывает сигналы нажатий и возвращает код соответсвующего события.
 *
 *
 * \return uint8_t
 */
uint8_t btnSM()
{
	
	switch(btnSt)
	{
		case BTN_CH1:
		{
			
			// шаг 1 проверка кнопок
			if(btnCheck())
			{
				// если хоть одна нажата -  запоминаем номер и переходим дальше
				btnTmr = BTN_DELAY1;
				btnSt = BTN_DL1;
			}
			break;
		}
		case BTN_DL1:
		{
			// шаг 2 ждем некоторое время и снова проверяем кнопки
			if(btnTmr == 0)
			{
				if(btnCheck())
				{
					// если кнопка всё ещё нажата переходим дальше
					btnTmr = BTN_DELAY2;
					btnSt = BTN_CH2;
				}
				else
				{
					// иначе переходим на шаг 1
					btnSt = BTN_CH1;
				}
			}
			break;
		}
		case BTN_CH2:
		{
			// шаг 3 ждем отпускания кнопки
			if(btnCheck() == 0)
			{
				if(btnTmr == 0)
				{
					//долгое нажатие
					btnSt = BTN_CH1;
					return BTN1_LCLICK;
				}
				else
				{
					// обычное нажатие
					btnSt = BTN_CH3;
					btnTmr = BTN_DELAY3;
				}
			}

			break;
		}
		case BTN_CH3:
		{
			if(btnTmr == 0)
			{
				
				btnSt = BTN_CH1;
				if(btnCheck())
				{
					btnCnt++;
				}
				else
				{
					if(btnCnt == 0)
						// первое нажатие
						return BTN1_CLICK;
					else if(btnCnt == 1)
					{
						// второе нажатие
						btnCnt = 0;
						return BTN1_DCLICK;
					}
				}
				
			}
			break;
		}
	}
	
	if(btnTmr > 0)
		btnTmr--;
	
	return 0;
}

ISR(PCINT0_vect)
{
	// прерывание вызывается при каждом полупериоде
	//GIMSK = 0;	//
	//_delay_us(10);
	
	//if(PINB & 1<<P_VCOMP)
		//return;
	//PORTB ^=LEDP_GREEN;

	if(mode == 1)
	{
		//фазовый режим
		tmr = pgm_read_byte(&rmsCVal[v]);
		//tmr += PER_DELAY;
	}
	else if(mode == 2)
	{
		// если пауза не прошла выходим
		if(tmr)
			return;
		//числоимпульсный режим	
		//начало периода	
		if((PINB & 1<<P_VCOMP) == 0)
			{
				// считаем кол-во периодов и сравниваем с значением вых. мощности

				if(cnt < v)
					powerOn();
				else
					powerOff();

				if(cnt	< 10)
					cnt++;
				else
					cnt = 0;;
				// делаем паузу на время перехода сетевого напряжения через 0
				// для защиты от ложных срабатываний
				tmr = 50;
			}
	}
	else
	{
		powerOff();
	}
	
}

ISR(TIM0_COMPA_vect)
{

		

		if(tmr > 0)
		{
			if(mode == 1)
				powerOff();	
			tmr--;
		}
		else
		{
			if(mode == 1)
				powerOn();
		}

}

int main(void)
{
	
	uint8_t  btnCode, sMode;
	uint8_t ledCnt;

	cli();
	// настройка портов
	PORTB = (1<<P_BTN) | (1<<P_TOUT)|(1<<P_VCOMP);
	DDRB =  (1<<P_TOUT) | LEDP_GREEN | LEDP_RED;
	// прерывание по изменению лог уровня на P_VCOMP
	GIMSK = 1<<PCIE;
	PCMSK = 1<<P_VCOMP;

	// частота таймера 25600Гц 
	TCCR0A = (1<<WGM01); // режим CTC
	TCCR0B = (1<<CS00); // без делителя
	OCR0A = F_CPU/(100*256); // частота сети 50Гц * 2 *дискретность 256
	TIMSK0 = 1<<OCIE0A;// прерывание по совпадению

	v = 0;

	//mode = 0; // рабочий работы: 0 - выключен, 1 - фазовый, 2 - числоимпульсный, 100 - сервисный
	sMode = 0;// сервисный режим: 0 - выключен, 1 - фазовый, 2 - числоимпульсный
	
	ledCnt = 0xff;// счетчик периода для светодиодов
	
	sei();
    while (1) 
    {

		ledCnt--; 
		ledSM(v, ledCnt);
		
		btnCode = btnSM();
		
		if(mode == 0)
		{
			// устройство выключено
			if(btnCode == BTN1_LCLICK)
			{
				// включение
				
				mode = eeprom_read_byte(&eMode);
				if((mode != 1) && (mode != 2))
					mode = 1;	// по умолчанию включен фазовый режим
				v = eeprom_read_byte(&eSaveVal);
				if(v == 0)
					v = 10;		// по умолчанию выходная мощность 100%
			}
			else if(btnCode == BTN1_DCLICK)
			{
				// вход в сервисный режим
				
				sMode = eeprom_read_byte(&eMode);
				mode = 100;
			}
		}

		else if((mode == 1) || (mode == 2))
		{
			// устройство находится в рабочем режиме
			if(btnCode == BTN1_CLICK)
			{
				// установка мощности
				if(v < 10)
					v++;
				else
					v = 1;
			}
			else if(btnCode == BTN1_LCLICK)
			{
					// выключение
					if(v != eeprom_read_byte(&eSaveVal))
					{
						// сохранение текущего значения
						eeprom_write_byte(&eSaveVal, v);
					}
					mode = 0;
					v = 0;
			}
		}
		else if(mode == 100)
		{
			// устройство находится в сервисном режиме
			
			if(btnCode == BTN1_CLICK)
			{
				// установка рабочего режима работы
				if(sMode == 1)
					sMode = 2;
				else if(sMode == 2)
					sMode = 1;
			}
			if(btnCode == BTN1_LCLICK)
			{
				// выход из сервисного режима
				// сохранение настроек
				eeprom_write_byte(&eMode, sMode);
				v = 0;
				sMode = 0;
				mode = 0;
			}
			
			if(sMode == 1)
				v = 2;
			else if(sMode == 2)
				v = 10;
		}
		
		_delay_ms(5);
	}

}



