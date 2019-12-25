#include "ClientORAM.hpp"
#include "ORAM.hpp"
#include "Utils.hpp"

unsigned long int ClientORAM::exp_logs[9];
unsigned long int ClientORAM::thread_max = 0;

char ClientORAM::timestamp[16];


zmq::context_t** ClientORAM::context = new zmq::context_t*[NUM_SERVERS];
zmq::socket_t**  ClientORAM::socket = new zmq::socket_t*[NUM_SERVERS];

ClientORAM::ClientORAM()
{
    unsigned int num_xor_queries; 
    
    #if defined(RSSS)
        num_xor_queries = SSS_PRIVACY_LEVEL+1;
    #else //if defined SPDZ
        num_xor_queries = NUM_SERVERS;
    #endif
    
    xor_queries= new unsigned char**[num_xor_queries];
    for(int i = 0 ; i < num_xor_queries; i++)
    {
        xor_queries[i]= new unsigned char*[NUM_SHARE_PER_SERVER];
        for(int j = 0 ; j < NUM_SHARE_PER_SERVER; j++)
        {
            xor_queries[i][j] = new unsigned char[PATH_LENGTH/8];
            memset(xor_queries[i][j],0,PATH_LENGTH/8);
        }
	}
    
    this->pos_map = new TYPE_POS_MAP[NUM_BLOCK+1];
    
    this->metaData = new TYPE_ID*[NUM_NODES];
	for (int i = 0 ; i < NUM_NODES; i++)
    {
        this->metaData[i] = new TYPE_ID[BUCKET_SIZE];
        memset(this->metaData[i],0,sizeof(TYPE_ID)*BUCKET_SIZE);
    }
    
    retrievedShare = new zz_p*[NUM_SERVERS];
	retrievedMacShare = new zz_p*[NUM_SERVERS];
    for(int k = 0 ; k < NUM_SERVERS; k++)
    {
        retrievedShare[k] = new zz_p[DATA_CHUNKS];
        retrievedMacShare[k] = new zz_p[DATA_CHUNKS];
    }
    
    
    recoveredBlock = new zz_p[DATA_CHUNKS];
    recoveredMacBlock = new zz_p[DATA_CHUNKS];
    
    retrieval_in = new unsigned char*[NUM_SERVERS];
    
    for(int i = 0; i < NUM_SERVERS ; i++)
    {
        retrieval_in[i] = new unsigned char[SERVER_RETRIEVAL_REPLY_LENGTH];
    }
    
	this->sharedMatrix = new TYPE_DATA**[NUM_SERVERS];
	for (TYPE_INDEX i = 0 ; i < NUM_SERVERS; i++)
	{
		this->sharedMatrix[i] = new TYPE_DATA*[H+1];
		for(TYPE_INDEX j = 0 ; j < H+1; j++)
		{
			this->sharedMatrix[i][j] = new TYPE_DATA[evictMatSize];
		}
	}

	this->evictMatrix = new TYPE_DATA*[H+1];
	for(TYPE_INDEX i = 0 ; i < H+1; i++)
	{
		this->evictMatrix[i] = new TYPE_DATA[evictMatSize];
	}
	
	this->sharedVector = new TYPE_DATA*[NUM_SERVERS];
	for(int i = 0; i < NUM_SERVERS; i++)
	{
		this->sharedVector[i] = new TYPE_DATA[(H+1)*BUCKET_SIZE];
	}
	
    this->retrieval_query_out = new unsigned char*[NUM_SERVERS];
    for (TYPE_INDEX i = 0 ; i < NUM_SERVERS ; i++)
    {
        this->retrieval_query_out[i] = new unsigned char[CLIENT_RETRIEVAL_OUT_LENGTH]; 
    }
    
    
        
    this->evict_out = new unsigned char*[NUM_SERVERS];
    for (TYPE_INDEX i = 0 ; i < NUM_SERVERS ; i++)
    {
        this->evict_out[i] = new unsigned char[CLIENT_EVICTION_OUT_LENGTH]; //(H+1)*evictMatSize*sizeof(TYPE_DATA) + sizeof(TYPE_INDEX)
    }
    
    
    thread_socket_args = new struct_socket[NUM_SERVERS];
    
	#if defined(PRECOMP_MODE) // ================================================================================================
		this->precompOnes = new TYPE_DATA*[NUM_SERVERS];
		for (TYPE_INDEX i = 0 ; i < NUM_SERVERS ; i++){
			this->precompOnes[i] = new TYPE_DATA[PRECOMP_SIZE];
		}
		
		this->precompZeros = new TYPE_DATA*[NUM_SERVERS];
		for (TYPE_INDEX i = 0 ; i < NUM_SERVERS ; i++){
			this->precompZeros[i] = new TYPE_DATA[PRECOMP_SIZE];
		}

		
		auto start = time_now;
		ORAM.precomputeShares(0, precompZeros, PRECOMP_SIZE);
		ORAM.precomputeShares(1, precompOnes, PRECOMP_SIZE);
		auto end = time_now;
		cout<< "	[ClientKaryORAMO] " << 2*PRECOMP_SIZE << " Logical Values Precomputed in" << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
	#endif //defined(PRECOMP_MODE) ================================================================================================
	
    
    
	time_t now = time(0);
	char* dt = ctime(&now);
	FILE* file_out = NULL;
	string path = clientLocalDir + "lastest_config";
	string info = "Height of Tree: " + to_string(HEIGHT) + "\n";
	info += "Number of Blocks: " + to_string(NUM_BLOCK) + "\n";
	info += "Bucket Size: " + to_string(BUCKET_SIZE) + "\n";
	info += "Eviction Rate: " + to_string(EVICT_RATE) + "\n";
	info += "Block Size (B): " + to_string(BLOCK_SIZE) + "\n";
	info += "ID Size (B): " + to_string(sizeof(TYPE_ID)) + "\n";
	info += "Number of Chunks: " + to_string(DATA_CHUNKS) + "\n";
	info += "Total Size of Data (MB): " + to_string((NUM_BLOCK*(BLOCK_SIZE+sizeof(TYPE_ID)))/1048576.0) + "\n";
	info += "Total Size of ORAM (MB): " + to_string(BUCKET_SIZE*NUM_NODES*(BLOCK_SIZE+sizeof(TYPE_ID))/1048576.0) + "\n";
	
	#if defined(PRECOMP_MODE)
		info += "PRECOMPUTATION MODE: Active\n";
	#else
		info += "PRECOMPUTATION MODE: Inactive\n";
	#endif 
	
	if((file_out = fopen(path.c_str(),"w+")) == NULL){
		cout<< "	File Cannot be Opened!!" <<endl;
		exit;
	}
	fputs(dt, file_out);
	fputs(info.c_str(), file_out);
	fclose(file_out);
	
    //socket
    for(int i = 0 ; i < NUM_SERVERS;i ++)
    {
        context[i] = new zmq::context_t(1);
        socket[i] = new zmq::socket_t(*context[i],ZMQ_REQ);
        string send_address = SERVER_ADDR[i]+ ":" + std::to_string(SERVER_PORT+i*NUM_SERVERS+i);
        cout<<"Connecting to "<<send_address<<" for communication with Server "<< i << " ...";
        socket[i]->connect(send_address);
        cout<<"OK!"<<endl;
    }
    
	tm *now_time = localtime(&now);
	if(now != -1)
		strftime(timestamp,16,"%d%m_%H%M",now_time);
        
}

ClientORAM::~ClientORAM()
{
}


int ClientORAM::init()
{
    
    this->numRead = 0;
    this->numEvict = 0;

    auto start = time_now;
    auto end = time_now;

    for ( TYPE_INDEX i = 0 ; i <= NUM_BLOCK; i++ )
    {
        this->pos_map[i].pathID = -1;
        this->pos_map[i].pathIdx = -1;
    }

    start = time_now;
    ORAM::build(this->pos_map,this->metaData);
    
    for(int i = 0 ; i < NUM_SERVERS; i++)
    {
        string copy_cmd = "cp ";
        for (int j = 0 ; j < NUM_SERVERS; j++)
        {
            copy_cmd += " ../data/"+to_string(j);
        }
        copy_cmd += " ../data/S" + to_string(i) +"/ -rf";
        cout<<copy_cmd<<endl;
        system(copy_cmd.c_str());
    }
    
    
    end = time_now;
	cout<<endl;
    cout<< "Elapsed Time for Setup on Disk: "<<std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<<" ns"<<endl;
    cout<<endl;
    std::ofstream output;
    string path2 = clientLocalDir + "lastest_config";
    output.open(path2, std::ios_base::app);
    output<< "INITIALIZATION ON CLIENT: Performed\n";
    output.close();

    //transfer to servers all ORAM structure (send files) //IMPLEMENT LATER
    
    //save state
    saveState();
}





int ClientORAM::saveState()
{
    // 11. store local info to disk
	FILE* local_data = NULL;
	if((local_data = fopen(clientTempPath.c_str(),"wb+")) == NULL){
		cout<< "	[ClientS3CORAM] File Cannot be Opened!!" <<endl;
		exit(0);
	}
	fwrite(this->pos_map, 1, (NUM_BLOCK+1)*sizeof(TYPE_POS_MAP), local_data);
	fwrite(&this->numEvict, sizeof(this->numEvict), 1, local_data);
	fwrite(&this->numRead, sizeof(this->numRead), 1, local_data);

	fclose(local_data);
    
    return 0;
}


/**
 * Function Name: load 
 *
 * Description: Loads client storage data from disk for previously generated ORAM structure 
 * in order to continue ORAM operations. Loaded data includes postion map, current number of evictions,
 * current number of reads/writes.
 * 
 * @return 0 if successful
 */ 
int ClientORAM::loadState()
{
	FILE* local_data = NULL;
	if((local_data = fopen(clientTempPath.c_str(),"rb")) == NULL){
		cout<< "	[load] File Cannot be Opened!!" <<endl;
		exit(0);
	}
	
    fread(this->pos_map, 1, (NUM_BLOCK+1)*sizeof(TYPE_POS_MAP), local_data);
	fread(&this->numEvict, sizeof(this->numEvict), 1, local_data);
	fread(&this->numRead, sizeof(this->numRead), 1, local_data);
    
    //scan position map to load bucket metaData (for speed optimization)
    TYPE_INDEX fullPathIdx[H+1];
    for(TYPE_INDEX i = 1 ; i < NUM_BLOCK+1; i++)
    {
        ORAM::getFullPathIdx(fullPathIdx,pos_map[i].pathID);
        this->metaData[fullPathIdx[pos_map[i].pathIdx/ BUCKET_SIZE]][pos_map[i].pathIdx%BUCKET_SIZE] = i;
    }
    
	std::ofstream output;
	string path = clientLocalDir + "lastest_config";
	output.open(path, std::ios_base::app);
	output<< "SETUP FROM LOCAL DATA\n";
	output.close();
	
    return 0;
}


/**
 * Function Name: sendORAMTree
 *
 * Description: Distributes generated and shared ORAM buckets to servers over network
 * 
 * @return 0 if successful
 */  
int ClientORAM::sendORAMTree()
{
    unsigned char*  oram_buffer_out = new unsigned char [BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS]; 
    memset(oram_buffer_out,0, BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS);
    int CMD = CMD_SEND_ORAM_TREE;       
    unsigned char buffer_in[sizeof(CMD_SUCCESS)];
	unsigned char buffer_out[sizeof(CMD)];

    memcpy(buffer_out, &CMD,sizeof(CMD));
    
    zmq::context_t context(1);
    zmq::socket_t socket(context,ZMQ_REQ);

    struct_socket thread_args[NUM_SERVERS];
    for (int i = 0; i < NUM_SERVERS; i++)
    {
		string ADDR = SERVER_ADDR[i]+ ":" + std::to_string(SERVER_PORT+i*NUM_SERVERS+i); 
		cout<< "	[sendORAMTree] Connecting to " << ADDR <<endl;
        socket.connect( ADDR.c_str());
            
        socket.send(buffer_out, sizeof(CMD));
		cout<< "	[sendORAMTree] Command SENT! " << CMD <<endl;
        socket.recv(buffer_in, sizeof(CMD_SUCCESS));
		
        for(TYPE_INDEX j = 0 ; j < NUM_NODES; j++)
        {
            //load data to buffer
            FILE* fdata = NULL;
            string path = rootPath + to_string(i) + "/" + to_string(j);
            if((fdata = fopen(path.c_str(),"rb")) == NULL)
            {
                cout<< "	[sendORAMTree] File Cannot be Opened!!" <<endl;
                exit(0);
            }
            long lSize;
            fseek (fdata , 0 , SEEK_END);
            lSize = ftell (fdata);
            rewind (fdata);
            if(fread(oram_buffer_out ,1 , BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS, fdata) != sizeof(char)*lSize){
                cout<< "	[sendORAMTree] File loading error be Read!!" <<endl;
                exit(0);
            }
            fclose(fdata);
            //send to server i
            socket.send(oram_buffer_out,BUCKET_SIZE*sizeof(TYPE_DATA)*DATA_CHUNKS,0);
            socket.recv(buffer_in,sizeof(CMD_SUCCESS));
        }
        socket.disconnect(ADDR.c_str());
    }
    socket.close();	
    return 0;
}




/**
 * Function Name: thread_socket_func & send
 *
 * Description: Generic threaded socket function for send and receive operations
 * 
 * @return 0 if successful
 */  
void* ClientORAM::thread_socket_func(void* args)
{
	struct_socket* opt = (struct_socket*) args;
	
	
	sendNrecv(opt->peer_idx, opt->data_out, opt->data_out_size, opt->data_in, opt->data_in_size, opt->CMD);

		
    pthread_exit((void*)opt);
}
int ClientORAM::sendNrecv(int peer_idx, unsigned char* data_out, size_t data_out_size, unsigned char* data_in, size_t data_in_size, int CMD)
{

    unsigned char buffer_in[sizeof(CMD_SUCCESS)];
	unsigned char buffer_out[sizeof(CMD)];
	
    try
    {
        cout<< "	[ThreadSocket] Sending Command to"<< SERVER_ADDR[peer_idx] << endl;
        memcpy(buffer_out, &CMD,sizeof(CMD));
        socket[peer_idx]->send(buffer_out, sizeof(CMD));
		cout<< "	[ThreadSocket] Command SENT! " << CMD <<endl;
        socket[peer_idx]->recv(buffer_in, sizeof(CMD_SUCCESS));
		
		auto start = time_now;
		cout<< "	[ThreadSocket] Sending Data..." << endl;
		socket[peer_idx]->send (data_out, data_out_size);
		cout<< "	[ThreadSocket] Data SENT!" << endl;
        if(data_in_size == 0)
            socket[peer_idx]->recv(buffer_in,sizeof(CMD_SUCCESS));
        else
            socket[peer_idx]->recv(data_in,data_in_size);
            
		auto end = time_now;
		if(thread_max < std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count())
			thread_max = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	}
    catch (exception &ex)
    {
        cout<< "	[ThreadSocket] Socket error!"<<endl;
		exit(0);
    }
	return 0;
}


void ClientORAM::recoverRetrievedBlock()
{
    
#if defined(XOR_PIR)
    unsigned char** xor_answers = new unsigned char*[NUM_SHARE_PER_SERVER];
    for(int i = 0 ; i < NUM_SHARE_PER_SERVER; i++)
    {
        xor_answers[i] = new unsigned char[BLOCK_SIZE*2];
    }
    for(int i = 0 ; i < SSS_PRIVACY_LEVEL + 1; i++ )
    {
        memcpy(xor_answers[0],&retrieval_in[i][0],BLOCK_SIZE*2);
        memcpy(xor_answers[1],&retrieval_in[(SSS_PRIVACY_LEVEL+i)%(SSS_PRIVACY_LEVEL+1)][BLOCK_SIZE*2],BLOCK_SIZE*2);
            
        memset(retrievedShare[i],0,BLOCK_SIZE);
        memset(retrievedMacShare[i],0,BLOCK_SIZE);
        
        ORAM::xor_reconstruct(xor_answers,retrievedShare[i],retrievedMacShare[i]);
    }    
    memset(recoveredBlock,0,BLOCK_SIZE);    
    ORAM::recoverSecret(retrievedShare,retrievedMacShare,recoveredBlock,recoveredMacBlock);
#else // if defined RSSS or SPDZ

    ORAM::recoverSecret(retrieval_in,recoveredBlock,recoveredMacBlock);
#endif

    
#if defined (RSSS)
    //check if recovered Block * MAC KEY != recovered Mac
    for(int i = 0 ; i < DATA_CHUNKS; i++)
    {
        if(GLOBAL_MAC_KEY*recoveredBlock[i] != recoveredMacBlock[i])
        {
            cout<<"BLOCK WAS TAMPERED!!!";
            cout<<"recovered MAC: "<<recoveredMacBlock[i]<<endl;
            cout<<"correct MAC: " << GLOBAL_MAC_KEY*recoveredBlock[i]  <<endl;
            cin.get();
            //exit(0);
        }
    }
#else
    //SPDZ uses something different, check this !?
#endif

}

int ClientORAM::retrieve(TYPE_ID blockID)
{
    auto start = time_now;
    auto end = time_now;
    
    // 1. get the path & index of the block of interest
    TYPE_INDEX pathID = pos_map[blockID].pathID;
	cout << "	[ClientORAM] PathID = " << pathID <<endl;
    cout << "	[ClientORAM] Location = " << 	pos_map[blockID].pathIdx <<endl;
    
	// 3. create shares of retrieval query 
	start = time_now;
    
    this->createRetrievalQuery(pos_map[blockID].pathIdx,pathID);
    
	end = time_now;
	cout<< "	[ClientORAM] Retrieval Query Created in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
	exp_logs[1] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	
	// 4. send to server & receive the answer
    
    start = time_now;
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        thread_socket_args[i] = struct_socket(i, retrieval_query_out[i], CLIENT_RETRIEVAL_OUT_LENGTH, retrieval_in[i], SERVER_RETRIEVAL_REPLY_LENGTH, CMD_RETRIEVE,NULL);
		pthread_create(&thread_sockets[i], NULL, &ClientORAM::thread_socket_func, (void*)&thread_socket_args[i]);
    }
      
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        pthread_join(thread_sockets[i], NULL);
    }
    end = time_now;
    cout<< "	[ClientORAM] All Shares Retrieved in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
    exp_logs[2] = thread_max;
    thread_max = 0;
	
    // 5. recover the block
	start = time_now;
    this->recoverRetrievedBlock();
    end = time_now;
	cout<< "	[ClientORAM] Recovery Done in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()<< " ns"<<endl;
	exp_logs[3] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	
    cout << "	[ClientORAM] Block-" << recoveredBlock[0] <<" is Retrieved" <<endl;
    
}

int ClientORAM::updatePosMap(TYPE_ID blockID)
{
    TYPE_INDEX fullPathIdx[H+1];
    ORAM::getFullPathIdx(fullPathIdx,pos_map[blockID].pathID);
    
    if(pos_map[blockID].pathIdx != -1)
    {
        this->metaData[fullPathIdx[pos_map[blockID].pathIdx / BUCKET_SIZE ]][pos_map[blockID].pathIdx % BUCKET_SIZE] = 0;   
    }
    // 6.1. assign to random path
    pos_map[blockID].pathID = Utils::RandBound(N_leaf)+(N_leaf-1);
}


int ClientORAM::createRetrievalQuery(int pIdx, TYPE_ID pID)
{
        #if defined (XOR_PIR)
            unsigned char*** xor_queries;
            xor_queries= new unsigned char**[(SSS_PRIVACY_LEVEL+1)];
            for(int i = 0 ; i < SSS_PRIVACY_LEVEL+1; i++)
            {
                xor_queries[i]= new unsigned char*[NUM_SHARE_PER_SERVER];
                for(int j = 0 ; j < NUM_SHARE_PER_SERVER; j++)
                {
                    xor_queries[i][j] = new unsigned char[CLIENT_RETRIEVAL_QUERY_SIZE];
                    memset(xor_queries[i][j],0,CLIENT_RETRIEVAL_QUERY_SIZE);
                }
            }
            for(int i = 0 ; i < SSS_PRIVACY_LEVEL+1; i++)
            {
                ORAM::xor_createQuery(pIdx,PATH_LENGTH,xor_queries[i]);
            }

            for (int i = 0; i < SSS_PRIVACY_LEVEL+1; i++)
            {
                memcpy(&retrieval_query_out[i][0], xor_queries[i][0], CLIENT_RETRIEVAL_QUERY_SIZE);
                memcpy(&retrieval_query_out[i][CLIENT_RETRIEVAL_QUERY_SIZE], xor_queries[(i+1)%(SSS_PRIVACY_LEVEL+1)][1],CLIENT_RETRIEVAL_QUERY_SIZE);
                memcpy(&retrieval_query_out[i][CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_DATA)], &pID, sizeof(pID));
            }
        #else //if defined SPDZ or RSSS without XOR-PIR
            unsigned int num_queries;             
            #if defined(RSSS)
                num_queries = SSS_PRIVACY_LEVEL+1;
            #else //if defined SPDZ
                num_queries = NUM_SERVERS;
            #endif
            ORAM::sss_createQuery(pIdx,PATH_LENGTH,retrieval_query_out);
            for (int i = 0; i < num_queries; i++)
            {
                memcpy(&retrieval_query_out[i][CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_ID)], &pID, sizeof(pID));
            }
            
        #endif
    return 0;
}
    