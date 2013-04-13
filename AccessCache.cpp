#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
//{{{
    done = false;
    number_nodes = 0;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    operation_mode = DEFAULT_MODE;
    enable_benchmarking = false;
//}}}
}

AccessCache::AccessCache(int num_stores)
{
//{{{
    done = false;
    number_nodes = 0;
    enable_benchmarking = false;
    
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    AddStores(num_stores);

    operation_mode = DEFAULT_MODE;
//}}}
}

AccessCache::AccessCache(int num_stores, Mode mode)
{
//{{{
    done = false;
    number_nodes = 0;
    enable_benchmarking = false;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    AddStores(num_stores);

    operation_mode = mode;
//}}}
}

AccessCache::AccessCache(int num_stores, Mode mode, bool benchmark)
{
//{{{
    done = false;
    number_nodes = 0;
    enable_benchmarking = benchmark;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    AddStores(num_stores);

    operation_mode = mode;
//}}}
}

AccessCache::~AccessCache()
{

}

void AccessCache::setRegs(int transaction_id, unsigned char operation, unsigned short address)
{
    control_reg = (operation << 5) | ((unsigned char)transaction_id);
    address_reg = address;
}

void AccessCache::AddStores(int num_stores)
{
//{{{
    number_nodes += num_stores;

    for(int i = 0; i < num_stores; i++)
    {
        //add a new node
        this->nodes.push_back(Node_Desc());
        
        //each node needs a representation of all other nodes to store parallel access desc 
        this->nodes.back().pending_accesses.resize(number_nodes);
        this->nodes.back().confirmed_accesses.resize(number_nodes);
    }
//}}}
}

int AccessCache::AddStores()
{
//{{{
    this->nodes.push_back(Node_Desc());
    return this->nodes.size() - 1;
//}}}
}

void AccessCache::SetupFSM()
{
    transit.state = st_ready;
    transit.event = no_ev;
}

bool AccessCache::RunFSM()
{
//{{{
    //init the transit state struct
    SetupFSM();

    do
    {
        //run current state, which will return the next state
        transit = Transit(transit);
    }
    while(transit.state != st_ready);

    if(transit.event == ev_accept)
        return true;
    else
        return false;
//}}}
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
    #if DEBUG
        cout<<"\t>>Entered READY state..."<<endl;
    #endif
        
    //check valid inputs first?
    st_tr.state = st_acknowledge;

    return st_tr;
//}}}
}

StateTransition AccessCache::Acknowledge(StateTransition st_tr)
{
//{{{
    #if DEBUG
        cout<<"\t>>Entered ACKNOWLEDGE state..."<<endl;
    #endif

    switch(operation_mode)
    {
    //{{{
        case(mutex_md):
        {
            //check RWStores for conflicts
            if(isMutexConflict(address_reg))
            {
                st_tr.state = st_aborted;
                st_tr.event = ev_abort;
            }
            else
            {
                st_tr.state = st_accepted;
                st_tr.event = no_ev;
            }
            break;
        }
        case(rwMutex_md):
        {
            //check RWStores for conflicts
            if(isMutexRWConflict(address_reg))
            {
                st_tr.state = st_aborted;
                st_tr.event = ev_abort;
            }
            else
            {
                st_tr.state = st_accepted;
                st_tr.event = no_ev;
            }
            break;
        }

        case(opt_md):
        {
            //check RWStores for conflicts
            if(isOptimisticConflict(address_reg))
            {
                st_tr.state = st_aborted;
                st_tr.event = ev_abort;
            }
            else
            {
                st_tr.state = st_accepted;
                st_tr.event = no_ev;
            }            
            break;
        }
    //}}}
    }

    return st_tr;
//}}}
}

StateTransition AccessCache::Accepted(StateTransition st_tr)
{
//{{{
    #if DEBUG
        cout<<"\t>>Entered ACCEPTED state..."<<endl;
    #endif

    st_tr.state = st_ready;
    st_tr.event = ev_accept;

    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);
    
    //if just an operation, add it to the store
    if(operation != COMMIT_T)
    {
        if(operation == READ_T)
            nodes[(int)transaction_id].rw_store.Enqueue_Read(address_reg);
        else
            nodes[(int)transaction_id].rw_store.Enqueue_Write(address_reg);

        setCtrlOperation(ACCEPT_T);
    }
    else
    {
        //if commit, clear store
        nodes[(int)transaction_id].rw_store.Clear_All();

        setCtrlOperation(COMMIT_T);
    }

    return st_tr;
//}}}
}

StateTransition AccessCache::Aborted(StateTransition st_tr)
{
//{{{
    #if DEBUG
        cout<<"\t>>Entered ABORTED state..."<<endl;
    #endif

    st_tr.state = st_ready;
    st_tr.event = ev_abort;

    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);
    
    setCtrlOperation(ABORT_T);

    //clear set store
    nodes[(int)transaction_id].rw_store.Clear_All();

    //transaction status set to idle
    nodes[(int)transaction_id].rw_store.setIdle();

    return st_tr;
//}}}
}

void AccessCache::setCtrlOperation(unsigned char op)
{
    control_reg = (control_reg & 0x1F) | (op<<5);
}

void AccessCache::extractFromControl(unsigned char &transaction_id, unsigned char &operation)
{
    transaction_id = (short)(control_reg & 0x1F);
    operation = (char)((control_reg & 0xE0)>>5);
}

bool AccessCache::isMutexConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    if(operation == READ_T || operation == WRITE_T)
    {
        //if any onther transaction is reading or writing ABORT
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if(nodes[i].rw_store.isAccess(address))
                    return true; //conflict found
            }
        }
    }
    else if(operation == COMMIT_T)
    {
        //if any onther transaction is reading or writing ABORT
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if(nodes[i].rw_store.isAccess(address))
                    if(nodes[i].rw_store.isCommit()) //this shouldn't be necessary for Mutex, bue w/e
                        return true; //conflict found
            }
        }
    }
    return false;
//}}}
}

bool AccessCache::isMutexRWConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    //if not a commit, just an access
    if(operation == READ_T || operation == WRITE_T)
    {
        //check all the stores
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont check own transaction store
            if(i != (int)transaction_id)
            {
                //conflict if someone is writing
                if(nodes[i].rw_store.isWrite(address))
                    return true; //conflict found
            }
        }
    }
    else if(operation == COMMIT_T)
    {
        //if any onther transaction is reading ABORT
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                //if its made it this far, are there really any checks to be made
                //Only would need to check if there was priority transactions
                //if(rw_stores[i].isWrite(address))
                //    if(rw_stores[i].isCommit()) //this shouldn't be necessary for Mutex, bue w/e
                //        return true; //conflict found
            }
        }
    }
//}}}
}

bool AccessCache::isOptimisticConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    //did a write commit before it?
    if(nodes[(int)transaction_id].rw_store.isAbort())
        return true;

    if(operation == READ_T)
    {
        return false; //no conflict
    }
    else if(operation == WRITE_T)
    {
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                //write-write conflicts can never turn out okay, abort them
                if(nodes[i].rw_store.isWrite(address))
                    return true; //conflict found
            }
        }
    }
    //attempting a commit (isAbort already checked above!)
    else if(operation == COMMIT_T)
    {
       short temp_addr;
       //dequeue all the writes from transaction being commited
       while((temp_addr = nodes[(int)transaction_id].rw_store.Dequeue_Write()) >= 0)
       {
           //check that oher transactions are not useing the address
            for(int i = 0; i < nodes.size(); i++)
            {
                //dont waste time checking own transaction
                if(i != (int)transaction_id)
                {
                    //if the address is accessed bu the transaction at all (read is the only real scenario)
                    if(nodes[i].rw_store.isAccess(temp_addr))
                    {
                        //abort the other transaction
                        nodes[i].rw_store.setAbort();
                    }
                }
            }
       } 
       return false;

    }
//}}}
}

bool AccessCache::isOptimisticConflict_benchmark(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    //did a write commit before it?
    if(nodes[(int)transaction_id].rw_store.isAbort())
        return true;

    if(operation == READ_T)
    {
        if(enable_benchmarking)
        {
            //setup the access identifier
            temp_accesss.address = address;
            temp_accesss.node_one = (int)transaction_id;
            temp_accesss.node_one_op = operation;

            //find transactions that are also using this address
            //go through all the nodes
            for(int i = 0; i < nodes.size(); i++)
            {
                //dont check itself
                if( i != (int)transaction_id)
                {
                    //get the access that a transaction has on the address, if any
                    if( (temp_accesss.node_two_op = nodes[i].rw_store.getAccess(address)) != 0xFF)
                    {
                        //set node identifier
                        temp_accesss.node_two = i;
                        
                        //add the access descriptor to pending
                        nodes[(int)transaction_id].pending_accesses[i].push_back(temp_accesss);
                    }
                }
            }
        }

        return false; //no conflict
    }
    else if(operation == WRITE_T)
    {
        for(int i = 0; i < nodes.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if((temp_accesss.node_two_op = nodes[i].rw_store.getAccess(address)) != 0xFF)
                {
                    //write-write conflicts can never turn out okay, abort them
                    if(temp_accesss.node_two_op == WRITE_T)
                    {
                        //aborted, so pending parallel access are not going to happen
                        nodes[(int)transaction_id].pending_accesses.clear();
                        return true; //conflict found
                    }
                    else
                    {
                        //set node identifier
                        temp_accesss.node_two = i;
                        //add the access descriptor
                        nodes[(int)transaction_id].pending_accesses[i].push_back(temp_accesss);
                    }
                }
            }
        }
    }
    //attempting a commit (isAbort already checked above!)
    else if(operation == COMMIT_T)
    {
       short temp_addr;

       //need to abort any transactions that are accessing this transactions write addresses
       //dequeue all the writes from transaction being commited
       while((temp_addr = nodes[(int)transaction_id].rw_store.Dequeue_Write()) >= 0)
       {
           //check that oher transactions are not useing the address
            for(int i = 0; i < nodes.size(); i++)
            {
                //dont waste time checking own transaction
                if(i != (int)transaction_id)
                {
                    //if the address is accessed bu the transaction at all (read is the only real scenario)
                    if((temp_accesss.node_two_op = nodes[i].rw_store.getAccess(address)) != 0xFF)                   
                    {
                            //delete pending accesses from this transaction (i) in the commiting transaction (transaction_id)
                            nodes[(int)transaction_id].pending_accesses[i].clear();

                            //abort the other transaction
                            nodes[i].rw_store.setAbort();
                    }
                }
            }
       } 
       
       //append all of pending access to confirmed accesses
       for(int i = 0; i < nodes[(int)(transaction_id)].pending_accesses.size(); i++)
       {
            nodes[(int)(transaction_id)].confirmed_accesses[i].insert(
                    nodes[(int)(transaction_id)].confirmed_accesses[i].end(),
                    nodes[(int)(transaction_id)].pending_accesses[i].begin(),
                    nodes[(int)(transaction_id)].pending_accesses[i].end());
       }

       return false;
    }
//}}}
}

void AccessCache::printParallelAccesses(int node_id)
{
//{{{
    ParallelAccess_Desc temp_desc;

    cout<<"---Parallel Accesses with node " <<node_id<<"---"<<endl;

    for(int i = 0; i < nodes[node_id].confirmed_accesses.size(); i++)
    {
        for(int j = 0; j < nodes[node_id].confirmed_accesses[i].size(); j++)
        {
    
            temp_desc = nodes[node_id].confirmed_accesses[i][j];

            cout<<"ADDRESS = ["<<temp_desc.address<<"]"<<endl;
            cout<<"This node performed "<< getFriendlyOperationName(temp_desc.node_one_op)<<endl;
            cout<<"Opposing node "<< temp_desc.node_two << " performed "<< getFriendlyOperationName(temp_desc.node_two_op);
        }
    }
//}}}
}

string AccessCache::getFriendlyOperationName(unsigned char op)
{
//{{{
    switch(op)
    {
        case(READ_T):
            {
                return "read";
                break;
            }

        case(WRITE_T):
            {
                return "write";
                break;
            }

        case(ACCEPT_T):
            {
                return "accept";
                break;
            }
        case(COMMIT_T):
            {
                return "commit";
                break;
            }
        case(ABORT_T):
            {
                return "abort";
                break;
            }
        default:
            return "unkown";
    }
//}}}
}
