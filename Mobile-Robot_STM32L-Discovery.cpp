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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ADC_HandleTypeDef hadc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

//SONIDO INTERMITENTE ZUMBADOR
	 char intermitente = 0;
	 unsigned short tiempo = 500;

//CALCULAR DISTANCIA
 	 unsigned int distancia_cm = 0; //distancia en cm que mide el sensor
 	 unsigned int distancia_tiempo = 0;//diferencias de tiempo entre el primer flanco del pulso y el segundo

//SENSOR ULTRASONIDOS
 	 //ECHO-> RECIBIR EL PULSO
 	 unsigned short inicio_ECHO = 0;
 	 unsigned short fin_ECHO = 0;

 	 //TRIGGER->GENERAR EL PULSO
 	 unsigned short inicio_pulso = 0;
 	 unsigned short fin_pulso = 0;

//DC PWM PARA LAS DISTINTAS VELOCIDADES
 	 unsigned short DC = 0;
 	 unsigned short DC_MAX = 0;
 	 unsigned short DC_MED = 0;

//VALOR QUE LEO DEL ADC
     unsigned short valor = 0;

//CAMBIO DE ESTADO-> PROTOCOLO DE GIRO
 	 unsigned int girar_derecha=0;
 	 unsigned int girar_izquierda=0;
 	 unsigned int vuelta_atras=0;
 	 unsigned short automatico = 0;

//INTERRUPCIONES
 	 uint8_t fin =0; //GIROS Y PARADAS
 	 uint8_t fin_trigger=0; //TRIGGER
 	 uint8_t cambio_logico = 0; //ECHO

//MODULO BLUETOOTH
 	uint8_t mensaje_recibir[7] = "       ";
 	uint8_t mensaje_Transmitir[6] = "      ";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_TS_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void generar_pulso();
static void recibir_pulso();
static void calcular_distancia();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void TIM3_IRQHandler(void){ //MODO TOC
	if ((TIM3->SR & 0x0004)!=0){//CANAL 2-->SONIDO INTERMITENTE
			if(intermitente){
				intermitente=0;
			}else{
				intermitente=1;
			}
			TIM3->CCR2 += tiempo;
			}

		if ((TIM3->SR & 0x0002)!=0){//CANAL 1-->GIROS Y PARADAS
			if(fin==0){
				fin=1;
			}
		}
		TIM3->SR = 0x0000;//limpio el flag
}


	void TIM2_IRQHandler(void) { //por problemas con el sensor no hemos podido implementarlas
		if ((TIM2->SR & 0x0004)!=0){	//CANAL 2(TOC)-->ENVIAR EL PULSO

			if(fin_trigger==0){
				fin_trigger=1;
			}

		} else if((TIM2->SR & (1<<1))!=0){//CANAL 1(TIC)-->RECIBIR EL PULSO
			if(cambio_logico==0){
				cambio_logico=1;
			}
		}

		TIM2->SR = 0x0000;//limpio el flag
	}


static void generar_pulso(){ //TRIGGER

	  GPIOD->BSRR |= (1<<2);
	  inicio_pulso = TIM2->CNT;
	  fin_pulso = inicio_pulso + 20;
	  while(TIM2->CNT < fin_pulso);
	  GPIOD->BSRR |= (1<<(2+16));

}

static void recibir_pulso(){ //ECHO

	  while((TIM2->SR & 0x0002)==0);
	  inicio_ECHO = TIM2->CCR1;
	  TIM2->SR = 0;
	  while((TIM2->SR & 0x0002)==0);
	  fin_ECHO = TIM2->CCR1;
	  TIM2->SR = 0;

}

static void calcular_distancia(){ //CALCULO LA DISTANCIA
	  distancia_tiempo = fin_ECHO - inicio_ECHO;
	  distancia_cm = (unsigned int)((distancia_tiempo*343)/20000); //formula
}


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
  MX_ADC_Init();
  MX_TS_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  //---------------------------MODULO BLUETOOTH------------------------------------------------------------------------------------------------------------------------------------------------------------

  HAL_UART_Receive_IT(&huart1, mensaje_recibir, 1); //implementación de interrupciones a la recepción de la usart1 (modulo bluetooth)

//trasmisión sin interrupciones
 unsigned char mensaje_inicial_USART[40] ="Control del robot: 1, 2, 3, 4, 5, 6 o 7";
 HAL_UART_Transmit(&huart1, mensaje_inicial_USART, 40, 10000);
 mensaje_Transmitir[0] = 0x0A;                 // Añado un salto de línea
 HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);
 mensaje_Transmitir[0] = 0x0D;                  // Y un comienzo de línea
 HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);


 //---------------------------GPIO------------------------------------------------------------------------------------------------------------------------------------------------------------
  //PONGO PA1 (ZUMBADOR) COMO SALIDA DIGITAL
  	GPIOA->MODER &=~(1 << 3);//COLOCO 0
  	GPIOA->MODER |= (1 << 2); //COLOCO 1

  //PONGO PD2 (TRIG) COMO SALIDA DIGITAL
    GPIOD->MODER &= ~(1 << 5);//COLOCO 0
    GPIOD->MODER |= (1 << 4); //COLOCO 1

  //PONGO PA5 (ECHO) COMO FUNCION ALTERNATIVA
    GPIOA->MODER|= (1 << 11);//COLOCO 1
    GPIOA->MODER &=~ (1 << 10); //COLOCO 0
    GPIOA->AFR[0]|=(0x01 << (5*4));// AFR para decir que el PA5 es AF1
  //(TIM2)
    GPIOA->AFR[0]&=~(0x0E << (5*4));

  //MOTOR DERECHO
  // PB8-> AF TIM4-CH3
    GPIOB->MODER|=0x00000001 << (2*8 +1);  // MODER = 10 (AF) para el bit 7 del
    GPIOB->MODER&=~(1 << (2*8));
    GPIOB->AFR[1]|=(0x02 << (0*4));        // AFR[0] para decir que el PB7 tiene    CAMBIO
    // la AF2 (TIM4)
 // PA11 COMO SALIDA DIGITAL
    GPIOA->MODER &= ~(1 << (11*2 +1));//COLOCO 0
    GPIOA->MODER |= (1 << (11*2));//COLOCO 1

  //MOTOR IZQUIERDO
  // PB9-> AF TIM4-CH3
      GPIOB->MODER |= (1 << (9*2 + 1));
      GPIOB->MODER &= ~(1 << (9*2));
      GPIOB->AFR[1]|=(0x02 << (1*4)); // AF2 para PB9 -> Reg1 1 del AFR[1]
  // PA12  COMO SALIDA DIGITAL
      GPIOA->MODER &= ~(1 << (12*2 +1));//COLOCO 0
      GPIOA->MODER |= (1 << (12*2));//COLOCO 1

  //POTENCIÓMETRO-> PA4 COMO ANALÓGICO
      GPIOA->MODER |= 0x00000300;

//---------------------------TIM3------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Selección del reloj interno: CR1, CR2, SMRC
  	TIM3->CR1 = 0x0000;  // ARPE = 0 -> No es PWM, es TOC
  	                         // CEN = 0; Contador apagado
  	TIM3->CR2 = 0x0000;  // Siempre "0" en este curso
  	TIM3->SMCR = 0x0000; // Siempre "0" en este curso

  // Configuración del funcionamiento del contador: PSC, CNT, ARR y CCRx
  	TIM3->PSC = 31999;   // Preescalado = 32000 -> Frecuencia del contador = 32000/32000 = 1 paso por segundo
  	TIM3->CNT = 0;       // Inicializo el valor del contador a cero
  	TIM3->ARR = 0xFFFF;  // Valor recomendado = FFFF
  	TIM3->CCR1 = 500 ;   // Registro donde se guarda el valor que marca la comparación existosa en TOC.

  // Selección de IRQ o no: DIER
    TIM3->DIER = 0x0004; // Se genera INT al terminar de contar -> CCyIE = 1

  // Modo de salida del contador
 	TIM3->CCMR1 = 0x0000;  // CCyS = 0 (TOC)
  	                           // OCyM = 000 (no hay salida por el pin HW asociado al
  	 // TIM4)
  	                           // OCyPE = 0 (sin precarga)
  	 TIM3->CCER = 0x0000;   // CCyP = 0 (siempre para TOC)
  	                           // CCyE = 0 (desactivada la salida hardware)

  // Habilitación de contador y limpieza de flags
  	 TIM3->EGR |= 0x0001;   // UG = 1 -> Se genera evento de actualización
  	 TIM3->SR = 0;          // Limpio los flags del contador
  	 TIM3->CR1 |= 0x0001;   // CEN = 1 -> Arranco el contador

  // Habilitación de la interrupción TIM3_IRQ en el NVIC (posición 29).
  	 NVIC->ISER[0] |= (1 << 29);

 //---------------------------TIM2------------------------------------------------------------------------------------------------------------------------------------------------------------

     // Selección del reloj interno: CR1, CR2, SMRC

     TIM2->CR1 =0x0000;  // ARPE = 0 -> No es PWM, es TIC
     TIM2->CR2 = 0x0000;  // Siempre "0" en este curso
     TIM2->SMCR = 0x0000; // Siempre "0" en este curso

     // Configuración del funcionamiento del contador: PSC, CNT, ARR y CCRx

     TIM2->PSC = 31;   // Preescalado = 31 micro segundos
     TIM2->CNT = 0;       // Inicializo el valor del contador a cero
     TIM2->ARR = 0xFFFF;  // Valor recomendado = FFFF

     // Selección de IRQ o no: DIER

     TIM2->CCMR1 = 0x0001;  // CCyS = 1 (TIC); OCyM = 000 y OCyPE = 0 (siempre en TIC)
     TIM2->CCER = 0x000B;  //habilitado la captura de ambos flancos

     // Habilitación de contador y limpieza de flags

     TIM2->EGR |= 0x0001;   // UG = 1 -> Se genera evento de actualización
     TIM2->SR = 0;          // Limpio los flags del contador
     TIM2->CR1 |= 0x0001;   // CEN = 1 -> Arranco el contador

//---------------------------ADC------------------------------------------------------------------------------------------------------------------------------------------------------------
     // Configuración ADC
     ADC1->CR2 &= ~(0x00000001);    // ADON = 0 (ADC apagado)
     ADC1->CR1 = 0x00000000;        // RES = 00 (resolución = 12 bits)
                                    // SCAN = 0 (modo scan deshabilitado)
                                    // EOCIE = 0 (deshabilitada la interupción por
    // EOC)

     ADC1->CR2 = 0x00000412;        // EOCS = 1 (activado el bit EOC al acabar cada
    // conversión)
                                    // DELS = 001 (retardo de la conversión hasta
    // que se lea el dato anterior)
                                    // CONT = 1 (conversión continua)
     ADC1->SMPR1 = 0;               // Sin sampling time (4 cycles)
     ADC1->SMPR2 = 0;
     ADC1->SMPR3 = 0;
     ADC1->SQR1 = 0x00000000;       // 1 elemento solo en la secuencia
     ADC1->SQR5 = 0x00000004;       // El elemento es el canal AIN4
     ADC1->CR2 |= 0x00000001;       // ADON = 1 (ADC activado)

     while ((ADC1->SR&0x0040)==0);  // Si ADCONS = 0, o sea no estoy para convertir,

     ADC1->CR2 |= 0x40000000;       // Si ADCONS = 1, arranco la conversion (SWSTART
    // = 1)

 //---------------------------TIM4------------------------------------------------------------------------------------------------------------------------------------------------------------
         // Selección del reloj interno: CR1, CR2, SMRC
         TIM4->CR1 = 0x0080;                    // ARPE = 1 -> Es PWM; CEN = 0;
                // Contador apagado
         TIM4->CR2 = 0x0000;                    // Siempre "0" en este curso
         TIM4->SMCR = 0x0000;                   // Siempre "0" en este curso

         // Configuración del funcionamiento del contador: PSC, CNT, ARR y CCRx
         TIM4->PSC = 3200;      // Preescalado=32000 -> f_contador=32000000/32000 =      CAMBIO
       // 1000 pasos/segundo
         TIM4->CNT = 0;          // Inicializo el valor del contador a cero
         TIM4->ARR = 9;          // Pongo una frecuencia PWM de 100 Hz y sólo cuento
       // 10 pasos      // El Duty cycle se pone a 1 inicialmente             CAMBIO
         // Selección de IRQ o no: DIER
         TIM4->DIER = 0x0000;   // No se genera INT al terminar de contar -> CCyIE = 0

         // Modo de salida
         TIM4->CCMR2 = 0x6868;  // CCyS = 0   (TOC, PWM)
                        	  // OCyM = 110 (PWM con el primer semiciclo a 1)
                                // OCyPE = 1  (con precarga)
         TIM4->CCER = 0x1100;   // CCyP = 0   (siempre en PWM)
                                // CCyE = 1   (activada la salida hardware)
         // Habilitación de contador y limpieza de flags
         TIM4->EGR |= 0x0001;   // UG = 1 -> Se genera evento de actualización
         TIM4->SR = 0;          // Limpio los flags del contador
         TIM4->CR1 |= 0x0001;   // CEN = 1 -> Arranco el contado
         TIM4->CCR3 = 0;
         TIM4->CCR4 = 0;

//DESACTIVO ZUMBADOR PARA QUE NO SUENE FUNCIONA A NIVEL BAJO->PONGO UN 1
	     GPIOA->BSRR = (1<<1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //MEDIMOS VALOR DEL POTENIOMETRO
	    valor=ADC1->DR;

	  //AJUSTAMOS EL DC DEL PWM
	    if(valor<1024){
	  		 DC_MAX=5;
	  		 DC_MED=1;
	  	}
	  	 else if(valor<3072 && valor>=1024){
	  		 DC_MAX=7;
	  		 DC_MED=3;
	  	}
	  	else{
	  		 DC_MAX=8;
	  		 DC_MED=6;
	  	 }

			  switch(mensaje_recibir[0]){

			  	case 0x31://PARAR-->RECEPCION DE UN 1

			  	HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Parar", sizeof("Modo Parar"), 10000);

			  	mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  	HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  	mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  	HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  	GPIOA->BSRR = (1<<12)<<16;
			  	TIM4->CCR3 = 0;
			  	TIM4->CCR4 = 0;
			  	GPIOA->BSRR = (1<<11)<<16;

			  	break;

			  	case 0x32://ADELANTE-->RECEPCIÓN DE UN 2
			  		HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Adelante", sizeof("Modo Adelante"), 10000);

			  		mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		GPIOA->BSRR = (1<<12)<<16;
			  		GPIOA->BSRR = (1<<11)<<16;

			  		TIM4->CCR3 = DC_MAX;
			  		TIM4->CCR4 = DC_MAX;

			  	break;
			  	case 0x33://MARCHA ATR�?S-->RECEPCION DE UN 3
			  		HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Atras", sizeof("Modo Atras"), 10000);

			  		mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		GPIOA->BSRR = (1<<12);
			  		TIM4->CCR3 = 0;
			  		TIM4->CCR4 = 0;
			  		GPIOA->BSRR = (1<<11);

			  	break;
			  	case 0x34://DERECHA--> RECEPCION DE UN 4
			  		HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Giro Derecha", sizeof("Modo Giro Derecha"), 10000);

			  		mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);
			  		GPIOA->BSRR = (1<<12)<<16;
			  		TIM4->CCR3 = DC_MED;
			  		TIM4->CCR4 = 0;
			  		GPIOA->BSRR = (1<<11)<<16;

			  	break;
			  	case 0x35://IZQUIERDA-->RECIBO UN 5
			  		HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Giro Izq", sizeof("Modo Giro Izq"), 10000);

			  		mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		GPIOA->BSRR = (1<<12)<<16;
			  		TIM4->CCR3 = 0;
			  		TIM4->CCR4 = DC_MED;
			  		GPIOA->BSRR = (1<<11)<<16;

			  	break;

			  	case 0x36:
			  		automatico = 1;
			  		HAL_UART_Transmit(&huart1, (unsigned char*) "Modo Automatico", sizeof("Modo Automatico"), 10000);

			  		mensaje_Transmitir[0] = 0x0A;         // Añado un salto de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);

			  		mensaje_Transmitir[0] = 0x0D;          // Y un comienzo de línea
			  		HAL_UART_Transmit(&huart1, mensaje_Transmitir, 1, 10000);


			  		while (automatico){
			  		//MEDIMOS DISTANCIA
			  			TIM2->CNT=0;
			  			generar_pulso();
			  			recibir_pulso();
			  			calcular_distancia();

			  			if(distancia_cm >10){ // DITANCIA > 10
			  				if(distancia_cm < 20){ //DISTANCIA ENTRE 10 Y 20
			  					//SONIDO INTERMITENTE
			  				  	if(intermitente){
			  				  		GPIOA->BSRR = (1<<1);
			  				  	}else{
			  				  		GPIOA->BSRR = (1<<1)<<16;
			  				  	}

			  				  	//CAMINO DESPACIO
			  				  	TIM4->CCR3 = DC_MED;
			  				  	GPIOA->BSRR = (1<<11)<<16;

			  				  	TIM4->CCR4 = DC_MED;
			  				  	GPIOA->BSRR = (1<<12)<<16;

			  				}else{ //DISTANCIA >20

			  					//APAGO EL PITIDO
			  				  	GPIOA->BSRR = (1<<1);

			  				  	//CAMINO RÁPIDO
			  				  	TIM4->CCR3 = DC_MAX;
			  				  	GPIOA->BSRR = (1<<11)<<16;

			  				  	TIM4->CCR4 = DC_MAX;
			  				  	GPIOA->BSRR = (1<<12)<<16;
			  				  }
			  			}else{ //DISTANCIA <10CM

			  				//PITIDO CONSTANTE
			  				GPIOA->BSRR=(1<<1)<<16;

			  				//PARO LOS MOTORES
			  				TIM4->CCR3 = 0;
			  				TIM4->CCR4 = 0;

			  				girar_derecha=1;
			  			}

			  			//GIRAR DERECHA
			  			if(girar_derecha){
			  				//PARAMOS MEDIO SEGUNDO
			  				TIM3 -> CCR1 = TIM3->CNT + 500;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0);
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);

			  				//GIRO 90º A LA DERECHA
			  				TIM3 -> CCR1 = TIM3->CNT + 500;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0){
			  					TIM4->CCR4 = 0;
			  					GPIOA->BSRR = (1<<11);
			  				}
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);

			  				TIM4->CCR4 = 0;
			  				GPIOA->BSRR = (1<<11)<<16;

			  				//PARAMOS MEDIO SEGUNDO
			  				TIM3 -> CCR1 = TIM3->CNT + 500;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0);
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);

			  				girar_derecha = 0;
			  				//VOLVEMOS A MEDIR LA DISTANCIA
			  				generar_pulso();
			  				recibir_pulso();
			  				calcular_distancia();

			  				if(distancia_cm<10){
			  				  girar_izquierda=1;
			  				  TIM4->CCR4=0;
			  				  TIM4->CCR3=0;

			  				}
			  			}

			  			if(girar_izquierda){
			  				//PARAMOS MEDIO SEGUNDO
			  				TIM3 -> CCR1 = TIM3->CNT + 500;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0);
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);


			  				//GIRAMOS 180º A LA IZQUIERDA
			  				TIM3 -> CCR1 = TIM3->CNT + 1000;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0){
			  					TIM4->CCR3 = 0;
			  					GPIOA->BSRR = (1<<12);
			  				}
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);
			  				TIM4->CCR3=0;
			  				GPIOA->BSRR = (1<<12)<<16;

			  				//PARAMOS MEDIO SEGUNDO
			  				TIM3 -> CCR1 = TIM3->CNT + 500;
			  				TIM3->DIER = 0x0002;
			  				while(fin==0);
			  				fin=0;
			  				TIM3->DIER &= ~(1<<1);

			  				girar_izquierda=0;

			  				//PROCESO DISTANCIA
			  				generar_pulso();
			  				recibir_pulso();
			  				calcular_distancia();

			  				if(distancia_cm<10){
			  				  vuelta_atras=1;
			  				  TIM4->CCR4=0;
			  				  TIM4->CCR3=0;
			  				 }
			  		}

			  		if(vuelta_atras){
			  			//PARAMOS MEDIO SEGUNDO
			  			TIM3 -> CCR1 = TIM3->CNT + 500;
			  			TIM3->DIER = 0x0002;
			  			while(fin==0);
			  			fin=0;
			  			TIM3->DIER &= ~(1<<1);

			  			//GIRAMOS 90º HACIA LA IZQUIERDA
			  			TIM3 -> CCR1 = TIM3->CNT + 500;
			  			TIM3->DIER = 0x0002;
			  			while(fin==0){
			  				TIM4->CCR3 = 0;
			  				GPIOA->BSRR = (1<<11);
			  			}
			  			fin=0;
			  			TIM3->DIER &= ~(1<<1);

			  			TIM4->CCR4 = 0;
			  			GPIOA->BSRR = (1<<11)<<16;
			  			TIM4->CCR3=0;
			  			GPIOA->BSRR = (1<<11)<<16;
			  			//PARAMOS MEDIO SEGUNDO
			  			TIM3 -> CCR1 = TIM3->CNT + 500;
			  			TIM3->DIER = 0x0002;
			  			while(fin==0);
			  			fin=0;
			  			TIM3->DIER &= ~(1<<1);
			  			vuelta_atras=0;

			  			generar_pulso();
			  			recibir_pulso();
			  			calcular_distancia();

			  		}
			  }
			  break;

	      }





	          	  //TIM2->CNT = 0;


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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_CC3;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc.Init.DMAContinuousRequests = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TS Initialization Function
  * @param None
  * @retval None
  */
static void MX_TS_Init(void)
{

  /* USER CODE BEGIN TS_Init 0 */

  /* USER CODE END TS_Init 0 */

  /* USER CODE BEGIN TS_Init 1 */

  /* USER CODE END TS_Init 1 */
  /* USER CODE BEGIN TS_Init 2 */

  /* USER CODE END TS_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : SEG14_Pin SEG15_Pin SEG16_Pin SEG17_Pin
                           SEG18_Pin SEG19_Pin SEG20_Pin SEG21_Pin
                           SEG22_Pin SEG23_Pin */
  GPIO_InitStruct.Pin = SEG14_Pin|SEG15_Pin|SEG16_Pin|SEG17_Pin
                          |SEG18_Pin|SEG19_Pin|SEG20_Pin|SEG21_Pin
                          |SEG22_Pin|SEG23_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_LCD;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG1_Pin SEG2_Pin COM0_Pin COM1_Pin
                           COM2_Pin SEG12_Pin */
  GPIO_InitStruct.Pin = SEG1_Pin|SEG2_Pin|COM0_Pin|COM1_Pin
                          |COM2_Pin|SEG12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_LCD;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG6_Pin SEG7_Pin SEG8_Pin SEG9_Pin
                           SEG10_Pin SEG11_Pin SEG3_Pin SEG4_Pin
                           SEG5_Pin */
  GPIO_InitStruct.Pin = SEG6_Pin|SEG7_Pin|SEG8_Pin|SEG9_Pin
                          |SEG10_Pin|SEG11_Pin|SEG3_Pin|SEG4_Pin
                          |SEG5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_LCD;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(huart, mensaje_recibir, 1); // Vuelve a activar Rx por haber acabado
 // el buffer
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
