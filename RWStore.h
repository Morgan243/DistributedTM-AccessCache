#include <queue>
#include <iostream>


class RWStore
{
    private:
        short rd_back, wr_back;
        std::queue<short> read_sets;
        std::queue<short> write_sets;


    public:
        RWStore();
        ~RWStore();

        //should this be here or in Cache?
        unsigned char status;

        void Clear_All();

        //--Read Set queue handlers
        //returns size of queue
        int Push_Read(short rd_addr);
        short Pop_Read();
        void Clear_Reads();

        //--Write Set queue handlers
        //returns size of queue
        int Push_Write(short wr_addr);
        short Pop_Write();
        void Clear_Writes();
};