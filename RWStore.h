#include <queue>
#include <iostream>


class RWStore
{
    private:
        std::queue<short> read_sets;
        std::queue<short> write_sets;


    public:
        RWStore();
        ~RWStore();

        //Read Set queue handlers
        void Push_Read(short rd_addr);
        short Pop_Read();

        //Write Set queue handlers
        void Push_Write(short wr_addr);
        short Pop_Write();
};
