#include <iostream>
#include "config.h"
#include "ClientORAM.hpp"
#include "ServerORAM.hpp"



#if defined(CORAM_LAYOUT)
        #include "ServerKaryORAMC.hpp"
        #include "ClientKaryORAMC.hpp"
#else
    #if defined(TRIPLET_EVICTION)
        #include "ServerBinaryORAMO.hpp"
        #include "ClientBinaryORAMO.hpp"
    #else
        #include "ServerKaryORAMO.hpp"
        #include "ClientKaryORAMO.hpp"
    #endif
    
#endif

#include "Utils.hpp"

using namespace std;


#include <thread>

unsigned int nthreads = std::thread::hardware_concurrency();

#include "ORAM.hpp"


int main(int argc, char **argv)
{    
    #if defined(SEEDING)
        register_prng(&sober128_desc);
    #endif
    string mkdir_cmd = "mkdir -p ";
    string mkdir_localState = mkdir_cmd + clientLocalDir;
    string mkdir_unsharedData = mkdir_cmd + clientDataDir;
    string mkdir_log = mkdir_cmd + logDir;
    
    system(mkdir_localState.c_str());
    system(mkdir_unsharedData.c_str());
    system(mkdir_log.c_str());
    for(int i = 0 ; i < NUM_SERVERS; i++)
    {
        string mkdir_sharedData = mkdir_cmd +  rootPath + to_string(i);
        system(mkdir_sharedData.c_str());
    }

    int choice;
    zz_p::init(P);
    //set random seed for NTL
    ZZ seed = conv<ZZ>("123456789101112131415161718192021222324");
    #if !defined(SEEDING)
        SetSeed(seed);
    #endif
    
	cout << "CLIENT(1) or SERVER(2): ";
	cin >> choice;
	cout << endl;
	
	if(choice == 2)
	{
		int serverNo;
        int selectedThreads;
		cout << "Enter the Server No (1-"<< NUM_SERVERS <<"):";
		cin >> serverNo;
		cin.clear();
		cout << endl;
        
        do
        {
            cout<< "How many computation threads to use? (1-"<<nthreads<<"): ";
            cin>>selectedThreads;
		}while(selectedThreads>nthreads);
        
#if defined(CORAM_LAYOUT)
        ServerORAM*  server = new ServerKaryORAMC(serverNo-1,selectedThreads);
#else
    #if defined(TRIPLET_EVICTION)
        ServerORAM*  server = new ServerBinaryORAMO(serverNo-1,selectedThreads);
    #else
        ServerORAM* server = new ServerKaryORAMO(serverNo-1,selectedThreads);
    #endif
#endif
        
        
        
		server->start();
	}
	else if (choice == 1)
	{
        
#if defined(CORAM_LAYOUT)
        ClientKaryORAMC*  client = new ClientKaryORAMC();
#else
    #if defined(TRIPLET_EVICTION)
        ClientORAM* client = new ClientBinaryORAMO();
    #else
        ClientORAM* client = new ClientKaryORAMO();
    #endif
#endif
		
        
        int access, start;
		char response = ' ';
		int random_access;
        int subOpt;
        cout<<"LOAD PREBUILT DATA (1) OR CREATE NEW ORAM (2)? "<<endl;
        cin>>subOpt;
        cout<<endl;
        if(subOpt==1)
        {
            client->loadState();
        }
        else
        {
            client->init();
            do
            {
                cout << "TRANSMIT INITIALIZED ORAM DATA TO NON-LOCAL SERVERS? (y/n)";
                cin >> response;
                response = tolower(response);
            }
            while( !cin.fail() && response!='y' && response!='n' );
            if (response == 'y')
            {
                client->sendORAMTree();
            }
		    
        }
		cout << "SERVERS READY? (Press ENTER to Continue)";
		cin.ignore();
		cin.ignore();
		cin.clear();
		cout << endl<<endl<<endl;
        for(int j = 0 ; j < 10 ; j++)
        {
            for(int i = 1 ; i < NUM_BLOCK+1; i++)
            {
                client->access(i);
                //cin.get();
            }
        }
        cout<<"Done!";
    }
	else
	{
		cout << "COME ON!!" << endl;
	}
    
    return 0;
}
