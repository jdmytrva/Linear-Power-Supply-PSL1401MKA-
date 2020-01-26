#include "General.h"
#define ADDRESS_FLASH_CALIBRATTION FLASH_BASE+1024*124
#define ADDRESS_FLASH_SETTINGS     FLASH_BASE+1024*123
#define ADDRESS_FLASH_WHEN_OFF     FLASH_BASE+1024*122


struct StructCalibrationValuetoSaveInFlash CalibrationData=
{
		0,
		ADDRESS_FLASH_CALIBRATTION,
		0,
		ADDRESS_FLASH_CALIBRATTION+4,
		0,
		ADDRESS_FLASH_CALIBRATTION+8,
		0,
		ADDRESS_FLASH_CALIBRATTION+12,
		0,
		ADDRESS_FLASH_CALIBRATTION+16,
		0,
		ADDRESS_FLASH_CALIBRATTION+20,
		0,
		ADDRESS_FLASH_CALIBRATTION+24,
		0,
		ADDRESS_FLASH_CALIBRATTION+28,
		0,
		ADDRESS_FLASH_CALIBRATTION+32,
		0,
		ADDRESS_FLASH_CALIBRATTION+36

};
struct StructCalibrationValuetoSaveInFlash CalibrationDataForVerify=
{
		0,
		ADDRESS_FLASH_CALIBRATTION,
		0,
		ADDRESS_FLASH_CALIBRATTION+4,
		0,
		ADDRESS_FLASH_CALIBRATTION+8,
		0,
		ADDRESS_FLASH_CALIBRATTION+12,
		0,
		ADDRESS_FLASH_CALIBRATTION+16,
		0,
		ADDRESS_FLASH_CALIBRATTION+20,
		0,
		ADDRESS_FLASH_CALIBRATTION+24,
		0,
		ADDRESS_FLASH_CALIBRATTION+28,
		0,
		ADDRESS_FLASH_CALIBRATTION+32,
		0,
		ADDRESS_FLASH_CALIBRATTION+36

};
struct StructCalibrationValuetoSaveInFlash CalibrationDataFactory=
{
		0,//CRC
		ADDRESS_FLASH_CALIBRATTION,
		0,//Calibration0ValueForCurrent
		ADDRESS_FLASH_CALIBRATTION+4,
		0,//Calibration0ValueForCurrent1
		ADDRESS_FLASH_CALIBRATTION+8,
		43,//CalibrationForCurrent_mA1
		ADDRESS_FLASH_CALIBRATTION+12,
		50,//CalibrationForCurrent_mkA1
		ADDRESS_FLASH_CALIBRATTION+16,
		990,//CalibrationFor_U_PS1
		ADDRESS_FLASH_CALIBRATTION+20,
		990,//CalibrationFor_U_PS2
		ADDRESS_FLASH_CALIBRATTION+24,
		990,//CalibrationFor_U_PS3
		ADDRESS_FLASH_CALIBRATTION+28,
		130,//ResistanceComp_Ishunt_Wires mOm 116
		ADDRESS_FLASH_CALIBRATTION+32,
		34,//ResistanceComp_MOSFET mOm
		ADDRESS_FLASH_CALIBRATTION+36
};
//=================================================

struct StructSettingsValuetoSaveInFlash SettingsData=
{
		0,
		ADDRESS_FLASH_SETTINGS,
		0,
		ADDRESS_FLASH_SETTINGS+4,
		0,
		ADDRESS_FLASH_SETTINGS+8,
		0,
		ADDRESS_FLASH_SETTINGS+12,
		0,
		ADDRESS_FLASH_SETTINGS+16,
		0,
		ADDRESS_FLASH_SETTINGS+20,
		0,
		ADDRESS_FLASH_SETTINGS+24,
		0,
		ADDRESS_FLASH_SETTINGS+28
};
struct StructSettingsValuetoSaveInFlash SettingsDataForVerify=
{
		0,
		ADDRESS_FLASH_SETTINGS,
		0,
		ADDRESS_FLASH_SETTINGS+4,
		0,
		ADDRESS_FLASH_SETTINGS+8,
		0,
		ADDRESS_FLASH_SETTINGS+12,
		0,
		ADDRESS_FLASH_SETTINGS+16,
		0,
		ADDRESS_FLASH_SETTINGS+20,
		0,
		ADDRESS_FLASH_SETTINGS+24,
		0,
		ADDRESS_FLASH_SETTINGS+28
};

struct StructSettingsValuetoSaveInFlash SettingsDataFactory=
{
		0,//CRC
		ADDRESS_FLASH_SETTINGS,
		1,//option1 -  Menu Position
		ADDRESS_FLASH_SETTINGS+4,
		15,//Charge time
		ADDRESS_FLASH_SETTINGS+8,
		1050,//LowVoltage
		ADDRESS_FLASH_SETTINGS+12,
		1450,//MaxVoltage
		ADDRESS_FLASH_SETTINGS+16,
		10,//Swing_Chrg_time
		ADDRESS_FLASH_SETTINGS+20,
		10,//Swing_DChrg_time
		ADDRESS_FLASH_SETTINGS+24,
		6,//ChargeAdapt %
		ADDRESS_FLASH_SETTINGS+28
};
//=====================================
struct StructValuetoSaveInFlashWhenPowerOFF SaveDataWhenPowerOff=
{
		0,
		ADDRESS_FLASH_WHEN_OFF,
		0,
		ADDRESS_FLASH_WHEN_OFF+4,
		0,
		ADDRESS_FLASH_WHEN_OFF+8
};

struct StructValuetoSaveInFlashWhenPowerOFF SaveDataWhenPowerOffForVerify=
{
		0,
		ADDRESS_FLASH_WHEN_OFF,
		0,
		ADDRESS_FLASH_WHEN_OFF+4,
		0,
		ADDRESS_FLASH_WHEN_OFF+8
};

struct StructValuetoSaveInFlashWhenPowerOFF SaveDataWhenPowerOffFactory=
{
		0,//CRC
		ADDRESS_FLASH_WHEN_OFF,
		0,//BatteryCapacityDischargePreviousValue
		ADDRESS_FLASH_WHEN_OFF+4,
		0,//BatteryCapacityDischargeCurrent
		ADDRESS_FLASH_WHEN_OFF+8
};


volatile uint16_t U_OUT_ForSetResistance=0;
volatile int32_t Current_mkA = 0;
volatile int32_t Current_mA = 0;
volatile int32_t Current = 0;
volatile uint8_t resistor01 = 1;
volatile int32_t CurrentTimer = 0;
volatile int32_t CurrentTimerCap = 0;
volatile uint32_t CurrentSum=0;
volatile uint32_t CurrentSumCap = 0;
volatile uint32_t CurrentCapacity = 0;


void WriteInLOG(char  str [17])
{
	static uint16_t i_log=0;
	uint8_t i;
	uint8_t j;

	char *number;
	number = itoa(LoggingData.RecordsQuantity);


	for(i=0;number[i]!='\0';i++)
	{
		LoggingData.Records[LoggingData.RecordsQuantity][i] = number[i];
	}
	LoggingData.Records[LoggingData.RecordsQuantity][i] = ':';
	i++;

	for(j=0;str[j]!='\0';i++,j++)
	{
		if (i<16) LoggingData.Records[LoggingData.RecordsQuantity][i] = str[j];
		else
		{
			LoggingData.Records[LoggingData.RecordsQuantity][i] = '\0';
			break;
		}
	}
	for (;i<16;i++)
	{
		LoggingData.Records[LoggingData.RecordsQuantity][i] = ' ';
	}
	Print_to_USART1_d(i,"ii: ",0);
	LoggingData.Records[LoggingData.RecordsQuantity][i] = '\0';

	LoggingData.RecordsQuantity++;
	if (LoggingData.RecordsQuantity>=(MAX_LOG_ITEMS))
	{
		for (i=0;i<MAX_LOG_ITEMS-10;i++)
		{
			number = itoa(i);
			uint8_t k;
			for(k=0;number[k]!='\0';k++)
			{
				LoggingData.Records[i][k] = number[k];
			}
			LoggingData.Records[i][k] = ':';
			for (j=k+1;LoggingData.Records[i][j]!='\0';j++)
			{
				if (i>=10)
					LoggingData.Records[i][j] = LoggingData.Records[i+10][j];
				else
					LoggingData.Records[i][j] = LoggingData.Records[i+10][j+1];
			}
			j--;
			for (;j<16;j++)
			{
				LoggingData.Records[i][j] = ' ';
			}
			LoggingData.Records[i][j] = '\0';
		}

		LoggingData.RecordsQuantity=MAX_LOG_ITEMS-10;
	}
	flash_write_block();
	Print_to_USART1_d(LoggingData.RecordsQuantity,"Q: ",0);
}

void WriteInLOGc(char  str [17],char c)
{
	static uint16_t i_log=0;
	uint8_t i;
	uint8_t j;

	char *number;
	number = itoa(LoggingData.RecordsQuantity);


	for(i=0;number[i]!='\0';i++)
	{
		LoggingData.Records[LoggingData.RecordsQuantity][i] = number[i];
	}
	LoggingData.Records[LoggingData.RecordsQuantity][i] = c;
	i++;

	for(j=0;str[j]!='\0';i++,j++)
	{
		if (i<16) LoggingData.Records[LoggingData.RecordsQuantity][i] = str[j];
		else
		{
			LoggingData.Records[LoggingData.RecordsQuantity][i] = '\0';
			break;
		}
	}
	for (;i<16;i++)
	{
		LoggingData.Records[LoggingData.RecordsQuantity][i] = ' ';
	}
	Print_to_USART1_d(i,"ii: ",0);
	LoggingData.Records[LoggingData.RecordsQuantity][i] = '\0';

	LoggingData.RecordsQuantity++;
	if (LoggingData.RecordsQuantity>=(MAX_LOG_ITEMS))
	{
		for (i=0;i<MAX_LOG_ITEMS-10;i++)
		{
			number = itoa(i);
			uint8_t k;
			for(k=0;number[k]!='\0';k++)
			{
				LoggingData.Records[i][k] = number[k];
			}
			LoggingData.Records[i][k] = c;
			for (j=k+1;LoggingData.Records[i][j]!='\0';j++)
			{
				if (i>=10)
					LoggingData.Records[i][j] = LoggingData.Records[i+10][j];
				else
					LoggingData.Records[i][j] = LoggingData.Records[i+10][j+1];
			}
			j--;
			for (;j<16;j++)
			{
				LoggingData.Records[i][j] = ' ';
			}
			LoggingData.Records[i][j] = '\0';
		}

		LoggingData.RecordsQuantity=MAX_LOG_ITEMS-10;
	}
	flash_write_block();
	Print_to_USART1_d(LoggingData.RecordsQuantity,"Q: ",0);
}



void InfoToUARTBeforeStart(void)
{

	Print_to_USART1_d(CalibrationData.CRC_data,"CRC(Calibration) =",0);
	Print_to_USART1_d(CalibrationData.Calibration0ValueForCurrent ,"Calibration0ValueForCurrent =",0);
	Print_to_USART1_d(CalibrationData.Calibration0ValueForCurrent1 ,"Calibration0ValueForCurrent1 =",0);
	Print_to_USART1_d(CalibrationData.CalibrationForCurrent_mA1 ,"CalibrationForCurrent_mA1 =",0);
	Print_to_USART1_d(CalibrationData.CalibrationForCurrent_mkA1 ,"CalibrationForCurrent_mkA1 =",0);
	Print_to_USART1_d(CalibrationData.CalibrationFor_U_PS1 ,"CalibrationFor_U_PS1 =",0);
	Print_to_USART1_d(CalibrationData.CalibrationFor_U_PS2 ,"CalibrationFor_U_PS2 =",0);
	Print_to_USART1_d(CalibrationData.CalibrationFor_U_PS3 ,"CalibrationFor_U_PS3 =",0);
	Print_to_USART1_d(CalibrationData.ResistanceComp_Ishunt_Wires ,"ResistanceComp_Ishunt_Wires =",0) ;
	Print_to_USART1_d(CalibrationData.ResistanceComp_MOSFET ,"ResistanceComp_MOSFET =",0) ;

	Print_to_USART1_d(SettingsData.CRC_data,"CRC(SettingsData) =",0);
	Print_to_USART1_d(SettingsData.Option1 ,"Option1(menu position) =",0);
	Print_to_USART1_d(SettingsData.ChargeTime ,"ChargeTime =",0);
	Print_to_USART1_d(SettingsData.LowVoltage ,"LowVoltage =",0);
	Print_to_USART1_d(SettingsData.MaxVoltage ,"MaxVoltage =",0);
	Print_to_USART1_d(SettingsData.ChargeAdapt ,"ChargeAdapt =",0);
	Print_to_USART1_d(SettingsData.Swing_Chrg_time ,"Swing_Chrg_time =",0);
	Print_to_USART1_d(SettingsData.Swing_DChrg_time ,"Swing_DChrg_time =",0);

	Print_to_USART1_d(SaveDataWhenPowerOff.CRC_data,"CRC(SaveDataWhenPowerOff) =",0);
	Print_to_USART1_d(SaveDataWhenPowerOff.BatteryCapacityDischargePreviousValue ,"BatteryCapacityDischargePreviousValue =",0);
	Print_to_USART1_d(SaveDataWhenPowerOff.BatteryCapacityDischargeCurrent ,"BatteryCapacityDischargeCurrent =",0);

}

