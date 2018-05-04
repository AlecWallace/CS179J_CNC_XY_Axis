/*
 * CS179JCheckPoint1.cpp
 *
 * Created: 4/30/2018 7:15:59 PM
 * Author : Alec Wallace
 */ 

#include <avr/io.h>
#include "motorDriver.h"

//const int sizeX = 5;
//const int sizeY = 5;

int main(void){
	DDRA = 0x0F; PORTA = 0xF0; // in/out
	DDRB = 0x0F; PORTB = 0xF0; // in/Out
	DDRC = 0x0F; PORTC = 0xF0; // in/out //change for the LCD
	DDRD = 0x0F; PORTD = 0xF0; // in/out
	
	PORTD = PORTD & 0xF0;
    /* Replace with your application code */
    while (1){
		
		if(((~PIND>>5) & 0x01)==1){
			PORTD = PORTD & 0xF0;
			PORTD = PORTD | 0x03;
			if(testMotor(&PORTA,&PINA,&PORTB,&PINB)){
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x02;
			}
			else{
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x01;
				for(int i=0; i<1000; i++){
					asm("nop");
				}
				if(!restageMotorError(&PORTA,&PINA,&PORTB,&PINB)){
					PORTD = PORTD & 0xF0;
					PORTD = PORTD | 0x0F;
				}
			}
		}
		//if button press 2 show working
			//use algorithm
		if(((~PIND>>4) & 0x01)==1){
			PORTD = PORTD & 0xF0;
			PORTD = PORTD | 0x04;
			
			int matrix[sizeY][sizeX];
			for(int i=0; i<sizeY; i++){
				for(int j=0; j<sizeX; j++){
					matrix[i][j] = 1;
				}
			}
			matrix[1][3] = 7;
			matrix[1][4] = 7;
			matrix[2][1] = 7;
			matrix[2][3] = 7;
			matrix[3][1] = 7;
			matrix[3][3] = 7;
			
			point tempQ[sizeX*sizeY];
			int pathSize = 0;
			depthFirstSearch(matrix, tempQ, pathSize);
			//if(true){
			if(traverse(&PORTA,&PINA,&PORTB,&PINB, tempQ, pathSize)){
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x07;
			}
			else{
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x0F;
			}
		}
    }
}
