/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "string.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
FATFS fs;		//	file system
FIL fil;		// file
FRESULT fresult;	//	store result

char buffer[70000];	//	to store data
int hist[256] = {0};
//int newGrayHist[256] = {0};
//int idealPixelCount[256] = {0};

char destbuffer[69840];

//char *destbuffer;
UINT br, bw;	//	file read/write count

/*	capacity related variables	*/
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
  /* Implement your write code here, this is used by puts and printf for example */
  for(int i=0 ; i<len ; i++)
    ITM_SendChar((*ptr++));
  return len;
}

//	find size of data in buffer
int bufsize(char *buf){
	int i=0;
	while(*buf++ != '\0'){
		i++;
	}
	return i;
}

//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	MAYBE COULD BE ADJUSTED BC I PUT BUFFER IN HEAP INSTEAD OF BSS? - "free"***
void bufclear(char * buf, int size){
	memset(buf, 0, size);
}

void *unpackbits(char *testbuf, char *testdestbuf, int destcount) {

  	int runlength;

  	while (destcount > 0) {
  		runlength = *testbuf++;
  		if (runlength == -128)
  			continue;
  		else if (runlength >= 128)
  		{
  			runlength = 256 - runlength + 1;
  			if (runlength > destcount)
  				return NULL;
  			memset(testdestbuf, *testbuf, runlength);
  			testbuf += 1;
  			testdestbuf += runlength;
  		}
  		else
  		{
  			++runlength;
  			if (runlength > destcount)
  				return NULL;
  			memcpy(testdestbuf, testbuf, runlength);
  			testbuf += runlength;
  			testdestbuf += runlength;
  		}
  		destcount -= runlength;
  	}

}

void histogramEqualization(char *inputbuffer, int bufferSize) {
	int i;

	for(i = 0; i < bufferSize; i++){
		hist[(int)*inputbuffer++]++;
	}

	int newGrayHist[256] = {0};
	float idealPixelCount[256];
	for(i = 1; i < 256; i++){ //cumulative probability pixel counts
		newGrayHist[i] = newGrayHist[i-1] + hist[i];
	}

	for(i = 0; i < 256; i++){ //
		idealPixelCount[i] = (float)(newGrayHist[i]*255)/(float)(bufferSize-1);
	}

	for(i = 0; i < bufferSize; i++){
		buffer[i] = (int)idealPixelCount[destbuffer[i]];
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	 HAL_Init();

	  FATFS FatFs; 	//Fatfs handle
	  FIL fil; 		//File handle
	  FRESULT fres;
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
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(1000);	//	delay for SD card to process mounting

  //	Mount SD Card
  fresult = f_mount(&fs, "/", 1);
  if (fresult != FR_OK) //printf ("ERROR!!! in mounting SD CARD...\n\n");
	  printf("Error in mounting SD CARD...\n\n");
  else printf("SD CARD mounted successfully...\n\n");

  /*********************	Card Capacity Details	***********************/
  f_getfree("", &fre_clust, &pfs);

  total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
  sprintf(buffer, "SD CARD Total Size: \t%lu\n", total);
  printf(buffer);
  bufclear(buffer, strlen(buffer));
  free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
  sprintf(buffer, "SD CARD Free Space: \t%lu\n", free_space);
  printf(buffer);
  bufclear(buffer, strlen(buffer));



  /************* The following operation is using PUTS and GETS *********************/

	/* Open file to write/ create a file if it doesn't exist */
//	fresult = f_open(&fil, "poutcrop.tif", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

	/* Writing text */
//	f_puts("This data is from the FILE1.txt. And it was written using ...f_puts... ", &fil);

	/* Close file */
//	fresult = f_close(&fil);

//	if (fresult == FR_OK)printf ("File1.txt created and the data is written \n");

  bufclear(buffer, strlen(buffer));
  bufclear(destbuffer, strlen(destbuffer));

	/* Open file to read */
  fresult = f_open(&fil, "pout.tif", FA_READ);
  f_read (&fil, buffer, f_size(&fil), &br);
  if(fres == FR_OK) {
	  while(1){
	  }
  }

  // FIND OUT HOW TO GET PIXEL COUNT OR NUMBER OF PIXELS
  int imgSize = 69841;
  int slaveSize = 26190;
  int masterSize = 17460;
  // SLAVES GET 26190
  // MASTER GETS 17460

  unpackbits(buffer + 8, destbuffer, 69840+1);
  bufclear(buffer, strlen(buffer));

  //	#ifndef entire image
  histogramEqualization(destbuffer, imgSize);

  //	#ifndef master
//  histogramEqualization(destbuffer, masterSize);

//  //	#ifndef slave
//  histogramEqualization(destbuffer + masterSize, slaveSize);
//
//  //	#ifndef other slave
//  histogramEqualization(destbuffer + masterSize + slaveSize, slaveSize);


  fres = f_open(&fil, "pout.txt", FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);

  char s3[961];
  char s4[10];
  int byteSizeWrote = 0;


  int i = 0;
  int lineInc = 1;
  int newLineCount = 291;
  int totalPixelSize = 69840;

// CONVERT DATA TO STRING AND COPIES TO TXT FILE ONE LINE AT A TIME
  for(i = 0; i < totalPixelSize; i++){
	  if((int)buffer[i] > 99){
		  byteSizeWrote = byteSizeWrote+4;
	  } else if ((int)buffer[i] > 9){
		  byteSizeWrote = byteSizeWrote+3;
	  } else {
		  byteSizeWrote = byteSizeWrote+2;
	  }
	  if(i == 0 || byteSizeWrote == 0){ // if start
		  sprintf(s3, "%d", (int)buffer[i]);
		  sprintf(s4, "%s", ",");
		  strcat(s3,s4);
//		  newLineFlag = 1;
	  } else if(i == totalPixelSize){ //last line
		  sprintf(s4, "%d", (int)buffer[i]);
		  strcat(s3, s4);
		  	  } else if (i == ((totalPixelSize/newLineCount)*lineInc)-1) { // if last line
		  		  lineInc++;
		  		  sprintf(s4, "%d", (int)buffer[i]);
		  		  strcat(s3, s4);
		  		  sprintf(s4, "%c", 13);
		  		  strcat(s3, s4);
		  		  sprintf(s4, "%c", 10);
		  		  strcat(s3, s4);
		  		  byteSizeWrote = byteSizeWrote + 1;
		  		  fres = f_write(&fil, (char*)s3, byteSizeWrote, &bw);
		  		  if(fres == FR_OK) {
		  		  }
		  		  byteSizeWrote = 0;
		  		  bufclear(s3, strlen(s3));
//		  		  newLineFlag = 0;
		  	  } else {
		  		  sprintf(s4, "%d", (int)buffer[i]);
		  		  strcat(s3, s4);
		  		  sprintf(s4, "%s", ",");
		  		  strcat(s3, s4);
		  	  }
  }


  /* Close file */
  f_close(&fil);
  //We're done, so de-mount the drive
  f_mount(NULL, "", 0);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
