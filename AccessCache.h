#ifndef ACCESS_CACHE_H
#define ACCESS_CACHE_H
#include "RWStore.h"
#include <vector>

#define DEBUG 1

enum State {st_ready = 0, st_acknowledge, st_accepted, st_aborted};
enum Event {ev_control = 0, ev_abort, ev_commit, no_ev, ev_error};

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

        
        StateTransition Transit(StateTransition st_tr);
            
        //--State functions
        StateTransition Ready(StateTransition st_tr);
        StateTransition Acknowledge(StateTransition st_tr);
        StateTransition Accepted(StateTransition st_tr);
        StateTransition Aborted(StateTransition st_tr);

        void RunFSM();
    public:
        AccessCache();
        ~AccessCache();

        void SetupFSM();

        unsigned char control_reg;                          //what (LSB) transaction & (MSB) operation? (0bXXTT TTTT)
        unsigned short address_reg;                         //what address is the operation taking place?
        
        void Check();

};
#endif
