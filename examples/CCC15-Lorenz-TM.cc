#include "iRRAM.h"
#include <vector>
#include "TaylorModel.h"
/*******************************************************/

using namespace iRRAM;

void compute(){
  int count;
  cin >> count;
   
  std::vector<TM> x, x_new;
  x_new.push_back(TM(REAL(1)));
  x_new.push_back(TM(REAL(1)));
  x_new.push_back(TM(REAL(1)));
  
  REAL t = 0.01;
  //choice of a, b, c from https://de.wikipedia.org/wiki/Lorenz-Attraktor
  REAL a = 10;
  REAL b = 28;
  REAL c = REAL(8)/3;
  
  for( int i=0; i<=count; i++){
    x = x_new;
//     cout << "xxxxxx "<< x[0]<<"\n";
//     cout << "yyyyyy "<< x[1]<<"\n";
   
    TM::polish(x);
    
    cout <<setRwidth(15);
    cout << i*t <<" " << REAL(x[0]) <<" " << REAL(x[1])<<" " << REAL(x[2]) <<"\n";
//     cout << "xxxxxx "<< x[0]<<"\n";
//     cout << "yyyyyy "<< x[1]<<"\n";
    
    x_new[0] = x[0] + (x[1]-x[0])*a*t;
    x_new[1] = x[1] + (x[0]*(b-x[2])-x[1])*t;
    x_new[2] = x[2] + (x[0]*x[1]-x[2]*c)*t;

  }
}
