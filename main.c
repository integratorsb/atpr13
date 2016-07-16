/*
 * atpr13.c
 *
 * Created: 13.01.2016 9:30:03
 * Author : Integrator.sb
 */ 
/*
������������ ��������� ��������.
�������� ������������ �� 0 �� ����������� ������������ � ����� 10%.
�� ������ ������������ ����������� �������������� ������ �� ���� �������:	
	1) ����� ������� - �������� ������������ ��������� ���� �������� ����������
	2) ����� ��������������� - �������� ������������ ������������ �������� �������� ����������
	
	������� �������� �������� ���������� ����������� ���������
	��������
	�� �����				- 0% ��������
	������� ��������		- 10%
	�������					- 20%
	������/�������			- 30%
	������ ������ ��������	- 40%
	������ ��������			- 50%
	������					- 60%
	������/�������			- 70%
	������� ������ ��������	- 80%
	������� ��������		- 90%
	�������					- 100%


����������.
1. ��������� / ����������. ��������� ������ ������� ������ - ��� ��� ���� ��������,
� ������ ���������� ������� �������� �������� ������������ � EEPROM
2. ��������� ��������. �������� ��������� ������� ������ - ��������� ��������.
��� ������ ������� �������� �������� ����� ������������ �� 10%, 
���� �������� �������� ������������ �� ������ ������ ������ ������� � 10%.
3. ����� ������ ������. ������� ������� ��������� ���������� � ��������� ���������(��. �.1).
����� ������� �������� �� ������ ���������� ����������� � ��������� �����, 
��� ���� ���������� ���������, ��� ���� ���������� ������� ����� ������:
������� - �������
������� - ���������������
����� ������ ������ ��������� ������������� ��������� �������� �� ������.
��� ������ �� ���������� ������ � ���������� �������� ���������� ��������� ������� ������� �� ������, 
��� ���� ��������� ������ � ��������� ��������� � ����� ���������.
�� ����� ������ ��������� � ��������� ������ �������� �������� �����������.
*/
#define F_CPU 4800000UL 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>



#define PER_DELAY  1//����� ��������

#define P_BTN PORTB2	// ������
#define P_VCOMP PORTB1	// ���� ����������� ����������
#define P_TOUT PORTB0	// ����� ���������� �����������

#define powerOn()	(PORTB &= ~(1<<P_TOUT)) // �������� ������� ���. 0
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
 * \brief ������� ��������� �����������
 * 
 * \param st ��������� �����������
 * \param t	������� �������, ������������ �������� ������������ LED_PERIOD
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
	//// ���������������� ledSM
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

#define BTN_DELAY1 5	// ����������� ����� ������� ������
#define BTN_DELAY2 100	// ����� ������� �������
#define BTN_DELAY3 50	// ����� ����� ������� ��������
// ���� ��������� ������
#define BTN1_CLICK 1	//������� ��������� �������
#define BTN1_DCLICK 2	//������� �������
#define BTN1_LCLICK 3	//������ �������

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
 * \brief ������� ��������� ������, ������������ ������� ������� � ���������� ��� ��������������� �������.
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
			
			// ��� 1 �������� ������
			if(btnCheck())
			{
				// ���� ���� ���� ������ -  ���������� ����� � ��������� ������
				btnTmr = BTN_DELAY1;
				btnSt = BTN_DL1;
			}
			break;
		}
		case BTN_DL1:
		{
			// ��� 2 ���� ��������� ����� � ����� ��������� ������
			if(btnTmr == 0)
			{
				if(btnCheck())
				{
					// ���� ������ �� ��� ������ ��������� ������
					btnTmr = BTN_DELAY2;
					btnSt = BTN_CH2;
				}
				else
				{
					// ����� ��������� �� ��� 1
					btnSt = BTN_CH1;
				}
			}
			break;
		}
		case BTN_CH2:
		{
			// ��� 3 ���� ���������� ������
			if(btnCheck() == 0)
			{
				if(btnTmr == 0)
				{
					//������ �������
					btnSt = BTN_CH1;
					return BTN1_LCLICK;
				}
				else
				{
					// ������� �������
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
						// ������ �������
						return BTN1_CLICK;
					else if(btnCnt == 1)
					{
						// ������ �������
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
	// ���������� ���������� ��� ������ �����������
	//GIMSK = 0;	//
	//_delay_us(10);
	
	//if(PINB & 1<<P_VCOMP)
		//return;
	//PORTB ^=LEDP_GREEN;

	if(mode == 1)
	{
		//������� �����
		tmr = pgm_read_byte(&rmsCVal[v]);
		//tmr += PER_DELAY;
	}
	else if(mode == 2)
	{
		// ���� ����� �� ������ �������
		if(tmr)
			return;
		//��������������� �����	
		//������ �������	
		if((PINB & 1<<P_VCOMP) == 0)
			{
				// ������� ���-�� �������� � ���������� � ��������� ���. ��������

				if(cnt < v)
					powerOn();
				else
					powerOff();

				if(cnt	< 10)
					cnt++;
				else
					cnt = 0;;
				// ������ ����� �� ����� �������� �������� ���������� ����� 0
				// ��� ������ �� ������ ������������
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
	// ��������� ������
	PORTB = (1<<P_BTN) | (1<<P_TOUT)|(1<<P_VCOMP);
	DDRB =  (1<<P_TOUT) | LEDP_GREEN | LEDP_RED;
	// ���������� �� ��������� ��� ������ �� P_VCOMP
	GIMSK = 1<<PCIE;
	PCMSK = 1<<P_VCOMP;

	// ������� ������� 25600�� 
	TCCR0A = (1<<WGM01); // ����� CTC
	TCCR0B = (1<<CS00); // ��� ��������
	OCR0A = F_CPU/(100*256); // ������� ���� 50�� * 2 *������������ 256
	TIMSK0 = 1<<OCIE0A;// ���������� �� ����������

	v = 0;

	//mode = 0; // ������� ������: 0 - ��������, 1 - �������, 2 - ���������������, 100 - ���������
	sMode = 0;// ��������� �����: 0 - ��������, 1 - �������, 2 - ���������������
	
	ledCnt = 0xff;// ������� ������� ��� �����������
	
	sei();
    while (1) 
    {

		ledCnt--; 
		ledSM(v, ledCnt);
		
		btnCode = btnSM();
		
		if(mode == 0)
		{
			// ���������� ���������
			if(btnCode == BTN1_LCLICK)
			{
				// ���������
				
				mode = eeprom_read_byte(&eMode);
				if((mode != 1) && (mode != 2))
					mode = 1;	// �� ��������� ������� ������� �����
				v = eeprom_read_byte(&eSaveVal);
				if(v == 0)
					v = 10;		// �� ��������� �������� �������� 100%
			}
			else if(btnCode == BTN1_DCLICK)
			{
				// ���� � ��������� �����
				
				sMode = eeprom_read_byte(&eMode);
				mode = 100;
			}
		}

		else if((mode == 1) || (mode == 2))
		{
			// ���������� ��������� � ������� ������
			if(btnCode == BTN1_CLICK)
			{
				// ��������� ��������
				if(v < 10)
					v++;
				else
					v = 1;
			}
			else if(btnCode == BTN1_LCLICK)
			{
					// ����������
					if(v != eeprom_read_byte(&eSaveVal))
					{
						// ���������� �������� ��������
						eeprom_write_byte(&eSaveVal, v);
					}
					mode = 0;
					v = 0;
			}
		}
		else if(mode == 100)
		{
			// ���������� ��������� � ��������� ������
			
			if(btnCode == BTN1_CLICK)
			{
				// ��������� �������� ������ ������
				if(sMode == 1)
					sMode = 2;
				else if(sMode == 2)
					sMode = 1;
			}
			if(btnCode == BTN1_LCLICK)
			{
				// ����� �� ���������� ������
				// ���������� ��������
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



