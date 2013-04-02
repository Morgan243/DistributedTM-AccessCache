#include "AccessCache.h"

using namespace std;

AccessCache::AccessCache()
{
    done = false;
    SetupFSM();
    transit.state = st_ready;
    transit.event = no_ev;
    //this->isConflict = &AccessCache::isMutexConflict;
    //transit.function = &AccessCache::Ready;
}

AccessCache::~AccessCache()
{

}

void AccessCache::SetupFSM()
{
    transit.state = st_ready;
    transit.event = no_ev;
}

void AccessCache::RunFSM()
{
    while(!done)
    {
        //run current state, which will return the next state
        transit = Transit(transit);
    }
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
    #ifdef DEBUG
        cout<<"Entered READY state..."<<endl;
    #endif
        
        //check valid inputs first?
        st_tr.state = st_acknowledge;

        return st_tr;
//}}}
}

StateTransition AccessCache::Acknowledge(StateTransition st_tr)
{
//{{{
    #ifdef DEBUG
        cout<<"Entered ACKNOWLEDGE state..."<<endl;
    #endif

    //check RWStores for conflicts
    if(isMutexConflict(address_reg))
//}}}
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

void AccessCache::extractFromControl(unsigned char &transaction_id, unsigned char &operation)
{
    transaction_id = (short)(control_reg & 0x3F);
    operation = (char)((control_reg & 0xC0)>>6);
}

bool AccessCache::isMutexConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    if(operation == READ || operation == WRITE)
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
    else if(operation == COMMIT)
    {

    }
//}}}
}

bool AccessCache::isMutexRWConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    if(operation == READ || operation == WRITE)
    {
        for(int i = 0; i < rw_stores.size(); i++)
        {
            //dont waste time checking own transaction
            if(i != (int)transaction_id)
            {
                if(rw_stores[i].isWrite(address))
                    return true; //conflict found
            }
        }
    }
    else if(operation == COMMIT)
    {

    }
//}}}
}

bool AccessCache::isOptimisticConflict(short address)
{
//{{{
    unsigned char transaction_id, operation;

    extractFromControl(transaction_id, operation);

    if(operation == READ)
    {
        //do anything?
        return false; //no conflict
    }
    else if(operation == WRITE)
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
    else if(operation == COMMIT)
    {

    }
//}}}
}
