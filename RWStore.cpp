#include "RWStore.h"

using namespace std;

RWStore::RWStore()
{
    rd_front = wr_front = 0;
}

RWStore::~RWStore()
{
}

void RWStore::Clear_All()
{
    Clear_Reads();
    Clear_Writes();
}

int RWStore::Enqueue_Read(unsigned short rd_addr)
{
//{{{
    try
    {
        this->read_sets.push_back(rd_addr);
        return this->read_sets.size();
    }
    catch (exception &e)
    {
        cout<<"Exception in read enqueue: "<< e.what()<<endl;
    }
//}}}
}

short RWStore::Dequeue_Read()
{
//{{{
    if(this->read_sets.size() != 0)
    {
        rd_front = this->read_sets.front();
        this->read_sets.erase(this->read_sets.begin());

        return rd_front;
    }
    else 
        return -1;
//}}}
}

void RWStore::Clear_Reads()
{
    this->read_sets.clear();
}

int RWStore::Enqueue_Write(unsigned short wr_addr)
{
//{{{
    try
    {
        this->write_sets.push_back(wr_addr);
        return write_sets.size();
    }
    catch (exception &e)
    {
        cout<<"Exception in write enqueue: "<<e.what()<<endl;
    }
//}}}
}

short RWStore::Dequeue_Write()
{
//{{{
   if(this->write_sets.size() != 0)
   {
       wr_front = this->write_sets.front();
       this->write_sets.erase(this->write_sets.begin());

       return wr_front;
   }
   else
       return -1;
//}}}
}

void RWStore::Clear_Writes()
{
    this->write_sets.clear();
}

void RWStore::setIdle()
{
    status = STATUS_IDLE;
}

void RWStore::setBegin()
{
    status = STATUS_UNFINISHED;
}

void RWStore::setCommit()
{
    status = STATUS_COMMIT;
}

void RWStore::setAbort()
{
    status = STATUS_ABORT;
}


bool RWStore::isCommit()
{
//{{{
    if(status == STATUS_COMMIT)
        return true;
    else
        return false;
//}}}
}

bool RWStore::isAbort()
{
//{{{
    if(status == STATUS_ABORT)
        return true;
    else
        return false;
//}}}
}

bool RWStore::isRead(unsigned short address)
{
//{{{
    for(int i = 0; i < read_sets.size(); i++)
    {
       if(read_sets[i] == address)
       {
            return true;
       } 
    }

    return false;
//}}}
}

bool RWStore::isWrite(unsigned short address)
{
//{{{
    for(int i = 0; i < write_sets.size(); i++)
    {
       if(write_sets[i] == address)
       {
            return true;
       } 
    }

    return false;
//}}}
}

bool RWStore::isAccess(unsigned short address)
{
//{{{
    if(isRead(address) || isWrite(address))
        return true;
    else
        return false;
//}}}
}

