/*
 * config.h
 *
 *      Author:  thanghoang
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "vector"
#include <math.h>
#include "fstream"
#include <iostream>
#include "stdio.h"
#include "stdlib.h"
#include <cstring>
#include <bitset>
#include <algorithm>
#include <assert.h>
#include <boost/progress.hpp>
#include <map>
#include <chrono>

#include <string>
#include <sstream>
#include <bitset>

typedef long long TYPE_DATA;
template <typename T>
static inline std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}


static const unsigned long long P = 1073742353; //288230376152137729; //prime field - should have length equal to the defined TYPE_DATA




//=== SECRET SHARING PARAMETER================================================
#define NUM_SERVERS 3
#define SSS_PRIVACY_LEVEL 2

const TYPE_DATA GLOBAL_MAC_KEY = 12574462961634974;
const TYPE_DATA MAC_KEY[SSS_PRIVACY_LEVEL+1] = {60986730870412112, 16792083382561605, -65204351291338743};


//#define XOR_PIR
#define RSSS


#define CORAM_LAYOUT
//#define TRIPLET_EVICTION
#define K_ARY 2


#define XOR_PRIVACY_LEVEL 1

#define NTL_LIB //disable it if compiled for android
//=== PARAMETERS ============================================================
#define BLOCK_SIZE 4096
#define HEIGHT 4


#if defined(CORAM_LAYOUT)
    #define BUCKET_SIZE (K_ARY)
    #define EVICT_RATE 1
    const unsigned long long NUM_BLOCK = pow(K_ARY,HEIGHT);
    const unsigned long long NUM_NODES = (int) (pow(K_ARY,HEIGHT+1)-1)/(K_ARY-1);
#else
    #if defined (TRIPLET_EVICTION)
        #define K_ARY 2
        #define BUCKET_SIZE 74 
        #define EVICT_RATE 37
        const unsigned long long NUM_BLOCK = ((int) (pow(2,HEIGHT-1))*EVICT_RATE-1);
        const unsigned long long  NUM_NODES = (int) (pow(2,HEIGHT+1)-1);
    #else
        #define BUCKET_SIZE 600 
        #define EVICT_RATE (BUCKET_SIZE/2)
        const unsigned long long NUM_BLOCK = ((pow(K_ARY,HEIGHT-1))*EVICT_RATE-1);
        const unsigned long long NUM_SLIDES = K_ARY; //should be divisible with BUCKET_SIZE
        const unsigned long long NUM_NODES = (int) (pow(K_ARY,HEIGHT+1)-1)/(K_ARY-1) + pow(K_ARY,HEIGHT); 
    #endif
#endif

#define STASH_SIZE 80

const int H = HEIGHT; 



//=== MODES ==================================================================
//#define PRECOMP_MODE //for shares of *0* and *1* precomputation
//============================================================================


//=== SECRET SHARING PARAMETER================================================
#define NUM_SERVERS 3
#define PRIVACY_LEVEL 1
const long long int vandermonde[NUM_SERVERS] = {3 , -3 + P, 1};
//const long long int vandermonde[NUM_SERVERS] = {7, -21+P, 35, -35+P, 21, -7+P, 1};

/** Vandermonde Values for Different Number of Servers (7, 5, 3)*/
//{7, -21+P, 35, -35+P, 21, -7+P, 1};//{5, -10+P, 10, -5+P, 1};//{3, -3+P, 1};
//============================================================================


//=== SERVER INFO ============================================================

	//SERVER IP ADDRESSES
const std::string SERVER_ADDR[NUM_SERVERS] = {"tcp://localhost", "tcp://localhost", "tcp://localhost"};//, "tcp://localhost", "tcp://localhost", "tcp://localhost", "tcp://localhost"}; 	
#define SERVER_PORT 25555        //define the first port to generate incremental ports for client-server /server-server communications

//============================================================================


//=== PATHS ==================================================================
const std::string rootPath = "../data/";

const std::string clientLocalDir = rootPath + "client_local/";
const std::string clientDataDir = rootPath + "client/";

const std::string clientTempPath = rootPath + "client_local/local_data";

const std::string logDir = "../" + to_string(H) + "_" + to_string(BLOCK_SIZE) + "/" + "log/";
//=============================================================================


//=== NON-MODIFIABLE PARAMETER ================================================
#if defined(NTL_LIB)
    #include "NTL/ZZ.h"
    #include "NTL/tools.h"
    #include "NTL/GF2E.h"
    #include <NTL/WordVector.h>
    #include <NTL/vector.h>
    #include "NTL/lzz_p.h"
    #include "NTL/ZZ_p.h"
    using namespace NTL;
#endif

using namespace std;

#define time_now std::chrono::high_resolution_clock::now()
//=============================================================================


//=== DATA TYPE ===============================================
typedef unsigned long long TYPE_ID;
typedef long long int TYPE_INDEX;
typedef struct type_pos_map
{
	TYPE_INDEX pathID;
	TYPE_INDEX pathIdx;
}TYPE_POS_MAP;


#define DATA_CHUNKS BLOCK_SIZE/sizeof(TYPE_DATA)
const TYPE_INDEX PRECOMP_SIZE = BUCKET_SIZE*(2*HEIGHT+1)*BUCKET_SIZE*(2*HEIGHT+1);

const TYPE_INDEX N_leaf = pow(K_ARY,H);



//====================================================================



//=== SOCKET COMMAND =========================================
#define CMD_SEND_ORAM_TREE        		0x000010
#define CMD_WRITE_ROOT		            	0x00000E 

#define CMD_EVICT			        0x000040
#define CMD_RETRIEVE		        0x000050
#define CMD_START_REDUC			        0x000052
#define CMD_SUCCESS                     "OK"
//============================================================





//MACROS
#define BIT_READ(character, position, the_bit)	((*the_bit = *character & (1 << position)))	
#define BIT_SET(character, position) ((*character |= 1 << position))	
#define BIT_CLEAR(character, position) ((*character &= ~(1 << position)))
#define BIT_TOGGLE(character, position)	((*character ^= 1 << position))
#define BIT_CHECK(var,pos) !!((*var) & (1<<(pos)))




//IF USING RSSS
const int NUM_MULT = 3;
const int NUM_SHARE_PER_SERVER = 2;





#if defined(CORAM_LAYOUT)
    const TYPE_INDEX evictMatSize = (BUCKET_SIZE+1)*(BUCKET_SIZE+1);
#else
    #if defined(TRIPLET_EVICTION)
        const TYPE_INDEX evictMatSize = 2*BUCKET_SIZE*BUCKET_SIZE;
    #else
        const TYPE_INDEX evictMatSize = BUCKET_SIZE*BUCKET_SIZE;
    #endif
#endif

#if defined (XOR_PIR)
    const unsigned long long CLIENT_RETRIEVAL_QUERY_SIZE = ceil((H+1)*BUCKET_SIZE/8.0); 
    const unsigned long long CLIENT_RETRIEVAL_OUT_LENGTH = sizeof(TYPE_ID) + CLIENT_RETRIEVAL_QUERY_SIZE*(NUM_SHARE_PER_SERVER);
    const unsigned long long SERVER_RETRIEVAL_REPLY_LENGTH =  2*BLOCK_SIZE*(NUM_SHARE_PER_SERVER);
#else
    const unsigned long long CLIENT_RETRIEVAL_QUERY_SIZE = (H+1)*BUCKET_SIZE*sizeof(TYPE_DATA); 
    const unsigned long long CLIENT_RETRIEVAL_OUT_LENGTH = sizeof(TYPE_ID) + CLIENT_RETRIEVAL_QUERY_SIZE;
    const unsigned long long SERVER_RETRIEVAL_REPLY_LENGTH =  2*BLOCK_SIZE;
#endif




const unsigned long long PATH_LENGTH = (H+1)*BUCKET_SIZE;
const unsigned long long BUCKET_DATA_SIZE = BUCKET_SIZE*BLOCK_SIZE;



#if defined (CORAM_LAYOUT)

    const unsigned long long CLIENT_EVICTION_OUT_LENGTH =  2*(BLOCK_SIZE*2+ (H+1)*evictMatSize*sizeof(TYPE_DATA)) +sizeof(TYPE_INDEX) ;//  for OnionORAM -> (H+1)*evictMatSize*sizeof(TYPE_DATA) + sizeof(TYPE_INDEX);
    const unsigned long long SERVER_RESHARE_IN_OUT_LENGTH =  2*2*(2*(BUCKET_SIZE+1)*BLOCK_SIZE); // 1st2: MAC, 2nd2: for 2 shares for RSSS, / server -> for onion ORAM:  BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS
    
    
    const int NUM_CONCURR_EVICT = 2; // THIS IS FOR EVICTION COMPUTATION IN TWO PARALLEL PATH
    
    const int MAT_PRODUCT_INPUT_DB_LENGTH = BUCKET_SIZE+1;
    const int MAT_PRODUCT_OUTPUT_LENGTH = BUCKET_SIZE+1;
    
    const int EVICT_MAT_NUM_COL = BUCKET_SIZE+1;
    const int EVICT_MAT_NUM_ROW = BUCKET_SIZE+1;
#else
    const unsigned long long CLIENT_EVICTION_OUT_LENGTH =  ((H+1)*evictMatSize*sizeof(TYPE_DATA)) +sizeof(TYPE_INDEX) ;//  for OnionORAM -> (H+1)*evictMatSize*sizeof(TYPE_DATA) + sizeof(TYPE_INDEX);
    const unsigned long long SERVER_RESHARE_IN_OUT_LENGTH =  2*2*((BUCKET_SIZE*BLOCK_SIZE)); // 1st2: MAC, 2nd2: for 2 shares for RSSS / server -> for onion ORAM:  BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS
    
    const int NUM_CONCURR_EVICT = 1;
    
    const int MAT_PRODUCT_INPUT_DB_LENGTH = 2*BUCKET_SIZE;
    const int MAT_PRODUCT_OUTPUT_LENGTH = BUCKET_SIZE;
    
    const int EVICT_MAT_NUM_COL = 2*BUCKET_SIZE;
    const int EVICT_MAT_NUM_ROW = BUCKET_SIZE;
    
#endif




const int NUM_XOR_QUERY_PER_SERVER = 2;
                 
 //rule for RSSS multiplication: 00 01 10
const int  RSSS_MULT_ORDER[NUM_MULT][2]= { {0,0}, {0,1}, {1,0}};
                               
#endif /* CONFIG_H_ */
