#include <queue>
#include <vector>
#include <iostream>

#define STATUS_IDLE 0x00
#define STATUS_COMMIT 0x01
#define STATUS_UNFINISHED 0x02

class RWStore
{
    private:
        //shorts are used for byte addressing
        short rd_front, wr_front;             //variables that hold fron values of queue
        std::vector<short> read_sets;          //all the adresses that transaction has read
        std::vector<short> write_sets;         //all the addresses that transaction has written

        unsigned char status;                 //indicates the finality of the transactions changes


    public:
        RWStore();
        ~RWStore();

        void Clear_All();                     //flush the read and wrtie set queues

        //--Read Set queue handlers
        //returns size of queue
        int Enqueue_Read(short rd_addr);
        short Dequeue_Read();
        void Clear_Reads();

        //--Write Set queue handlers
        //returns size of queue
        int Enqueue_Write(short wr_addr);
        short Dequeue_Write();
        void Clear_Writes();

        void setIdle();
        void setBegin();
        void setCommit();
        bool isCommit();

        bool isRead(short address);
        bool isWrite(short address);
        bool isAccess(short address);         //check for either read or write
};
