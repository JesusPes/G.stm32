/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

/*

	Keil uVision4 
	ARM 32bit Cortex-M3 Microcnotroller 36Mhz(STM32F101C6)
	
	To control CM6631 USB2.0 High speed True HD Audio Processor

	tool:ULINK2

*/

#include <stdio.h>
#include <string.h>
#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions */




#define dprint(...)  //printf(__VA_ARGS__)

/* Input Pin define */
#define PWR_SW_A6 	GPIO_Pin_6
#define SW_1_A7 	GPIO_Pin_7


#define DSD64_128_A2 GPIO_Pin_2 
#define SF_0_B7 GPIO_Pin_7 
#define SF_1_B8 GPIO_Pin_8 
#define SF_2_B9 GPIO_Pin_9 
#define PCM_DSD_B5 GPIO_Pin_5 

/* Output Pin define */

#define LED0_B0 GPIO_Pin_0
#define LED1_B1 GPIO_Pin_1
#define LED2_B2 GPIO_Pin_2
#define LED3_B10 GPIO_Pin_10

#define LED4_B12 GPIO_Pin_12
#define LED5_B13 GPIO_Pin_13
#define LED6_B14 GPIO_Pin_14
#define LED7_B15 GPIO_Pin_15

#define USB_RST_B6  GPIO_Pin_6
#define MUTE_RY_A9 GPIO_Pin_9
#define MUTE_A3  GPIO_Pin_3
#define SEL_ANA_A8 GPIO_Pin_8
/*
	when gpio is connected to input(gate pin) of Si2301ds
	p-channel 1.25W 2.5v mosfet which is conected to switching regulator
	usb_rst voltage is down(3.3v->2v)
*/
#define ON_PWR_A11 GPIO_Pin_11  //pb
#define ON_PWR_A14 GPIO_Pin_14

#ifdef _itsnotworking_for_debug_printf
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever needed */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}
#endif

enum state_machine
{ 
	
	POWER_OFF, 
	USB_IN, 
	ANALOG_IN,
	IDLE_STATE
} ;

enum state_event
{ 
	
	INIT, 
	EVENT1, 
	EVENT2,
	IDLE
} ;

volatile unsigned long TimeTick;
volatile unsigned long TimeTick2;

unsigned char state;
unsigned char current_event;
unsigned char current_state;
unsigned long EventTimeTick;
unsigned char g_led=0;

GPIO_InitTypeDef  GPIO_InitStructure;





/* SysTick interrupt happens every 10 ms */
void SysTick_Handler (void) {
  TimeTick++;
	TimeTick2++;
}

void wait (int delay)  {                        /* wait function */
  int  d;

  for (d = 0; d < delay; d++);           /* only to delay for LED flashes */
}

void Delay (unsigned long tick) {
  unsigned long timetick;

  timetick = TimeTick;
  while ((TimeTick - timetick) < tick);
}

void GPIOA_Configure()
{

//GPIO_InitTypeDef GPIO_InitStructure;
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);


/*MUTE_RY */ /*MUTE */ /*SEL_ANA */  /*ON_PWR */
//GPIO_InitStructure.GPIO_Pin = MUTE_RY_A9 |MUTE_A3|SEL_ANA_A8  |ON_PWR_A11;

GPIO_InitStructure.GPIO_Pin =  MUTE_RY_A9 |MUTE_A3|SEL_ANA_A8 ;
//GPIO_InitStructure.GPIO_Pin |=ON_PWR_A11;  //problem !!!
GPIO_InitStructure.GPIO_Pin |=ON_PWR_A14;  //problem !!!

GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; //GPIO_Speed_50MHz ; //GPIO_Speed_50MHz;
GPIO_Init(GPIOA, &GPIO_InitStructure);
/*
	!! After GPIO_Init, usb_rst voltage is drop 3.3v  to 2v ,  its because ON_PWR_A11 !
*/


/* PWR_SW */ /* SW_1   */ 
GPIO_InitStructure.GPIO_Pin = DSD64_128_A2 | PWR_SW_A6 |SW_1_A7;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // GPIO_Mode_IN_FLOATING; 
//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;


GPIO_Init(GPIOA, &GPIO_InitStructure);


// it should be here otherwisw usb_rst voltage is down to 3.3v -> 2v  !!!!
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

}

void GPIOB_Configure (void) {

	/* Enable GPIOB clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PC6..PC9 as outputs push-pull, max speed 50 MHz */

	GPIO_InitStructure.GPIO_Pin =  USB_RST_B6 |GPIO_Pin_0 | GPIO_Pin_1  | GPIO_Pin_2 | GPIO_Pin_10 |GPIO_Pin_12 | GPIO_Pin_13  | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; //GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	#if 1
	 /*  SF_0, SF_1,SF_2, PCM_DSD input port*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7 | GPIO_Pin_8  | GPIO_Pin_9 | GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	#endif
	/* led oN and OFF */
	
	GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_2);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_10);
	
	GPIO_ResetBits(GPIOB,  GPIO_Pin_12);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_13);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_14);
	GPIO_ResetBits(GPIOB,  GPIO_Pin_15);
	Delay(100);
	
	GPIO_SetBits(GPIOB,  GPIO_Pin_0);
	GPIO_SetBits(GPIOB,  GPIO_Pin_1);
	GPIO_SetBits(GPIOB,  GPIO_Pin_2);
	GPIO_SetBits(GPIOB,  GPIO_Pin_10);
	
	GPIO_SetBits(GPIOB,  GPIO_Pin_12);
	GPIO_SetBits(GPIOB,  GPIO_Pin_13);
	GPIO_SetBits(GPIOB,  GPIO_Pin_14);
	GPIO_SetBits(GPIOB,  GPIO_Pin_15);

	
}



unsigned char get_state()
{
	
	unsigned char pwr_sw;
	unsigned char sw_1;
	pwr_sw = GPIO_ReadInputDataBit(GPIOA,PWR_SW_A6);
	sw_1 = GPIO_ReadInputDataBit(GPIOA,SW_1_A7);

	if(TimeTick2 > 100)
	{
		dprint(" 2 seconds elapsed \n");
		g_led++;
		if(g_led %2==0)
		{
			GPIO_SetBits(GPIOB, LED0_B0);
		}
		else
		{
			GPIO_ResetBits(GPIOB, LED0_B0);
		}
		TimeTick2=0;
	}	

	
	if(pwr_sw==0 && sw_1 == 1)	/* powr off state */
	{
		
		GPIO_SetBits(GPIOB,	LED4_B12);
		GPIO_ResetBits(GPIOB,	LED5_B13);
		return POWER_OFF;
	}
	else if(pwr_sw ==1 && sw_1 ==1 ) /* usb input state */
	{
		
		GPIO_ResetBits(GPIOB,	LED4_B12);
		GPIO_ResetBits(GPIOB,	LED5_B13);
		return USB_IN;
	}
	else if(pwr_sw==1 &&	sw_1 ==0 ) /* analog input state */
	{
		
		GPIO_ResetBits(GPIOB,	LED4_B12);
		GPIO_SetBits(GPIOB,	LED5_B13);
		return ANALOG_IN;
	}
	else
	{

	}
	
	return IDLE_STATE;
}

int main (void) {
	unsigned long cnt;
	//unsigned char buf[6];
	//unsigned char led_on=0;
	unsigned char pwr_sw;
	unsigned char sw_1;
	unsigned char sf_1;
	unsigned char sf_2;
	unsigned char sf_0;
	
	unsigned char pcm_dsd;
	unsigned char dsd64_128;
	
	/* SystemInit() is called by startup_stm32f10x.ld.s  
		  or you can use STM32F10X.s and call SetupClock() in here first
	*/
	
	/* SysTick event each 10 ms with input clock equal to 9MHz (HCLK/8) */
	SysTick_SetReload(90000/2);
	/* Enable the SysTick Counter */
	SysTick_CounterCmd(SysTick_Counter_Enable);
	/* Enable SysTick interrupt */
	SysTick_ITConfig(ENABLE);


	
	GPIOA_Configure();
  	GPIOB_Configure();
	//add_for_usart_in_the_main();

	/* RESET USB */
	GPIO_ResetBits(GPIOB,  USB_RST_B6);
	Delay(10);
	GPIO_SetBits(GPIOB,  USB_RST_B6);
	Delay(10);

	
	//printf(" main loop\n");
	
	GPIO_ResetBits(GPIOA, ON_PWR_A11); /*ON_PWR*/
	GPIO_SetBits(GPIOA, MUTE_A3); /*MUTE*/
	GPIO_ResetBits(GPIOA, MUTE_RY_A9); /*MUTE_RY*/
	GPIO_ResetBits(GPIOA, SEL_ANA_A8); /*SEL_ANA*/


	state=USB_IN;
	current_event=INIT;
	current_state=IDLE_STATE;


	while(1)
	{

		Delay(2);
		
		sf_0 = GPIO_ReadInputDataBit(GPIOB,SF_0_B7);
		sf_1 = GPIO_ReadInputDataBit(GPIOB,SF_1_B8);
		sf_2 = GPIO_ReadInputDataBit(GPIOB,SF_2_B9);
		pcm_dsd = GPIO_ReadInputDataBit(GPIOB,PCM_DSD_B5);
		dsd64_128 = GPIO_ReadInputDataBit(GPIOA,DSD64_128_A2);
		state=get_state();

		if(state != current_state )	
		{
			current_event=INIT;
			current_state=state;
		
		}
		switch(current_state)
		{

			case POWER_OFF:
				
				if(current_event==INIT)
				{
					dprint("power_off\n");
					
					GPIO_SetBits(GPIOA, MUTE_A3); /*MUTE*/
					GPIO_ResetBits(GPIOA, MUTE_RY_A9); /*MUTE_RY*/
					
					EventTimeTick = TimeTick +10	;
					current_event=EVENT1;
				}
				else if(current_event==EVENT1)
				{
					
					if(TimeTick > EventTimeTick) 
					{
					GPIO_SetBits(GPIOA, ON_PWR_A11); /*ON_PWR*/
					current_event=IDLE;
					}
				}
				else if(current_event == IDLE)
				{
					dprint("power_off - idle event\n");
				}
				break;

			case USB_IN:
				if(current_event==INIT)
				{
					dprint("usb_in event0\n");
					
					GPIO_SetBits(GPIOA, MUTE_RY_A9); /*MUTE_RY*/
					GPIO_SetBits(GPIOA, MUTE_A3); /*MUTE*/
					EventTimeTick = TimeTick +10	;
					current_event=EVENT1;
				}	
				else if(current_event==EVENT1)
				{
					dprint("usb_in event1\n");
					
					if(TimeTick > EventTimeTick) 
					{
						dprint("TimeTick_100m event1\n");
					
						GPIO_ResetBits(GPIOA, SEL_ANA_A8); /*SEL_ANA*/
						GPIO_ResetBits(GPIOA, ON_PWR_A11); /*ON_PWR*/
						EventTimeTick = TimeTick + 10;
						current_event=EVENT2;

					}
				}	
				else if(current_event ==EVENT2)
				{
					dprint("usb_in event2\n");

					if(TimeTick > EventTimeTick) 
					{
						dprint("TimeTick_100m event2\n");
						
						GPIO_ResetBits(GPIOA, MUTE_A3); /*MUTE*/
						current_event=IDLE;
					}
				}
				else if(current_event == IDLE)
				{
					dprint("usb in - idle event\n");
				}
				break;
				
			case ANALOG_IN:
				if(current_event==INIT)
				{
					dprint("analog_in\n");
					
					GPIO_SetBits(GPIOA, MUTE_RY_A9); /*MUTE_RY*/
					GPIO_SetBits(GPIOA, MUTE_A3); /*MUTE*/
					EventTimeTick = TimeTick +10	;
					current_event=EVENT1;
				} 
				else if(current_event==EVENT1)
				{
					dprint("analog event1\n");
					
					if(TimeTick > EventTimeTick) 
					{
						dprint("TimeTick_100m event1\n");
					
						GPIO_SetBits(GPIOA, SEL_ANA_A8); /*SEL_ANA*/
						GPIO_ResetBits(GPIOA, ON_PWR_A11); /*ON_PWR*/
						EventTimeTick = TimeTick + 10;
						current_event=EVENT2;
				
					}
				} 
				else if(current_event ==EVENT2)
				{
					dprint("analog event2\n");
					if(TimeTick > EventTimeTick) 
					{
						dprint("TimeTick_100m event2\n");
						
						GPIO_ResetBits(GPIOA, MUTE_A3); /*MUTE*/
						current_event=IDLE;
					}
				
				}
				else if(current_event == IDLE)
				{
					dprint("analog in - idle event\n");
				}
				break;

			default:
				dprint("default\n");
				break;

					
		}

	}






}


#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number */

  printf("\n\r Wrong parameter value detected on\r\n");
  printf("       file  %s\r\n", file);
  printf("       line  %d\r\n", line);
    
  /* Infinite loop */
  /* while (1)
  {
  } */
}
#endif

