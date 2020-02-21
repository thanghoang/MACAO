/*
 * ClientBinaryORAMO.cpp
 *
 *      Author:  thanghoang
 */

#include "ClientBinaryORAMO.hpp"
#include "Utils.hpp"
#include "ORAM.hpp"


    
ClientBinaryORAMO::ClientBinaryORAMO() : ClientORAM()
{
    this->write_root_out = new unsigned char*[NUM_SERVERS];
    for (int i = 0 ; i < NUM_SERVERS; i ++)
    {
        this->write_root_out[i]= new unsigned char[BLOCK_SIZE*2+sizeof(TYPE_INDEX)];
        memset(this->write_root_out[i], 0, BLOCK_SIZE*2+sizeof(TYPE_INDEX) );
    }
    
}

ClientBinaryORAMO::~ClientBinaryORAMO()
{
    
}

/**
 * Function Name: access
 *
 * Description: Starts access operation for a block with its ID to be retrived from distributed servers. 
 * This operations consists of several subroutines: generating shares for logical access vector, 
 * retrieving shares from servers, recovering secret block from shares, assigning new path for the block,
 * re-share/upload the block back to servers, run eviction subroutine acc. to EVICT_RATE
 * 
 * @param blockID: (input) ID of the block to be retrieved
 * @return 0 if successful
 */  
int ClientBinaryORAMO::access(TYPE_ID blockID)
{
    auto start = time_now, end = time_now;
    ClientORAM::retrieve(blockID);
    
    cout << "	[ClientBinaryORAMO] Block-" << recoveredBlock[0] <<" is Retrieved" <<endl;
    if (recoveredBlock[0] == blockID)
        cout << "	[ClientBinaryORAMO] SUCCESS!!!!!!" << endl;
    else
        cout << "	[ClientBinaryORAMO] ERROR!!!!!!!!" << endl;
		
    assert(recoveredBlock[0] == blockID && "ERROR: RECEIEVED BLOCK IS NOT CORRECT!!!!!!");
	
    
    // 6. update position map
    this->updatePosMap(blockID);
    
    //write the retrieved block to root
    this->writeRoot();

    
    // 9. Perform eviction
    if(this->numRead == 0)
    {
        this->evict();
        
        this->numEvict = (numEvict+1) % N_leaf;
    }
    
    //save state
    this->saveState();
    
	return 0;
}


int ClientBinaryORAMO::updatePosMap(TYPE_ID blockID)
{
    ClientORAM::updatePosMap(blockID);
    
    pos_map[blockID].pathIdx = numRead;
    this->metaData[0][numRead] = blockID;
    
}

int ClientBinaryORAMO::evict()
{
    cout << "================================================================" << endl;
	cout << "STARTING EVICTION-" << this->numEvict+1 <<endl;
	cout << "================================================================" << endl;
        
    // 9.1. create permutation matrices
	for(TYPE_INDEX i = 0 ; i < H+1; i++)
    {
        memset(this->evictMatrix[i], 0, evictMatSize*sizeof(TYPE_DATA));
    }

	auto start = time_now;
    this->getEvictMatrix();
	auto end = time_now;
	cout<< "	[ClientBinaryORAMO] Evict Matrix Created in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
	exp_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	
		
	// 9.2. create shares of  permutation matrices 
	cout<< "	[ClientBinaryORAMO] Sharing Evict Matrix..." << endl;
		
	boost::progress_display show_progress((H+1)*evictMatSize);
    TYPE_DATA data_shares[NUM_SERVERS];
    TYPE_DATA mac_shares[NUM_SERVERS];
    
    start = time_now;
    for (TYPE_INDEX i = 0; i < H+1; ++i) 
    {
        for (TYPE_INDEX j = 0; j < evictMatSize; ++j)
        {
            #if defined(SEEDING)
                #if defined(SPDZ)
                    ORAM::createShares(this->evictMatrix[i][j], data_shares, mac_shares,prng_client,0);
                    this->sharedMatrix_MAC[0][i][j] = mac_shares[0];
                #else
                    ORAM::createShares(this->evictMatrix[i][j], data_shares, NULL,prng_client,0);
                #endif
                this->sharedMatrix[0][i][j] = data_shares[0];
                
            #else
                ORAM::createShares(this->evictMatrix[i][j], data_shares, mac_shares);
                for (int k = 0; k < NUM_SERVERS; k++) 
                {   
                    this->sharedMatrix[k][i][j] = data_shares[k];
                    #if defined (SPDZ)
                        this->sharedMatrix_MAC[k][i][j] = mac_shares[k];
                    #endif
                }
            #endif
			++show_progress;
        }
    }
    end = time_now;
    cout<< "	[ClientBinaryORAMO] Shared Matrix Created in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
    exp_logs[6] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        
    // 9.3. send permutation matrices to servers
    start = time_now;
    
    long long curBufferIdx = 0;
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        #if defined(SEEDING)
            memcpy(&evict_out[i][0], &numEvict, sizeof(TYPE_INDEX));
            curBufferIdx = sizeof(TYPE_INDEX);
        #else
           memcpy(&evict_out[i][CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX)], &numEvict, sizeof(TYPE_INDEX));
        #endif
    }
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        for (TYPE_INDEX y = 0 ; y < H+1; y++)
        {
            memcpy(&evict_out[i][curBufferIdx + y*evictMatSize*sizeof(TYPE_DATA)], this->sharedMatrix[i][y], evictMatSize*sizeof(TYPE_DATA));
            #if (defined(RSSS) && !defined(SEEDING))
                memcpy(&evict_out[i][(CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX))/2+curBufferIdx + y*evictMatSize*sizeof(TYPE_DATA)], this->sharedMatrix[(i+1)%3][y], evictMatSize*sizeof(TYPE_DATA));
            #endif
            #if defined (SPDZ)
              memcpy(&evict_out[i][curBufferIdx + (y+H+1)*evictMatSize*sizeof(TYPE_DATA)], this->sharedMatrix_MAC[i][y], evictMatSize*sizeof(TYPE_DATA));
            #endif
        }
        #if defined(SEEDING)
            break;
        #endif   
    }
    #if defined(SEEDING) && defined(RSSS)
        memcpy(evict_out[2],evict_out[0],CLIENT_EVICTION_OUT_LENGTH);
    #endif
    
    long long n = CLIENT_EVICTION_OUT_LENGTH;
    #if defined (SPDZ)
        int m = 2*sizeof(TYPE_DATA);
    #else
        int m = 4*sizeof(TYPE_DATA);
    #endif
    for (int i = 0 ; i < NUM_SERVERS; i++)
    {
        thread_socket_args[i] = struct_socket(i, evict_out[i], n, lin_rand_com_in[i], m, CMD_EVICT,  NULL);  
        pthread_create(&thread_sockets[i], NULL, &ClientBinaryORAMO::thread_socket_func, (void*)&thread_socket_args[i]);
    
        #if defined(SEEDING)
            n = sizeof(TYPE_INDEX);
            #if defined(RSSS)
            if(i==1)
                n = CLIENT_EVICTION_OUT_LENGTH;
            #endif
        #endif
    }
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        pthread_join(thread_sockets[i], NULL);
    }
    end = time_now;
    cout<< "	[ClientBinaryORAMO] Eviction DONE in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
    
    exp_logs[8] = thread_max;
    thread_max = 0;
	
    
    checkRandLinCom();
    
	
	cout << "================================================================" << endl;
	cout << "EVICTION-" << this->numEvict+1 << " COMPLETED" << endl;
	cout << "================================================================" << endl;
        
}

int ClientBinaryORAMO::writeRoot()
{
    TYPE_DATA data_shares[NUM_SERVERS];
    TYPE_DATA mac_shares[NUM_SERVERS];
    #if defined(SEEDING)
        unsigned long long currBufferIdx = sizeof(TYPE_DATA) ;
    #else // RSSS or SPDZ
        unsigned long long currBufferIdx = 0 ;
    #endif
    
    for(int u = 0 ; u < DATA_CHUNKS; u++ )
    {
        
        #if defined(SEEDING)
            ORAM::createShares(recoveredBlock[u]._zz_p__rep, data_shares,mac_shares,prng_client,0);
            int k = 0;                
            memcpy(&write_root_out[k][currBufferIdx], &data_shares[k], sizeof(TYPE_DATA));
            memcpy(&write_root_out[k][currBufferIdx + BLOCK_SIZE], &mac_shares[k], sizeof(TYPE_DATA));
        #else // RSSS or SPDZ
            ORAM::createShares(recoveredBlock[u]._zz_p__rep, data_shares,mac_shares);
            for(int k = 0; k < NUM_SERVERS; k++) 
            {

                memcpy(&write_root_out[k][currBufferIdx], &data_shares[k], sizeof(TYPE_DATA));
                memcpy(&write_root_out[k][currBufferIdx + BLOCK_SIZE], &mac_shares[k], sizeof(TYPE_DATA));
            }
        #endif
        currBufferIdx += sizeof(TYPE_DATA);
    }
    for ( int k = 0 ; k < NUM_SERVERS; k++)
    {
        #if defined(SEEDING)
            memcpy(&write_root_out[k][0], &numRead, sizeof(TYPE_DATA));
        #else // RSSS or SPDZ
            memcpy(&write_root_out[k][BLOCK_SIZE*2], &numRead, sizeof(TYPE_DATA));
        #endif
    }
	// 8. upload the share to numRead-th slot in root bucket
    
    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++) 
    {
        #if defined(SEEDING)
            if(k==0)
            {
                thread_socket_args[k] = struct_socket(k, write_root_out[k], BLOCK_SIZE*2+sizeof(TYPE_DATA), NULL, 0, CMD_WRITE_ROOT,NULL);
            }
            else
            {
                thread_socket_args[k] = struct_socket(k, write_root_out[k], sizeof(TYPE_DATA), NULL, 0, CMD_WRITE_ROOT,NULL);
            }
        #else // RSSS or SPDZ
            thread_socket_args[k] = struct_socket(k, write_root_out[k], BLOCK_SIZE*2+sizeof(TYPE_DATA), NULL, 0, CMD_WRITE_ROOT,NULL);
        #endif
        
		pthread_create(&thread_sockets[k], NULL, &ClientBinaryORAMO::thread_socket_func, (void*)&thread_socket_args[k]);
    }
    
	this->numRead = (this->numRead+1)%EVICT_RATE;
    cout << "	[ClientBinaryORAMO] Number of Read = " << this->numRead <<endl;
    
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        pthread_join(thread_sockets[i], NULL);
        cout << "	[ClientBinaryORAMO] Block upload completed!" << endl;
    }
}

/**
 * Function Name: getEvictMatrix
 *
 * Description: Generates logical eviction matrix to evict blocks from root to leaves according to 
 * eviction number and source, destination and sibling buckets by scanning position map.
 * 
 * @return 0 if successful
 */  
int ClientBinaryORAMO::getEvictMatrix()
{
	ORAM ORAM;
    TYPE_INDEX* src_idx = new TYPE_INDEX[H];
    TYPE_INDEX* dest_idx = new TYPE_INDEX[H];
    TYPE_INDEX* sibl_idx = new TYPE_INDEX[H];
    
    string evict_str = ORAM.getEvictString(numEvict);
    
    ORAM.getEvictIdx(src_idx,dest_idx,sibl_idx,evict_str);
	cout<< "	[ClientBinaryORAMO] Creating Evict Matrix..." << endl;
	boost::progress_display show_progress(H);
	
    for(int h = 1 ; h  < H+1; h++)
    {
        //for all real blocks in destination bucket, keep it as is
        TYPE_INDEX curDest_idx = dest_idx[h-1];
        TYPE_INDEX curSrc_idx = src_idx[h-1];
        TYPE_INDEX curSibl_idx = sibl_idx[h-1];
        vector<TYPE_ID> lstEmptyIdx;
        vector<TYPE_ID> lstEmptyIdxSibl; //only used when h =H 
        if( h == H) // this is for sibling bucket at leaf level
        {
            
            for ( int ii = 0 ; ii < BUCKET_SIZE ; ii++)
            {
                if(metaData[curSibl_idx][ii]!=0)
                {
                    TYPE_ID blockID = metaData[curSibl_idx][ii];
                    TYPE_INDEX j = pos_map[blockID].pathIdx;
                    evictMatrix[h][ (j-BUCKET_SIZE*h)*BUCKET_SIZE*2 + (j-BUCKET_SIZE*(h-1) )  ] = 1; 
                }
                else
                {
                    lstEmptyIdxSibl.push_back(ii);
                }   
            }
        }
        else 
        {
            for(int ii = 0 ; ii < BUCKET_SIZE; ii++)
            {
                metaData[curSibl_idx][ii]=0;
            }
        }
        for ( int ii = 0 ; ii < BUCKET_SIZE ; ii++)
        {
            if(metaData[curDest_idx][ii]!=0)
            {
                TYPE_ID blockID = metaData[curDest_idx][ii];
                TYPE_INDEX j = pos_map[blockID].pathIdx;
                evictMatrix[h-1][ (j-BUCKET_SIZE*h)*BUCKET_SIZE*2 + (j-BUCKET_SIZE*(h-1) )  ] = 1; 
            }
            else
            {
                lstEmptyIdx.push_back(ii);
            }   
        }
        for(int ii = 0 ; ii< BUCKET_SIZE ; ii++)
        {
            if(metaData[curSrc_idx][ii]!=0)
            {
                TYPE_ID blockID = metaData[curSrc_idx][ii];
                metaData[curSrc_idx][ii]=0;
                    
                TYPE_INDEX j = pos_map[blockID].pathIdx;
                TYPE_INDEX s = pos_map[blockID].pathID;
                TYPE_INDEX fullPath[H+1];
                ORAM.getFullPathIdx(fullPath,s);
                if( fullPath[h] == curDest_idx)
                {
                    if(lstEmptyIdx.size()==0)
                    {
                        cout<< "	[ClientBinaryORAMO] Overflow!!!!. Please check the random generator or select smaller evict_rate"<<endl;
                        cin.get();
                        return -1;
                        
                    }
                    int emptyIdx = lstEmptyIdx[lstEmptyIdx.size()-1];
                    lstEmptyIdx.pop_back();
                    evictMatrix[h-1][ (emptyIdx)*BUCKET_SIZE*2 + (  j - BUCKET_SIZE*(h-1) ) ] = 1;     
                    pos_map[blockID].pathIdx = BUCKET_SIZE*h + emptyIdx;
                    metaData[curDest_idx][emptyIdx] = blockID;
                }
                else
                {
                    if(h<H)
                    {
                        pos_map[blockID].pathIdx= BUCKET_SIZE + j;
                        metaData[curSibl_idx][j%BUCKET_SIZE]=blockID;
                    }
                    else
                    {
                        if(lstEmptyIdxSibl.size()==0)
                        {
                            cout<< "	[ClientBinaryORAMO] Overflow!!!, Please check the random generator or select smaller evict_rate"<<endl;
                            cin.get();
                            return -1;
                        }
                        int emptyIdx = lstEmptyIdxSibl[lstEmptyIdxSibl.size()-1];
                        lstEmptyIdxSibl.pop_back();
                        evictMatrix[H][ (emptyIdx)*BUCKET_SIZE*2 + (  j - BUCKET_SIZE*(h-1) ) ] = 1;     
                        pos_map[blockID].pathIdx = BUCKET_SIZE*h + emptyIdx;
                        metaData[curSibl_idx][emptyIdx] = blockID;
                        
                        }
                }
            }
        }
		++show_progress;
    }
    
	return 0;
}


