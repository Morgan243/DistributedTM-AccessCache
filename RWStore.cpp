#include "RWStore.h"

using namespace std;

RWStore::RWStore()
{
    rd_back = wr_back = 0;
}

RWStore::~RWStore()
{

}

int RWStore::Push_Read(short rd_addr)
{
    this->read_sets.push(rd_addr);
    this->read_sets.size();
}

short RWStore::Pop_Read()
{
    rd_back = this->read_sets.back();
    this->read_sets.pop();

    return rd_back;
}

int RWStore::Push_Write(short wr_addr)
{
    this->write_sets.push(wr_addr);
    return write_sets.size();
}



short RWStore::Pop_Write()
{
   wr_back = this->write_sets.back();
   this->write_sets.pop();

   return wr_back;
}
