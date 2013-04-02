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
        //run current state, which will return the next state
        transit = Transit(transit);
    }
}

StateTransition AccessCache::Transit(StateTransition st_tr)
{
//{{{
    //call appropriate state method
    switch(st_tr.state)
    {
        //READ
        case(st_ready):
            {
                return Ready(st_tr);
                break;
            }
        //ACKNOWLEDGE
        case(st_acknowledge):
            {
                return Acknowledge(st_tr);
                break;
            }
        //ACCEPTED
        case(st_accepted):
            {
                return Accepted(st_tr);
                break;
            }
        //ABORTED
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
//{{{
    #ifdef DEBUG
        cout<<"Entered READY state..."<<endl;
    #endif
        
        //check valid inputs first?
        st_tr.state = st_acknowledge;

        return st_tr;
//}}}
}

StateTransition AccessCache::Acknowledge(StateTransition st_tr)
{
//{{{
    #ifdef DEBUG
        cout<<"Entered ACKNOWLEDGE state..."<<endl;
    #endif

    //check RWStores for conflicts

//}}}
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
