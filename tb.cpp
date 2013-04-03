#include <iostream>
#include "AccessCache.h"

using namespace std;

int main(int argc, char *argv[])
{
    bool done = false;
    int num_transactions = 0;
    cout<<"Short size on this machine is: "<< sizeof(short) <<" bytes"<<endl;

    cout<<"How many transactions do you want?"<<endl;
    cin>>num_transactions;
    cout<<"Creating access cache with "<<num_transactions<<" transactions..."<<endl;
    AccessCache accessCache(num_transactions);
    cout<<"\t Completed!"<<endl;

    char select;
    short address = 0;
    while(!done)
    {
        for( int i = 0; i < num_transactions && !done; i++)
        {
            cout<<"For transaction "<<i<<", read, write, commit, do nothing, or exit? (r,w,c,n,e):"<<endl;
            cin>>select;

            if(select == 'r')
            {
            //{{{
                cout<<"Enter the address to be read (short int):"<<endl;
                cin>>address;
                accessCache.setRegs(i, READ, address);

                if(accessCache.RunFSM())
                {
                    cout<<"\t*Read added successfully!"<<endl;
                }
                else
                {
                    cout<<"\tREAD ABORTED!"<<endl;
                }
            //}}}
            }
            else if (select == 'w')
            {
            //{{{
                cout<<"Enter the address to be written (short int):"<<endl;
                cin>>address;
                accessCache.setRegs(i, WRITE, address);

                if(accessCache.RunFSM())
                {
                    cout<<"\t*Write added successfully!"<<endl;
                }
                else
                {
                    cout<<"\tWRITE ABORTED!"<<endl;
                }
            //}}}
            }
            else if (select == 'c')
            {
            //{{{
                cout<<"Commit underway..."<<endl;
                accessCache.setRegs(i, COMMIT, address);

                if(accessCache.RunFSM())
                {
                    cout<<"\tCommit was successful!"<<endl;
                }
                else
                {
                    cout<<"\tCOMMIT ABORTED!"<<endl;
                }
            //}}}
            }
            else if(select == 'e')
            {
            //{{{
                cout<<"exiting program..."<<endl;
                done = true;
            //}}}
            }
        }    
    }

    return 0;
}

