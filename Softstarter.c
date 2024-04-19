/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Vetor_Rampa.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//GERAÇÃO E CONTROLE DE PULSO
#define N_BORDAS 2

#define PULSO_INICIAL_CCR 17400
#define LARGURA_PULSO_CCR 540

#define T_RAMPA_MAX 170
#define T_RAMPA_MIN 0

//INTERFACE SERIAL
#define TIMEOUT 100
#define T_MSG 40
#define T_CMD 4

//CONVERSÃO ANALOGICO DIGITAL
#define N_CONVERSOES 2

#define VREF 1.21
#define ADCMAX 4095.0

//SOBRECORRENTE
#define TAXA_DECAIMENTO 10
#define MULTIPLICADOR_CMD 10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//MAQUINA DE ESTADOS PRINCIPAL
typedef enum {PARADA = 0, SUBINDO, DESCENDO, BYPASS} Rampa_t;
static Rampa_t Rampa = PARADA;

//ESTADOS DE VETORES
enum {AD = 0, VINT};
enum {PRIMEIRO = 0, SEGUNDO, TERCEIRO, QUARTO};

//VARIAVEIS DE PULSO
typedef enum {SUBIDA = 0, DESCIDA} Borda_t;
static Borda_t Borda = SUBIDA;

static uint16_t novoPulso = PULSO_INICIAL_CCR;
static uint16_t Pulso [N_BORDAS] = {PULSO_INICIAL_CCR, (PULSO_INICIAL_CCR + LARGURA_PULSO_CCR)};

static uint16_t i;
static uint16_t angulo;

//CONVERSÃO ANALOGICO DIGITAL E SOBRECORRENTE
static volatile uint16_t ADC_Val[N_CONVERSOES] = { [ 0 ... (N_CONVERSOES - 1) ] = 0 };

static float I_Atual;
static float I_Nominal = 450.0;
static float I_LimiteRampa = 1.5;     // 150%
static float I_Max = 2.0;             // 200%

//TRANSMISSÃO E RECEPÇÃO SERIAL
static uint8_t comando[T_CMD] =  { [ 0 ... (T_CMD - 1) ] = 0 };;
static char msg[T_MSG] =  { [ 0 ... (T_MSG - 1) ] = 0 };;

//CALCULOS PARA TEMPO DE RAMPA
static uint16_t t_partida;
static int ARR_partida;

static float T_p;               // PERIODO DE PARTIDA
static float F_p;               // FREQUÊNCIA DE PARTIDA

static uint16_t t_descida;
static int ARR_descida;

static float T_d;              // PERIODO DE DESCIDA
static float F_d;              // FREQUÊNCIA DE PARTIDA

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

  htim3.Instance->CCMR1 |= TIM_CCMR1_OC1CE;  // DÁ CLEAR NO OC DE FORMA A NÃO ATRAPALHAR OS PULSOS

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim10);

  HAL_UART_Receive_DMA(&huart2, comando, T_CMD);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Val, N_CONVERSOES);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	UNUSED(htim);

	if( (htim->Instance == TIM3) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) )
	{
		Borda = (Borda == DESCIDA) ? SUBIDA : DESCIDA;

		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (Pulso[Borda]));
	}
	else
	{
		__NOP();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	UNUSED(htim);

	if(htim->Instance == TIM2)
	{

		if(Rampa == SUBINDO && i <= T_RAMPA_MAX )
		{

			HAL_GPIO_WritePin(Emergencia_GPIO_Port, Emergencia_Pin, GPIO_PIN_RESET);

			Pulso[SUBIDA] = Rampa_SoftStarter[i];
			Pulso[DESCIDA] = (Pulso[SUBIDA] + LARGURA_PULSO_CCR);

			strcpy(msg, "RDS\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
			
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

			if(I_Atual<(I_Nominal*I_LimiteRampa))
			{
				i++;
			}
			else
			{
				i--;

				strcpy(msg, "SCR\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);
			}

			if(i == T_RAMPA_MAX)
			{
				i = T_RAMPA_MAX;

				Rampa = BYPASS;

				htim3.Instance->CCMR1 &=~ TIM_CCMR1_OC1CE;
			}
		}

		if(Rampa == DESCENDO && i >= T_RAMPA_MIN)
		{

			HAL_GPIO_WritePin(Bypass_GPIO_Port, Bypass_Pin, GPIO_PIN_RESET);

			Pulso[SUBIDA]= Rampa_SoftStarter[i];
			Pulso[DESCIDA] = (Pulso[SUBIDA] + LARGURA_PULSO_CCR);

			strcpy(msg, "RDD\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

			if(I_Atual<(I_Nominal*I_LimiteRampa))
			{
				i--;
			}
			else
			{
				i = i - TAXA_DECAIMENTO;

				strcpy(msg, "SCR\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);
			}

			if(i == T_RAMPA_MIN)
			{
				i = T_RAMPA_MIN;

				Rampa = PARADA;

				HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);

				htim3.Instance->CCMR1 |= TIM_CCMR1_OC1CE;
			}
		}

		if(Rampa == PARADA)
		{
			i = T_RAMPA_MIN;

			Pulso[SUBIDA]= Rampa_SoftStarter[i];
			Pulso[DESCIDA] = (Pulso[SUBIDA] + LARGURA_PULSO_CCR);

			HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);

			htim3.Instance->CCMR1 |= TIM_CCMR1_OC1CE;

			strcpy(msg, "OFF\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);
		}

		if(Rampa == BYPASS)
		{
			i = T_RAMPA_MAX;

			htim3.Instance->CCMR1 &=~ TIM_CCMR1_OC1CE;

			HAL_GPIO_WritePin(Bypass_GPIO_Port, Bypass_Pin, GPIO_PIN_SET);

			strcpy(msg, "ON\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);
		}

	}
	
	if(htim->Instance == TIM10)
	{
		  HAL_ADC_Start_IT(&hadc1);

		  switch(Rampa)
		  {

		  	  case SUBINDO:

		  	  case DESCENDO:

		  		 angulo = i;

		  		sprintf(msg, "%i\n", angulo);
		  		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

		  		  	  break;

		  	  case BYPASS:

		  		angulo = 180;

		  		sprintf(msg, "%i\n", angulo);
		  		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

		  			break;

		  	  case PARADA:

		  		angulo = 0;

		  		sprintf(msg, "%i\n", angulo);
		  		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

		  			break;

		  	  default:

		  		__NOP();

		  			break;
		  }
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	UNUSED(huart);

		switch(comando[PRIMEIRO])
		{

			case 'L':

				if(Rampa == PARADA && t_partida != 0)
				{
					Rampa = SUBINDO;

					HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);

					__HAL_TIM_SET_AUTORELOAD(&htim2, ARR_partida);

				}

					break;

			case 'D':

				if(Rampa == BYPASS && t_descida != 0)
				{
					Rampa = DESCENDO;

					HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);

					__HAL_TIM_SET_AUTORELOAD(&htim2, ARR_descida);

				}

					break;

			case 'E':

				HAL_GPIO_WritePin(Emergencia_GPIO_Port, Emergencia_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Bypass_GPIO_Port, Bypass_Pin, GPIO_PIN_RESET);

				i = T_RAMPA_MIN;

				Rampa = PARADA;

					break;

			case 'l':

				t_partida = (comando[SEGUNDO] - '0') * MULTIPLICADOR_CMD +(comando[TERCEIRO] - '0');

				T_p = ((float)t_partida / 170);
				F_p =  1/T_p;
				ARR_partida = 10000 / F_p;

					break;

			case 'd':

				t_descida = (comando[SEGUNDO] - '0') * MULTIPLICADOR_CMD + (comando[TERCEIRO] - '0');

				T_d = ((float)t_descida / 170);
				F_d =  1/T_d;
				ARR_descida = 10000 / F_d;

					break;

			default:

				__NOP();

					break;

		}

		HAL_UART_Receive_DMA(&huart2, comando, T_CMD);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{

	UNUSED(hadc);

		float Vref = ( VREF * ADCMAX ) / ADC_Val[VINT];       // CALIBRA O ADC COM A TENSÃO INTERNA DA MCU

		ADC_Val[AD] = ((ADC_Val[AD] *( Vref * 1000)) / 4095); // CONVERTE PARA mA

		I_Atual = ( ADC_Val[AD]*0.2) + (I_Atual*0.8);         // FAZ UMA MÉDIA ENTRE V�?RIAS MEDIDAS DE CORRENTE

		sprintf(msg, "%.2fmA\r\n", I_Atual);

		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), TIMEOUT);

		if((I_Atual >= (I_Nominal*I_Max)) && (Rampa == SUBINDO || Rampa == BYPASS))
		{
			HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);

			htim3.Instance->CCMR1 |= TIM_CCMR1_OC1CE;

			HAL_GPIO_WritePin(Bypass_GPIO_Port, Bypass_Pin, GPIO_PIN_RESET);

			Rampa = PARADA;

			i = T_RAMPA_MIN;

			strcpy(msg, "CDG\n"); // UTILIZADO PARA MANDAR COMANDO SERIAL A IHM DO CUBE MONITOR
		}

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
