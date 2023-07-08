#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

//condition : taille de chaque seqquence est minimum egale a 4 maximum egale a 8
char liste [6][10]={"rbv","rmvb","brbrm","vmrmvr","brmbrrvm", "mrvrmvmvbv"};
char* mot; 


void init_LD1(){
	RCC.AHB1ENR |= 0x01;
	GPIOA.MODER = (GPIOA.MODER & 0xFFC0FFFF) | 0x00150000;  // Initialisation des ports A8 (rouge), A9 (vert) et A10 (bleu)
	GPIOA.OTYPER &= ~(0x1<<5);
	GPIOA.OSPEEDR |= 0x03<<10;
	GPIOA.PUPDR &= 0xFFC0FFFF;
	
    //GPIOA.MODER = GPIOA.MODER & ~(0x3F << 16); // Initialisation des ports A8 (rouge), A9 (vert) et A10 (bleu)
}

// Initialisation du mode du bouton poussoir
void init_PB(){
	GPIOC.MODER = GPIOC.MODER & ~(0x3<<26);
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
volatile int won = 0;

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


void allumer (char lettre){
	switch(lettre){
		case 'r':
			LD1_red_on();
			break; 
		case 'b':
			LD1_blue_on();
			break; 
		case 'v':
			LD1_green_on();
			break; 
		case 'm':
			LD1_blue_on();
			LD1_red_on();
			break ;
		default:
			break; 
	}
}

void eteindre (char lettre){
		switch(lettre){
		case 'r':
			LD1_red_off();
			break; 
		case 'b':
			LD1_blue_off();
			break; 
		case 'v':
			LD1_green_off();
			break; 
		case 'm':
			LD1_blue_off();
			LD1_red_off();
			break ;
		default:
			break; 
	}
}


volatile int i = 0;
volatile int length;
volatile int on = 1;

void __attribute__((interrupt)) SysTick_Handler(){

	if (won) { // Si on gagne, on ré-initialise le compteur incr pour commencer une nouvelle séquence
		won = 0;
		incr = 0;
		i = 0;
	}

	if (incr == 15000){ // Compteur maximum atteint (15 secondes)
		lose();
		exit(1);
	}
	else if (on && (incr % 500 == 0) && i < length) { // On allume
		allumer(mot[i]);
		on = 0;
	}
	else if (!on &&(incr % 1000 == 0) && i < length){ // On éteint
		eteindre(mot[i]);
		on = 1;
		i++;
	}

	incr++;
}

int button_appuye(int button){
  if (button != 0) return 0;
  return 1;
}

// Cette fonction permet au joueur de commencer le jeu, une fois que le bouton poussoir a été appuyé.
void start(){
	init_PB(); // Initialisation du bouton poussoir
	int button; 

	puts("Appuyez sur le bouton poussoir pour débuter le jeu\n");

	while (1){
		button = GPIOC.IDR & 0x1<<13;
		if (button_appuye(button)){ 
			tempo_500ms();
			return;
		}
	}
}


// On fait clignoter la LED grâce aux interruptions (1 seconde allumée, 1/2 éteinte, jusqu'à ce que la séquence à reproduire soit terminée)

// Si le joueur se trompe, c'est la fin : il faut relancer le programme pour rejouer (la LED s'allume 3 fois en rouge)
// Sinon, la LED s'allume 3 fois en vert pour indiquer que c'est gagné 

int main(){
	start();

	int index_mot = 0; 
	mot = liste[index_mot];
	length = strlen(mot); // Longueur de séquence

    init_LD1(); // Initialisation de la LED tricolore
	systick_init(1000); // Interruption initialisée 

	char *player = malloc(strlen(mot)*sizeof(char));

    while (1){
		puts("Tapez au clavier la séquence de lumières : \n");
		int i = 0;
		length = strlen(mot); // Longueur de séquence

		while (i < length){
			char c = _getc();
			player[i] = c;
			i++;
		}
		
		// Affichage du mot proposé par le joueur
		printf("Mot du joueur : %s\n", player);

		// Victoire ou défaite du joueur
		if (strcmp(mot, player) == 0){ // Si la séquence tapée est correcte, le joueur a gagné
			win(); 
			won = 1;
			mot = liste[index_mot++];
		}
		else{ // Défaite
			lose();
			return 0;
		}

    }
}