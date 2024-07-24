// jona test timer

#include "Cheetah/TimerAlarm.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

using namespace hmta;

// ----------------------------------------------------------------------------


class noto {

public:

    
noto ( std::string s ) : _s ( s ) {  }

bool operator () () {

      std::cout << "noto:  " << _s << std::endl;

      return(true);

}

private:
     std::string  _s;

};


// ----------------------------------------------------------------------------

int main(int, char *[])  {

    const struct ::timespec rqt = {50, 0};
           
        noto  noto1("noto1");
        TimerAlarm<noto>  timer(noto1, 5);
        std::cout << "starting...  \n";
        timer.arm ();
         
        std::cout << "sleeping...  \n";
        nanosleep( &rqt , 0);     
    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
