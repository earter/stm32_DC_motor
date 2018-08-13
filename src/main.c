#include "stm32f1xx.h"
//#include "uart.h"
#include <string.h>

UART_HandleTypeDef uart;
TIM_HandleTypeDef tim4;
uint8_t value;

void uart_send_string(char* s)
{
 HAL_UART_Transmit(&uart, (uint8_t*)s, strlen(s), 1000);
}

void send_char(char c)
{
 HAL_UART_Transmit(&uart, (uint8_t*)&c, 1, 1000);
}

int __io_putchar(int ch)
{
 if (ch == '\n')
 send_char('\r');
 send_char(ch);
 return ch;
}


//float pid(float w_ref, float w_meas){
//	return w_pid;
//}


int main(void)
{
	SystemCoreClock = 8000000; // taktowanie zegara 8Mhz
	HAL_Init();

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_TIM4_CLK_ENABLE(); // timer4 on
	__HAL_RCC_USART2_CLK_ENABLE();

	// konfiguracja gpiob
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_AF_PP; //ustawienie trybu wyjscia na alterntive fcn
	gpio.Pin = GPIO_PIN_6|GPIO_PIN_7; // timer channel 1 and 2
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &gpio);

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_0|GPIO_PIN_1;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio);

	gpio.Mode = GPIO_MODE_IT_RISING;
	gpio.Pull = GPIO_PULLUP;
	gpio.Pin = GPIO_PIN_10|GPIO_PIN_13|GPIO_PIN_11|GPIO_PIN_12;
	HAL_GPIO_Init(GPIOC, &gpio);

	 gpio.Mode = GPIO_MODE_AF_PP;
	 gpio.Pin = GPIO_PIN_2;
	 gpio.Pull = GPIO_NOPULL;
	 gpio.Speed = GPIO_SPEED_FREQ_LOW;
	 HAL_GPIO_Init(GPIOA, &gpio);

	 gpio.Mode = GPIO_MODE_AF_INPUT;
	 gpio.Pin = GPIO_PIN_3;
	 HAL_GPIO_Init(GPIOA, &gpio);

	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); // enable interrupts

	// konfiguracja timera
	tim4.Instance = TIM4;
	tim4.Init.Period = 100 - 1; // rozdzielczosc PWM=100
	tim4.Init.Prescaler = 400 - 1; //f_pwm = 8MHz/400 = 20kHz
	tim4.Init.ClockDivision = 0;
	tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim4.Init.RepetitionCounter = 0;
	tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_PWM_Init(&tim4);

	// ustawienie kanalow w tryb pwm
	TIM_OC_InitTypeDef oc;
	oc.OCMode = TIM_OCMODE_PWM1;
	oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc.OCNPolarity = TIM_OCNPOLARITY_LOW;
	oc.OCFastMode = TIM_OCFAST_ENABLE;
	oc.OCIdleState = TIM_OCIDLESTATE_SET;
	oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	oc.Pulse =80;
	HAL_TIM_PWM_ConfigChannel(&tim4, &oc, TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&tim4, &oc, TIM_CHANNEL_2);

	// uart configuration
	 uart.Instance = USART2;
	 uart.Init.BaudRate = 115200;
	 uart.Init.WordLength = UART_WORDLENGTH_8B;
	 uart.Init.Parity = UART_PARITY_NONE;
	 uart.Init.StopBits = UART_STOPBITS_1;
	 uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	 uart.Init.OverSampling = UART_OVERSAMPLING_16;
	 uart.Init.Mode = UART_MODE_TX_RX;
	 HAL_UART_Init(&uart);

	// uruchomienie timera - w przerwaniu
//	HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_2);
	while (1) {
		 if (__HAL_UART_GET_FLAG(&uart, UART_FLAG_RXNE) == SET)
		 {
		 HAL_UART_Receive(&uart, &value, 1, 100);
		 printf("Odebrano: %c\r\n", value);
		 switch(value){
		 case 'a': // backward
		 		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
		 		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		 		HAL_TIM_PWM_Stop(&tim4, TIM_CHANNEL_2);
		 		HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_1);
		 		uart_send_string("backward\r\n");
		 		break;
		 case 'd':  //forward
		 		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		 		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		 		HAL_TIM_PWM_Stop(&tim4, TIM_CHANNEL_1);
		 		HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_2);
		 		uart_send_string("forward\r\n");
		 		break;
		 case 'w':  //fast
		 	    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		 	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		 	    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_1, 80);
		 	    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_2, 80);
		 	    uart_send_string("fast\r\n");
		 	   break;
		 case 's': //slow
		 	    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
		 	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		 	    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_1, 20);
		 	    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_2, 20);
		 	    uart_send_string("slow\r\n");
		 	   break;
		 	}
		 }
	}
}

