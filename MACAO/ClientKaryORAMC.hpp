#ifndef CLIENTKARYCORAMC_H
#define CLIENTKARYCORAMC_H

#include "ClientORAM.hpp"

class ClientKaryORAMC : public ClientORAM
{
private:

    //specific variable for S3CORAM
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
    int access(TYPE_ID blockID);
    
    int init();
    //retrieval_vector
    //int getLogicalVector(TYPE_DATA* logicalVector, TYPE_ID blockID);
    int updatePosMap(TYPE_ID blockID);
    
    int evict();
    
    //eviction_matrix
    int getEvictMatrix();
    
    int isRetrievedBlockInStash(TYPE_ID blockID);
    

};

#endif // CLIENTCORAM_H
