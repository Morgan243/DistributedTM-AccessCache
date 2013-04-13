#ifndef ACCESS_CACHE_H
#define ACCESS_CACHE_H
#include "RWStore.h"
#include <vector>

#define DEBUG 0


enum State {st_ready = 0, st_acknowledge, st_accepted, st_aborted};
enum Event {no_ev = 0, ev_control, ev_abort, ev_accept, ev_commit, ev_error};
enum Mode {mutex_md, rwMutex_md, opt_md};

#define DEFAULT_MODE mutex_md

struct StateTransition
{
    State state;
    Event event;
};

//Keep track of critical section parallelism for benchmarking
struct ParallelAccess_Desc
{
    unsigned short address;             //what address did this happen on
    int node_one, node_two;             //id of nodes involved
    char node_one_op, node_two_op;      //what access type did each node do
    unsigned int transaction_mark;
};

struct Node_Desc
{
    RWStore rw_store;        //each transactions read and write set FIFO and status reg
   
    //each vector<ParallelAccess_Desc> represents a nodes parallel access with this node
    std::vector< std::vector<ParallelAccess_Desc> > confirmed_accesses; 
    //keep track of all the parallel access that are made throughout execution.   
    std::vector< std::vector <ParallelAccess_Desc> > pending_accesses; 
};

class AccessCache
{
    private:
        bool done, enable_benchmarking;                           //transition through state machine until this is true
        Mode operation_mode;
        unsigned int number_nodes;
        ParallelAccess_Desc temp_accesss;
        std::vector<Node_Desc> nodes;

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

        bool isMutexRWConflict_benchmark(short address);
        bool isOptimisticConflict_benchmark(short address);

        void setCtrlOperation(unsigned char op);

        void extractFromControl(unsigned char &transaction_id, unsigned char &operation);

    public:
        AccessCache();
        AccessCache(int num_stores);
        AccessCache(int num_stores, Mode mode);
        AccessCache(int num_stores, Mode mode, bool benchmark);
        ~AccessCache();

        unsigned char control_reg;                          //what (LSB) transaction & (MSB) operation? (0bXXXT TTTT)
        unsigned short address_reg;                         //what address is the operation taking place?

        void setRegs(int transaction_id, unsigned char operation, unsigned short address);

        void SetupFSM();
        void AddStores(int num_stores);
        int AddStores();
        
        bool RunFSM();
        
        void Check();
        void printParallelAccesses(int node_id);
        std::string getFriendlyOperationName(unsigned char op);

};
#endif
