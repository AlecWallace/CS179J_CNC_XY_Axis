//https://www.dfrobot.com/wiki/index.php/TB6600_Stepper_Motor_Driver_SKU:_DRI0043
//When “EN” is Low, the motor is in a free states (Off-line mode). In this mode, you can adjust the motor shaft position manually; when “EN” is High (Vacant), the motor will be in an automatic control mode.
//"Direction" is the motor direction signal pin,
//"PULSE" is the motor pulse signal pin. Once the driver get a pulse, the motor move a step.

#include <avr/io.h>

const int sizeX = 5; 
const int sizeY = 5;

const int stepXMax = 10;
const int stepYMax = 10;
//Port D needs to be set to output for this
int stepCounterY = 0;
int stepCounterX = 0;
//Starting at 0 will cause issues for restarts

class point{
  public:
  int x,y;
  point(){
      x = 0;
      y = 0;
  }
  point(int tx, int ty){
      x = tx;
      y = ty;
  }
  void getPossibleNext(int matrix[sizeY][sizeX], point history[100], int &size){
    //edge cases
    if(y!=0){
      if(matrix[y-1][x] <= 5 && matrix[y-1][x] >= 0){
        history[size].x = x;
        history[size].y = y-1;
        size++;
      }
    }
    if(y<sizeY-1){
      if(matrix[y+1][x] <= 5 && matrix[y+1][x] >= 0){
        history[size].x = x;
        history[size].y = y+1;
        size++;
      }

    }
    if(x!=0){
      if(matrix[y][x-1] <= 5 && matrix[y][x-1] >= 0){
        history[size].x = x-1;
        history[size].y = y;
        size++;
      }
      
    }
    if(x<sizeX-1){
      if(matrix[y][x+1] <= 5 && matrix[y][x+1] >= 0){
        history[size].x = x+1;
        history[size].y = y;
        size++;
      }
    }
  }
  bool operator==(const point &p){
    if(this->x == p.x && this->y == p.y){
      return true;
    }
    return false;
  }
};

void initMotor(volatile uint8_t *port){
  //set EN high
  //set direction pin forward
  *port = *port | 0x03;
}

//
void turnOffMotor(volatile uint8_t *port){
  //set EN low
  *port = *port & 0xFE;
}

//1600 is a full rotation

//moves one step
//doesn't change the direction pin
bool step(volatile uint8_t *port){
  for(int i=0; i<400; i++){
		*port = *port | 0x04;
		for(int i=0; i<500; i++){//50
			asm("nop");
		}
		*port = *port & 0xFB;
		for(int i=0; i<7000; i++){
			asm("nop");
		}
  }
  *port = *port | 0x04;
  return true;
}

//handle errors here
//move int steps forward
//fails if overflow
//X is true Y is false
bool moveForward(volatile uint8_t *port, volatile uint8_t *pin, int steps, bool XY){
  //set direction pin pos
  *port = *port | 0x02;
  //set pulse pin for #steps
  for(int i=0; i<steps; i++){
    step(port);
    if(XY){
      stepCounterX++;
    }
    else{
      stepCounterY++;
    }
    //error checking
    if(((~*pin>>6) & 0x01)==1){
      if(XY && stepCounterX != stepXMax){
        //jumped ahead in steps
        return false;
      }
      if(!XY && stepCounterY != stepYMax){
        //jumped ahead in steps
        return false;
      }
    }
    /*if(stepCounterX == stepXMax && !(((~*pin>>6) & 0x01)==1)){
      //missed steps
      return false;
    }
    if(stepCounterY == stepYMax && !(((~*pin>>6) & 0x01)==1)){
      //missed steps
      return false;
    }*/
  }
  return true;
}

//move int steps back
//fails on underflow
bool moveBack(volatile uint8_t *port, volatile uint8_t *pin, int steps, bool XY){
  //set direction pin neg
  *port = *port & 0xFD;
  //set pulse pin for #steps
    for(int i=0; i<steps; i++){
    if(XY && stepCounterX == 0){
      return false;
    }
    if(!XY && stepCounterY == 0){
      return false;
    }
    step(port);
    if(XY){
      stepCounterX--;
    }
    else{
      stepCounterY--;
    }
    //error checking
    if(((~*pin>>5) & 0x01)==1){
      if(XY && stepCounterX != 0){
        //jumped ahead in steps
        return false;
      }
      if(!XY && stepCounterY != 0){
        //jumped ahead in steps
        return false;
      }
    }
    /*if(stepCounterX == 0 && !(((~*pin>>5) & 0x01)==1)){
      //missed steps
      return false;
    }
    if(stepCounterY == 0 && !(((~*pin>>5) & 0x01)==1)){
      //missed steps
      return false;
    }*/
  }
  return true;
}

//resets to 0
//returns false if failed
bool restageMotor(volatile uint8_t *port, volatile uint8_t *pin, bool XY){
  //bring stepCounter back to 0 with moveBack
  if(XY){
    return moveBack(port,pin,stepCounterX,XY);
  }
  return moveBack(port,pin,stepCounterY,XY);
}

bool restageMotorError(volatile uint8_t *port1, volatile uint8_t *pin1 ,volatile uint8_t *port2, volatile uint8_t *pin2){
  //restage after an error don't count steps back
  //TODO
  *port1 = *port1 & 0xFD;
  *port2 = *port2 & 0xFD;
  bool flagOneDone = false;
  bool flagTwoDone = false;
  for(int i=0; i<stepYMax+stepXMax; i++){
    if(((~*pin1>>5) & 0x01)==1 || flagOneDone){
      flagOneDone = true;
    }
    else{
      //step back
      step(port1);
    }
    if(((~*pin2>>5) & 0x01)==1 || flagTwoDone){
      flagTwoDone = true;
    }
    else{
      //step back
      step(port2);
    }
    if(flagOneDone && flagTwoDone){
      return true;
    }
  }
  if(flagOneDone && flagTwoDone){
    return true;
  }
  return false;
}


// test the edges of the space
bool testMotor(volatile uint8_t *port1, volatile uint8_t *pin1 ,volatile uint8_t *port2, volatile uint8_t *pin2){
  initMotor(port1);
  initMotor(port2);
  if(moveForward(port1,pin1,stepXMax,true)){
    if(moveBack(port1,pin1,stepXMax,true)){
      if(moveForward(port2,pin2,stepYMax,false)){
        return moveBack(port2,pin2,stepYMax,false);
      }
    }
  }
  return false;
}

void depthFirstSearch(int matrix[sizeY][sizeX], point path[sizeX*sizeY], int &pathSize){
  //stack<point> history;
  point history[100]; //takes up a lot of memory might consider switching to breadth first search
  //vector<point> path;
  
  int step = -1;

  history[0].x = 0;
  history[0].y = 0;
  
  path[0].x = 0;
  path[0].y = 0;
  pathSize++;
  
  int size = 1;
  
  while(size > 0){
    point p = history[size-1];
    size--;
    if(matrix[p.y][p.x] >=0){
      matrix[p.y][p.x] = step;
      path[pathSize-1].x = p.x;
      path[pathSize-1].y = p.y;
      pathSize++;
      //addPoint(path,p,pathSize); //TODO
      step--;

      p.getPossibleNext(matrix,history,size);
    }
  }
}

//the meat of what we are doing
bool traverse(volatile uint8_t *port1, volatile uint8_t *pin1 ,volatile uint8_t *port2, volatile uint8_t *pin2, point path[sizeX*sizeY], int size){
  initMotor(port1);
  initMotor(port2);
  moveForward(port2,pin2,1,false);
  moveForward(port1,pin1,1,true);
  
  restageMotor(port1,pin1,true);
  restageMotor(port2,pin2,false);
  
  for(int i=0; i<size-1;i++){
    int tempX = path[i+1].x-path[i].x;
    int tempY = path[i+1].y-path[i].y;
    if(tempX == 1 && tempY == 0){
      if(!moveForward(port1,pin1,1,true)){
        return false;
      }
    }
    if(tempX == -1 && tempY == 0){
      if(!moveBack(port1,pin1,1,true)){
        return false;
      }
    }
    if(tempY == 1 && tempX == 0){
      if(!moveForward(port2,pin2,1,false)){
        return false;
      }
    }
    if(tempY == -1 && tempX == 0){
      if(!moveBack(port2,pin2,1,false)){
        return false;
      }
    }
    else{
      //backtracking
      //If the next step in the queue is more than 1 step away
      //Needs more testing
      for(int j=i; j>1; j--){
        int backtrackTempX = path[j-1].x-path[j].x;
        int backtrackTempY = path[j-1].y-path[j].y;
        if(backtrackTempX == 1 && backtrackTempY == 0){
          if(!moveForward(port1,pin1,1,true)){
            return false;
          }
        }
        else if(backtrackTempX == -1 && backtrackTempY == 0){
          if(!moveBack(port1,pin1,1,true)){
            return false;
          }
        }
        else if(backtrackTempY == 1 && backtrackTempX == 0){
          if(!moveForward(port2,pin2,1,false)){
            return false;
          }
        }
        else if(backtrackTempY == -1 && backtrackTempX == 0){
          if(!moveBack(port2,pin2,1,false)){
            return false;
          }
        }
        //we reached the backpoint
        if((path[j-1].x-path[i+1].x == 1) && (path[j-1].y-path[i+1].y == 0)){
          if(!moveForward(port1,pin1,1,true)){
            return false;
          }
          break;
        }
        if((path[j-1].x-path[i+1].x == -1) && (path[j-1].y-path[i+1].y == 0)){
          if(!moveBack(port1,pin1,1,true)){
            return false;
          }
          break;
        }
        if((path[j-1].y-path[i+1].y == 1) && (path[j-1].x-path[i+1].x == 0)){
          if(!moveForward(port2,pin2,1,false)){
            return false;
          }
          break;
        }
        if((path[j-1].y-path[i+1].y == -1) && (path[j-1].x-path[i+1].x == 0)){
          if(!moveBack(port2,pin2,1,false)){
            return false;
          }
          break;
        }
      }
    }
  }
  //if error restage motor error()
  return true;
}

//interupt handeler Stop
//stop the motor