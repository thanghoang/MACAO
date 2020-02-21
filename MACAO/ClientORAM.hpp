#ifndef CLIENTORAM_HPP
#define CLIENTORAM_HPP



#include "config.h"
#include <pthread.h>
#include "zmq.hpp"
#include "struct_socket.h"


class ClientORAM
{

protected:

    
    TYPE_INDEX** metaData;
    TYPE_POS_MAP* pos_map;

#if defined(CORAM_LAYOUT)    
    TYPE_DATA** STASH;
    TYPE_INDEX* metaStash;
    
#endif
    //variables for retrieval operation
	TYPE_INDEX numRead;
	TYPE_DATA** sharedVector;
    
    #if defined(SEEDING)
        prng_state* prng_client;
    #endif
    
    unsigned char** retrieval_query_out;	
    
    
    unsigned char** retrieval_in;
    unsigned char** lin_rand_com_in;
    

    //variables for eviction
    TYPE_INDEX numEvict;
    TYPE_DATA*** sharedMatrix;
    TYPE_DATA*** sharedMatrix_MAC;

	TYPE_DATA** evictMatrix;
    unsigned char** evict_out;

    //thread
	pthread_t thread_sockets[NUM_SERVERS];

public:
    ClientORAM();
    ~ClientORAM();
    
    static zmq::context_t **context;
    static zmq::socket_t **socket;
    
    struct_socket* thread_socket_args;
    
    //main functions
    
    virtual int access(TYPE_INDEX blockID) = 0;
    
    virtual int init();
    
    virtual int loadState();
    virtual int saveState();
    int sendORAMTree();
    
    //retrieval subroutine
    int retrieve(TYPE_INDEX blockID);
    //eviction_matrix
    virtual int getEvictMatrix() =0;
    
    virtual int updatePosMap(TYPE_INDEX blockID);
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
    
    
    int createRetrievalQuery(int pIdx, TYPE_INDEX pID);
    void recoverRetrievedBlock();
    
    
    zz_p** retrievedShare;
    zz_p** retrievedMacShare;
       
    zz_p* recoveredBlock;
    zz_p* recoveredMacBlock;    

    int checkRandLinCom();
};

#endif // CLIENTORAM_HPP
