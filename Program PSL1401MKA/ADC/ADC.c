#include "ADC.h"
#include "stm32f10x_dma.h"
#include <stm32f10x_conf.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_conf.h>
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "usart.h"
#include "General/General.h"

volatile uint16_t U_OUT = 0;
volatile uint16_t U_PS = 0;
volatile uint16_t U_IN = 0;
volatile int16_t Temperature = 0;
volatile uint16_t U_Controller = 0;
volatile uint16_t U_12V = 0;
volatile uint16_t U_5V_1Wire = 0;
volatile uint16_t U_5V = 0;


volatile int32_t OffTimer = 0;
volatile uint8_t HighCurrentStatus = 0;
uint8_t EnterToSwitch = 0;
volatile int16_t U_OUTtmp = 0;
volatile int16_t Ut = 0;
volatile int32_t Current_1of3=0;
volatile int32_t I_Raw_mkA=0;
volatile int32_t I_Raw_mA=0;

#define TIM3_PERIOD 5
#define TIME_OUT_FOR_CURRENT_SEC 5 //sec


void ADC1_CH_DMA_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6 ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_DMA1 , ENABLE ) ;

	DMA_InitStructure.DMA_BufferSize = NUMBER_OF_CHANNELS;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RegularConvData;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd ( DMA1_Channel1 , ENABLE ) ;
	RCC_ADCCLKConfig ( RCC_PCLK2_Div6 ) ;

	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = NUMBER_OF_CHANNELS;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);//I mkA
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);//I mA
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_239Cycles5);//U
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_239Cycles5);//
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 5, ADC_SampleTime_239Cycles5);//
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 6, ADC_SampleTime_239Cycles5);//
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 7, ADC_SampleTime_239Cycles5);//vref
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 8, ADC_SampleTime_41Cycles5);//temp
	ADC_Cmd ( ADC1 , ENABLE ) ;
	ADC_DMACmd ( ADC1 , ENABLE ) ;
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd ( ADC1 , ENABLE ) ;
}

void init_timer3()
{

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);


  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);

  base_timer.TIM_Prescaler = 24000 - 1;
  base_timer.TIM_Period = TIM3_PERIOD;
  TIM_TimeBaseInit(TIM3, &base_timer);


  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  TIM_Cmd(TIM3, ENABLE);


  NVIC_EnableIRQ(TIM3_IRQn);
  NVIC_SetPriority(TIM3_IRQn, 2);
}
void TIM3_IRQHandler()
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
	  	CurrentTimer++;
	  	CurrentTimerCap++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);


		//a= (22+95)*1000/(1470-330);
		//b= 22 - (a*330/1000);
		// while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET );
		DMA_ClearFlag(DMA1_FLAG_TC1);
		DMA_ClearITPendingBit( DMA1_IT_TC1);
		DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, DISABLE);
		Ut= (RegularConvData[2] * CalibrationData.CalibrationFor_U_PS1) / RegularConvData[6];
		U_PS = MedianFilter(Ut);
		if (U_PS < 3) U_PS = 0;
		U_Controller = 491520 / RegularConvData[6];// Uref V/10;  1200 * 4096/ChVref
		Ut = (RegularConvData[2] * CalibrationData.CalibrationFor_U_PS2) / RegularConvData[6];
		U_OUTtmp = 0;
		U_OUT_ForSetResistance = U_OUTtmp;
		if (U_OUTtmp<3) U_OUTtmp = 0;
		Ut = (RegularConvData[4] * CalibrationData.CalibrationFor_U_PS3) / RegularConvData[6];
		U_IN = middle_of_3Umax(Ut);
		I_Raw_mA = (RegularConvData[1] * CalibrationData.CalibrationForCurrent_mA1*10) / RegularConvData[6] ;//  Current A/10

		Current_mA=1000*MedianFilter1(I_Raw_mA);

		I_Raw_mkA= (RegularConvData[0] * CalibrationData.CalibrationForCurrent_mkA1*100) / RegularConvData[6] ;//  Current A/10
		Current_mkA = MedianFilter2(I_Raw_mkA);
		if (Current_mkA<=2)  Current_mkA = 0;
		Current_1of3 = middle_of_3_ImkA(I_Raw_mkA);








		if (Current_1of3>8500)
		{
			CurrentSum =CurrentSum + Current_mA;
			CurrentSumCap =CurrentSumCap + I_Raw_mA;
		}else
		{
			CurrentSum = CurrentSum + Current_mkA;
			if (Current_mkA>10) CurrentSumCap =CurrentSumCap + I_Raw_mkA;
			else CurrentSumCap =CurrentSumCap + Current_mkA;
		}
		if (CurrentTimer == 80)
		{
			Current = CurrentSum/80 ;
			CurrentSum = 0;
			CurrentTimer = 0;
		}


		if (CurrentTimerCap == 166)
		{
			CurrentCapacity = CurrentCapacity+CurrentSumCap/167;
			CurrentSumCap = 0;
			CurrentTimerCap = 0;
			Print_to_USART1("1sec");
		}

		DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
  }
}


uint16_t MedianFilter(uint16_t datum)
{
struct pair{
  struct pair *point; /* Pointers forming list linked in sorted order */
  uint16_t value; /* Values to sort */
};

/* Buffer of nwidth pairs */
 static struct pair buffer[MEDIAN_FILTER_SIZE] = {{0}};
/* Pointer into circular buffer of data */
 static struct pair *datpoint = buffer;
/* Chain stopper */
 static struct pair small = {NULL, STOPPER};
/* Pointer to head (largest) of linked list.*/
 static struct pair big = {&small, 0};

/* Pointer to successor of replaced data item */
 struct pair *successor;
/* Pointer used to scan down the sorted list */
 struct pair *scan;
/* Previous value of scan */
 struct pair *scanold;
/* Pointer to median */
 struct pair *median;
 uint16_t i;

if (datum == STOPPER){
   datum = STOPPER + 1; /* No stoppers allowed. */
}

 if ( (++datpoint - buffer) >= MEDIAN_FILTER_SIZE){
   datpoint = buffer; /* Increment and wrap data in pointer.*/
 }

 datpoint->value = datum; /* Copy in new datum */
 successor = datpoint->point; /* Save pointer to old value's successor */
 median = &big; /* Median initially to first in chain */
 scanold = NULL; /* Scanold initially null. */
 scan = &big; /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint){
   scan->point = successor;
 }

 scanold = scan; /* Save this pointer and */
 scan = scan->point ; /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE; ++i){
  /* Handle odd-numbered item in chain */
  if (scan->point == datpoint){
    scan->point = successor; /* Chain out the old datum.*/
  }

  if (scan->value < datum){ /* If datum is larger than scanned value,*/
    datpoint->point = scanold->point; /* Chain it in here. */
    scanold->point = datpoint; /* Mark it chained in. */
    datum = STOPPER;
  };

  /* Step median pointer down chain after doing odd-numbered element */
  median = median->point; /* Step median pointer. */
  if (scan == &small){
    break; /* Break at end of chain */
  }
  scanold = scan; /* Save this pointer and */
  scan = scan->point; /* step down chain */

  /* Handle even-numbered item in chain. */
  if (scan->point == datpoint){
    scan->point = successor;
  }

  if (scan->value < datum){
    datpoint->point = scanold->point;
    scanold->point = datpoint;
    datum = STOPPER;
  }

  if (scan == &small){
    break;
  }

  scanold = scan;
  scan = scan->point;
}

return median->value;
}

uint16_t MedianFilter1(uint16_t datum)
{


struct pair{
  struct pair *point; /* Pointers forming list linked in sorted order */
  uint16_t value; /* Values to sort */
};

/* buffer1 of nwidth pairs */
 static struct pair buffer1[MEDIAN_FILTER_SIZE1] = {{0}};
/* Pointer into circular buffer1 of data */
 static struct pair *datpoint1 = buffer1;
/* Chain stopper */
 static struct pair small1 = {NULL, STOPPER};
/* Pointer to head (largest) of linked list.*/
 static struct pair big1 = {&small1, 0};

/* Pointer to successor of replaced data item */
 struct pair *successor;
/* Pointer used to scan down the sorted list */
 struct pair *scan;
/* Previous value of scan */
 struct pair *scanold;
/* Pointer to median */
 struct pair *median;
 uint16_t i;

if (datum == STOPPER){
   datum = STOPPER + 1; /* No stoppers allowed. */
}

 if ( (++datpoint1 - buffer1) >= MEDIAN_FILTER_SIZE1){
   datpoint1 = buffer1; /* Increment and wrap data in pointer.*/
 }

 datpoint1->value = datum; /* Copy in new datum */
 successor = datpoint1->point; /* Save pointer to old value's successor */
 median = &big1; /* Median initially to first in chain */
 scanold = NULL; /* Scanold initially null. */
 scan = &big1; /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint1){
   scan->point = successor;
 }

 scanold = scan; /* Save this pointer and */
 scan = scan->point ; /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE1; ++i){
  /* Handle odd-numbered item in chain */
  if (scan->point == datpoint1){
    scan->point = successor; /* Chain out the old datum.*/
  }

  if (scan->value < datum){ /* If datum is larger than scanned value,*/
    datpoint1->point = scanold->point; /* Chain it in here. */
    scanold->point = datpoint1; /* Mark it chained in. */
    datum = STOPPER;
  };

  /* Step median pointer down chain after doing odd-numbered element */
  median = median->point; /* Step median pointer. */
  if (scan == &small1){
    break; /* Break at end of chain */
  }
  scanold = scan; /* Save this pointer and */
  scan = scan->point; /* step down chain */

  /* Handle even-numbered item in chain. */
  if (scan->point == datpoint1){
    scan->point = successor;
  }

  if (scan->value < datum){
    datpoint1->point = scanold->point;
    scanold->point = datpoint1;
    datum = STOPPER;
  }

  if (scan == &small1){
    break;
  }

  scanold = scan;
  scan = scan->point;
}

return median->value;
}

uint16_t MedianFilter2(uint16_t datum)
{


struct pair{
  struct pair *point; /* Pointers forming list linked in sorted order */
  uint16_t value; /* Values to sort */
};

/* buffer2 of nwidth pairs */
 static struct pair buffer2[MEDIAN_FILTER_SIZE2] = {{0}};
/* Pointer into circular buffer2 of data */
 static struct pair *datpoint2 = buffer2;
/* Chain stopper */
 static struct pair small2 = {NULL, STOPPER};
/* Pointer to head (largest) of linked list.*/
 static struct pair big2 = {&small2, 0};

/* Pointer to successor of replaced data item */
 struct pair *successor;
/* Pointer used to scan down the sorted list */
 struct pair *scan;
/* Previous value of scan */
 struct pair *scanold;
/* Pointer to median */
 struct pair *median;
 uint16_t i;

if (datum == STOPPER){
   datum = STOPPER + 1; /* No stoppers allowed. */
}

 if ( (++datpoint2 - buffer2) >= MEDIAN_FILTER_SIZE2){
   datpoint2 = buffer2; /* Increment and wrap data in pointer.*/
 }

 datpoint2->value = datum; /* Copy in new datum */
 successor = datpoint2->point; /* Save pointer to old value's successor */
 median = &big2; /* Median initially to first in chain */
 scanold = NULL; /* Scanold initially null. */
 scan = &big2; /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint2){
   scan->point = successor;
 }

 scanold = scan; /* Save this pointer and */
 scan = scan->point ; /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE2; ++i){
  /* Handle odd-numbered item in chain */
  if (scan->point == datpoint2){
    scan->point = successor; /* Chain out the old datum.*/
  }

  if (scan->value < datum){ /* If datum is larger than scanned value,*/
    datpoint2->point = scanold->point; /* Chain it in here. */
    scanold->point = datpoint2; /* Mark it chained in. */
    datum = STOPPER;
  };

  /* Step median pointer down chain after doing odd-numbered element */
  median = median->point; /* Step median pointer. */
  if (scan == &small2){
    break; /* Break at end of chain */
  }
  scanold = scan; /* Save this pointer and */
  scan = scan->point; /* step down chain */

  /* Handle even-numbered item in chain. */
  if (scan->point == datpoint2){
    scan->point = successor;
  }

  if (scan->value < datum){
    datpoint2->point = scanold->point;
    scanold->point = datpoint2;
    datum = STOPPER;
  }

  if (scan == &small2){
    break;
  }

  scanold = scan;
  scan = scan->point;
}

return median->value;
}
//mediana 3
int32_t middle_of_3(int32_t a, int32_t b, int32_t c)
{
   int32_t middle;

   if ((a <= b) && (a <= c)){
      middle = (b <= c) ? b : c;
   }
   else{
      if ((b <= a) && (b <= c)){
         middle = (a <= c) ? a : c;
      }
      else{
         middle = (a <= b) ? a : b;
      }
   }

   return middle;
}
int32_t middle_of_3_ImkA(int32_t value)
{
   static int32_t InputValueI1[3]={0,0,0};
   int32_t middle,a,b,c;
   InputValueI1[2] = InputValueI1[1];
   InputValueI1[1] = InputValueI1[0];
   InputValueI1[0] = value;
   a = InputValueI1[2];
   b = InputValueI1[1];
   c = InputValueI1[0];
   if ((a <= b) && (a <= c)){
      middle = (b <= c) ? b : c;
   }
   else{
      if ((b <= a) && (b <= c)){
         middle = (a <= c) ? a : c;
      }
      else{
         middle = (a <= b) ? a : b;
      }
   }

   return middle;
}

int32_t middle_of_3_ImA(int32_t value)
{
   static int32_t InputValueI2[3]={0,0,0};
   int32_t middle,a,b,c;
   InputValueI2[2] = InputValueI2[1];
   InputValueI2[1] = InputValueI2[0];
   InputValueI2[0] = value;
   a = InputValueI2[2];
   b = InputValueI2[1];
   c = InputValueI2[0];
   if ((a <= b) && (a <= c)){
      middle = (b <= c) ? b : c;
   }
   else{
      if ((b <= a) && (b <= c)){
         middle = (a <= c) ? a : c;
      }
      else{
         middle = (a <= b) ? a : b;
      }
   }

   return middle;
}

int32_t middle_of_3Umax(int32_t value)
{
   static int32_t InputValueU[3]={0,0,0};
   int32_t middle,a,b,c;
   InputValueU[2] = InputValueU[1];
   InputValueU[1] = InputValueU[0];
   InputValueU[0] = value;
   a = InputValueU[2];
   b = InputValueU[1];
   c = InputValueU[0];
   if ((a <= b) && (a <= c)){
      middle = (b <= c) ? b : c;
   }
   else{
      if ((b <= a) && (b <= c)){
         middle = (a <= c) ? a : c;
      }
      else{
         middle = (a <= b) ? a : b;
      }
   }

   return middle;
}
//mediana >3
