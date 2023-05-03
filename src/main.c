#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

void init_LD1(){
	RCC.AHB1ENR |= 0x01;
	GPIOA.MODER = (GPIOA.MODER & 0xFFC0FFFF) | 0x00150000;  // Initialisation des ports A8 (rouge), A9 (vert) et A10 (bleu)
	GPIOA.OTYPER &= ~(0x1<<5);
	GPIOA.OSPEEDR |= 0x03<<10;
	GPIOA.PUPDR &= 0xFFC0FFFF;
	
    //GPIOA.MODER = GPIOA.MODER & ~(0x3F << 16); // Initialisation des ports A8 (rouge), A9 (vert) et A10 (bleu)
}

/* Initialisation du timer système (systick) */
void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

/* Initialisation de l'USART */
void init_USART(){
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFFF0F) | 0x000000A0;
	GPIOA.AFRL = (GPIOA.AFRL & 0xFFFF00FF) | 0x00007700;
	USART2.BRR = get_APB1CLK()/9600;
	USART2.CR3 = 0;
	USART2.CR2 = 0;
}

void _putc(const char c){
	while( (USART2.SR & 0x80) == 0)
	  ;  
	USART2.DR = c;
}

void _puts(const char *c){
	int len = strlen(c);
	for (int i=0;i<len;i++){
		_putc(c[i]);
	}
}

char _getc(){
  while( (USART2.SR & 0x20) == 0); 
  return USART2.DR;
  
}


void LD1_red_on(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 8);
	/* GPIOA.ODR = 0xFFFFFFFF; */
}

void LD1_green_on(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 9);
}

void LD1_blue_on(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 10);
}

void tempo_500ms(){
	volatile uint32_t duree;
	/* estimation, suppose que le compilateur n'optimise pas trop... */
	for (duree = 0; duree < 5600000 ; duree++){
		;
	}
}

void LD1_red_off(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 8);
}

void LD1_green_off(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 9);
}

void LD1_blue_off(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 10);
}


volatile int incr = 0;

void win(){
	for(int i = 0; i < 3; i++){
		LD1_green_on();
		tempo_500ms();
		LD1_green_off();
		tempo_500ms();
	}
	// incr = 0; // Pour que l'interruption permette de relancer une séquence (jsp si ça va marcher)
}

void lose(){
	for(int i = 0; i < 3; i++){
		LD1_red_on();
		tempo_500ms();
		LD1_red_off();
		tempo_500ms();
	}
}

void __attribute__((interrupt)) SysTick_Handler(){
   /* Le fait de définir cette fonction suffit pour
	 * qu'elle soit utilisée comme traitant,
	 * cf les fichiers de compilation et d'édition de lien
	 * pour plus de détails.
	 */

	switch(incr){
		case 0: // Allumer en rouge
			LD1_red_on();
			break;
		case 1000: // Eteindre le rouge
			LD1_red_off();
			break;
		case 1500: // Allumer en bleu
			LD1_blue_on();
			break;
		case 2500: // Eteindre le bleu
			LD1_blue_off();
			break;
		case 3000: // Allumer en vert
			LD1_green_on();
			break;
		case 4000: // Eteindre le vert
			LD1_green_off();
			break;
		case 4500: // Allumer en magenta
			LD1_red_on();
			LD1_blue_on();
			break;
		case 5500: // Eteindre le magenta
			LD1_red_off();
			LD1_blue_off();
			break;
		/* case 6000: // Le joueur peut taper la séquence au clavier
			_puts("Séquence tapée par le joueur : \n");
			int n = 0;
			while (n < 4){
				char c = _getc();

				n++;
			}*/
		case 13500: // 5500 + 8000 (8 secondes plus tard), on déclenche la défaite <= fin du prog
			lose();
			exit(1);
			break; 

	}
  
  incr = incr + 1;
}


// On fait clignoter la LED grâce aux interruptions (1 seconde allumée, 1/2 éteinte, jusqu'à ce que la séquence à reproduire soit terminée)

// La séquence générée dure 4*1s + 4* 0,5s = 4+2 = 6 secondes 
// Le joueur a 8 secondes pour reproduire la séquence, après la fin de celle-ci
// Si le joueur se trompe, c'est la fin : il faut relancer le programme pour rejouer (la LED s'allume 3 fois en rouge)
// Sinon, la LED s'allume 3 fois en vert pour indiquer que c'est gagné 

int main(){
    init_LD1(); // Initialisation de la LED tricolore
	systick_init(1000); // Interruption initialisée 
	char seq[4] = "rbvm";
	char *player = malloc(4*sizeof(char));
    while (1){
		puts("Tapez au clavier la séquence de lumières : \n");
		int i = 0;
		while (i < 4){
			char c = _getc();
			player[i] = c;
			i++;
		}
		// Victoire ou défaite du joueur
		if (strcmp(seq, player) == 0){ // Si la séquence tapée est correcte, le joueur a gagné
			win(); // gagne
			return 0;
		}
		else{ // Défaite
			lose();
			return 0;
		}

    }
}