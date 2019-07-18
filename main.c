/*This Code runs only on an 8 Bit Mikrocontroller for example ATtiny 13a or something comparable
*For specific adjustments use the appropriate Datasheed
*
*When you push the 'ON-Button' of your EBike and the Back Wheel stands in the first 1.5 Seconds still, the selectet Mode 
*is 'Original', this means the Speedlimit is the same as befor.
*If the Wheel is spinning arount in the first 1.5 Seconds, the selectet Mode is 'Tuned', so the Speedlimit is double as original.
*When the Bike stands still longer then 10 Seconds after a Trip in Mode 'Tune', the difference will be submitted at a high frequency
*on the Port of the Speedsensor.
*
*The code is very simple and therefore self-explanatory
*Do not use in Traffic
*/


#define F_CPU 128000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define TaktPin 0					
#define SignalPin 3					
#define MAXTIME 370					
#define MAXLOW 60
#define MIN_HIGH 20					
#define MAKE_DISTANCE 10000

uint16_t Counter = 0;
uint16_t Counter_dist = 0;
volatile unsigned long millis = 0;
unsigned long dist_st = 0;
unsigned long Time_befor = 0;
unsigned long Time_Low = 0;
int Time = 0;
uint8_t dist_switch = 0;

struct
{
	unsigned Modi:1;
	unsigned Pegel:1;
	unsigned switch_dist:1;
}b_switch;

void Time_HIGH(void);


int main(void)
{
	TCCR0A = (1<<WGM01);
	TCCR0B |= (1<<CS00);
	
	OCR0A = 125;
	TIMSK0 |= (1<<OCIE0A);
	ACSR |= (1<<ACD);
	
	sei();
	
	DDRB |= (1<<TaktPin);
	PORTB |= (1<<TaktPin
	DDRB &= ~(1<<SignalPin);						
	PORTB |= (1<<SignalPin);						
	
	_delay_ms(20);
	
	b_switch.Modi = 0;
	b_switch.Pegel = 0;
	b_switch.switch_dist = 0;
	
	while (millis < 1500)
	{
		static uint8_t Flank = 0;

		if((PINB & (1<<SignalPin))&&(Flank == 0))
		Flank = 1;
		
		else if((!(PINB & (1<<SignalPin)))&&(Flank == 1))
		Flank = 2;

		else if((PINB & (1<<SignalPin))&&(Flank == 2))
		b_switch.Modi=1;
	}
	
	while(1)
	{
		Time_HIGH();
		
		if(b_switch.Modi==0){
			if (PINB & (1<<SignalPin))
			{
				PORTB |= (1<<TaktPin);
			}
			else if((millis - Time_Low) < MAXLOW)
			{
				PORTB &= ~(1<<TaktPin);
			}
			else
			{
				PORTB |= (1<<TaktPin);
			}
		}
		
		else
		{
			if ((Time>MAXTIME)&&((Time_Low+MAKE_DISTANCE)>millis))
			{
				if (PINB & (1<<SignalPin))
				{
					PORTB |= (1<<TaktPin);
				}
				else if((millis - Time_Low) < MAXLOW)
				{
					PORTB &= ~(1<<TaktPin);
				}
				else
				{
					PORTB |= (1<<TaktPin);
				}
				dist_switch = 0;
			}
			
			else if(Time<=MAXTIME)
			{
				if ((Counter%2)&&(!(PINB & (1<<SignalPin)))&&(Time > MIN_HIGH))
				{
					PORTB &= ~(1<<TaktPin);
					
					if(b_switch.switch_dist == 0)
					{
						Counter_dist++;
						b_switch.switch_dist = 1;
					}
				}
				else
				{
					PORTB |= (1<<TaktPin);
					b_switch.switch_dist = 0;
				}
			}
			else if((Counter_dist>10)&&((Time_Low+MAKE_DISTANCE)<=millis))
			{
				if(!(dist_switch))
				{
					PORTB &= ~(1<<TaktPin);
					dist_st = millis;
					Counter_dist--;
					dist_switch = 1;
				}
				else if (((dist_st+4)<=millis)&&(dist_switch==1))
				{
					dist_switch = 2;
					PORTB |= (1<<TaktPin);
				}
				else if (((dist_st+20)<millis)&&(dist_switch==2))
				{
					dist_switch = 0;
					if(Counter_dist<=11)
					Counter_dist = 0;
				}
			}
		}

	}
}


void Time_HIGH(){
	
	if ((!(PINB & (1<<SignalPin)))&&(b_switch.Pegel==0))
	{
		b_switch.Pegel=1;
		Time=(int) millis-Time_befor;
		Time_Low = millis;
	}
	
	if ((PINB & (1<<SignalPin))&&(b_switch.Pegel))
	{
		b_switch.Pegel=0;
		Time_befor = millis;
		Counter++;
	}
}

ISR(TIM0_COMPA_vect)
{
	millis++;
}


