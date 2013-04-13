#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
//{{{
    done = false;
    number_nodes = 0;
    enable_benchmarking = false;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    operation_mode = DEFAULT_MODE;
//}}}
}

AccessCache::AccessCache(int num_stores)
{
    Init(num_stores, DEFAULT_MODE, false); 
}

AccessCache::AccessCache(int num_stores, Mode mode)
{
    Init(num_stores, mode, false);
}

AccessCache::AccessCache(int num_stores, Mode mode, bool benchmark)
{
    Init(num_stores, mode, benchmark);
}

AccessCache::~AccessCache()
{
}

void AccessCache::Init(int num_stores, Mode mode, bool benchmark)
{
 //{{{
    done = false;
    number_nodes = 0;
    enable_benchmarking = benchmark;
    operation_mode = mode;

    SetupFSM();

    transit.state = st_ready;
    transit.event = no_ev;

    AddStores(num_stores);

//}}}   
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
        
        if(this->enable_benchmarking)
        {
            //each node needs a representation of all other nodes to store parallel access desc 
            this->nodes.back().pending_accesses.resize(number_nodes);
            this->nodes.back().confirmed_accesses.resize(number_nodes);
        }
    }
//}}}
}

int AccessCache::AddStores()
{
//{{{
    number_nodes++;

    //add the new node/store
    this->nodes.push_back(Node_Desc());

    if(this->enable_benchmarking)
    {
        //add one vector to each existing node and add a vector for each on the new node
        for(int i = 0; i < nodes.size() - 1; i++)
        {
            this->nodes[i].pending_accesses.push_back(vector<ParallelAccess_Desc>());
            this->nodes[i].confirmed_accesses.push_back(vector<ParallelAccess_Desc>());

            this->nodes.back().pending_accesses.push_back(vector<ParallelAccess_Desc>());
            this->nodes.back().confirmed_accesses.push_back(vector<ParallelAccess_Desc>());
        }
            //add one more for itself (shouldnt be doing this, but meh not one is reading this)
            this->nodes.back().pending_accesses.push_back(vector<ParallelAccess_Desc>());
            this->nodes.back().confirmed_accesses.push_back(vector<ParallelAccess_Desc>());
    }

//    if(this->enable_benchmarking)
//    {
//        //give the new node all a vector of parallel accesses for each existing node
//        //this->nodes.back().pending_accesses.resize(number_nodes);
//        this->nodes.back().pending_accesses.push_back(vector<ParallelAccess_Desc>());
//        //this->nodes.back().confirmed_accesses.resize(number_nodes);
//        this->nodes.back().confirmed_accesses.push_back(vector<ParallelAccess_Desc>());
//    }

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
            if(enable_benchmarking)
            {
                //check RWStores for conflicts
                if(isOptimisticConflict_benchmark(address_reg))
                {
                    st_tr.state = st_aborted;
                    st_tr.event = ev_abort;
                }
                else
                {
                    st_tr.state = st_accepted;
                    st_tr.event = no_ev;
                }            
            }
            else
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
        //nodes[(int)transaction_id].rw_store.Clear_All();

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
    {
        cout<<"Node "<<(int)transaction_id<<" was aborted, clearing pending..."<<endl<<endl;
        //being aborted, clear the pending parallel
        clearPendingParallel((int)transaction_id);

        return true;
    }

    if(operation == READ_T)
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
                temp_accesss.node_two_op = nodes[i].rw_store.getAccess(address);

                //get the access that a transaction has on the address, if any
                if( (temp_accesss.node_two_op  == READ_T) || (temp_accesss.node_two_op  == WRITE_T))
                {

                    cout<<"Operation READ performed in parallel on ["<<address<<"] "<< 
                        (unsigned int)temp_accesss.node_two_op<<endl;
                    cout<<"\tThis node: "<< (int) transaction_id<<endl;
                    cout<<"\tOther node: "<<i<<endl<<endl;

                    //set node identifier
                    temp_accesss.node_two = i;
                    
                    //add the access descriptor to pending
                    nodes[(int)transaction_id].pending_accesses[i].push_back(temp_accesss);
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
                //get the other nodes access
                temp_accesss.node_two_op = nodes[i].rw_store.getAccess(address);

                //check for legitimacy
                if( temp_accesss.node_two_op == READ_T || temp_accesss.node_two_op == WRITE_T)
                {
                    //write-write conflicts can never turn out okay, abort them
                    if(temp_accesss.node_two_op == WRITE_T)
                    {
                        cout<<"Operation WRITE being aborted on ["<<address<<"]"<<endl;
                        cout<<"\tThis node: "<< (int) transaction_id<<endl;
                        cout<<"\tOther node: "<<i<<endl<<endl;

                        //aborted, so pending parallel accesses are not going to happen
                        clearPendingParallel((int)transaction_id);

                        return true; //conflict found
                    }
                    else
                    {
                        cout<<"Operation WRITE performed in parallel on ["<<address<<"]"<<endl;
                        cout<<"\tThis node: "<< (int) transaction_id<<endl;
                        cout<<"\tOther node: "<<i<<endl<<endl;

                        //set node identifier
                        temp_accesss.node_two = i;
                        //add the access descriptor
                        nodes[(int)transaction_id].pending_accesses[i].push_back(temp_accesss);

                        return false;
                    }
                }
            }
        }
    }
    //attempting a commit (isAbort already checked above!)
    else if(operation == COMMIT_T)
    {
       short temp_addr;
       vector<unsigned short> temp_writes = nodes[(int)transaction_id].rw_store.Get_Writes();

       //need to abort any transactions that are accessing this transactions write addresses
       //dequeue all the writes from transaction being commited
       //while((temp_addr = nodes[(int)transaction_id].rw_store.Dequeue_Write()) >= 0)
       for(int k = 0 ; k < temp_writes.size(); k++)
       {
           //check that oher transactions are not useing the address
            for(int i = 0; i < nodes.size(); i++)
            {
                //dont waste time checking own transaction
                if(i != (int)transaction_id)
                {

                    //get the opposing nodes access
                    temp_accesss.node_two_op = nodes[i].rw_store.getAccess(temp_writes[k]); 

                    //get the access that a transaction has on the address, if any
                    if( (temp_accesss.node_two_op  == READ_T) || (temp_accesss.node_two_op  == WRITE_T))
                    {
                            cout<<"Commit is aborting another transaction from node "<<i<<" on ["<< temp_writes[k]<<"]"<<endl<<endl;
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

        clearPendingParallel((int)transaction_id);

        nodes[(int)transaction_id].rw_store.setCommit();

        return false;
    }
//}}}
}

void AccessCache::clearNodeSets(int t_id)
{
    nodes[t_id].rw_store.Clear_All();
    nodes[t_id].rw_store.setIdle();
}

void AccessCache::clearPendingParallel(int t_id)
{
    for(int i = 0; i < nodes[t_id].pending_accesses.size(); i++)
    {
        nodes[t_id].pending_accesses[i].clear();
    }
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
            cout<<"\tThis node performed "<< getFriendlyOperationName(temp_desc.node_one_op)<<endl;
            cout<<"\tOpposing node "<< temp_desc.node_two << " performed "<< getFriendlyOperationName(temp_desc.node_two_op)<<endl<<endl;
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
