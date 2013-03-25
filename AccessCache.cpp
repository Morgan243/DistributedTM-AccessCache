#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
    done = false;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;
    //transit.function = &AccessCache::Ready;
}

AccessCache::~AccessCache()
{

}

void AccessCache::SetupFSM()
{
    transit.state = st_ready;
    transit.event = no_ev;
}

void AccessCache::RunFSM()
{
    while(!done)
    {
        transit = Transit(transit);
    }
}

StateTransition AccessCache::Transit(StateTransition st_tr)
{
//{{{
    switch(st_tr.state)
    {
        case(st_ready):
            {
                return Ready(st_tr);
                break;
            }
        case(st_acknowledge):
            {
                return Acknowledge(st_tr);
                break;
            }
        case(st_accepted):
            {
                return Accepted(st_tr);
                break;
            }
        case(st_aborted):
            {
                return Aborted(st_tr);
                break;
            }
    }
//}}}
}

StateTransition AccessCache::Ready(StateTransition st_tr)
{
    #ifdef DEBUG
        cout<<"Entered READY state..."<<endl;
    #endif
}

StateTransition AccessCache::Acknowledge(StateTransition st_tr)
{
    #ifdef DEBUG
        cout<<"Entered ACKNOWLEDGE state..."<<endl;
    #endif

}

StateTransition AccessCache::Accepted(StateTransition st_tr)
{
    #ifdef DEBUG
        cout<<"Entered ACCEPTED state..."<<endl;
    #endif

}

StateTransition AccessCache::Aborted(StateTransition st_tr)
{
    #ifdef DEBUG
        cout<<"Entered ABORTED state..."<<endl;
    #endif

}
