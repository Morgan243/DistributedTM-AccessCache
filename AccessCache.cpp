#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
//{{{
    done = false;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;

    operation_mode = DEFAULT_MODE;
//}}}
}

AccessCache::AccessCache(int num_stores)
{
//{{{
    done = false;
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
    for(int i = 0; i < num_stores; i++)
    {
        this->rw_stores.push_back(RWStore());
    }
//}}}
}

int AccessCache::AddStores()
{
//{{{
    this->rw_stores.push_back(RWStore());
    return this->rw_stores.size() - 1;
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
            rw_stores[(int)transaction_id].Enqueue_Read(address_reg);
        else
            rw_stores[(int)transaction_id].Enqueue_Write(address_reg);

        setCtrlOperation(ACCEPT_T);
    }
    else
    {
        //if commit, clear store
        rw_stores[(int)transaction_id].Clear_All();

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
    rw_stores[(int)transaction_id].Clear_All();

    //transaction status set to idle
    rw_stores[(int)transaction_id].setIdle();

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
        for(int i = 0; i < rw_stores.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if(rw_stores[i].isAccess(address))
                    return true; //conflict found
            }
        }
    }
    else if(operation == COMMIT_T)
    {
        //if any onther transaction is reading or writing ABORT
        for(int i = 0; i < rw_stores.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if(rw_stores[i].isAccess(address))
                    if(rw_stores[i].isCommit()) //this shouldn't be necessary for Mutex, bue w/e
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
        for(int i = 0; i < rw_stores.size(); i++)
        {
            //dont check own transaction store
            if(i != (int)transaction_id)
            {
                //conflict if someone is writing
                if(rw_stores[i].isWrite(address))
                    return true; //conflict found
            }
        }
    }
    else if(operation == COMMIT_T)
    {
        //if any onther transaction is reading ABORT
        for(int i = 0; i < rw_stores.size(); i++)
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
    if(rw_stores[(int)transaction_id].isAbort())
        return true;

    if(operation == READ_T)
    {
        //do anything?
        return false; //no conflict
    }
    else if(operation == WRITE_T)
    {
        for(int i = 0; i < rw_stores.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                //write-write conflicts can never turn out okay, abort them
                if(rw_stores[i].isWrite(address))
                    return true; //conflict found
            }
        }
    }
    //attempting a commit (isAbort already checked above!)
    else if(operation == COMMIT_T)
    {
       short temp_addr;
       //dequeue all the writes from transaction being commited
       while((temp_addr = rw_stores[(int)transaction_id].Dequeue_Write()) >= 0)
       {
           //check that oher transactions are not useing the address
            for(int i = 0; i < rw_stores.size(); i++)
            {
                //dont waste time checking own transaction
                if(i != (int)transaction_id)
                {
                    //if the address is accessed bu the transaction at all (read is the only real scenario)
                    if(rw_stores[i].isAccess(temp_addr))
                    {
                        //abort the other transaction
                        rw_stores[i].setAbort();
                    }
                }
            }
       } 
       return false;

    }
//}}}
}
