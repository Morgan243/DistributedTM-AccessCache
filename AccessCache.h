#ifndef ACCESS_CACHE_H
#define ACCESS_CACHE_H
#include "RWStore.h"
#include <vector>

#define DEBUG 1

#define READ_T 0x00
#define WRITE_T 0x01
#define ACCEPT_T 0x02
#define COMMIT_T 0x03
#define ABORT_T 0x04

enum State {st_ready = 0, st_acknowledge, st_accepted, st_aborted};
enum Event {no_ev = 0, ev_control, ev_abort, ev_accept, ev_commit, ev_error};
enum Mode {mutex_md, rwMutex_md, opt_md};

#define DEFAULT_MODE mutex_md

struct StateTransition
{
    State state;
    Event event;
};

class AccessCache
{
    private:
        bool done;                                              //transition through state machine until this is true
        Mode operation_mode;
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

        void setCtrlOperation(unsigned char op);

        void extractFromControl(unsigned char &transaction_id, unsigned char &operation);

    public:
        AccessCache();
        AccessCache(int num_stores);
        AccessCache(int num_stores, Mode mode);
        ~AccessCache();

        unsigned char control_reg;                          //what (LSB) transaction & (MSB) operation? (0bXXXT TTTT)
        unsigned short address_reg;                         //what address is the operation taking place?

        void setRegs(int transaction_id, unsigned char operation, unsigned short address);

        void SetupFSM();
        void AddStores(int num_stores);
        int AddStores();
        
        bool RunFSM();
        
        void Check();

};
#endif
