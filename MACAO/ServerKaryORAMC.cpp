#include "ServerKaryORAMC.hpp"

#include "Utils.hpp"
#include "struct_socket.h"

#include "ORAM.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

#include "struct_thread_computation.h"
#include "struct_thread_loadData.h"



ServerKaryORAMC::ServerKaryORAMC(TYPE_INDEX serverNo, int selectedThreads) : ServerORAM(serverNo,selectedThreads)
{

}

ServerKaryORAMC::ServerKaryORAMC()
{
}

ServerKaryORAMC::~ServerKaryORAMC()
{
}


int ServerKaryORAMC::prepareEvictComputation()
{
    #if defined(SEEDING)
        unsigned long long currBufferIdx = sizeof(TYPE_DATA);
        unsigned long long tmp = 0;
        unsigned long long tmp2[EVICT_MAT_NUM_COL];
        for(int e = 0 ; e < 2; e++)
        {
            //holdBlock
            for(int i = 0  ; i < DATA_CHUNKS; i++)
            {
                if(serverNo==0)
                {
                    memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER][i],&evict_in[currBufferIdx ],sizeof(TYPE_DATA));
                    memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER][i],&evict_in[currBufferIdx+BLOCK_SIZE],sizeof(TYPE_DATA));
                }
                else
                {
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[serverNo]);
                    this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER][i][0] = tmp;               
                    
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[serverNo]);
                    this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER][i][0] = tmp;
                }
                #if defined(RSSS)
                    if(serverNo==2)
                    {
                        memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+1][i],&evict_in[currBufferIdx],sizeof(TYPE_DATA));
                        memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+1][i],&evict_in[currBufferIdx+BLOCK_SIZE],sizeof(TYPE_DATA));
                    }
                    else
                    {
                        sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo+1)%3]);
                        this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+1][i][0] = tmp;
                        
                        sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo+1)%3]);
                        this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+1][i][0] = tmp;
                    }
                #endif
                
                currBufferIdx +=sizeof(TYPE_DATA);
            }
            currBufferIdx += BLOCK_SIZE;

           

           
            for (TYPE_INDEX y = 0 ; y < H+1 ; y++)
            {
                for (TYPE_INDEX i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
                {
                    if(serverNo==0)
                    {
                        memcpy(this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                        #if defined(SPDZ)
                            memcpy(this->vecEvictMatrix_MAC[e*NUM_SHARE_PER_SERVER][y][i], &evict_in[currBufferIdx + (H+1)*evictMatSize*sizeof(TYPE_DATA)], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));                            
                        #endif
                    }
                    else
                    {
                        for(TYPE_INDEX j = 0 ; j < EVICT_MAT_NUM_COL; j++)
                        {
                            sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[serverNo]);
                            this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER][y][i][j] = tmp;
                            #if defined(SPDZ)
                                sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[serverNo]);
                                this->vecEvictMatrix_MAC[e*NUM_SHARE_PER_SERVER][y][i][j] = tmp;
                            #endif    
                        }
                    }
                    #if defined(RSSS)
                        if(serverNo==2)
                        {
                            memcpy(this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+1][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                        }
                        else
                        {
                            sober128_read((unsigned char*)&tmp2,EVICT_MAT_NUM_COL*sizeof(TYPE_DATA),&prng_client[(serverNo+1)%3]);
                            for(TYPE_INDEX j = 0 ; j < EVICT_MAT_NUM_COL; j++)
                            {
                                this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+1][y][i][j] = tmp2[j];
                            }
                        }
                    #endif
                    currBufferIdx += EVICT_MAT_NUM_COL*sizeof(TYPE_DATA);
                }
                
            }
            #if defined(SPDZ)
                 currBufferIdx += (H+1)* evictMatSize*sizeof(TYPE_DATA);
            #endif




        }
    #else 
        unsigned long long currBufferIdx = 0;
        for(int e = 0 ; e < 2; e++)
        {
            //holdBlock
            for(int i = 0  ; i < DATA_CHUNKS; i++)
            {
                
                    memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER][i],&evict_in[currBufferIdx ],sizeof(TYPE_DATA));
                    memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER][i],&evict_in[currBufferIdx+BLOCK_SIZE],sizeof(TYPE_DATA));
                #if defined(RSSS)    
                
                    memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+1][i],&evict_in[(CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX))/2+currBufferIdx],sizeof(TYPE_DATA));
                    memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+1][i],&evict_in[(CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX))/2+currBufferIdx+BLOCK_SIZE],sizeof(TYPE_DATA));
                #endif 
                currBufferIdx +=sizeof(TYPE_DATA);
            }
            currBufferIdx += BLOCK_SIZE;
            int x = 0;
            for (TYPE_INDEX y = 0 ; y < H+1 ; y++)
            {
                for (TYPE_INDEX i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
                {
                    
                        memcpy(this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                    #if defined(RSSS)
                        //memcpy(this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+1][y][i], &client_evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                        memcpy(this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+1][y][i], &evict_in[(CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX))/2+currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                    #endif
                    currBufferIdx += EVICT_MAT_NUM_COL*sizeof(TYPE_DATA);
                }
            }
            for (TYPE_INDEX y = 0 ; y < H+1 ; y++)
            {
                for (TYPE_INDEX i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
                {
                    #if defined(SPDZ)
                        memcpy(this->vecEvictMatrix_MAC[e*NUM_SHARE_PER_SERVER][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                        currBufferIdx += EVICT_MAT_NUM_COL*sizeof(TYPE_DATA);
                    #else // RSSS 
                        //memcpy(this->vecEvictMatrix_MAC[e*NUM_SHARE_PER_SERVER+1][y][i], &client_evict_in[currBufferIdx], EVICT_MAT_NUM_COL*sizeof(TYPE_DATA));
                    #endif
                    
                }
            }
        }
    #endif
    for(int e = 0 ; e < 2 ; e++)
    {  
        string strEvictPath = ORAM::getEvictString(n_evict);
        ORAM::getFullEvictPathIdx(fullEvictPathIdx[e],strEvictPath);
        n_evict = (n_evict+1) % N_leaf;
    }
}

/**
 * Function Name: evict
 *
 * Description: Starts eviction operation with the command of the client by receiving eviction matrix
 * and eviction path no from the client. According to eviction path no, the server performs 
 * matrix multiplication with its buckets and eviction matrix to evict blocks. After eviction operation,
 * the degree of the sharing polynomial doubles. Thus all the servers distributes their shares and perform 
 * degree reduction routine simultaneously. 
 * 
 * @param socket: (input) ZeroMQ socket instance for communication with the client
 * @return 0 if successful
 */  
 
int ServerKaryORAMC::evict(zmq::socket_t& socket)
{
    this->X1 = 0; 
    this->Y1 = 0;
    this->X2 = 0; 
    this->Y2 = 0;
    recvClientEvictData(socket);
    
    prepareEvictComputation();  
    
    int es = 0, ee  = 1;
    unsigned long long buffer_length;
    
    FILE* file_out = NULL;
    string path;
    
    for(int h = 0; h < H+1 ; h++)
    {
        //since the root conflicts with two eviction paths, we process it first separately
        if(h==0)
        {
            buffer_length =  SERVER_RESHARE_IN_OUT_LENGTH/2;
        }
        else
        {
            ee = 2;
            es = 0;
            buffer_length =   SERVER_RESHARE_IN_OUT_LENGTH;
        }   
start:
        cout<<endl;
		cout << "	==============================================================" << endl;
		cout<< "	[evict] Starting Eviction-" << h+1 <<endl;
        
        //== THREADS FOR LISTENING =======================================================================================
        cout<< "	[evict] Creating Threads for Receiving Ports..." << endl;
        
        #if defined(SEEDING) && defined(RSSS)
        
            recvSocket_args[0] = struct_socket(0, NULL, 0, reshares_in[0], buffer_length, NULL,false);
            pthread_create(&thread_recv[0], NULL, &ServerORAM::thread_socket_func, (void*)&recvSocket_args[0]);
        #else
            for(TYPE_INDEX k = 0; k < NUM_SERVERS-1; k++)
            {
                recvSocket_args[k] = struct_socket(k, NULL, 0, reshares_in[k], buffer_length, NULL,false);
                pthread_create(&thread_recv[k], NULL, &ServerORAM::thread_socket_func, (void*)&recvSocket_args[k]);
            }
        #endif
        cout << "	[evict] CREATED!" << endl;
        //===============================================================================================================
        
        auto start = time_now;
        int step = ceil((double)DATA_CHUNKS/(double)numThreads);
        int endIdx;
        
        for(int e = es ; e < ee; e++)
        {
            
                this->readBucket_evict(fullEvictPathIdx[e][h], serverNo, this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER],this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER]);
            #if defined(RSSS)
                this->readBucket_evict(fullEvictPathIdx[e][h], (serverNo+1)%3, this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+1],this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+1]);
            #endif
        }
        auto end = time_now;
        long load_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        cout<< "	[evict] Evict Nodes READ from Disk in " << load_time <<endl;
        server_logs[7] += load_time;
            
        //perform matrix product
        cout<< "	[evict] Multiplying Evict Matrix..." << endl;
        start = time_now;
        this->preReSharing(h,es,ee); 	// SERVER SIDE COMPUTATION
        end = time_now;
        cout<< "	[evict] Multiplied in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[8] += std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        
        start = time_now;
        cout<< "	[evict] ReSharing..." << endl;
        this->reShare(h,es,ee);
        end = time_now;
        cout<< "	[evict] Reshared CREATED in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[9] += std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    
		//== THREADS FOR SENDING ============================================================================================
		cout<< "	[evict] Creating Threads for Sending Shares..."<< endl;
        #if defined(SEEDING) && defined(RSSS)
            sendSocket_args[1] = struct_socket(1,  reshares_out[1], buffer_length, NULL, 0, NULL, true);
            pthread_create(&thread_send[1], NULL, &ServerORAM::thread_socket_func, (void*)&sendSocket_args[1]);
        
            cout<< "	[evict] CREATED!" <<endl;
            //=================================================================================================================
            cout<< "	[evict] Waiting for Threads..." <<endl;
            
            pthread_join(thread_send[1], NULL);
            pthread_join(thread_recv[0], NULL);
            
        #else
            for (int i = 0; i < NUM_SERVERS-1; i++)
            {
                sendSocket_args[i] = struct_socket(i,  reshares_out[i], buffer_length, NULL, 0, NULL, true);
                pthread_create(&thread_send[i], NULL, &ServerORAM::thread_socket_func, (void*)&sendSocket_args[i]);
            }
            cout<< "	[evict] CREATED!" <<endl;
            //=================================================================================================================
            cout<< "	[evict] Waiting for Threads..." <<endl;
            for (int i = 0; i < NUM_SERVERS-1; i++)
            {
                pthread_join(thread_send[i], NULL);
                pthread_join(thread_recv[i], NULL);
            }
        #endif
		cout<< "	[evict] DONE!" <<endl;
		server_logs[10] += thread_max;
		thread_max = 0;
		
		cout << "	[evict] Post Resharing Computation..." << endl;
        
        this->postReSharing(h,es,ee);

        //write to file
        start = time_now;
    
        for(int e = es ; e < ee; e++)
        {            
            //this->writeBucket_reverse_mode(fullEvictPathIdx[e][h],serverNo,vecReShares[e][serverNo],vecReShares_MAC[e][serverNo]);
            this->writeBucket(fullEvictPathIdx[e][h],serverNo,vecReShares[e][serverNo],vecReShares_MAC[e][serverNo]);
            
            #if defined(RSSS)
                //this->writeBucket_reverse_mode(fullEvictPathIdx[e][h],(serverNo+1)%3,vecReShares[e][(serverNo+1)%3],vecReShares_MAC[e][(serverNo+1)%3]);
                this->writeBucket(fullEvictPathIdx[e][h],(serverNo+1)%3,vecReShares[e][(serverNo+1)%3],vecReShares_MAC[e][(serverNo+1)%3]);
            
            #endif
            
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER][j],&vecReShares[e][this->serverNo][j][BUCKET_SIZE]._zz_p__rep,sizeof(TYPE_DATA));
                // MAC
                memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER][j],&vecReShares_MAC[e][this->serverNo][j][BUCKET_SIZE]._zz_p__rep,sizeof(TYPE_DATA));
                
                #if defined(RSSS)
                    memcpy(this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+1][j],&vecReShares[e][(this->serverNo+1)%3][j][BUCKET_SIZE]._zz_p__rep,sizeof(TYPE_DATA));
                    // MAC
                    memcpy(this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+1][j],&vecReShares_MAC[e][(this->serverNo+1)%3][j][BUCKET_SIZE]._zz_p__rep,sizeof(TYPE_DATA));
                #endif
            }                
        }
    
        end = time_now;
        server_logs[12] += std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        if (h==0 && es == 0)
        {
            es =1;
            ee =2;
            goto start;
        }
    }
    memcpy(&lin_rand_com_out[0], &this->X1, sizeof(TYPE_DATA));
    memcpy(&lin_rand_com_out[sizeof(TYPE_DATA)], &this->Y1, sizeof(TYPE_DATA));
    #if defined(RSSS)
        memcpy(&lin_rand_com_out[2*sizeof(TYPE_DATA)], &this->X2, sizeof(TYPE_DATA));
        memcpy(&lin_rand_com_out[3*sizeof(TYPE_DATA)], &this->Y2, sizeof(TYPE_DATA));
    #endif
    
    #if defined(SPDZ)
        socket.send(lin_rand_com_out,2*sizeof(TYPE_DATA));
    #else // RSSS
        socket.send(lin_rand_com_out,4*sizeof(TYPE_DATA));
    #endif
	cout<< "	[evict] ACK is SENT!" <<endl;

    return 0;
}



int ServerKaryORAMC::readBucket_evict(TYPE_ID bucketID, int shareID, zz_p** output_data, zz_p** output_mac)
{
    FILE* file_in = NULL;
    string path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID);
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }
    #if !defined(REVERSE_STORAGE_LAYOUT)
        for(int i = 0 ; i < BUCKET_SIZE; i++)
        {
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                fread(&output_data[j][i+1], 1, sizeof(TYPE_DATA), file_in);
            }
        }
    #else
        for(int j = 0 ; j < DATA_CHUNKS; j++)
        {
            fread(&output_data[j][1], 1, sizeof(TYPE_DATA)*BUCKET_SIZE, file_in);
        }
    #endif
    
    fclose(file_in);

    path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID)+ "_mac";
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }
    #if !defined(REVERSE_STORAGE_LAYOUT)
        for(int i = 0 ; i < BUCKET_SIZE; i++)
        {
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                fread(&output_mac[j][i+1], 1, sizeof(TYPE_DATA), file_in);
            }
        }
    #else
        for(int j = 0 ; j < DATA_CHUNKS; j++)
        {
            fread(&output_mac[j][1], 1, sizeof(TYPE_DATA)*BUCKET_SIZE, file_in);
        }
    #endif
    fclose(file_in);
}
