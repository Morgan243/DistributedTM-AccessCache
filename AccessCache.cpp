#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
    done = false;
    //SetupFSM();
    state = st_ready;
    transit.state = st_ready;
    transit.event = no_ev;
    //transit.function = &AccessCache::Ready;
}

AccessCache::~AccessCache()
{

}

void AccessCache::SetupFSM()
{
    StateTransition tempST;

    //--Setup READY state
    tempST.state = st_ready;
    //tempST.function = &AccessCache::Ready;

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
    //tempST.function = &AccessCache::Acknowledge;

        tempST.event = ev_control;
        state_interconnect.push_back(tempST);

    //--Setup ACCEPTED state
    tempST.state = st_accepted;
    //tempST.function = &AccessCache::Accepted;
        
        tempST.event = no_ev;
        state_interconnect.push_back(tempST);

    //--Setup ABORTED state
    tempST.state = st_aborted;
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
