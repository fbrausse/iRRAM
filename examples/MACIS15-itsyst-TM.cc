#include "iRRAM.h"
#include "TaylorModel.h"
using namespace iRRAM;

void compute(){

  REAL c;
  cin >> c;
  int n;
  cin >> n;

  TM   x = REAL(0.125 );

  
  for( int i=0; i<=n; i++){
    
    cout << x<< "\n";
    TM::polish(x);
    
    cout << x<< "\n";
    cout << REAL(x) << "\n";

    x=x*c*(REAL(1)-x);

  }
}

