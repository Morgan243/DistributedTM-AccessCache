#ifndef ACCESS_CACHE_H
#define ACCESS_CACHE_H
#include "RWStore.h"
#include <vector>


enum State {st_ready = 0, st_acknowledge, st_accepted, st_aborted};
enum Event {ev_control = 0, ev_abort, ev_commit, no_ev, ev_error};

//prototype class here
class AccessCache;

StateTransition (AccessCache::*function) (State src, Event ev);

struct StateTransition
{
    State state;
    Event event;
};

class AccessCache
{
    private:
        bool done;
        std::vector<RWStore> rw_stores;
        std::vector<StateTransition> state_interconnect;
        State state;
        StateTransition transit;

        
    public:
        AccessCache();
        ~AccessCache();

        void SetupFSM();
        void RunFSM();
            
        //--State functions
        StateTransition Ready(State src, Event ev);
        StateTransition Acknowledge(State src, Event ev);
        StateTransition Accepted(State src, Event ev);
        StateTransition Aborted(State src, Event ev);
};
#endif
