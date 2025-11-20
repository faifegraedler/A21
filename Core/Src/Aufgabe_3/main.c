#include "main.h"
#include "io.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

TIM_HandleTypeDef htim16;

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  while (1)
  {
    // Prüfen, ob der Endlagesensor am Bandanfang aktiv ist
    if (HAL_GPIO_ReadPin(Endlage_Bandanfang_GPIO_Port,Endlage_Bandanfang_Pin)==1) {
      // Der Sensor ist aktiv, daher das Förderband vorwärts drehen
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 1);
    }

    // Prüfen, ob der Endlagesensor am Bandende aktiv ist
    if (HAL_GPIO_ReadPin(Endlage_Bandende_GPIO_Port,Endlage_Bandende_Pin)==1) {
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 0);
    }
  }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  //  Error_Handler();
  }

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

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LD2_Pin Vorwaertsfahrt_Pin */
  GPIO_InitStruct.Pin = Vorwaertsfahrt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Vorwaertsfahrt_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Endlage_Bandanfang_Pin */
  GPIO_InitStruct.Pin = Endlage_Bandanfang_Pin ;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Endlage_Bandanfang_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Endlage_Bandende_Pin */
  GPIO_InitStruct.Pin = Endlage_Bandende_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT ;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Endlage_Bandende_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
