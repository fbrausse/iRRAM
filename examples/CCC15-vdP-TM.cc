#include "iRRAM.h"
#include <vector>
#include "TaylorModel.h"
/*******************************************************/

using namespace iRRAM;

void compute(){
  int count;
  cin >> count;
   
  TM x=REAL(1);
  TM y=REAL(1);
  
  
  REAL t = 0.01;
  REAL alpha = 3;
  
  for( int i=0; i<=count; i++){
   
    
    cout << "x_new "<< x<<"\n";
    cout << "y_new "<< y<<"\n";
    TM::polish(x,y);
    cout << "x_polish "<< x<<"\n";
    cout << "y_polish "<< y<<"\n";
     
    cout <<setRwidth(15);
    cout << i*t <<" " << REAL(x) <<" " << REAL(y) <<"\n";
    
    TM x_new = x + y*t;
    TM y_new = y + (y*alpha - x - x*x*y*alpha)*t;
    x=x_new;
    y=y_new;

  }
}
