/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions */



#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


void SetupClock (void)
{
  RCC_DeInit ();                        /* RCC system reset(for debug purpose)*/
  RCC_HSEConfig (RCC_HSE_ON);           /* Enable HSE */

  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

  RCC_HCLKConfig   (RCC_SYSCLK_Div1);   /* HCLK   = SYSCLK  */
  RCC_PCLK2Config  (RCC_HCLK_Div1);     /* PCLK2  = HCLK    */
  RCC_PCLK1Config  (RCC_HCLK_Div2);     /* PCLK1  = HCLK/2  */
  RCC_ADCCLKConfig (RCC_PCLK2_Div6);    /* ADCCLK = PCLK2/6 */

  FLASH_SetLatency(FLASH_Latency_2);    /* Flash 2 wait state */
  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

  /* PLLCLK = 8MHz * 9 = 72 MHz */
  RCC_PLLConfig (RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

  RCC_PLLCmd (ENABLE);                  /* Enable PLL */

  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

  /* Select PLL as system clock source */
  RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

  /* Wait till PLL is used as system clock source */
  while (RCC_GetSYSCLKSource() != 0x08);

  /* SysTick event each 10 ms with input clock equal to 9MHz (HCLK/8) */
  SysTick_SetReload(90000);

  /* Enable the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Enable);

  /* Enable SysTick interrupt */
  SysTick_ITConfig(ENABLE);
}

void USART_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;

/* USART1 configuration ------------------------------------------------------*/
  /* USART1 configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
    
  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);
}
/*******************************************************************************
* Function Name  : PUTCHAR_PROTOTYPE
* Description    : Retargets the C library printf function to the USART.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (u8) ch);

  /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {
  }

  return ch;
}
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART1 Tx (PA.09) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  /* Configure USART1 Rx (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void add_for_usart_in_the_main()
{
		GPIO_Configuration(); 
		USART_Configuration();
		/* Enable USART1 and GPIOA clock */
		//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 ,  ENABLE);

}
	

