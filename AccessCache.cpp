#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
    done = false;
    SetupFSM();
    state = st_ready;
    transit.state = st_ready;
    transit.event = no_ev;
    transit.function = &AccessCache::Ready;
}

AccessCache::~AccessCache()
{

}

void AccessCache::SetupFSM()
{
    StateTransition tempST;

    //--Setup READY state
    tempST.state = st_ready;
    tempST.function = &AccessCache::Ready;

        //Entry point transition
        tempST.event = no_ev;
        state_interconnect.push_back(tempST);

        //Transit from accepted
        tempST.event = ev_commit;
        state_interconnect.push_back(tempST);

        //Transit from aborted
        tempST.event = ev_abort;
        state_interconnect.push_back(tempST);

    //--Setup ACKNOWLEDGE state
    tempST.state = st_acknowledge;
    tempST.function = &AccessCache::Acknowledge;

        tempST.event = ev_control;
        state_interconnect.push_back(tempST);

    //--Setup ACCEPTED state
    tempST.state = st_accepted;
    tempST.function = &AccessCache::Accepted;
        
        tempST.event = no_ev;
        state_interconnect.push_back(tempST);

    //--Setup ABORTED state
    tempST.state = st_aborted;
    //tempST.function


}

void AccessCache::RunFSM()
{
    while(!done)
    {
        this->*(transit.function( transit.state, transit.event));
    }
}

StateTransition AccessCache::Ready(State src, Event ev)
{

}

StateTransition AccessCache::Acknowledge(State src, Event ev)
{

}

StateTransition AccessCache::Accepted(State src, Event ev)
{

}

StateTransition AccessCache::Aborted(State src, Event ev)
{

}
