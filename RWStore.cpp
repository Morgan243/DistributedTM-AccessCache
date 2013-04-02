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

int RWStore::Enqueue_Read(short rd_addr)
{
    this->read_sets.push_back(rd_addr);
    this->read_sets.size();
}

short RWStore::Dequeue_Read()
{
    rd_front = this->read_sets.front();
    this->read_sets.erase(this->read_sets.begin());

    return rd_front;
}

void RWStore::Clear_Reads()
{
    this->read_sets.clear();
}

int RWStore::Enqueue_Write(short wr_addr)
{
    this->write_sets.push_back(wr_addr);
    return write_sets.size();
}



short RWStore::Dequeue_Write()
{
   wr_front = this->write_sets.front();
   this->write_sets.erase(this->write_sets.begin());


   return wr_front;
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

bool RWStore::isCommit()
{
//{{{
    if(status == STATUS_COMMIT)
        return true;
    else
        return false;
//}}}
}

bool RWStore::isRead(short address)
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

bool RWStore::isWrite(short address)
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

bool RWStore::isAccess(short address)
{
//{{{
    if(isRead(address) || isWrite(address))
        return true;
    else
        return false;
//}}}
}



