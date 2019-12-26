#ifndef CLIENTORAM_HPP
#define CLIENTORAM_HPP



#include "config.h"
#include <pthread.h>
#include "zmq.hpp"
#include "struct_socket.h"


class ClientORAM
{

protected:

    //client storage for ORAM operations
    TYPE_ID** metaData; //this is for position_map scanning optimization
    TYPE_POS_MAP* pos_map;

#if defined(CORAM_LAYOUT)    
    TYPE_DATA** STASH;
    TYPE_INDEX* metaStash;
    
#endif
    //variables for retrieval operation
	TYPE_INDEX numRead;
	TYPE_DATA** sharedVector;
    
 
    
    unsigned char** retrieval_query_out;	

    
    
    unsigned char** retrieval_in;
    

    
    
    //variables for eviction
    TYPE_INDEX numEvict;
    TYPE_DATA*** sharedMatrix;
//#if defined (SPDZ)
//    TYPE_DATA*** sharedMatrixMAC;
//#endif
	TYPE_DATA** evictMatrix; 
    unsigned char** evict_out;

    //thread
	pthread_t thread_sockets[NUM_SERVERS];

//#if defined(PRECOMP_MODE)
//	TYPE_DATA** precompOnes;
//	TYPE_DATA** precompZeros;
//#endif


public:
    ClientORAM();
    ~ClientORAM();
    
    
    
    static zmq::context_t **context;
    static zmq::socket_t **socket;
    
    struct_socket* thread_socket_args;
    
    //main functions
    
    virtual int access(TYPE_ID blockID) = 0;
    
    virtual int init();
    
    virtual int loadState();
    virtual int saveState();
    int sendORAMTree();
    
    //retrieval subroutine
    int retrieve(TYPE_ID blockID);
    //eviction_matrix
    virtual int getEvictMatrix() =0;
    
    virtual int updatePosMap(TYPE_ID blockID);
    virtual int evict()=0;
    
    //socket
	static void* thread_socket_func(void* args);	
    static int sendNrecv(int peer_idx, unsigned char* data_out, size_t data_out_size, unsigned char* data_in, size_t data_in_size, int CMD);

    //logging
	static unsigned long int exp_logs[9]; 	
    static unsigned long int thread_max;
	static char timestamp[16];    
    
    
    
    
    
    
    
    //if using XOR-PIR + RSSS
    unsigned char*** xor_queries;
    TYPE_DATA** xor_answers;
    
    
    int createRetrievalQuery(int pIdx, TYPE_ID pID);
    void recoverRetrievedBlock();
    
    
    
    zz_p** retrievedShare;
    zz_p** retrievedMacShare;
       
    zz_p* recoveredBlock;
    zz_p* recoveredMacBlock;
    
    
    
    
};

#endif // CLIENTORAM_HPP
