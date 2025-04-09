/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
#include "derivative.h" /* include peripheral declarations */

//PTB0 es el potenciometro
//PTB1 es el sensor de temperatura
//PTB2 es el sensor de voltaje
//PTB3 es el sensor de corriente

//PTC8 es el LED de bateria
//PTC9 es el buzzer activo
//PTC11 es el switch

unsigned long mV_Pot=0;
unsigned char secuencia_canales[]={(1<<6)+8,(1<<6)+9,(1<<6)+12,(1<<6)+13}; //PTB0,1,2,3
unsigned char resultado_canales[4];
unsigned char ADC_selector=0;
unsigned long DAC_data;
unsigned short dato=0;
unsigned char frecuencia_char;
unsigned long prevVal;
unsigned char i_Buzzer=0;
unsigned char Overheat=0;

void clk_init (void)
{
// FIRC = 4 MHz. BusClk = 4 MHz
// UART0: FIRC. UART1: BusClk. UART2: BusClk. TPM: FIRC. IIC: BusClk. PIT:
	MCG_C1|=(1<<6) + (1<<1);	//MCGOUTCLK : IRCLK. CG: Clock gate, MCGIRCLK enable pag 116
	MCG_C2|=1;					//Mux IRCLK : FIRC (4 MHz) pag 116
	MCG_SC=0;					//Preescaler FIRC 1:1 pag 116

	SIM_CLKDIV1=0;				//OUTDIV4=OUTDIV1= 1:1 pag 116. Busclk 4 MHz
	SIM_SOPT2|=15<<24;			//Seleccion MCGIRCLK tanto para UART0 como para TPM
}

void UART1_init(void)
{
	
	SIM_SCGC5|=(1<<13);		//PORTE
	PORTE_PCR0=(3<<8);		//UART1 TX
	PORTE_PCR1=(3<<8);		//UART1 RX
	
	SIM_SCGC4|=(1<<11);	//UART1
	UART1_BDL=26;	//baud_rate (9600)=4MHz/(16*9600)
	UART1_C2=(3<<2)+(1<<5); //Transmit & Receive Enabled, Receiver Interrupt Enable
	NVIC_ISER=(1<<13);
}

void ADC_init()
{
	SIM_SCGC6|=(1<<27); //ADC0
	NVIC_ISER=(1<<15); //Hab intr NVIC ADC
}

void PIT_init()
{
	SIM_SCGC6|=(1<<23); //PIT
	PIT_MCR=0;
	PIT_LDVAL0=400000;  //10 ms entre cada channel, clk : 4 MHz
	PIT_TCTRL0=3; //TEN=1, TIEN=1
	NVIC_ISER=(1<<22); //Intr PIT
}

void DAC0_init(void)
{
	SIM_SCGC6 |= (1<<31);    // clock to DAC module SIM_SCG6=SIMSCG6|
	DAC0_C0 = (1<<7)+ (1<<5);      // enable DAC and use software trigger
}

void TMP0_C2_init(void)
{
	SIM_SCGC5|=(1<<9);    //PORTA
	PORTA_PCR5=(3<<8);    //TPM0_CH2
		
	SIM_SCGC6|=(1<<24);	  //TPM0
	TPM0_SC=(1<<3)+2;	  //CMOD=1 (4 mHz), preescaler 4
	TPM0_C2SC=(1<<2)+(1<<6);     //input capture, falling edge, CHIE=1
	NVIC_ISER=(1<<17);
	//Podria intentar rising edge
}

void GPIO_init()
{
	SIM_SCGC5|=(1<<11); //Activar Puerto C
	PORTC_PCR8=(1<<8);	//Hacer GPIO PTC8
	PORTC_PCR9=(1<<8);	//Hacer GPIO PTC9
	PORTC_PCR11=(1<<8);	//GPIO PTC11
	GPIOC_PDDR=(3<<8);	//Output Pines 8 y 9
	GPIOC_PCOR=(3<<8);	//Apagar las salidas
}

void PIT_IRQHandler()
{
	if (PIT_TFLG0==1) //Si es el timer 0
	{
		PIT_TFLG0=1; //Apaga bandera
		if (ADC_selector==0)
		{
			UART1_D='/';
			do{}while ((UART1_S1 & (1<<7))==0);	
		}	
		ADC0_SC1A=secuencia_canales[ADC_selector];	//Inicia conversion ADC depende del ADC_selector

	}
}

void ADC0_IRQHandler()
{
	resultado_canales[ADC_selector++]=ADC0_RA; //Resultado ADC
	if (ADC_selector==1)
	{
		if ((Overheat==0) && (GPIOC_PDIR & (1<<11))==0)
		{
			mV_Pot=resultado_canales[0]*6+1000;
			DAC_data=(4095*mV_Pot/3300)+30;	//Treat data to convert into DAC writing values
			DAC0_DAT0L = DAC_data & 0xff;  //write low byte
			DAC0_DAT0H = (DAC_data >> 8); // write high byte
		}
		if (Overheat==1)
		{
			DAC_data=0;	//Treat data to convert into DAC writing values
			DAC0_DAT0L = DAC_data & 0xff;  //write low byte
			DAC0_DAT0H = (DAC_data >> 8); // write high byte
		}
		UART1_D=frecuencia_char;	//Manda un identificador para poder separar el codigo despues
		do{}while ((UART1_S1 & (1<<7))==0);		
	}
	if (ADC_selector==2)
	{
		GPIOC_PCOR=(1<<9); //Apagar Buzzer
		UART1_D=resultado_canales[1];	//Temperatura
		do{}while ((UART1_S1 & (1<<7))==0);
		
		if (resultado_canales[1]>100)
		{
			//Overheat=1;//Sobrecalentado
		}
		if (resultado_canales[1]<100)
		{
			Overheat=0;
		}
	}
	if (ADC_selector==3)
	{
		UART1_D=resultado_canales[2];	//Bateria
		do{}while ((UART1_S1 & (1<<7))==0);		
		if (resultado_canales[2]<210)//Bateria baja
		{
			GPIOC_PSOR=(1<<8); //Prender LED
			
			if (i_Buzzer++==10)
			{
				GPIOC_PSOR=(1<<9);
				i_Buzzer=0;
			}
			
		}
		if (resultado_canales[2]>210)
		{
			GPIOC_PCOR=(3<<8);	//Apagar
		}
	}
	if (ADC_selector==4)
	{
		UART1_D=resultado_canales[ADC_selector-1];
		do{}while ((UART1_S1 & (1<<7))==0);	
		if (resultado_canales[3]>230)	//8 Ampers de corriente
		{
			Overheat=1;
		}
		if(resultado_canales[3]<230)
		{
			Overheat=0;
		}
		ADC_selector=0;
		
	}
		
}

void UART1_IRQHandler()
{
//	if (UART1_S1 & (1<<6)==1) //Transmission Complete
//	{
//
//	}
	unsigned char temp;
	if ((UART1_S1 & (1<<5))==(1<<5))	//Receive Data Register Full Flag
	{
		temp=UART1_D;	//Save UART value in temporal variable
		
		if ((Overheat==0) && ((GPIOC_PDIR & (1<<11))!=0))
		{
			if ((temp!=59) && (temp>='0') && (temp<='9')) 
			//Checar que no sea ";", que sea un número y que este activado el switch
			{
				dato=(dato*10)+temp-0x30;	
			}
			if (temp==59)	//If it is ; Key
			{
				DAC_data=(4095*dato/3300)+30;	//Treat data to convert into DAC writing values
				DAC0_DAT0L = DAC_data & 0xff;  //write low byte
				DAC0_DAT0H = (DAC_data >> 8); // write high byte
				dato=0;	//Limpiar dato, listo para proximo
			}
		}
	}
}

void FTM0_IRQHandler()
{
	unsigned long periodo;
	unsigned long frecuencia;
	
	TPM0_C2SC |=(1<<7);	//Apagar la bandera de la interrupcion
	periodo=(unsigned long)TPM0_C2V-prevVal;	//Leer la diferencia entre el valor pasado y el actual
	frecuencia=1000000/periodo+2;	//Calculos de frecuencia
	frecuencia_char=(unsigned char)frecuencia;	//Convertir frecuencia en variable char
	prevVal=TPM0_C2V;	//Guardar el valor actual como el anterior
}

int main(void)
{
	clk_init();
	UART1_init();
	ADC_init();
	PIT_init();
	DAC0_init();
	TMP0_C2_init();
	GPIO_init();
	
	while (1);
	
	return 0;
}
