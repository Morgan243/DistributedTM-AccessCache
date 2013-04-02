#ifndef ACCESS_CACHE_H
#define ACCESS_CACHE_H
#include "RWStore.h"
#include <vector>

#define DEBUG 1

enum State {st_ready = 0, st_acknowledge, st_accepted, st_aborted};
enum Event {no_ev = 0, ev_control, ev_abort, ev_commit, ev_error};

struct StateTransition
{
    State state;
    Event event;
};

class AccessCache
{
    private:
        bool done;                                              //transition through state machine until this is true
        std::vector<RWStore> rw_stores;                         //each transactions read and write set FIFO and status reg
        StateTransition transit;

        
        StateTransition Transit(StateTransition st_tr);
            
        //--State functions
        StateTransition Ready(StateTransition st_tr);
        StateTransition Acknowledge(StateTransition st_tr);
        StateTransition Accepted(StateTransition st_tr);
        StateTransition Aborted(StateTransition st_tr);

        bool (AccessCache::*isConflict)(short address);

        bool isMutexConflict(short address);
        bool isMutexRWConflict(short address);
        bool isOptimisticConflict(short address);

        void extractFromControl(unsigned char &transaction_id, unsigned char &operation);

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
