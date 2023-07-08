#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

static volatile char c=0;

// Initialisation de la led LD2
void init_LD2(){
	/* on positionne ce qu'il faut dans les différents
	   registres concernés */
	RCC.AHB1ENR |= 0x01;
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFF3FF) | 0x00000400;
	GPIOA.OTYPER &= ~(0x1<<5);
	GPIOA.OSPEEDR |= 0x03<<10;
	GPIOA.PUPDR &= 0xFFFFF3FF;
}

// Initialisation du monde du bouton poussoir
void init_PB(){
	GPIOC.MODER = GPIOC.MODER & ~(0x3<<26);
}

void tempo_500ms(){
	volatile uint32_t duree;
	/* estimation, suppose que le compilateur n'optimise pas trop... */
	for (duree = 0; duree < 5600000 ; duree++){
		;
	}

}

/* Initialisation du timer système (systick) */
void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

int button_allume(int button){
  if (button != 0) return 0;
  return 1;
}

int LED_boucle_infini_ex1(){
	init_LD2();
	init_PB();
	int button;
	while (1){ // boucle infinie
	
	  button = GPIOC.IDR & 0x1<<13;
	  
		if (button_allume(button)){ // Soucis cablage
		
		  GPIOA.ODR = GPIOA.ODR | 0x1<<5 ; // On allume la LED 0x0020
		  tempo_500ms();
		  GPIOA.ODR = GPIOA.ODR & ~(0x1<<5); // On éteint la LED
		}
	}
	return 1;
}

int clignoter_ex2(){
  init_LD2();
	while (1){ // boucle infinie
	

		GPIOA.ODR = GPIOA.ODR | 0x1<<5 ; // On allume la LED 0x0020
      
      for(int i=0; i<1; i++){
        tempo_500ms();
      }
		  
		GPIOA.ODR = GPIOA.ODR & ~(0x1<<5); // On éteint la LED
		  
      for(int i=0; i<1; i++){
        tempo_500ms();
      }
	}
	return 1;
}

int exo3(){
  init_LD2();
  init_PB();
  int button;
  while (1){
    button = GPIOC.IDR & 0x1<<13;
	  
		if (button_allume(button)){ // Soucis cablage
		
		  GPIOA.ODR = GPIOA.ODR | 0x1<<5 ; // On allume la LED 0x0020
		  tempo_500ms();
		  GPIOA.ODR = GPIOA.ODR & ~(0x1<<5); // On éteint la LED
		}
		else{
		  GPIOA.ODR = GPIOA.ODR | 0x1<<5 ; // On allume la LED 0x0020
      
      for(int i=0; i<1; i++){
        tempo_500ms();
      }
		  
		  GPIOA.ODR = GPIOA.ODR & ~(0x1<<5); // On éteint la LED
		  
      for(int i=0; i<1; i++){
        tempo_500ms();
      }
     }
  }
  return 1;
}

int main() {
  
	printf("\e[2J\e[1;1H\r\n");
	printf("\e[01;32m*** Welcome to Nucleo F446 ! ***\e[00m\r\n");

	printf("\e[01;31m\t%08lx-%08lx-%08lx\e[00m\r\n",
	       U_ID[0],U_ID[1],U_ID[2]);
	printf("SYSCLK = %9lu Hz\r\n",get_SYSCLK());
	printf("AHBCLK = %9lu Hz\r\n",get_AHBCLK());
	printf("APB1CLK= %9lu Hz\r\n",get_APB1CLK());
	printf("APB2CLK= %9lu Hz\r\n",get_APB2CLK());
	printf("\r\n");

    // Exécution de la question 3 de l'exercice 1.1
    exo3();

	while (1);
	return 0;
}

