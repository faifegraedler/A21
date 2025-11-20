#include "main.h"
#include "io.h"

#include <stdbool.h>

TIM_HandleTypeDef htim16;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM16_Init(void);

// Werkstück Counter, zählt wie viele Werkstücke sich auf dem Band befinden
volatile uint8_t CounterWerkstueck = 0;

// Flag, ob sich gerade ein Werkstück im Sensorbereich am Bandanfang befindet
volatile bool BandanfangBelegt = 0;

// Flag, ob sich gerade ein Werkstück im Sensorbereich am Bandende befindet
volatile bool BandendeBelegt = 0;

// Interrupt Counter
volatile uint32_t iCounter = 0;

int main(void)
{
  /*
   * Reset
   * Zurücksetzen der gesamten Peripherie am µC
   * Initialisierung des Flash Interface
   * Initialisierung des SysTick
   */
  HAL_Init();
  /* Konfiguration der SystemClock */
  SystemClock_Config();
  /* Initialisierung der Peripherie */
  // Initialisierung der GPIOs
  MX_GPIO_Init();
  // Initialisierung des Timer 16
  MX_TIM16_Init();

  // Starten des Timer 16
  HAL_TIM_Base_Start_IT(&htim16);

  while (1)
  {
    // Prüfen, ob der Endlagesensor am Bandanfang aktiv ist
    if (HAL_GPIO_ReadPin(Endlage_Bandanfang_GPIO_Port,Endlage_Bandanfang_Pin)==1) {
      /*
       * Prüfen ob anfangBelegt == 0 ist. Falls ja, dann handelt
       * es sich um eine steigende Flanke am Sensor.
       */
      if (!BandanfangBelegt) {
        // Werkstück neu am Anfang, daher anfangBelegt setzen
        BandanfangBelegt = 1;
        // CounterWerkstueck eins hochzählen
        CounterWerkstueck++;
        // Interrupt Counter auf 0 setzen
        iCounter = 0;
      }
    } else {
      // Es befindet sich kein Werkstück im Anfangsbereich
      BandanfangBelegt = 0;
    }

    // Prüfen, ob der Endlagesensor am Bandende aktiv ist
    if (HAL_GPIO_ReadPin(Endlage_Bandende_GPIO_Port,Endlage_Bandende_Pin)==1) {
      // Es befindet sich ein Werkstück im Endbereich, endeBelegt auf 1 setzen
      BandendeBelegt = 1;
    } else {
      /*
       * Falls endeBelegt == 1 ist, dann hat ein Werkstück
       * den Sensorbereich am Bandende verlassen.
       * endeBelegt wird wieder auf 0 gesetzt
       * CounterWerkstueck wird eins heruntergezählt (falls CounterWerkstueck > 0 ist)
       * iCounter auf 0 zurücksetzen
       */
      if (BandendeBelegt) {
        BandendeBelegt = 0;
        if (CounterWerkstueck > 0) {
          CounterWerkstueck--;
          iCounter = 0;
        }
      }
    }

    /*
     * Falls sich ein Werkstück auf dem Band befindet (CounterWerkstueck > 0)
     * und kein Werkstück im Endbereich ist (BandendeBelegt == 0), dann
     * soll das Förderband vorwärts drehen
     */
    if (CounterWerkstueck > 0 && BandendeBelegt == 0) {
      // Förderband dreht vorwärts
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 1);
    } else {
      // Förderband anhalten
      HAL_GPIO_WritePin(Vorwaertsfahrt_GPIO_Port, Vorwaertsfahrt_Pin, 0);
    }
  }
}

// ISR der Timer
// Wird aufgerufen, wenn ein Timer des STM32 überläuft
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Es wird abgefragt, ob der Interrupt durch den Timer 16 ausgelöst wird
  if (htim == &htim16)
  {
    // Falls das Förderband vorwärts dreht, dann iCounter hochzählen
    // ToDo: Abfrage mit Logik implementieren
    if (HAL_GPIO_ReadPin(Vorwaertsfahrt_GPIO_Port,Vorwaertsfahrt_Pin)==1) {
      iCounter++;
    }

    // Initialisierungsfunktion des Timer 16 enthält die konfigurierten Überlaufzeiten
    // ToDo: Ausrechnen wie viele Überläufe für 5 Sekunden passieren müssen

    if(iCounter >500 /* Anzahl Überläufe */) {

      if(CounterWerkstueck > 0) {
        CounterWerkstueck = 0;
      }
      iCounter = 0;
    }
  }
}

// Initialisierung des Timers 16
static void MX_TIM16_Init(void)
{
  // 72 MHz Base Timer
  htim16.Instance = TIM16;
  // Prescaler
  // 72 MHz Timer wird nur jede 7200 Ticks erhöht
  // 72 MHz -> 10 kHz
  htim16.Init.Prescaler = 7200-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  // Counter Period
  // skalierter 10 kHz Timer läuft nur jede 100 Ticks über
  // 10 kHz -> 100 Hz
  // Interrupt wird alle 10 ms (100 Hz) ausgelöst
  htim16.Init.Period = 100 - 1;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
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

  /*Configure GPIO pins : LD2_Pin Vorwaertsfahrt_Pin */
  GPIO_InitStruct.Pin = Vorwaertsfahrt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Vorwaertsfahrt_GPIO_Port, &GPIO_InitStruct);

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
