#ifndef CLIENTKARYCORAMC_H
#define CLIENTKARYCORAMC_H

#include "ClientORAM.hpp"

class ClientKaryORAMC : public ClientORAM
{
private:

    //specific variable
    TYPE_DATA** STASH;
    TYPE_INDEX* metaStash;
    
    
    int deepest[HEIGHT+2];
	int target[HEIGHT+2]; 
	int deepestIdx[HEIGHT+2];
    
    int countNumBlockInStash();

public:

    ClientKaryORAMC();
    ~ClientKaryORAMC();
   
    
    //main functions
    int loadState();
    int saveState();
    int access(TYPE_INDEX blockID);
    
    int init();
    int updatePosMap(TYPE_INDEX blockID);
    
    int evict();
    
    int getEvictMatrix();
    int isRetrievedBlockInStash(TYPE_INDEX blockID);
    

};

#endif // CLIENTCORAM_H
