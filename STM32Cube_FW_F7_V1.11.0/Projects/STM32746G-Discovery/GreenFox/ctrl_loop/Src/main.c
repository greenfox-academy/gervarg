/**
 ******************************************************************************
 * @file    Templates/Src/main.c
 * @author  MCD Application Team
 * @version V1.0.3
 * @date    22-April-2016
 * @brief   STM32F7xx HAL API Template project
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32746g_discovery_lcd.h"
#include <string.h>

/** @addtogroup STM32F7xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	uint32_t ovf;
	uint32_t prev;
	uint32_t last;
} input_capture_data_t;

TIM_HandleTypeDef Timer_IT;
TIM_HandleTypeDef Timer_PWM;
TIM_IC_InitTypeDef IC_conf;
TIM_OC_InitTypeDef sConfig;
GPIO_InitTypeDef gpio_init_structure;
GPIO_InitTypeDef gpio_IC_init;
GPIO_InitTypeDef gpio_adc_init;
UART_HandleTypeDef uart_handle;
p_ctrler_t P_controller;
pi_ctrler_t PI_controller;
pid_ctrler_t PID_controller;
ADC_HandleTypeDef adc_handle;
ADC_ChannelConfTypeDef adc_chconf;


volatile input_capture_data_t input_capture;
volatile int counter = 0;
volatile float T_period  = 0;
volatile float freq = 0;
volatile int freq_aver[3] = {0};
volatile float aver = 0;
volatile int adc_values[3] = {0};
volatile float adc_aver = 0;
volatile int adc_count = 0;




/* Private define ------------------------------------------------------------*/
#define IC_PERIOD 65535
#define ADC_NORMAL_VOLTAGE		8	// The voltage of the fully recharged battery
#define ADC_VOLTAGE_DIVIDER		4   /* The value measured on ADC of the fully charged
//#define USE_P_CTRLER

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
float get_freq();
static uint16_t ADC_Measure(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
	/* This project template calls firstly two functions in order to configure MPU feature
	 and to enable the CPU Cache, respectively MPU_Config() and CPU_CACHE_Enable().
	 These functions are provided as template implementation that User may integrate
	 in his application, to enhance the performance in case of use of AXI interface
	 with several masters. */

	/* Configure the MPU attributes as Write Through */
	MPU_Config();

	input_capture.ovf = 0;
	input_capture.last = 0;
	input_capture.prev = 0;

	/* Enable the CPU Cache */
	CPU_CACHE_Enable();



	/* STM32F7xx HAL library initialization:
	 - Configure the Flash ART accelerator on ITCM interface
	 - Configure the Systick to generate an interrupt each 1 msec
	 - Set NVIC Group Priority to 4
	 - Low Level Initialization
	 */
	HAL_Init();

	/* Configure the System clock to have a frequency of 216 MHz */
	SystemClock_Config();
	clock_enable_init();
	timer_pwm_init();
	gpio_pwm_init();
	gpio_adc_ini();
	uart_init();
	IC_init();

//	p_init(&P_controller);
//	P_controller.ref = 30;
//	P_controller.out_max = 100;
//	P_controller.out_min = 20;

//	pi_init(&PI_controller);
//	PI_controller.ref = 50;
//	PI_controller.out_max = 100;
//	PI_controller.out_min = 30;

	pid_init(&PID_controller);
	PID_controller.ref = 50;
	PID_controller.out_max = 100;
	PID_controller.out_min = 30;


	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);




	/* Add your application code here */
	BSP_LED_Init(LED_GREEN);
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	BSP_LCD_SelectLayer(0);
	BSP_LCD_DisplayOn();
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	printf("Input capture start\r\n");

	char buff[100];
	/* Infinite loop */
	while (1) {

		BSP_LED_Toggle(LED_GREEN);
		//TIM3->CCR1 = 450;



//		P_controller.sense = freq / 5;
//		TIM3-> CCR1 = (int) p_control(&P_controller);
//		PI_controller.sense = freq / 5;
//		TIM3-> CCR1 = (int) pi_control(&PI_controller);
		PID_controller.ref = ADC_Measure() / 41;

		PID_controller.sense = aver / 5;
		TIM3-> CCR1 = (int) pid_control(&PID_controller);
		sprintf(buff, "%f ", aver);
		HAL_UART_Transmit(&uart_handle, (uint8_t *) &buff, 15, 0xFFFF);
		BSP_LCD_ClearStringLine(0);
		BSP_LCD_DisplayStringAtLine(0, (uint8_t *) buff);


		sprintf(buff, "The duty cycle is %d%%", TIM3->CCR1);
		BSP_LCD_ClearStringLine(1);
		BSP_LCD_DisplayStringAtLine(1, (uint8_t *) buff);

		sprintf(buff, "ADC %d", ADC_Measure() / 41);
		BSP_LCD_ClearStringLine(2);
		BSP_LCD_DisplayStringAtLine(2, (uint8_t *) buff);
		HAL_Delay(100);
	}
}

static uint16_t ADC_Measure(void)
{
	HAL_ADC_Start(&adc_handle);
	if (HAL_ADC_PollForConversion(&adc_handle, 10) == HAL_OK)
		  {
		    // ADC conversion completed
			adc_values[adc_count] = HAL_ADC_GetValue(&adc_handle);
			adc_count++;
			if (adc_count == 4) {
				int sum = 0;
				for (int i = 0; i < adc_count - 1; i++) {
					sum += adc_values[i];
				}
				adc_aver = sum / (adc_count - 1);
				adc_count = 0;
			}
		    return adc_aver;
		  }
	return 1;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	input_capture.prev = input_capture.last;
	input_capture.last = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
	T_period = (float)((input_capture.ovf * IC_PERIOD +
							input_capture.last - input_capture.prev) * 0.001);

	freq = get_freq();
	freq_aver[counter] = freq;
	counter++;
	if (counter == 4){
		int sum = 0;
		for (int i = 0; i < counter - 1; i++) {
			sum += freq_aver[i];
		}
		aver = sum / (counter - 1);
		counter = 0;
	}
	input_capture.ovf = 0;


}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	input_capture.ovf++;
}

void set_pwm(void)
{
	TIM3-> CCR1 = P_controller.out;
}

float get_freq()
{
	return (1 / T_period) * 1000;
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 25
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/* activate the OverDrive to reach the 216 Mhz Frequency */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
		Error_Handler();
	}
}

PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
	HAL_UART_Transmit(&uart_handle, (uint8_t *) &ch, 1, 0xFFFF);

	return ch;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	/* User may add here some code to deal with this error */
	while (1) {
	}
}

/**
 * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
 * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
 *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
 * @param  None
 * @retval None
 */
static void MPU_Config(void) {
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x20010000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void) {
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
