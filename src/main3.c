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

// Initialisation du mode du bouton poussoir
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

void tempo_100ms(){
	volatile uint32_t duree;
	/* estimation, suppose que le compilateur n'optimise pas trop... */
	for (duree = 0; duree < 1600000 ; duree++){
		;
	}

}

// Initialisation de l'USART
void init_USART(){
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFFF0F) | 0x000000A0;
	GPIOA.AFRL = (GPIOA.AFRL & 0xFFFF00FF) | 0x00007700;
	USART2.BRR = get_APB1CLK()/9600;
	USART2.CR3 = 0;
	USART2.CR2 = 0;
}

// Ecrire un char
void _putc(const char c){
	while( (USART2.SR & 0x80) == 0) // Tant que la donnée n'est pas transférée (TXNE)
	  ;  
	USART2.DR = c; // On écrit le caractère
}

// Ecrire une chaîne de caractères
void _puts(const char *c){
	int len = strlen(c);
	for (int i=0;i<len;i++){
		_putc(c[i]);
	}
}

// Récupère un char
char _getc(){
  while( (USART2.SR & 0x20) == 0); // Tant que la transmission n'est pas terminée (TC)
  return USART2.DR;
  
}

/* Initialisation du timer système (systick) */
void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

/* Fonction non bloquante envoyant une chaîne par l'UART */
int _async_puts(const char* s) {
	/* Cette fonction doit utiliser un traitant d'interruption
	 * pour gérer l'envoi de la chaîne s (qui doit rester
	 * valide pendant tout l'envoi). Elle doit donc être
	 * non bloquante (pas d'attente active non plus) et
	 * renvoyer 0.
	 *
	 * Si une chaîne est déjà en cours d'envoi, cette
	 * fonction doit renvoyer 1 (et ignorer la nouvelle
	 * chaîne).
	 *
	 * Si s est NULL, le code de retour permet de savoir
	 * si une chaîne est encore en cours d'envoi ou si
	 * une nouvelle chaîne peut être envoyée.
	 */
	/* if (Chaine deja en cours d'envoie'){
		return 1;
	}
	if (s==NULL){

	}
	traitant
	*/
	return 0; 
}

int button_allume(int button){
  if (button != 0) return 0;
  return 1;
}

int affiche(){ 
  _puts("12");
  while (1){
    char c = _getc();
    _putc(c);
   }
  return 1;
}

/* La réception d'un caractère sur le clavier (entrée standard)
 ralentit la fréquence de clignotement, SANS interruption de l'USART2 */

int exo3(){
	init_LD2();
	int freq = 1; // Fréquence de clignotement
	while(1){
		// Si on reçoit un caractère, on réactualise freq
		if (USART2.SR && 0x20 != 0){ 
			freq++;
		}

		GPIOA.ODR = GPIOA.ODR | 0x1<<5 ; // On allume la LED 0x0020

		for(int i = 0; i < freq; i++){ // Ralentissement de freq, donc on laisse allumer plus longtemps...
			tempo_500ms();
		}

		GPIOA.ODR = GPIOA.ODR & ~(0x1<<5); // On éteint la LED

		for(int i = 0; i < freq; i++){ // ... et éteint plus longtemps
			tempo_500ms();
		}
	}
}

/* La réception d'un caractère sur le clavier (entrée standard)
 ralentit la fréquence de clignotement, AVEC interruption de l'USART2 */
// Reprendre la fonction exo5() et le mettre dans une interruption

volatile int incr = 0;
volatile int freq = 200;

void __attribute__((interrupt)) USART2_Handler(){
	if (incr % freq == 0){
		incr = 0;
		GPIOA.ODR = GPIOA.ODR ^ 0x1<<5 ; // On inverse l'état de la LED 0x0020
      	GPIOA.ODR = GPIOA.ODR | 0x1<<4 ;

	}
	if (USART2.SR && 0x20 != 0){ // Réception d'un caractère
		freq += 200;
	}

	incr++;
	// deux if permettant d'allumer et d'éteindre la led
	// un if qui receptionne l'entrée d'un carac et qui change la fréquence
}


volatile int canarticho = 1;
volatile int relache = 1;


void __attribute__((interrupt)) SysTick_Handler(){
   /* Le fait de définir cette fonction suffit pour
	 * qu'elle soit utilisée comme traitant,
	 * cf les fichiers de compilation et d'édition de lien
	 * pour plus de détails.
	 */
	int button = GPIOC.IDR & 0x1<<13;
	
	if (button_allume(button) && relache){
    canarticho+=2;
    relache = 0;
  }
  else if (!button_allume(button)){
    relache = 1;
  }
  
  if (incr >= 1000){
      incr = 0;
      GPIOA.ODR = GPIOA.ODR ^ 0x1<<5 ; // On inverse l'état de la LED 0x0020
      GPIOA.ODR = GPIOA.ODR | 0x1<<4 ;
    }
  
  incr = incr + canarticho;
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

	// Exécution de la question 3 de l'exercice 1.3 (plus grande question réussie)
	exo3();
	
	//init_LD2();
  	//init_USART();
	while (1);
	return 0;
}

