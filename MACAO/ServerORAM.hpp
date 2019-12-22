#ifndef SERVERORAM_HPP
#define SERVERORAM_HPP

#include "config.h"
#include <zmq.hpp>
#include <pthread.h>
#include "struct_socket.h"
#include "struct_thread_computation.h"

class ServerORAM
{
protected:
    
    
    //local variable
    std::string CLIENT_ADDR;
    TYPE_INDEX serverNo;
    TYPE_INDEX others[NUM_SERVERS-1];
    string myStoragePath;

    
    
    //variables for retrieval
    unsigned char* retrieval_query_in;
    unsigned char** retrieval_query;
    zz_p*** retrieval_path_db;
    zz_p*** retrieval_path_mac;
    
#if defined(XOR_PIR)
    unsigned char*** retrieval_answer_block;
    unsigned char*** retrieval_answer_mac;
#else
    unsigned char** retrieval_answer_block;
    unsigned char** retrieval_answer_mac;
#endif
    
    unsigned char* retrieval_answer_out;
    
    
    
    
    //variables for eviction
    unsigned char* evict_in;
    //this is only for RSSS with 3 servers
    unsigned char* client_evict_in;
    
    vector<zz_p***> vecReShares;
    vector<zz_p***> vecReShares_MAC;
    vector<zz_p**> vecLocalMatProduct_output;
    vector<zz_p**> vecLocalMatProduct_output_MAC;
    
    
    vector<zz_p***> vecEvictMatrix;
    
    vector<zz_p**> vecEvictPath_db;
    vector<zz_p**> vecEvictPath_MAC;

    
    unsigned char** reshares_in;
    unsigned char** reshares_out; //shares_buffer_out;
    
    
    TYPE_INDEX n_evict;
        
    
    struct_socket recvSocket_args[NUM_SERVERS-1];
    struct_socket sendSocket_args[NUM_SERVERS-1];

 
    //thread
    int numThreads;

    pthread_t thread_recv[NUM_SERVERS-1];
    pthread_t thread_send[NUM_SERVERS-1];
    
    vector<THREAD_COMPUTATION*> vecComp_args;
    vector<THREAD_COMPUTATION*> vecComp_MAC_args;
    
    vector<pthread_t*> vecThread_compute;
    vector<pthread_t*> vecThread_compute_MAC;
    
    
    unsigned char* bucket_data;  // only for recvORAMTree
    
    
    unsigned char* write_root_in;    // only for recvORAMTree
    unsigned char* client_write_root_in;    // only for recvORAMTree
    
    
    

    
public:
    ServerORAM();
    ~ServerORAM();
    ServerORAM(TYPE_INDEX serverNo, int selectedThreads); 
    
    
    
    //socket functions
    static zmq::context_t **context_send;
    static zmq::socket_t **socket_send;

    static zmq::context_t **context_recv;
    static zmq::socket_t **socket_recv;
    
    static int send(int peer_idx, unsigned char* input, size_t inputSize);
    static int recv(int peer_idx, unsigned char* output, size_t outputSize);

    
    //main functions
    int start();
    
    int recvORAMTree(zmq::socket_t& socket);
    
    virtual int retrieve(zmq::socket_t& socket);
    
    int writeRoot(zmq::socket_t& socket); // used in Onion_ORAM
    
    virtual int evict(zmq::socket_t& socket) = 0;
    
    // sub-eviction subroutines
    int recvClientEvictData(zmq::socket_t& socket);
    
    int preReSharing(int level, int es, int ee);
    int reShare(int level, int es, int ee);
    int postReSharing(int level, int es, int ee);
    
    
    // IO functions
    int readBucket(TYPE_ID bucketID, int shareID, zz_p** output_data, zz_p** output_mac);
    int updateRoot(int shareID, unsigned long long replaceWriteIdx, unsigned char* input, unsigned char* mac); 
    int copyBucket(int shareID, TYPE_ID srcBucketID, TYPE_ID destBucketID); 
    int writeBucket(TYPE_ID bucketID, int shareID, unsigned char* input); //to be updated with MAC
    int writeBucket_reverse_mode(int bucketID, int shareID, zz_p ** data, zz_p** mac);
    
    
    int readBucket_reverse(TYPE_ID bucketID, int bucketIdx, int shareID, zz_p** output_data, zz_p** output_mac);
    
    //thread functions
   
    static void* thread_socket_func(void* args);
    
    static void* thread_matProd_func(void* args);
    static void* thread_retrieval_by_XOR_func(void* args);
    static void* thread_retrieval_by_dotProd_func(void* args);
    //logs
    static unsigned long int server_logs[13]; 
    static unsigned long int thread_max;
    static char timestamp[16];



};

#endif // SERVERORAM_HPP