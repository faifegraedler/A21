#include "main.h"
#include "io.h"

#include <stdbool.h>

TIM_HandleTypeDef htim16;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void runInkrementSensorCheck();

// Inkremente Counter
volatile uint32_t nInkremente = 0;

// Interrupt Counter
volatile uint32_t iCounter = 0;

/*
 * Statuswort, gibt an in welchem Zustand sich die Anlage befindet
 * 0 = Initial, kein Werkstück eingelegt, Band steht still.
 * 1 = Werkstück am Bandanfang erkannt und exakt positioniert.
 * 2 = Band dreht vorwärts, bis Position erkannt ist
*/
volatile uint8_t status = 0;

// Flag, ob der Lichtsensor ein schwarzes Inkrement erkennt.
volatile uint8_t sensorSchwarz = 0;

// Flag, ob gerade ein WS sich im Sensorbereich am Bandanfang befindet
volatile uint8_t anfangBelegt = 0;

// Flag, ob gerade ein WS sich im Sensorbereich am Bandende befindet
volatile uint8_t endeBelegt = 0;

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
    // Prüfen, ob Positionssensor inkrementiert wird
    runInkrementSensorCheck();

    // State Machine
    if (status == 0)
    {
      // Prüfen, ob der Endlagesensor am Bandanfang aktiv ist
      if (HAL_GPIO_ReadPin(Endlage_Bandanfang_GPIO_Port,Endlage_Bandanfang_Pin)==1)
      {
        // Werkstück am Bandanfang eingelegt
        HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 1);
        // Fahre 0,5 Sekunden vor
        HAL_Delay(500);
        // Halte Förderband an
        HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 0);

        // Fahre zurück bis zum Endlagesensor am Bandanfang
        // ToDo: Rückwärtsfahrt aktivieren
        while (HAL_GPIO_ReadPin(Endlage_Bandanfang_GPIO_Port,Endlage_Bandanfang_Pin)==0)
        {
          // Verweile hier wenn Endlagesensor am Bandanfang nicht HIGH ist
          HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, 1);
        }
        // Anhalten
        HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, 0);

        // Status auf 1 wechseln
        status = 1;
      }
    }
    if (status == 1)
    {
      // Inkremente Zähler auf 0 setzen
      nInkremente = 0;
      // Förderband vorwärts drehen
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 1);
      // Status auf 2 wechseln
      status = 2;
    }
    if (status == 2)
    {
      uint8_t mittelposition = 25/*ToDo: Wert ermitteln (Tipp: Werte zwischen 0 - 50 ausprobieren)*/;
      if (nInkremente >= mittelposition)
      {
        // Förderband anhalten und in Status 3 wechseln
        status = 3;
      }
    }
    if (status == 3)
    {
      // ToDo: Implementieren
      HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, 0);
      HAL_Delay(5000);
      status = 4;
    }
    if (status == 4) {
      while (HAL_GPIO_ReadPin(Endlage_Bandende_GPIO_Port,Endlage_Bandende_Pin)==0) {
        HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 1);
      }
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 0);
      HAL_Delay(2000);
      status = 5;
    }
    if (status == 5) {
      while (HAL_GPIO_ReadPin(Endlage_Bandanfang_GPIO_Port,Endlage_Bandanfang_Pin)==0) {
        HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, 1);
      }
      HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, 0);
      HAL_Delay(2000);
    }

  }

}

void runInkrementSensorCheck() {
  if(HAL_GPIO_ReadPin(Inkrementalsensor_GPIO_Port,Inkrementalsensor_Pin)==1) {
    if(sensorSchwarz == 0) {
      // Flanke erkannt
      if(HAL_GPIO_ReadPin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin) == 1) {
        // Bei Vorwärtsfahrt hochzählen
        nInkremente++;
      }
      if(HAL_GPIO_ReadPin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin) == 1) {
        // Bei Rückwärtsfahrt runterzählen
        nInkremente++;
      }
    }
    sensorSchwarz = 1;
  }else {
    if(sensorSchwarz == 1) {
      // Flanke erkannt
      if(HAL_GPIO_ReadPin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin) == 1) {
        // Bei Vorwärtsfahrt hochzählen
        nInkremente++;
      }
      if(HAL_GPIO_ReadPin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin) == 1) {
        // Bei Rückwärtsfahrt runterzählen
        nInkremente++;
      }
    }
    sensorSchwarz = 0;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_TIM16;
  PeriphClkInit.Tim16ClockSelection = RCC_TIM16CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Rueckwaertsfahrt_GPIO_Port, Rueckwaertsfahrt_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Vorwaertsfahrt_Pin */
  GPIO_InitStruct.Pin = Vorwaertsfahrt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Vorwaertsfahrt_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Rueckwaertsfahrt_Pin */
  GPIO_InitStruct.Pin = Rueckwaertsfahrt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Rueckwaertsfahrt_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Endlage_Bandanfang_Pin */
  GPIO_InitStruct.Pin = Endlage_Bandanfang_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Endlage_Bandanfang_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Endlage_Bandende_Pin */
  GPIO_InitStruct.Pin = Endlage_Bandende_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Endlage_Bandende_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Inkrementalsensor_Pin */
  GPIO_InitStruct.Pin = Inkrementalsensor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Inkrementalsensor_GPIO_Port, &GPIO_InitStruct);

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
