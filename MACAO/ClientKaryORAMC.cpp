/*
 * ClientKaryORAMC.cpp
 *
 *      Author: thanghoang
 */

#include "ClientKaryORAMC.hpp"
#include "Utils.hpp"
#include "ORAM.hpp"


ClientKaryORAMC::ClientKaryORAMC() : ClientORAM()
{

    
    //specific
    this->STASH = new TYPE_DATA*[STASH_SIZE];
    this->metaStash = new TYPE_INDEX[STASH_SIZE];
    for(int i = 0 ; i < STASH_SIZE; i++)
    {
        this->STASH[i] = new TYPE_DATA[DATA_CHUNKS];
        memset(this->STASH[i],0,BLOCK_SIZE);
        this->metaStash[i] = -1;
    }

		
}

ClientKaryORAMC::~ClientKaryORAMC()
{
}


int ClientKaryORAMC::init()
{
    ClientORAM::init();
    loadState();
}

/**
 * Function Name: load (inherent+specific)
 *
 * Description: Loads client storage data from disk for previously generated ORAM structure 
 * in order to continue ORAM operations. Loaded data includes postion map, current number of evictions,
 * current number of reads/writes.
 * 
 * @return 0 if successful
 */ 
int ClientKaryORAMC::loadState()
{
    ClientORAM::loadState();
	FILE* local_data = NULL;
	if((local_data = fopen(clientTempPath.c_str(),"rb")) == NULL){
		cout<< "	[load] File Cannot be Opened!!" <<endl;
		exit(0);
	}
	
    fseek(local_data,(NUM_BLOCK+1)*sizeof(TYPE_POS_MAP)+sizeof(this->numEvict)+sizeof(this->numRead),SEEK_SET);
    
    for(int i = 0 ; i < STASH_SIZE ; i ++)
    {
        fread(&this->metaStash[i], sizeof(TYPE_INDEX), 1, local_data);
    }
    for(int i = 0 ; i < STASH_SIZE ; i ++)
    {
        fread(this->STASH[i], sizeof(TYPE_DATA) * DATA_CHUNKS, 1, local_data);
    }
    fclose(local_data);
    
    return 0;
}



int ClientKaryORAMC::saveState()
{
    ClientORAM::saveState();
    // 11. store local info to disk
	FILE* local_data = NULL;
	if((local_data = fopen(clientTempPath.c_str(),"ab")) == NULL){
		cout<< "	[ClientKaryORAMC] File Cannot be Opened!!" <<endl;
		exit(0);
	}
    for(int i = 0 ; i < STASH_SIZE ; i ++)
    {
        fwrite(&this->metaStash[i], sizeof(TYPE_INDEX), 1, local_data);
    }
    for(int i = 0 ; i < STASH_SIZE ; i ++)
    {
        fwrite(this->STASH[i], sizeof(TYPE_DATA) * DATA_CHUNKS, 1, local_data);
    }
    
	fclose(local_data);
    
    return 0;
}



int ClientKaryORAMC::countNumBlockInStash()
{
    int count = 0;
    for(int i = 0 ; i < STASH_SIZE ; i++)
    {
        if(metaStash[i]!=-1)
        {
            count++;
        }
    }
    return count;
}
int ClientKaryORAMC::isRetrievedBlockInStash(TYPE_ID blockID)
{
    for(int i = 0 ; i < STASH_SIZE; i++)
    {
        if(this->STASH[i][0]==blockID)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Function Name: access (specific)
 *
 * Description: Starts access operation for a block with its ID to be retrived from distributed servers. 
 * This operations consists of several subroutines: generating shares for logical access vector, 
 * retrieving shares from servers, recovering secret block from shares, assigning new path for the block,
 * re-share/upload the block back to servers, run eviction subroutine acc. to EVICT_RATE
 * 
 * @param blockID: (input) ID of the block to be retrieved
 * @return 0 if successful
 */  
int ClientKaryORAMC::access(TYPE_ID blockID) 
{
    int BlockIdxInStash = isRetrievedBlockInStash(blockID);
    
    ClientORAM::retrieve(blockID);
    
    if (recoveredBlock[0] == blockID || (recoveredBlock[0]==0 && BlockIdxInStash!=-1))
        cout << "	[ClientKaryORAMC] SUCCESS!!!!!!" << endl;
    else if(recoveredBlock[0]==0)
    {
        if(BlockIdxInStash!=-1)
        {
            cout << "	[ClientKaryORAMC] SUCCESS!!!!!!" << endl;
            cout << "	[ClientKaryORAMC] BLOCK-"<<STASH[BlockIdxInStash]<<" RETRIEVED FROM STASH[ " <<BlockIdxInStash<<"]!!!!!!" << endl;
        }
        else
        {
            cout << "	[ClientKaryORAMC] ERROR!!!!!!!!" << endl;
            exit(0);
        }
    } 
    else
    {
        cout << "	[ClientKaryORAMC] ERROR!!!!!!!!" << endl;
        exit(0);
    }
    
     
    // 6. update position map
    this->updatePosMap(blockID);
    
    //7. perform eviction
    this->evict();
    
    //save state
    saveState();
	

    return 0;
}


int ClientKaryORAMC::evict()
{
    auto start = time_now, end = time_now;
    
    for(int i = 0 ; i < NUM_SERVERS; i++)
    {
        memcpy(&evict_out[i][CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX)], &numEvict, sizeof(TYPE_INDEX));
    }
    unsigned long long currBufferIdx = 0;
    for(int e = 0 ; e < 2; e++)
    {
        cout << "================================================================" << endl;
        cout << "STARTING EVICTION-" << this->numEvict+1 <<endl;
        cout << "================================================================" << endl;
        
        // 9.1. create permutation matrices
        for(TYPE_INDEX i = 0 ; i < H+1; i++)
        {
            memset(this->evictMatrix[i], 0, evictMatSize*sizeof(TYPE_DATA));
        }
        start = time_now;
        this->getEvictMatrix();
        end = time_now;
        cout<< "	[ClientKaryORAMC] Evict Matrix Created in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
        exp_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            
            
        // 9.2. create shares of  permutation matrices 
		cout<< "	[ClientKaryORAMC] Sharing Evict Matrix..." << endl;
		
		boost::progress_display show_progress((H+1)*evictMatSize);
        TYPE_DATA matrixShares[NUM_SERVERS];
        start = time_now;
        for (TYPE_INDEX i = 0; i < H+1; ++i) 
        {
            for (TYPE_INDEX j = 0; j < evictMatSize; j++)
            {
                ORAM::createShares(this->evictMatrix[i][j], matrixShares, NULL);
                for (int k = 0; k < NUM_SERVERS; k++) 
                {   
                    this->sharedMatrix[k][i][j] = matrixShares[k];
                }
				++show_progress;
            }
        }
        end = time_now;
        cout<< "	[ClientKaryORAMC] Shared Matrix Created in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
        exp_logs[6] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        
        // 9.3. create shares of a selected block in stash
        TYPE_DATA data_shares[NUM_SERVERS];
        TYPE_DATA mac_shares[NUM_SERVERS];
        if(target[0]>=0)
        {
            for(int u = 0 ; u < DATA_CHUNKS; u++ )
            {
                ORAM::createShares(this->STASH[deepestIdx[0]][u], data_shares, mac_shares);
                for(int k = 0; k < NUM_SERVERS; k++) 
                {
                    memcpy(&evict_out[k][currBufferIdx], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&evict_out[k][currBufferIdx + BLOCK_SIZE], &mac_shares[k], sizeof(TYPE_DATA));
                }
                currBufferIdx += sizeof(TYPE_DATA);
            }
            memset(STASH[deepestIdx[0]],0,sizeof(TYPE_DATA)*DATA_CHUNKS);
            metaStash[deepestIdx[0]] = -1;
        }
        else
        {
            for(int u = 0 ; u < DATA_CHUNKS; u++ )
            {
                ORAM::createShares(this->STASH[0][u], data_shares, mac_shares); //!? Check THIS!
                for(int k = 0; k < NUM_SERVERS; k++) 
                {
                    memcpy(&evict_out[k][currBufferIdx], &data_shares[k], sizeof(TYPE_DATA));
                    memcpy(&evict_out[k][currBufferIdx + BLOCK_SIZE], &mac_shares[k], sizeof(TYPE_DATA));
                }
                currBufferIdx += sizeof(TYPE_DATA);
            }
        }
        currBufferIdx += BLOCK_SIZE;
        for (int i = 0; i < NUM_SERVERS; i++)
        {
            for (TYPE_INDEX y = 0 ; y < H+1; y++)
            {
                memcpy(&evict_out[i][currBufferIdx + y*evictMatSize*sizeof(TYPE_DATA)], this->sharedMatrix[i][y], evictMatSize*sizeof(TYPE_DATA));
            }
        }
        
        currBufferIdx += (H+1)*evictMatSize*sizeof(TYPE_DATA);
        
        this->numEvict = (numEvict+1) % N_leaf;
    }
        
    // 9.3. send shares of permutation matrices & stash block to servers
    start = time_now;
            
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        thread_socket_args[i] = struct_socket(i, evict_out[i], CLIENT_EVICTION_OUT_LENGTH, NULL,0, CMD_EVICT,  NULL);
        pthread_create(&thread_sockets[i], NULL, &ClientKaryORAMC::thread_socket_func, (void*)&thread_socket_args[i]);
    }
			
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        pthread_join(thread_sockets[i], NULL);
    }
    end = time_now;
    cout<< "	[ClientKaryORAMC] Eviction DONE in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
    
    exp_logs[8] = thread_max;
    thread_max = 0;
    
    cout << "================================================================" << endl;
    cout << "EVICTION-" << this->numEvict+ 1 << " & " << this->numEvict+2 <<" COMPLETED" << endl;
    cout << "================================================================" << endl;
    
}
int ClientKaryORAMC::updatePosMap(TYPE_ID blockID)
{
    int BlockIdxInStash = isRetrievedBlockInStash(blockID);
    
    ClientORAM::updatePosMap(blockID);
    
    if(pos_map[blockID].pathIdx != -1) //-1 means it is residing in Stash
    {
        //put the block into Stash
        for(int i = 0 ; i < STASH_SIZE ; i++)
        {
            if(metaStash[i]==-1)
            {
                metaStash[i] = pos_map[blockID].pathID;
                memcpy(STASH[i],recoveredBlock,BLOCK_SIZE);
                pos_map[blockID].pathIdx = -1; //-1 means it is residing in Stash
                break;
            }
        }
        
    }
    else
    {
        metaStash[BlockIdxInStash] = pos_map[blockID].pathID;
    }
    return BlockIdxInStash;
}

/**
 * Function Name: getEvictMatrix (specific)
 *
 * Description: Generates logical eviction matrix to evict blocks from root to leaves according to 
 * eviction number and source, destination and sibling buckets by scanning position map.
 * 
 * @return 0 if successful
 */  
int ClientKaryORAMC::getEvictMatrix()
{
    
    TYPE_INDEX fullEvictPathIdx[HEIGHT+1];
    string strEvictPath = ORAM::getEvictString(numEvict);
    ORAM::getFullEvictPathIdx(fullEvictPathIdx,strEvictPath);
    
    TYPE_INDEX evictPathID = fullEvictPathIdx[HEIGHT];
    
    TYPE_INDEX metaPath[BUCKET_SIZE*(HEIGHT+1)];
   
    for(int h = 0 ; h < HEIGHT+1; h++)
    {
        for(int z = 0 ; z < BUCKET_SIZE; z++)
        {
            metaPath[h*BUCKET_SIZE+z] = pos_map[metaData[fullEvictPathIdx[h]][z]].pathID;
        }
    }
    ORAM::prepareDeepest(metaStash,metaPath,evictPathID,deepest);
    ORAM::prepareTarget(metaPath,evictPathID,deepest,target);
    ORAM::getDeepestBucketIdx(metaStash,metaPath,evictPathID,deepestIdx);
    
    for(int h = 0 ; h < HEIGHT +1 ; h++)
    {
        memset(evictMatrix[h],0,sizeof(TYPE_INDEX)*evictMatSize);
    }

    int currPickedSlotIdx = deepestIdx[0];
    int currTarget = target[0];
    TYPE_ID currPickedBlockID = 0;
    if(currTarget >= 0)
    {
        currPickedBlockID = STASH[currPickedSlotIdx][0];
        metaStash[currPickedSlotIdx] = -1;
    }
    int pickedEmptySlotIdx[HEIGHT+1] ={-1};
    for(int h = 1 ; h < HEIGHT +2; h++) //h=1 denotes root level
    {
        if(currTarget==h || currTarget < 0)
        {
            if(target[h]>h)
            {
                currPickedSlotIdx = deepestIdx[h];
                
                //release the slot of this block to be empty
                metaPath[(h-1)*BUCKET_SIZE+currPickedSlotIdx] = -1;
                //pick the block at this level
                evictMatrix[h-1][(BUCKET_SIZE)*(BUCKET_SIZE+1) + (currPickedSlotIdx+1)] = 1;
                
            }
            if(currTarget == h)
            {
                //drop the holding block to this level
                int empty_slot_idx = ORAM::getEmptySlot(metaPath,(h-1));
                pickedEmptySlotIdx[h-1] = empty_slot_idx;
                evictMatrix[h-1][empty_slot_idx*(BUCKET_SIZE+1) + 0] = 1;
                
            }
            currTarget = target[h];
            if(target[h] > h)
            {
                currPickedBlockID = metaData[fullEvictPathIdx[h-1]][currPickedSlotIdx];
                //metaData[fullEvictPathIdx[h-1]][currPickedSlotIdx] = 0;
                
            }
            
        }
        else // if (currTarget >h) //not drop yet, but move to the holding block to deeper level
        {
            evictMatrix[h-1][BUCKET_SIZE * (BUCKET_SIZE+1) + 0] = 1;
        }
        //keep real blocks remain at the same position
        for(int z = 0 ; z < BUCKET_SIZE; z++)
        {
            if(metaPath[(h-1)*BUCKET_SIZE + z] > 0)
            {
                evictMatrix[h-1][z * (BUCKET_SIZE+1) + z+1] = 1;
            }
        }
    }

  //at the end, update the final position of blocks
    for(int h = H+1 ; h > 0; h--)
    {
        if(target[h]>0)
        {
            int residingLevel = target[h];
            metaData[fullEvictPathIdx[residingLevel-1]][pickedEmptySlotIdx[residingLevel-1]] = metaData[fullEvictPathIdx[h-1]][deepestIdx[h]];
            TYPE_ID blockID = metaData[fullEvictPathIdx[h-1]][deepestIdx[h]];
            metaData[fullEvictPathIdx[h-1]][deepestIdx[h]] = 0;
            pos_map[blockID].pathIdx = (residingLevel-1)*BUCKET_SIZE + pickedEmptySlotIdx[residingLevel-1];
        }
    }
    // at stash
    if(target[0]>0)
    {
        int residingLevel = target[0];
        metaData[fullEvictPathIdx[residingLevel-1]][pickedEmptySlotIdx[residingLevel-1]] = STASH[deepestIdx[0]][0];
        metaStash[deepestIdx[0]] = -1;
        pos_map[STASH[deepestIdx[0]][0]].pathIdx = (residingLevel-1)*BUCKET_SIZE + pickedEmptySlotIdx[residingLevel-1];
    } 
	return 0;
}
 
