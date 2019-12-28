/*
 * ORAM.hpp
 *
 *      Author: thanghoang
 */
 
#ifndef SSORAM_HPP
#define SSORAM_HPP

#include "config.h"

class ORAM
{
public:
    ORAM();
    ~ORAM();
    
    static int build(TYPE_POS_MAP* pos_map, TYPE_ID** metaData);

    static int getEvictIdx (TYPE_INDEX *srcIdx, TYPE_INDEX *destIdx, TYPE_INDEX *siblIdx, string evict_str);
    
	static string getEvictString(TYPE_ID n_evict);

	static int getFullPathIdx(TYPE_INDEX* fullPath, TYPE_INDEX pathID);

    
	//static int precomputeShares(TYPE_DATA input, TYPE_DATA** output, TYPE_INDEX output_size);
    
    
    //Circuit-ORAM layout
    static int getFullEvictPathIdx(TYPE_INDEX *fullPathIdx, string str_evict);
    static int getDeepestLevel(TYPE_INDEX PathID, TYPE_INDEX blockPathID);
    static void getDeepestBucketIdx(TYPE_INDEX* meta_stash, TYPE_INDEX* meta_path, TYPE_INDEX evictPathID, int* output);
    static int prepareDeepest(TYPE_INDEX* meta_stash, TYPE_INDEX* meta_path, TYPE_INDEX PathID, int* deepest);
    static int getEmptySlot(TYPE_INDEX* meta_path, int level);
    static int prepareTarget(TYPE_INDEX* meta_path, TYPE_INDEX pathID, int *deepest, int* target);
    
    
    
    

   // static int getSharedVector(TYPE_DATA* logicVector, TYPE_DATA** sharedVector, int vector_len);

    static int createShares(TYPE_DATA secret, TYPE_DATA* secret_shares, TYPE_DATA* mac_shares);
    static int createShares(TYPE_DATA secret, TYPE_DATA* secret_shares, TYPE_DATA* mac_shares, prng_state* pseudo_state, int secretShareIdx);
    
  
    
    //static int recoverSecret(CHUNK** shares, TYPE_DATA* secret);
    
    static int recoverSecret(zz_p** secret_shares, zz_p** mac_shares, zz_p* secret, zz_p* mac);
    static int recoverSecret(unsigned char** retrieval_in, zz_p* secret, zz_p* mac);
    
    static int xor_reconstruct(unsigned char** input, zz_p* output, zz_p* output_mac);
    
    static int xor_retrieve(unsigned char* query, zz_p** db, zz_p** db_mac, int start_db_idx, int end_db_idx, unsigned char* output, unsigned char* output_mac);
    
    static int xor_createQuery(TYPE_INDEX idx, unsigned int db_size, unsigned char** output);
    
    
    static int sss_createQuery(TYPE_INDEX idx, unsigned int DB_SIZE, unsigned char** output);
    static int sss_createQuery(TYPE_INDEX idx, unsigned int DB_SIZE, unsigned char** output, prng_state *prng);

};
    
#endif // SSORAM_HPP
