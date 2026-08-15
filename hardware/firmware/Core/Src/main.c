/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * Farooq Danesh Amooz- University of Tabriz - SSVEP/cVEP Robust Command Extraction Project
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;   /* drives cVEP update rate */
UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN 0 */

/* ============================================================
 * SECTION A: SSVEP frequencies (restored to real target values -
 * the 1-5 Hz debug values and the prescaler override from your
 * eye-test are REMOVED here. To redo that eye-test later, change
 * these 5 defines back to slow values temporarily - don't touch
 * the prescaler, changing these numbers alone is enough.)
 * ============================================================ */
#define HALFPERIOD_FORWARD   41667u   /* ~12 Hz, TIM2 CH1, PA0 */
#define HALFPERIOD_BACKWARD  62500u   /*  8 Hz,  TIM2 CH2, PA1 */
#define HALFPERIOD_LEFT      38462u   /* ~13 Hz, TIM2 CH3, PA2 */
#define HALFPERIOD_RIGHT     55556u   /* ~9 Hz,  TIM2 CH4, PA3 */
#define HALFPERIOD_STOP      50000u   /* 10 Hz,  TIM3 CH1, PA6 */

/* ============================================================
 * SECTION B: cVEP m-sequence stimulus
 * SCOPE NOTE: this generates a hardware-practical m-sequence +
 * cyclic-shift coded stimulus (per Sun et al., cited in your
 * uploaded paper's Future Developments section) - NOT the paper's
 * own independent white-noise-per-target design, and NOT any
 * decoding - decoding runs offline on your laptop, not here.
 * ============================================================ */
#define MSEQ_LENGTH   127u
static const uint8_t key_shift[5] = {0, 25, 50, 75, 100}; /* re-verify by
    cross-correlation before treating as final */
static uint8_t mseq[MSEQ_LENGTH];
static volatile uint32_t mseq_index = 0;

/* ============================================================
 * SECTION C: Mode + key selection (shared by both stimulus modes)
 * ============================================================ */
typedef enum { STIM_MODE_SSVEP = 0, STIM_MODE_CVEP = 1 } StimMode;
static volatile StimMode current_mode = STIM_MODE_SSVEP;

/* active_keys is a plain boolean array: 1 = key active, 0 = key off.
 * Order: Forward, Backward, Left, Right, STOP - same order everywhere
 * in this file and in your earlier netlist/pin table. */
static uint8_t active_keys[5] = {1, 1, 1, 1, 1};

static GPIO_TypeDef* key_port[5] = {GPIOA, GPIOA, GPIOA, GPIOA, GPIOA};
static const uint16_t key_pin[5] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_6
};

/* ---- Diagnostic voltmeter settings (unchanged from before) ---- */
#define TEST_SUPPLY_V   5.0f
#define TEST_RESISTOR_OHM  130.0f
float v_node;
float current_mA;

/* ============================================================
 * SECTION D: Photoresistor flicker-frequency verification
 * PURPOSE: measure the ACTUAL optical flicker frequency of whichever
 * key the photoresistor is physically aimed at, independent of the
 * frequency you programmed in Section A/B, so you can catch timing
 * bugs (wrong prescaler, wrong period, wrong APB1 clock assumption)
 * BEFORE trusting this panel in a real EEG session - this is exactly
 * the "verify with a photoresistor" step from the project's hardware
 * checklist.
 *
 * METHOD: software threshold-crossing (Schmitt-trigger-style) edge
 * counter, sampled by polling ADC2 as fast as the HAL allows. This is
 * a rough verification tool, NOT a lab-grade frequency counter -
 * cross-check against an oscilloscope or logic analyzer if you need
 * higher precision, per the earlier hardware discussion for this
 * project.
 *
 * SCOPE NOTE - cVEP mode: for the fixed-frequency SSVEP mode, the
 * measured Hz should closely match the relevant HALFPERIOD_* define.
 * For cVEP mode, the stimulus is a pseudorandom bit sequence, NOT a
 * clean periodic signal - this function will instead report a rough
 * threshold-crossing RATE (which should be well below the ~60 Hz
 * m-sequence update rate, since not every tick flips the bit), useful
 * only as a sanity check that the LED is actually toggling, not as a
 * "frequency" in the SSVEP sense. Don't interpret a cVEP-mode reading
 * as a target frequency.
 * ============================================================ */
#define PHOTO_ADC_CHANNEL        ADC_CHANNEL_5   /* PA5 - VERIFY free on your board */
#define PHOTO_SAMPLE_WINDOW_MS   1000u            /* measurement window length */
#define PHOTO_HYSTERESIS_COUNTS  50u              /* ADC-count hysteresis band around
                                                       the calibrated midpoint - TUNE
                                                       this to your photoresistor's
                                                       actual light/dark swing; too
                                                       small triggers on noise, too
                                                       large misses real transitions */

static uint16_t photo_threshold_high = 0;
static uint16_t photo_threshold_low  = 0;
static uint8_t  photo_calibrated     = 0;

/* Call once, before your first measurement, with the photoresistor
 * physically aimed at the LED key you intend to verify. Prompts you
 * over UART to present a fully DARK, then a fully LIT (steady, non-
 * flickering) state, and derives the threshold band from the actual
 * measured swing - this replaces a hardcoded guess with a real
 * calibration of YOUR specific photoresistor, ambient light, and
 * LED brightness. Re-run this if you change the LED, the ambient
 * lighting, or which key you're pointing at. */
void PHOTO_Calibrate(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = PHOTO_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5; /* slower/cleaner sample
                                                          is fine here - this
                                                          runs once, not in
                                                          the timing-critical
                                                          measurement loop */
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) Error_Handler();

    printf("PHOTO_Calibrate: cover the sensor / ensure the target LED is "
           "fully DARK, then wait...\r\n");
    HAL_Delay(1500);
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 10);
    uint16_t dark = (uint16_t)HAL_ADC_GetValue(&hadc2);

    printf("PHOTO_Calibrate: now ensure the target LED is fully LIT "
           "(steady DC, NOT flickering), then wait...\r\n");
    HAL_Delay(1500);
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 10);
    uint16_t lit = (uint16_t)HAL_ADC_GetValue(&hadc2);

    uint16_t lo = (lit < dark) ? lit : dark;
    uint16_t hi = (lit < dark) ? dark : lit;
    uint16_t mid = (uint16_t)((lo + hi) / 2u);

    /* Guard against a degenerate calibration (e.g. sensor not actually
     * connected, or LED not actually toggling brightness) - if the
     * swing is smaller than the hysteresis band itself, calibration
     * is meaningless and we refuse to proceed rather than silently
     * producing garbage frequency readings. */
    if ((hi - lo) <= PHOTO_HYSTERESIS_COUNTS) {
        printf("PHOTO_Calibrate FAILED: dark=%u lit=%u - swing too small. "
               "Check wiring, sensor placement, and ambient light, then "
               "retry.\r\n", dark, lit);
        photo_calibrated = 0;
        return;
    }

    photo_threshold_low  = (uint16_t)(mid - PHOTO_HYSTERESIS_COUNTS / 2u);
    photo_threshold_high = (uint16_t)(mid + PHOTO_HYSTERESIS_COUNTS / 2u);
    photo_calibrated = 1;

    printf("PHOTO_Calibrate OK: dark=%u lit=%u thresh_lo=%u thresh_hi=%u\r\n",
           dark, lit, photo_threshold_low, photo_threshold_high);
}

/* Measures the optical flicker rate over one PHOTO_SAMPLE_WINDOW_MS
 * window by counting rising threshold-crossings (dark->light
 * transitions) with hysteresis, then prints the result over UART.
 * Call this AFTER STIM_SetMode() has started flickering the key you
 * want to verify, with the photoresistor aimed at that key's LED. */
void PHOTO_MeasureFrequency(void)
{
    if (!photo_calibrated) {
        printf("PHOTO_MeasureFrequency: skipped - call PHOTO_Calibrate() "
               "first\r\n");
        return;
    }

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = PHOTO_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5; /* fastest available
                                                          sample time - we
                                                          want maximum time
                                                          resolution here,
                                                          not conversion
                                                          accuracy */
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) Error_Handler();

    uint8_t  state = 0;   /* 0 = below threshold ("dark"), 1 = above ("lit") */
    uint32_t rising_edges = 0;
    uint32_t t_start = HAL_GetTick();
    uint32_t t_now;

    do {
        HAL_ADC_Start(&hadc2);
        HAL_ADC_PollForConversion(&hadc2, 10);
        uint16_t sample = (uint16_t)HAL_ADC_GetValue(&hadc2);

        if (!state && sample > photo_threshold_high) {
            state = 1;
            rising_edges++;
        } else if (state && sample < photo_threshold_low) {
            state = 0;
        }
        t_now = HAL_GetTick();
    } while ((t_now - t_start) < PHOTO_SAMPLE_WINDOW_MS);

    float measured_hz = (float)rising_edges *
                         (1000.0f / (float)PHOTO_SAMPLE_WINDOW_MS);

    printf("PHOTO_MeasureFrequency [mode=%s]: %lu rising edges / %lu ms "
           "-> ~%.2f Hz measured\r\n",
           (current_mode == STIM_MODE_SSVEP) ? "SSVEP" : "cVEP",
           (unsigned long)rising_edges,
           (unsigned long)PHOTO_SAMPLE_WINDOW_MS,
           measured_hz);

    if (current_mode == STIM_MODE_SSVEP) {
        printf("  -> compare against the programmed HALFPERIOD_* value "
               "for whichever key the sensor is aimed at.\r\n");
    } else {
        printf("  -> cVEP mode: this is a transition RATE, not a target "
               "frequency - see Section D scope note.\r\n");
    }
}

/* ============================================================
 * GPIO mode switching - THIS is what makes combining the two
 * stimulus modes on the same pins possible. SSVEP needs the pins
 * wired to TIM2/TIM3 hardware (Alternate Function). cVEP needs
 * them as plain outputs, software-set inside the TIM4 interrupt.
 * ============================================================ */
static void GPIO_ConfigForSSVEP(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);
}

static void GPIO_ConfigForCVEP(void)
{
    /* Drive all keys low first to avoid a glitch during the switch */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                       GPIO_PIN_3 | GPIO_PIN_6, GPIO_PIN_RESET);

    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);
}

/* ============================================================
 * SSVEP: start only the keys marked active in `active`.
 * (Rewritten from your SSVEP_SelectedKeys to use a plain 0/1
 * boolean array instead of the key_nums[i]==i+1 check - the old
 * check required specific numeric values, not booleans, which
 * would misbehave for a caller passing {1,1,1,1,1}. This version
 * also explicitly STOPS inactive channels, which your original
 * version did not do - important so leftover flicker doesn't
 * persist from a previous selection.)
 * ============================================================ */
void SSVEP_SelectedKeys(uint8_t active[5])
{
    if (active[0]) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,
            __HAL_TIM_GET_COUNTER(&htim2) + HALFPERIOD_FORWARD);
        HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
    } else {
        HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    }

    if (active[1]) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,
            __HAL_TIM_GET_COUNTER(&htim2) + HALFPERIOD_BACKWARD);
        HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
    } else {
        HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
    }

    if (active[2]) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,
            __HAL_TIM_GET_COUNTER(&htim2) + HALFPERIOD_LEFT);
        HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_3);
    } else {
        HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_3);
    }

    if (active[3]) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,
            __HAL_TIM_GET_COUNTER(&htim2) + HALFPERIOD_RIGHT);
        HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_4);
    } else {
        HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_4);
    }

    if (active[4]) {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,
            __HAL_TIM_GET_COUNTER(&htim3) + HALFPERIOD_STOP);
        HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
    } else {
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
    }
}

/* Kept for convenience - identical behaviour to SSVEP_SelectedKeys
 * with all keys active. */
void SSVEP_StartAllKeys(void)
{
    uint8_t all[5] = {1, 1, 1, 1, 1};
    SSVEP_SelectedKeys(all);
}

/* Single-key test helpers - unchanged usage from before, still
 * useful if you need to isolate one key for troubleshooting. */
void SSVEP_TestForwardOnly(void)
{
    uint8_t only[5] = {1, 0, 0, 0, 0};
    SSVEP_SelectedKeys(only);
}
void SSVEP_TestStopOnly(void)
{
    uint8_t only[5] = {0, 0, 0, 0, 1};
    SSVEP_SelectedKeys(only);
}

/* ============================================================
 * cVEP: build the m-sequence lookup table once, using a 7-bit
 * Fibonacci LFSR (polynomial x^7+x^6+1 - VERIFY against an LFSR
 * reference table if you need a different length later).
 * ============================================================ */
static void MSEQ_Generate(void)
{
    uint8_t lfsr = 0x7Fu;
    for (uint32_t i = 0; i < MSEQ_LENGTH; i++)
    {
        mseq[i] = lfsr & 0x01u;
        uint8_t bit = ((lfsr >> 6) ^ (lfsr >> 5)) & 0x01u;
        lfsr = (uint8_t)((lfsr << 1) | bit) & 0x7Fu;
    }
}

/* Starts cVEP for only the keys marked active. Inactive keys are
 * forced LOW continuously (checked every tick in the ISR below). */
void CVEP_SelectedKeys(uint8_t active[5])
{
    memcpy(active_keys, active, 5);
    MSEQ_Generate();
    mseq_index = 0;
    HAL_TIM_Base_Start_IT(&htim4);
}

/* ============================================================
 * TOP-LEVEL MODE SWITCH - call this to choose SSVEP or cVEP,
 * and which keys are active, in one step. Handles stopping the
 * previous mode's timers and reconfiguring the GPIO pins so the
 * two modes never fight over the same pins.
 * ============================================================ */
void STIM_SetMode(StimMode mode, uint8_t active[5])
{
    /* Stop everything first */
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_3);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_4);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop_IT(&htim4);

    memcpy(active_keys, active, 5);

    if (mode == STIM_MODE_SSVEP)
    {
        GPIO_ConfigForSSVEP();
        HAL_TIM_MspPostInit(&htim2);   /* re-establish AF mapping cleanly */
        HAL_TIM_MspPostInit(&htim3);
        SSVEP_SelectedKeys(active_keys);
    }
    else /* STIM_MODE_CVEP */
    {
        GPIO_ConfigForCVEP();
        CVEP_SelectedKeys(active_keys);
    }
    current_mode = mode;
}

/* ============================================================
 * TIM2/TIM3 callback - SSVEP toggle re-arming (unchanged logic
 * from before, still only runs meaningfully while in SSVEP mode -
 * harmless if it fires with pins reconfigured, since Stop_IT in
 * STIM_SetMode prevents it from running in cVEP mode at all).
 * ============================================================ */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        switch (htim->Channel)
        {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1) + HALFPERIOD_FORWARD);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2) + HALFPERIOD_BACKWARD);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_3:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_3) + HALFPERIOD_LEFT);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_4:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_4) + HALFPERIOD_RIGHT);
            break;
        default:
            break;
        }
    }
    else if (htim->Instance == TIM3)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1) + HALFPERIOD_STOP);
        }
    }
}

/* ============================================================
 * TIM4 callback - cVEP update tick (~60 Hz). Writes each ACTIVE
 * key's current m-sequence bit to its GPIO; inactive keys are
 * forced LOW every tick, not just left alone, so a key you turned
 * off stays fully off even if it was mid-flicker when disabled.
 * ============================================================ */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        for (uint8_t k = 0; k < 5; k++)
        {
            if (active_keys[k])
            {
                uint32_t idx = (mseq_index + key_shift[k]) % MSEQ_LENGTH;
                HAL_GPIO_WritePin(key_port[k], key_pin[k],
                    mseq[idx] ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(key_port[k], key_pin[k], GPIO_PIN_RESET);
            }
        }
        mseq_index = (mseq_index + 1) % MSEQ_LENGTH;
    }
}

/* Redirect printf() to SWV/ITM (unchanged) */
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++) ITM_SendChar((uint32_t)ptr[i]);
    return len;
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();   /* NEW */
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /* ---- CHOOSE YOUR MODE HERE ----
   * Edit these two lines to pick what runs. Only ONE STIM_SetMode
   * call should be active (uncomment one, comment the other).
   */
  uint8_t all_keys[5]      = {1, 1, 1, 1, 1};   /* all 5 keys */
  /* uint8_t forward_only[5]  = {1, 0, 0, 0, 0}; */  /* example: single key */

  STIM_SetMode(STIM_MODE_SSVEP, all_keys);
  /* STIM_SetMode(STIM_MODE_CVEP, all_keys); */

  /* NEW - one-time photoresistor calibration. Aim the sensor at the
   * key you want to verify BEFORE this runs, and follow the UART
   * prompts (dark, then steady-lit). If you skip this call,
   * PHOTO_MeasureFrequency() below will just print a "skipped"
   * message every cycle instead of a measurement - it will not
   * crash, but it also will not verify anything until you calibrate. */
  PHOTO_Calibrate();

  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t adc_raw = HAL_ADC_GetValue(&hadc1);

    v_node = (adc_raw / 4095.0f) * 3.3f;
    current_mA = ((TEST_SUPPLY_V - v_node) / TEST_RESISTOR_OHM) * 1000.0f;

    printf("Mode=%s V_node=%f V I=%f mA\r\n",
           (current_mode == STIM_MODE_SSVEP) ? "SSVEP" : "cVEP",
           v_node, current_mA);

    /* NEW - optical flicker-frequency check. This call blocks for
     * ~PHOTO_SAMPLE_WINDOW_MS (1 second) while it measures, so the
     * overall loop period is now roughly (500 ms delay + ~1000 ms
     * measurement) per printed diagnostic line - expected and fine
     * for a bench-verification loop, but don't mistake this for the
     * panel's real-time behaviour during an actual EEG session,
     * where you would remove or gate this call. */
    PHOTO_MeasureFrequency();

    HAL_Delay(500);
    /* USER CODE END 3 */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();

  /* NOTE: ADC1 and ADC2 share this same ADC clock prescaler on
   * STM32F1 - configuring it once here covers both peripherals,
   * you do NOT need a second PeriphClkInit block for ADC2. */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

/* NEW - ADC2, dedicated to the photoresistor (PA5 / Channel 5).
 * Kept as a fully separate peripheral from ADC1 (used for the
 * existing LED-current diagnostic) so the two measurements can
 * never interfere with or overwrite each other's channel config.
 * If you have NOT yet enabled ADC2 in CubeMX, the manual clock
 * enable below makes this self-contained - but the PROPER long-
 * term fix is to enable ADC2 in CubeMX and regenerate, matching
 * how TIM4 was handled earlier in this file. */
static void MX_ADC2_Init(void)
{
  __HAL_RCC_ADC2_CLK_ENABLE();   /* manual clock enable - see note above */

  ADC_ChannelConfTypeDef sConfig = {0};
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK) Error_Handler();

  /* Default channel config here; PHOTO_Calibrate() and
   * PHOTO_MeasureFrequency() each reconfigure the sample time to
   * suit their own purpose (slow+clean for calibration, fast for
   * measurement), so this initial config is a safe placeholder. */
  sConfig.Channel = PHOTO_ADC_CHANNEL;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) Error_Handler();
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) Error_Handler();
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) Error_Handler();

  HAL_TIM_MspPostInit(&htim2);
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) Error_Handler();
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

  HAL_TIM_MspPostInit(&htim3);
}

/* TIM4, base timer only, drives the cVEP ~60 Hz update tick.
 * If you have NOT yet enabled TIM4 in CubeMX, the manual clock
 * enable and NVIC calls below make this self-contained - but the
 * PROPER long-term fix is to enable TIM4 in CubeMX (System Core /
 * Timers -> TIM4 -> Internal Clock, base only) and regenerate, so
 * future regenerations don't conflict with this manual setup. */
static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  __HAL_RCC_TIM4_CLK_ENABLE();   /* manual clock enable - see note above */

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;              /* 1 MHz tick, matches TIM2/TIM3 */
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 16665;              /* ~60.0 Hz update interrupt -
                                              VERIFY against your confirmed
                                              APB1 Timer clock value */
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) Error_Handler();

  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* IMPORTANT: you must also add, in stm32f1xx_it.c:
   *   void TIM4_IRQHandler(void) { HAL_TIM_IRQHandler(&htim4); }
   * (declare "extern TIM_HandleTypeDef htim4;" near the top of that
   * file too) - this project's stm32f1xx_it.c was not provided to
   * me, so I cannot edit it directly; add this yourself. */
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* NEW - PA5, analog input for the photoresistor (ADC2 Channel 5) */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
