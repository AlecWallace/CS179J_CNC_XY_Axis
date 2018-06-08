/*
 * CS179JFinalProject.cpp
 *
 * 
 * Author : Alec Wallace
 */ 

#include <avr/io.h>
#include "motorDriver.h"

int main(void){
	DDRA = 0x0F; PORTA = 0xF0; // in/out
	DDRB = 0x0F; PORTB = 0xF0; // in/Out
	DDRC = 0x0F; PORTC = 0xF0; // in/out
	DDRD = 0x0F; PORTD = 0xF0; // in/out
	
	PORTD = PORTD & 0xF0; //all PORTD is for debugging/ user info
    while (1){
		
    //Restage button
		if(((~PIND>>5) & 0x01)==1){
			restageMotor(&PORTA,&PINA, true);
			restageMotor(&PORTB,&PINB, false);
			PORTD = PORTD & 0xF0;
			PORTD = PORTD | 0x03;
		}
		
	
    //Draw button
		if(((~PIND>>4) & 0x01)==1){
			PORTD = PORTD & 0xF0;
			PORTD = PORTD | 0x04;
			
      //create image
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
			
      //create tool path
			point tempQ[sizeX*sizeY];
			int pathSize = 0;
			depthFirstSearch(matrix, tempQ, pathSize);
			
      //Run motors
			if(traverse(&PORTA,&PINA,&PORTB,&PINB, tempQ, pathSize)){
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x07;
			}
			else{ //motors failed
				PORTD = PORTD & 0xF0;
				PORTD = PORTD | 0x0F;
			}
		}
		
    //integration
		if(((PIND>>6) & 0x01)==1){
			testMotor(&PORTA,&PINA,&PORTB,&PINB);
			PORTD = PORTD & 0xF0;
			PORTD = PORTD | 0x03;
		}
    }
}
