/*
 * ORAM.cpp
 *
 *      Author:  thanghoang
 */
 
#include "ORAM.hpp"
#include "Utils.hpp"


ORAM::ORAM()
{
}

ORAM::~ORAM()
{
}


/**
 * Function Name: build
 *
 * Description: Builds ORAM buckets with random data based on generated position map
 * and creates shares for distributed servers are created from ORAM buckets. 
 * Buckets are stored in the disk storage as seperate files.
 * 
 * @param pos_map: (input) Randomly generated position map to build ORAM buckets
 * @param metaData: (output) metaData of position map for scanning optimizations
 * @return 0 if successful
 */   
int ORAM::build(TYPE_POS_MAP* pos_map, TYPE_ID** metaData)
{
	int div = ceil(NUM_BLOCK/(double)N_leaf);
	assert(div <= BUCKET_SIZE && "ERROR: CHECK THE PARAMETERS => LEAVES CANNOT STORE ALL");
	long long n_col,m_row;
    #if !defined (REVERSE_STORAGE_LAYOUT)
        n_col = DATA_CHUNKS; 
        m_row = BUCKET_SIZE;
    #else
         n_col = BUCKET_SIZE; 
         m_row = DATA_CHUNKS;
    #endif
  	TYPE_DATA** bucket = new TYPE_DATA*[m_row];
    for (int i = 0 ; i < m_row; i++ )
    {
        bucket[i] = new TYPE_DATA[n_col];
        memset(bucket[i],0,sizeof(TYPE_DATA)*n_col);
    }
    
    TYPE_DATA** temp = new TYPE_DATA*[m_row];
    for (int i = 0 ; i < m_row; i++)
    {
        temp[i] = new TYPE_DATA[n_col];
        memset(temp[i],0,sizeof(TYPE_DATA)*n_col);
    }
    
    /*TYPE_DATA** bucket = new TYPE_DATA*[BUCKET_SIZE];
    for (int i = 0 ; i < BUCKET_SIZE; i++ )
    {
        bucket[i] = new TYPE_DATA[DATA_CHUNKS];
        memset(bucket[i],0,sizeof(TYPE_DATA)*DATA_CHUNKS);
    }
    
    TYPE_DATA** temp = new TYPE_DATA*[BUCKET_SIZE];
    for (int i = 0 ; i < BUCKET_SIZE; i++)
    {
        temp[i] = new TYPE_DATA[DATA_CHUNKS];
        memset(temp[i],0,sizeof(TYPE_DATA)*DATA_CHUNKS);
    }*/
    
    
    FILE* file_out = NULL;
    FILE* file_mac_out = NULL;
    string path, path_mac;
		
    cout << "=================================================================" << endl;
    cout<< "[ORAM] Creating Buckets on Disk" << endl;
    
    boost::progress_display show_progress2(NUM_NODES);
    
    //generate bucket ID pools
    vector<TYPE_ID> blockIDs;
    for(TYPE_ID i = 0; i <NUM_BLOCK;i++)
    {
        blockIDs.push_back(i+1);
    }
    //random permutation using built-in function
    //std::random_shuffle ( blockIDs.begin(), blockIDs.end() );
    
    
    //non-leaf buckets are all empty
    for(TYPE_INDEX i = 0 ; i < NUM_NODES - N_leaf; i ++)
    {
        file_out = NULL;
        path = clientDataDir + to_string(i);
        if((file_out = fopen(path.c_str(),"wb+")) == NULL)
        {
            cout<< "[ORAM] File Cannot be Opened!!" <<endl;
            exit(0);
        }
        for(int ii = 0 ; ii <m_row; ii++)
        {
            fwrite(bucket[ii], 1, n_col*sizeof(TYPE_DATA), file_out);
        }
        /*for(int ii = 0 ; ii <BUCKET_SIZE; ii++)
        {
            fwrite(bucket[ii], 1, BLOCK_SIZE, file_out);
        }*/
        fclose(file_out);
    }
    
    //generate random blocks in leaf-buckets
    TYPE_INDEX iter= 0;
    for(TYPE_INDEX i = NUM_NODES - N_leaf ; i < NUM_NODES; i++)
    {
        #if defined (REVERSE_STORAGE_LAYOUT)
            memset(bucket[0],0,sizeof(TYPE_DATA)*BUCKET_SIZE);
            memset(bucket[1],0,sizeof(TYPE_DATA)*BUCKET_SIZE);
        #else
            for(int j  = 0 ; j < m_row; j++)
                memset(bucket[j],0,BLOCK_SIZE);
        #endif
        
        for(int ii = BUCKET_SIZE/2 ; ii<BUCKET_SIZE; ii++)
        {
            if(iter>=NUM_BLOCK)
                break;
            #if defined (REVERSE_STORAGE_LAYOUT)
                bucket[0][ii] = blockIDs[iter];
                bucket[1][ii] = blockIDs[iter];
            #else
                bucket[ii][0] = blockIDs[iter];
                bucket[ii][1] = blockIDs[iter];
            #endif
            
            pos_map[blockIDs[iter]].pathID = i;
            pos_map[blockIDs[iter]].pathIdx = ii+(BUCKET_SIZE*H)  ;
            metaData[i][ii]= blockIDs[iter];
            iter++;
        }
        //write bucket to file
        file_out = NULL;
        path = clientDataDir + to_string(i);
        if((file_out = fopen(path.c_str(),"wb+")) == NULL)
        {
            cout<< "[ORAM] File Cannot be Opened!!" <<endl;
            exit(0);
        }
        for(int ii = 0 ; ii < m_row; ii++)
        {
            fwrite(bucket[ii], 1, n_col*sizeof(TYPE_DATA), file_out);
        }
        /*
        for(int ii = 0 ; ii < BUCKET_SIZE; ii++)
        {
            fwrite(bucket[ii], 1, BLOCK_SIZE, file_out);
        }*/
        fclose(file_out);
    }
    
    
    cout << "=================================================================" << endl;
    cout<< "[ORAM] Creating Shares on Disk" << endl;
    boost::progress_display show_progress(NUM_NODES);
    
    TYPE_DATA*** bucketShares = new TYPE_DATA**[NUM_SERVERS];
    TYPE_DATA*** bucketMacShares = new TYPE_DATA**[NUM_SERVERS];
    
    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)
    {
        bucketShares[k] = new TYPE_DATA*[m_row];
        bucketMacShares[k] = new TYPE_DATA*[m_row];
        for(int i = 0 ; i < m_row ; i++ )
        {
            bucketShares[k][i] = new TYPE_DATA[n_col];
            bucketMacShares[k][i] = new TYPE_DATA[n_col];
        }
        /*bucketShares[k] = new TYPE_DATA*[BUCKET_SIZE];
        bucketMacShares[k] = new TYPE_DATA*[BUCKET_SIZE];
        for(int i = 0 ; i < BUCKET_SIZE ; i++ )
        {
            bucketShares[k][i] = new TYPE_DATA[DATA_CHUNKS];
            bucketMacShares[k][i] = new TYPE_DATA[DATA_CHUNKS];
        }*/
    }
        
		
    TYPE_DATA data_shares[m_row][NUM_SERVERS];
    TYPE_DATA mac_shares[m_row][NUM_SERVERS];
    //TYPE_DATA data_shares[BUCKET_SIZE][NUM_SERVERS];
    //TYPE_DATA mac_shares[BUCKET_SIZE][NUM_SERVERS];
    
    
    FILE* file_in = NULL;
    file_out = NULL;
        
    auto start = time_now;
    auto end = time_now;
    
    for (TYPE_INDEX i = 0; i < NUM_NODES; i++)
    {
        path = clientDataDir + to_string(i);
        if((file_in = fopen(path.c_str(),"rb")) == NULL){
            cout<< "[ORAM] File Cannot be Opened!!" <<endl;
            exit(0);
        }
        for(int ii = 0 ; ii < m_row; ii++)
        //for(int ii = 0 ; ii < BUCKET_SIZE; ii++)
        {
            fread(bucket[ii] ,1 , n_col*sizeof(TYPE_DATA), file_in);
            //fread(bucket[ii] ,1 , BLOCK_SIZE, file_in);
            
            for(TYPE_INDEX j = 0; j < n_col; j++)
            //for(TYPE_INDEX j = 0; j < DATA_CHUNKS; j++)
            {           
                #if defined(SEEDING)
                    createShares(bucket[ii][j], data_shares[ii],mac_shares[ii],NULL,0);         
                #else // RSSS or SPDZ
                    createShares(bucket[ii][j], data_shares[ii],mac_shares[ii]); 
                #endif
        
                for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
                {
                    /*if(bucket[ii][0]==1)
                    {
                        cout<<i<<endl;
                        cout<<data_shares[ii][k]<<endl;
                    }*/
                    memcpy(&bucketShares[k][ii][j], &data_shares[ii][k], sizeof(TYPE_DATA));
                    memcpy(&bucketMacShares[k][ii][j], &mac_shares[ii][k], sizeof(TYPE_DATA));
                    
                }                
            }
        }
        fclose(file_in);
        
        start = time_now;
        for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++) 
        {
            path = rootPath + to_string(k) + "/" + to_string(i);
            path_mac = rootPath + to_string(k) + "/" + to_string(i) + "_mac";
            if((file_out = fopen(path.c_str(),"wb+")) == NULL)
            {
                cout<< path << " Cannot Be Opened!!" <<endl;
                exit;
            }
            if((file_mac_out = fopen(path_mac.c_str(),"wb+")) == NULL)
            {
                cout<< path_mac << " Cannot Be Opened!!" <<endl;
                exit;
            }
            for(int ii = 0 ; ii< m_row ; ii++)
            //for(int ii = 0 ; ii< BUCKET_SIZE ; ii++)
            {
                fwrite(bucketShares[k][ii], 1, n_col*sizeof(TYPE_DATA), file_out);
                fwrite(bucketMacShares[k][ii], 1, n_col*sizeof(TYPE_DATA), file_mac_out);
                //fwrite(bucketShares[k][ii], 1, BLOCK_SIZE, file_out);
                //fwrite(bucketMacShares[k][ii], 1, BLOCK_SIZE, file_mac_out);
            }
            fclose(file_out);
            fclose(file_mac_out);
        }    
        end = time_now;
        ++show_progress;
    }
		
    cout<< "[ORAM] Elapsed Time for Init on Disk: "<< std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<" ns"<<endl;
		
    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)
    {
        for(int i = 0 ; i < m_row; i++)
        //for(int i = 0 ; i < BUCKET_SIZE; i++)
        {
            delete[] bucketShares[k][i];
            delete[] bucketMacShares[k][i];
        }
        delete[] bucketShares[k];
        delete[] bucketMacShares[k];
    }
    delete[] bucketShares;
    delete[] bucketMacShares;
    for(int i = 0 ; i < m_row; i++)
    //for(int i = 0 ; i < BUCKET_SIZE; i++)
    {
        delete[] bucket[i];
        delete[] temp[i];
    }
    delete[] bucket;
    delete[] temp;

	cout<<endl;
	cout << "=================================================================" << endl;
	cout<< "[ORAM] Shared ORAM Tree is Initialized for " << NUM_SERVERS << " Servers" <<endl;
	cout << "=================================================================" << endl;
	cout<<endl;
	
	return 0;
}


/**
 * Function Name: getEvictIdx
 *
 * Description: Determine the indices of source bucket, destination bucket and sibling bucket 
 * residing on the eviction path
 * 
 * @param srcIdx: (output) source bucket index array
 * @param destIdx: (output) destination bucket index array
 * @param siblIdx: (output) sibling bucket index array
 * @param str_evict: (input) eviction edges calculated by binary ReverseOrder of the eviction number 
 * @return 0 if successful
 */     
int ORAM::getEvictIdx (TYPE_INDEX *srcIdx, TYPE_INDEX *destIdx, TYPE_INDEX *siblIdx, string str_evict)
{
    srcIdx[0] = 0;
    if (str_evict[0]-'0' == 0) 
    {
        destIdx[0] = 1;
        siblIdx[0]  = 2;
    }
    else
    {
        destIdx[0] = 2;
        siblIdx[0] = 1;
    }
    for(int i = 0 ; i < H ; i ++)
    {
        if (str_evict[i]-'0' == 1 )
        {
            if(i < H-1)
                srcIdx[i+1] = srcIdx[i]*2 +2;
            if(i > 0)
            {
            
                destIdx[i] = destIdx[i-1]*2 +2;
                siblIdx[i] = destIdx[i-1]*2+1;
            }
        }
        else
        {
            if(i<H-1)
                srcIdx[i+1] = srcIdx[i]*2 + 1;
            if(i > 0 )
            {
                destIdx[i] = destIdx[i-1]*2 + 1;
                siblIdx[i] = destIdx[i-1]*2 + 2;
            }
        }
    }
    
	return 0;
}


/**
 * Function Name: getEvictString (support upto K_ARY = 10)
 *
 * Description: Generates the path for eviction acc. to eviction number based on reverse 
 * lexicographical order. 
 * [For details refer to 'Optimizing ORAM and using it efficiently for secure computation']
 * 
 * @param n_evict: (input) The eviction number
 * @return Bit sequence of reverse lexicographical eviction order
 */  
string ORAM::getEvictString(TYPE_ID n_evict)
{
    string s = "";
    while(n_evict!=0)
    {
        s+= to_string(n_evict%K_ARY);
        n_evict/= K_ARY;
    }
    while(s.size()<H)
    {
        s+="0";
    }
    return s;
}


/**
 * Function Name: getFullPathIdx
 *
 * Description: Creates array of the indexes of the buckets that are on the given path
 * 
 * @param fullPath: (output) The array of the indexes of buckets that are on given path
 * @param pathID: (input) The leaf ID based on the index of the bucket in ORAM tree.
 * @return 0 if successful
 */  
int ORAM::getFullPathIdx(TYPE_INDEX* fullPath, TYPE_INDEX pathID)
{
    TYPE_INDEX idx = pathID;
    for (int i = H; i >= 0; i--)
    {
        fullPath[i] = idx;
        idx = (idx-1) / K_ARY;
    }
	return 0;
}


/**
 * Function Name: createShares
 *
 * Description: Creates shares from an input based on Shamir's Secret Sharing algorithm
 * 
 * @param input: (input) The secret to be shared
 * @param output: (output) The array of shares generated from the secret
 * @return 0 if successful
 */  
int ORAM::createShares(TYPE_DATA secret, TYPE_DATA* secret_shares, TYPE_DATA* mac_shares)
{
    zz_p rand;
    zz_p sum;
    zz_p mac(secret);
    zz_p sum_mac;
    sum = 0; 
    sum_mac = 0;
    mac = mac * GLOBAL_MAC_KEY;
    for(int i = 1 ; i < SSS_PRIVACY_LEVEL+1 ; i++)
    {
        NTL::random(rand);
        secret_shares[i] = rand._zz_p__rep;
        sum = sum+secret_shares[i];
    
        //MAC
        if(mac_shares!=NULL)
        {
            NTL::random(rand);
            mac_shares[i] = rand._zz_p__rep;
            sum_mac += rand;
        }
    }
    secret_shares[0] = (secret - sum)._zz_p__rep;
    if(mac_shares!=NULL)
        mac_shares[0] =  (mac - sum_mac)._zz_p__rep;
    return 0;
    
}

// For Computational
int ORAM::createShares(TYPE_DATA secret, TYPE_DATA* secret_shares, TYPE_DATA* mac_shares, prng_state* pseudo_state, int secretShareIdx)
{
    zz_p rand;
    zz_p sum;
    zz_p mac(secret);
    zz_p sum_mac;
    unsigned long tmp;
    sum = 0; 
    sum_mac = 0;
    mac = mac * GLOBAL_MAC_KEY;
    for(int i = 0 ; i < SSS_PRIVACY_LEVEL+1 ; i++)
    {
        if(i == secretShareIdx)
            continue;
        if(pseudo_state!=NULL)
        {
            sober128_read((unsigned char*)&tmp,sizeof(unsigned long long),&pseudo_state[i]);
            rand = tmp;
        }
        else
        {
            NTL::random(rand);            
        }
    
        secret_shares[i] = rand._zz_p__rep;
        sum = sum+secret_shares[i];
    
        //MAC
        if(mac_shares!=NULL)
        {
            if(pseudo_state!=NULL)
            {
                sober128_read((unsigned char*)&tmp,sizeof(unsigned long long),&pseudo_state[i]);
                rand = tmp;
            }
            else
            {
                NTL::random(rand);            
            }
            //NTL::random(rand);
            
            mac_shares[i] = rand._zz_p__rep;
            sum_mac += rand;
        }
    }
    secret_shares[secretShareIdx] = (secret - sum)._zz_p__rep;
    if(mac_shares!=NULL)
        mac_shares[secretShareIdx] =  (mac - sum_mac)._zz_p__rep;
    return 0;
    
}


/**
 * Function Name: recoverSecret (for replies from XOR computation)
 *
 * Description: Recovers the secret from SSS_PRIVACY_LEVEL+1 shares 
 * 
 * @param shares: (input) Array of shared secret as data chunks
 * @param result: (output) Recovered secret from given shares
 * @return 0 if successful
 */  
int ORAM::recoverSecret(zz_p** secret_shares, zz_p** mac_shares, zz_p* secret, zz_p* mac)
{
    for( int i = 0 ; i < DATA_CHUNKS ; i++)
    {
        secret[i] = 0;
        mac[i] = 0;
        for(int j = 0 ; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            secret[i] += secret_shares[j][i];
            mac[i] += mac_shares[j][i];
        }
    }
    return 0;
}


/**
 * Function Name: recoverSecret (for replies from SSS computation)
 *
 * Description: Recovers the secret from SSS_PRIVACY_LEVEL+1 shares 
 * 
 * @param shares: (input) Array of shared secret as data chunks
 * @param result: (output) Recovered secret from given shares
 * @return 0 if successful
 */  
int ORAM::recoverSecret(unsigned char** retrieval_in, zz_p* secret)
{
    for( int i = 0, ii=0 ; i < BLOCK_SIZE ; i+=sizeof(zz_p), ii++)
    {
        secret[ii] = 0;
        for(int j = 0 ; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            secret[ii] += *((zz_p*)&retrieval_in[j][i]);
            
        }
    }
    return 0;
}


int ORAM::checkRandLinComb(zz_p* input, zz_p* input_mac)
{
    zz_p X1, Y1;
    for(int i = 0; i < NUM_SERVERS; i++)
    {
        X1 = *((zz_p*)&input[i]);
        Y1 += *((zz_p*)&input_mac[i]);
    }
    if(GLOBAL_MAC_KEY*X1 != Y1)
    {
        return -1;
    }
        
    return 0;
}
int ORAM::recoverSecret(unsigned char** retrieval_in, zz_p* secret, zz_p* mac)
{
    for( int i = 0, ii=0 ; i < BLOCK_SIZE ; i+=sizeof(zz_p), ii++)
    {
        secret[ii] = 0;
        for(int j = 0 ; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            secret[ii] += *((zz_p*)&retrieval_in[j][i]);
            
        }
    }
    for( int i = BLOCK_SIZE, ii=0 ; i < 2*BLOCK_SIZE ; i+=sizeof(zz_p),ii++)
    {
        mac[ii] = 0;
        for(int j = 0 ; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            mac[ii] += *((zz_p*)&retrieval_in[j][i]);
        }
    }
    
    return 0;
}

int ORAM::xor_createQuery(TYPE_INDEX idx, unsigned int DB_SIZE, unsigned char** output)
{
    int byte_len = ceil((double) DB_SIZE / 8.0);
    
    #if defined(SEEDING)
        ZZ randBits; 
        
        for(int i = 1 ; i < XOR_PRIVACY_LEVEL+1; i++)
        {
            randBits =  RandomBits_ZZ(byte_len*8);
            BytesFromZZ(output[i],randBits,byte_len);
        }
    #else
        ZZ randBits =  RandomBits_ZZ(byte_len*(XOR_PRIVACY_LEVEL)*8);
        
        unsigned char* pseudo_random_number = new unsigned char[byte_len*(XOR_PRIVACY_LEVEL)];
        
        memset(pseudo_random_number,0,byte_len*(XOR_PRIVACY_LEVEL));
        
        
        BytesFromZZ(pseudo_random_number,randBits,byte_len*(XOR_PRIVACY_LEVEL));
        
        
        for(int i = 1 ; i < XOR_PRIVACY_LEVEL+1; i++)
        {
            memcpy(output[i],&pseudo_random_number[byte_len*(i-1)],byte_len);
        }
    #endif
    
    memset(output[0],0,byte_len);
    if(idx>=0)
        BIT_SET(&output[0][idx/8],idx%8);
    
    unsigned long long *s1,*s2;
    
    for(int i = 1; i < XOR_PRIVACY_LEVEL+1; i++)
    {
        for(int j = 0 ; j < byte_len; j += sizeof(unsigned long long))
        {
            s2 = (unsigned long long*) &output[0][j];
            s1 = (unsigned long long*) &output[i][j]; 
            *s2 ^= *s1;
        }
    }
    
    return 0;
}


int ORAM::sss_createQuery(TYPE_INDEX idx, unsigned int DB_SIZE, unsigned char** output)
{

    TYPE_DATA data_shares[SSS_PRIVACY_LEVEL+1];
    TYPE_DATA mac_shares[SSS_PRIVACY_LEVEL+1];
    
    for (TYPE_INDEX i = 0; i < DB_SIZE; i++)
	{
		createShares(0,data_shares, mac_shares); 
        
		for (int j = 0; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            memcpy(&output[j][i*sizeof(TYPE_DATA)],&data_shares[j],sizeof(TYPE_DATA));
            #if defined(SPDZ)
                memcpy(&output[j][DB_SIZE*sizeof(TYPE_DATA) + i*sizeof(TYPE_DATA)],&mac_shares[j],sizeof(TYPE_DATA));
            #endif
		}
	}
    createShares(1,data_shares, mac_shares); 
    for (int j = 0; j < SSS_PRIVACY_LEVEL+1; j++)
    {
        memcpy(&output[j][idx*sizeof(TYPE_DATA)],&data_shares[j],sizeof(TYPE_DATA));
        #if defined(SPDZ)
            memcpy(&output[j][DB_SIZE*sizeof(TYPE_DATA) + idx*sizeof(TYPE_DATA)],&mac_shares[j],sizeof(TYPE_DATA));
        #endif
    }
	
	return 0;
}

// For Computational 
int ORAM::sss_createQuery(TYPE_INDEX idx, unsigned int DB_SIZE, unsigned char** output, prng_state *prng)
{

    TYPE_DATA data_shares[SSS_PRIVACY_LEVEL+1];
    TYPE_DATA mac_shares[SSS_PRIVACY_LEVEL+1];
    
    for (TYPE_INDEX i = 0; i < DB_SIZE; i++)
	{
        if(i==idx)
        {
            #if defined(SPDZ)
                createShares(1,data_shares, mac_shares,prng,0); //NULL if using RSSS, if using SPDZ, need to have MAC
            #else
                createShares(1,data_shares, NULL,prng,0);
            #endif
        }
        else
        {
            #if defined(SPDZ)
                createShares(0,data_shares, mac_shares,prng,0); //NULL if using RSSS, if using SPDZ, need to have MAC
            #else
                createShares(0,data_shares, NULL,prng,0);
            #endif
        }
		for (int j = 0; j < SSS_PRIVACY_LEVEL+1; j++)
        {
            memcpy(&output[j][i*sizeof(TYPE_DATA)],&data_shares[j],sizeof(TYPE_DATA));
            #if defined(SPDZ)
                memcpy(&output[j][DB_SIZE*sizeof(TYPE_DATA) + i*sizeof(TYPE_DATA)],&mac_shares[j],sizeof(TYPE_DATA));
            #endif
		}
	}
    
    /*for (int j = 0; j < SSS_PRIVACY_LEVEL+1; j++)
    {
        memcpy(&output[j][idx*sizeof(TYPE_DATA)],&data_shares[j],sizeof(TYPE_DATA));
    }*/
	
    
	return 0;
}

int ORAM::xor_reconstruct(unsigned char** input, zz_p* output, zz_p* output_mac)
{
    memset(output,0,BLOCK_SIZE);
    memset(output_mac,0,BLOCK_SIZE);
    unsigned long long *s1,*s2;
    
    for(int i = 0 ; i < NUM_SHARE_PER_SERVER;i++)
    {
        int jj = 0;
        for(int j = 0 ; j < BLOCK_SIZE; j += sizeof(unsigned long long),jj++)
        {
            s2 = (unsigned long long*)&input[i][j];
            output[jj]._zz_p__rep ^= *s2;
        }
        jj=0;
        for(int j = BLOCK_SIZE ; j < BLOCK_SIZE*2; j += sizeof(unsigned long long),jj++)
        {
            s2 = (unsigned long long*)&input[i][j];
            output_mac[jj]._zz_p__rep ^= *s2;
        } 
    }  
    return 0;
}
int ORAM::xor_retrieve(unsigned char* query, zz_p** db, zz_p** db_mac, int start_db_idx, int end_db_idx, unsigned char* output, unsigned char* output_mac)
{
    unsigned long long *s1,*s2;
    unsigned long long *s1_mac,*s2_mac;
    
    memset(output,0,BLOCK_SIZE);
    memset(output_mac,0,BLOCK_SIZE);

    for(int i = start_db_idx ; i < end_db_idx ; i++)
    {
        if(BIT_CHECK(&query[i/8],i%8))
        {
            for(int j = 0 ,jj=0; j < BLOCK_SIZE; j+=sizeof(unsigned long long),jj++)
            {
                s1 = (unsigned long long*)&output[j];
                *s1 ^= db[i][jj]._zz_p__rep;
                
                s1_mac = (unsigned long long*)&output_mac[j];
                *s1_mac ^= db_mac[i][jj]._zz_p__rep;
            }
        }
    }
    return 0;
}


/**
 * Function Name: precomputeShares
 *
 * Description: Creates several shares from an input based on Shamir's Secret Sharing algorithm
 * for precomputation purposes
 * 
 * @param input: (input) The secret to be shared
 * @param output: (output) 2D array of shares generated from the secret (PRIVACY_LEVEL x output_size)
 * @param output_size: (output) The size of generated shares from the secret
 * @return 0 if successful
 */  
//int ORAM::precomputeShares(TYPE_DATA input, TYPE_DATA** output, TYPE_INDEX output_size)
//{
//    unsigned long long random[PRIVACY_LEVEL];
//	cout << "=================================================================" << endl;
//	cout<< "[ORAM] Precomputing Shares for " << input << endl;
//    boost::progress_display show_progress(output_size);
//    
//	for(int k = 0; k < output_size; k++){
//		for ( int i = 0 ; i < PRIVACY_LEVEL ; i++)
//		{
//        #if defined (NTL_LIB)
//            zz_p rand;
//            NTL::random(rand);
//            memcpy(&random[i], &rand,sizeof(TYPE_DATA));
//        #else
//            random[i] = Utils::_LongRand()+1 % P;
//		#endif
//        }
//		for(unsigned long int i = 1; i <= NUM_SERVERS; i++)
//		{
//			output[i-1][k] = input;
//			for(int j = 1 ; j <= PRIVACY_LEVEL ; j++)
//			{
//				output[i-1][k] = (output[i-1][k] + Utils::mulmod(random[j-1],i)) % P;
//			}
//		}
//		++show_progress;
//	}
//	
//	return 0;
//}


// Circuit-ORAM layout


/**
* Function Name: getFullEvictPathIdx (!!? Combine with getEvictIdx in ORAM )
*
* Description: Determine the indices of source bucket, destination bucket and sibling bucket
* residing on the eviction path
*
* @param srcIdx: (output) source bucket index array
* @param destIdx: (output) destination bucket index array
* @param siblIdx: (output) sibling bucket index array
* @param str_evict: (input) eviction edges calculated by binary ReverseOrder of the eviction number
* @return 0 if successful
*/
int ORAM::getFullEvictPathIdx(TYPE_INDEX *fullPathIdx, string str_evict)
{
	fullPathIdx[0] = 0;
	for (int i = 0; i < HEIGHT; i++)
	{
        int val = str_evict[i] - '0';
        fullPathIdx[i+1] = (fullPathIdx[i] * K_ARY) + (val +1);
	}
	return 0;
}
int ORAM::getDeepestLevel(TYPE_INDEX PathID, TYPE_INDEX blockPathID)
{	
    TYPE_INDEX full_path_idx[HEIGHT+1];
    TYPE_INDEX full_path_idx_of_block[HEIGHT+1];
    
	getFullPathIdx(full_path_idx, PathID);

	getFullPathIdx(full_path_idx_of_block, blockPathID);
	for (int j = HEIGHT; j >= 0; j--)
	{
		if (full_path_idx_of_block[j] == full_path_idx[j])
		{
			return j;
		}
	}
	return -1;
}
void ORAM::getDeepestBucketIdx(TYPE_INDEX* meta_stash, TYPE_INDEX* meta_path, TYPE_INDEX evictPathID, int* output)
{
	for (int i = 0; i < H + 2; i++)
		output[i] = -1;
	int deepest = -1;
	for (int i = 0; i < STASH_SIZE; i++)
	{
		if (meta_stash[i] != -1)
		{
			int k = getDeepestLevel(evictPathID, meta_stash[i]);
			if (k >= deepest)
			{
				deepest = k;
				output[0] = i;
			}
		}
	}
	for (int i = 0; i < HEIGHT + 1; i++)
	{
		deepest = getDeepestLevel(evictPathID, meta_path[i*BUCKET_SIZE]);
		if (deepest != -1)
			output[i + 1] = 0;
		for (int j = 1; j < BUCKET_SIZE; j++)
		{
			int k = getDeepestLevel(evictPathID, meta_path[i*BUCKET_SIZE+j]);
			if (k > deepest)
			{
				deepest = k;
				output[i + 1] = j;
			}
		}
	}
}

int ORAM::prepareDeepest(TYPE_INDEX* meta_stash, TYPE_INDEX* meta_path, TYPE_INDEX PathID, int* deepest)
{
	int goal = -2;
	int src = -2;
    int deeperBlockIdx[HEIGHT+2];
    
	getDeepestBucketIdx(meta_stash, meta_path, PathID, deeperBlockIdx);

	for (int i = 0; i < HEIGHT + 2; i++)
	{
		deepest[i] = -2;
	}
	//if the stash is not empty
	if (deeperBlockIdx[0] != -1)
	{
		src = -1;
		goal = getDeepestLevel(PathID, meta_stash[deeperBlockIdx[0]]);
	}
	for (int i = 0; i < HEIGHT + 1; i++)
	{
		if (goal >= i)
		{
			deepest[i] = src;
		}
		int l = -2;
		if (deeperBlockIdx[i + 1] != -1)
			l = getDeepestLevel(PathID, meta_path[i*BUCKET_SIZE + deeperBlockIdx[i + 1]]);
		if (l > goal)
		{
			goal = l;
			src = i;
		}
	}

	for (int i = HEIGHT + 1; i >= 0; i--)
	{
		deepest[i + 1] = deepest[i];
		if (deepest[i + 1] != -2)
			deepest[i + 1] += 1;
	}
	deepest[0] = -2;
	return 0;
}

int ORAM::getEmptySlot(TYPE_INDEX* meta_path, int level)
{
	for (int i = level*BUCKET_SIZE; i < (level + 1)*BUCKET_SIZE; i++)
	{
		if (meta_path[i] == -1) //!?
			return (i % BUCKET_SIZE);
	}
	return -1;
}
int ORAM::prepareTarget(TYPE_INDEX* meta_path, TYPE_INDEX pathID, int *deepest, int* target)
{
	int dest = -2;
	int src = -2;
	for (int i = 0; i < HEIGHT + 2; i++)
	{
		target[i] = -2;
	}
	for (int i = HEIGHT + 1; i > 0; i--)
	{
		if (i == src)
		{
			target[i] = dest;
			dest = -2;
			src = -2;
		}
		if (i >= 0)
		{
			if (((dest == -2 && getEmptySlot(meta_path,i-1) != -1) || target[i] != -2) && deepest[i] != -2)
			{
				src = deepest[i];
				dest = i;
			}
		}
	}
	//Stash case
	if (src == 0)
	{
		target[0] = dest;
	}
	return 0;
}


int ORAM::createRetrievalTriplets(int n)
{
    srand(time(NULL));
    zz_p*** A = new zz_p**[n];
    zz_p** B = new zz_p*[n];
    zz_p** C = new zz_p*[n];
    for(int w = 0; w < n; w++)
    {
        A[w] = new zz_p*[DATA_CHUNKS];
        B[w] = new zz_p[PATH_LENGTH];
        C[w] = new zz_p[DATA_CHUNKS];

        for(int i = 0 ; i < DATA_CHUNKS; i++)
        {
            A[w][i] = new zz_p[PATH_LENGTH];
        }

        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            for(int j = 0; j < PATH_LENGTH; j++)
            {
                A[w][i][j] = rand();
            }
        }

        for(int i = 0; i < PATH_LENGTH; i++)
        {
            B[w][i] = rand();
        }

        perform_dot_product(A[w], B[w], C[w], DATA_CHUNKS, PATH_LENGTH);
    }


    zz_p**** shares_A = new zz_p***[NUM_SERVERS];
    zz_p**** shares_A_mac = new zz_p***[NUM_SERVERS];
    for(int k = 0; k < NUM_SERVERS; k++)
    {
        shares_A[k] = new zz_p**[n];
        shares_A_mac[k] = new zz_p**[n];
        for(int i = 0 ; i < n; i++)
        {
            shares_A[k][i] = new zz_p*[DATA_CHUNKS];
            shares_A_mac[k][i] = new zz_p*[DATA_CHUNKS];
            for(int j = 0; j < DATA_CHUNKS; j++)
            {
                shares_A[k][i][j] = new zz_p[PATH_LENGTH];
                shares_A_mac[k][i][j] = new zz_p[PATH_LENGTH];
            }
        }
    }
    zz_p*** shares_B = new zz_p**[NUM_SERVERS];
    zz_p*** shares_B_mac = new zz_p**[NUM_SERVERS];
    for(int k = 0; k < NUM_SERVERS; k++)
    {
        shares_B[k] = new zz_p*[n];
        shares_B_mac[k] = new zz_p*[n];
        for(int i = 0; i < n; i++)
        {
            shares_B[k][i] = new zz_p[PATH_LENGTH];
            shares_B_mac[k][i] = new zz_p[PATH_LENGTH];
        }
    }
    zz_p*** shares_C = new zz_p**[NUM_SERVERS];
    zz_p*** shares_C_mac = new zz_p**[NUM_SERVERS];
    for(int k = 0; k < NUM_SERVERS; k++)
    {
        shares_C[k] = new zz_p*[n];
        shares_C_mac[k] = new zz_p*[n];
        for(int i = 0; i < n; i++)
        {
            shares_C[k][i] = new zz_p[DATA_CHUNKS];
            shares_C_mac[k][i] = new zz_p[DATA_CHUNKS];
        }
    }
    TYPE_DATA data_shares[NUM_SERVERS];
    TYPE_DATA mac_shares[NUM_SERVERS];
    for(int w = 0; w < n; w++)
    {
        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            for(int j = 0; j < PATH_LENGTH; j++)
            {
                long x;
                conv(x, A[w][i][j]);
                
                createShares(x, data_shares, mac_shares); 
                

                for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
                {
                    memcpy(&shares_A[k][w][i][j], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&shares_A_mac[k][w][i][j], &mac_shares[k], sizeof(TYPE_DATA));
                } 
            }
        }
        

        for(int i = 0; i < PATH_LENGTH; i++)
        {
            long x;
            conv(x, B[w][i]);
            
            createShares(x, data_shares, mac_shares); 
           

            for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
            {
                memcpy(&shares_B[k][w][i], &data_shares[k], sizeof(TYPE_DATA));
                memcpy(&shares_B_mac[k][w][i], &mac_shares[k], sizeof(TYPE_DATA));
            } 
        }

        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            long x;
            conv(x, C[w][i]);
            createShares(x, data_shares, mac_shares); 

            for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
            {
                memcpy(&shares_C[k][w][i], &data_shares[k], sizeof(TYPE_DATA));
                memcpy(&shares_C_mac[k][w][i], &mac_shares[k], sizeof(TYPE_DATA));
            } 
        } 
    }

    writeTriplets(shares_A, n, DATA_CHUNKS, PATH_LENGTH, "retrieval_triplet_a");
    writeTriplets(shares_A_mac, n, DATA_CHUNKS, PATH_LENGTH, "retrieval_triplet_a_mac");
    writeTriplets(shares_B, n, PATH_LENGTH, "retrieval_triplet_b");
    writeTriplets(shares_B_mac, n, PATH_LENGTH, "retrieval_triplet_b_mac");
    writeTriplets(shares_C, n, DATA_CHUNKS, "retrieval_triplet_c");
    writeTriplets(shares_C_mac, n, DATA_CHUNKS, "retrieval_triplet_c_mac");

    clearMemory(A, n, DATA_CHUNKS);
    clearMemory(B, n);
    clearMemory(C, n);
    clearMemory(shares_A, NUM_SERVERS, n, DATA_CHUNKS);
    clearMemory(shares_A_mac, NUM_SERVERS, n, DATA_CHUNKS);
    clearMemory(shares_B, NUM_SERVERS, n);
    clearMemory(shares_B_mac, NUM_SERVERS, n);
    clearMemory(shares_C, NUM_SERVERS, n);
    clearMemory(shares_C_mac, NUM_SERVERS, n);
	return 0;
}


int ORAM::createEvictionTriplets(int n)
{
    srand(time(NULL));
    zz_p*** A = new zz_p**[n];
    zz_p*** B = new zz_p**[n];
    zz_p*** C = new zz_p**[n];
    for(int w = 0; w < n; w++)
    {
        A[w] = new zz_p*[DATA_CHUNKS];
        B[w] = new zz_p*[EVICT_MAT_NUM_ROW];
        C[w] = new zz_p*[DATA_CHUNKS];

        for(int i = 0 ; i < DATA_CHUNKS; i++)
        {
            A[w][i] = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
            C[w][i] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
        }
        for(int i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
        {
            B[w][i] = new zz_p[EVICT_MAT_NUM_COL];
        }

        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            for(int j = 0; j < MAT_PRODUCT_INPUT_DB_LENGTH; j++)
            {
                A[w][i][j] = rand();
            }
        }

        for(int i = 0; i < EVICT_MAT_NUM_ROW; i++)
        {
            for(int j = 0; j < EVICT_MAT_NUM_COL; j++)
            {
                B[w][i][j] = rand();
            }
        }
        perform_cross_product(A[w], B[w], C[w], DATA_CHUNKS, MAT_PRODUCT_OUTPUT_LENGTH, MAT_PRODUCT_INPUT_DB_LENGTH);
    }
    
    
    zz_p**** shares_A = new zz_p***[NUM_SERVERS];
    zz_p**** shares_C = new zz_p***[NUM_SERVERS];
    zz_p**** shares_A_MAC = new zz_p***[NUM_SERVERS];
    zz_p**** shares_C_MAC = new zz_p***[NUM_SERVERS];
    for(int k = 0; k < NUM_SERVERS; k++)
    {
        shares_A[k] = new zz_p**[n];
        shares_C[k] = new zz_p**[n];
        shares_A_MAC[k] = new zz_p**[n];
        shares_C_MAC[k] = new zz_p**[n];
        for(int i = 0 ; i < n; i++)
        {
            shares_A[k][i] = new zz_p*[DATA_CHUNKS];
            shares_C[k][i] = new zz_p*[DATA_CHUNKS];
            shares_A_MAC[k][i] = new zz_p*[DATA_CHUNKS];
            shares_C_MAC[k][i] = new zz_p*[DATA_CHUNKS];
            for(int j = 0; j < DATA_CHUNKS; j++)
            {
                shares_A[k][i][j] = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
                shares_C[k][i][j] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
                shares_A_MAC[k][i][j] = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
                shares_C_MAC[k][i][j] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
            }
        }
    }

    zz_p**** shares_B = new zz_p***[NUM_SERVERS];
    zz_p**** shares_B_MAC = new zz_p***[NUM_SERVERS];
    for(int k = 0; k < NUM_SERVERS; k++)
    {
        shares_B[k] = new zz_p**[n];
        shares_B_MAC[k] = new zz_p**[n];
        for(int i = 0 ; i < n; i++)
        {
            shares_B[k][i] = new zz_p*[EVICT_MAT_NUM_ROW];
            shares_B_MAC[k][i] = new zz_p*[EVICT_MAT_NUM_ROW];
            for(int j = 0; j < EVICT_MAT_NUM_ROW; j++)
            {
                shares_B[k][i][j] = new zz_p[EVICT_MAT_NUM_COL];
                shares_B_MAC[k][i][j] = new zz_p[EVICT_MAT_NUM_COL];
            }
        }
    }
    TYPE_DATA data_shares[NUM_SERVERS];
    TYPE_DATA mac_shares[NUM_SERVERS];

    for(int w = 0; w < n; w++)
    {
        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            for(int j = 0; j < MAT_PRODUCT_INPUT_DB_LENGTH; j++)
            {
                long x;
                conv(x, A[w][i][j]);
                createShares(x, data_shares, mac_shares); 

                for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
                {
                    memcpy(&shares_A[k][w][i][j], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&shares_A_MAC[k][w][i][j], &mac_shares[k], sizeof(TYPE_DATA));
                } 
            }
        }

        for(int i = 0; i < EVICT_MAT_NUM_ROW; i++)
        {
            for(int j = 0; j < EVICT_MAT_NUM_COL; j++)
            {
                long x;
                conv(x, B[w][i][j]);
                
                createShares(x, data_shares, mac_shares); 
                

                for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
                {
                    memcpy(&shares_B[k][w][i][j], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&shares_B_MAC[k][w][i][j], &mac_shares[k], sizeof(TYPE_DATA));
                } 
            }
        }

        for(int i = 0; i < DATA_CHUNKS; i++)
        {
            for(int j = 0; j < MAT_PRODUCT_OUTPUT_LENGTH; j++)
            {
                long x;
                conv(x, C[w][i][j]);
                    createShares(x, data_shares, mac_shares); 

                for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)  
                {
                    memcpy(&shares_C[k][w][i][j], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&shares_C_MAC[k][w][i][j], &mac_shares[k], sizeof(TYPE_DATA));
                } 
            }
        }
    }

    writeTriplets(shares_A, n, DATA_CHUNKS, MAT_PRODUCT_INPUT_DB_LENGTH, "evict_triplet_a");
    writeTriplets(shares_A_MAC, n, DATA_CHUNKS, MAT_PRODUCT_INPUT_DB_LENGTH, "evict_triplet_a_mac");
    writeTriplets(shares_B, n, EVICT_MAT_NUM_ROW, EVICT_MAT_NUM_COL, "evict_triplet_b");
    writeTriplets(shares_B_MAC, n, EVICT_MAT_NUM_ROW, EVICT_MAT_NUM_COL, "evict_triplet_b_mac");
    writeTriplets(shares_C, n, DATA_CHUNKS, MAT_PRODUCT_OUTPUT_LENGTH, "evict_triplet_c");
    writeTriplets(shares_C_MAC, n, DATA_CHUNKS, MAT_PRODUCT_OUTPUT_LENGTH, "evict_triplet_c_mac");

    clearMemory(A, n, DATA_CHUNKS);
    clearMemory(B, n, EVICT_MAT_NUM_ROW);
    clearMemory(C, n, DATA_CHUNKS);
    clearMemory(shares_A, NUM_SERVERS, n, DATA_CHUNKS);
    clearMemory(shares_A_MAC, NUM_SERVERS, n, DATA_CHUNKS);
    clearMemory(shares_B, NUM_SERVERS, n, EVICT_MAT_NUM_ROW);
    clearMemory(shares_B_MAC, NUM_SERVERS, n, EVICT_MAT_NUM_ROW);
    clearMemory(shares_C, NUM_SERVERS, n, DATA_CHUNKS);
    clearMemory(shares_C_MAC, NUM_SERVERS, n, DATA_CHUNKS);
    
	return 0;
}

int ORAM::perform_dot_product(zz_p** A, zz_p* B, zz_p* C, int row, int input_length)
{
    for(int l = 0 ; l < row; l++) 
    {
        C[l] = InnerProd_LL(A[l], B, input_length, P, zz_p::ll_red_struct());
    }
    return 0;
}


int ORAM::perform_cross_product(zz_p** A, zz_p** B, zz_p** C, int row, int output_length, int input_length)
{
    for(int l = 0 ; l < row; l++) 
    {
        for(int k = 0 ; k < output_length; k++)
        {
            C[l][k] = InnerProd_LL(A[l], B[k], input_length, P, zz_p::ll_red_struct());
        }
    }

    return 0;
}

int ORAM::writeTriplets(zz_p**** data, int n, int row, int col, string file_name)
{
    string path;
    FILE* file_out = NULL;
    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++) 
    {
        path = rootPath + to_string(k) + "/" + file_name;
        if((file_out = fopen(path.c_str(),"wb+")) == NULL)
        {
            cout<< path << " Cannot Be Opened!!" <<endl;
            exit;
        }
        for(int i = 0; i < n; i++)
        {
            for(int j = 0 ; j< row; j++)
            {
                fwrite(data[k][i][j], 1, col*sizeof(TYPE_DATA), file_out);
            }
        }
        fclose(file_out);
    }
    return 0;
}

int ORAM::writeTriplets(zz_p*** data, int n, int col, string file_name)
{
    string path;
    FILE* file_out = NULL;
    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++) 
    {
        path = rootPath + to_string(k) + "/" + file_name;
        if((file_out = fopen(path.c_str(),"wb+")) == NULL)
        {
            cout<< path << " Cannot Be Opened!!" <<endl;
            exit;
        }
        for(int i = 0; i < n; i++)
        {   
            fwrite(data[k][i], 1, col*sizeof(TYPE_DATA), file_out);
        }
        fclose(file_out);
    }
    return 0;
}


int ORAM::clearMemory(zz_p** data, int m)
{   
    for(int i = 0; i < m; i++)
    {
        delete[] data[i];
    }
    delete[] data;
    return 0;
}

int ORAM::clearMemory(zz_p*** data, int m, int n)
{   
    for(int i = 0; i < m; i++)
    {
        for(int j = 0; j < n; j++)
        {
            delete[] data[i][j];
        }
        delete[] data[i];
    }
    delete[] data;
    return 0;
}

int ORAM::clearMemory(zz_p**** data, int m, int n, int p)
{   
    for(int i = 0; i < m; i++)
    {
        for(int j = 0; j < n; j++)
        {
            for(int k = 0; k < p; k++)
            {
                delete[] data[i][j][k];
            }
            delete[] data[i][j];
        }
        delete[] data[i];
    }
    delete[] data;
    return 0;
}