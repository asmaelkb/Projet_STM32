#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

void init_LD1(){
    GPIOA.MODER = GPIOA.MODER & ~(0x111111 << 16); // Initialisation des ports A8 (rouge), A9 (vert) et A10 (bleu)
}

void allume_LD1_rouge(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 8);
}

void allume_LD1_vert(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 9);
}

void allume_LD1_bleu(){
    GPIOA.ODR = GPIOA.ODR | (0x1 << 10);
}

void tempo_500ms(){
	volatile uint32_t duree;
	/* estimation, suppose que le compilateur n'optimise pas trop... */
	for (duree = 0; duree < 5600000 ; duree++){
		;
	}

}

void eteindre_LD1_rouge(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 8);
}

void eteindre_LD1_vert(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 9);
}

void eteindre_LD1_bleu(){
    GPIOA.ODR = GPIOA.ODR & ~(0x1 << 10);
}




int main(){
    init_LD1();
    while (1){
        allume_LD1_rouge();
        tempo_500ms();
        eteindre_LD1_rouge();
        tempo_500ms();

        allume_LD1_vert();
        tempo_500ms();
        eteindre_LD1_vert();
        tempo_500ms();

        allume_LD1_bleu();
        tempo_500ms();
        eteindre_LD1_bleu();
        tempo_500ms();
    }
}