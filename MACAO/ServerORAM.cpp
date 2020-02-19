#include "ServerORAM.hpp"
#include "Utils.hpp"
#include "struct_socket.h"

#include "ORAM.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>


zmq::context_t** ServerORAM::context_send = new zmq::context_t*[NUM_SERVERS-1];
zmq::socket_t**  ServerORAM::socket_send = new zmq::socket_t*[NUM_SERVERS-1];


zmq::context_t** ServerORAM::context_recv = new zmq::context_t*[NUM_SERVERS-1];
zmq::socket_t** ServerORAM::socket_recv = new zmq::socket_t*[NUM_SERVERS-1];



unsigned long int ServerORAM::server_logs[13];
unsigned long int ServerORAM::thread_max = 0;
char ServerORAM::timestamp[16];



ServerORAM::ServerORAM()
{
}

ServerORAM::~ServerORAM()
{
}



ServerORAM::ServerORAM(TYPE_INDEX serverNo, int selectedThreads)
{
	this->numThreads = selectedThreads;
    this->serverNo = serverNo;
    #if defined(SEEDING)

        this->prng_client = new prng_state[NUM_SERVERS];
        this->prng_server = new prng_state*[NUM_SERVERS];
        for(int i = 0 ; i < NUM_SERVERS;i++)
        {
            prng_client[i] = prng_state();

            sober128_start(&this->prng_client[i]);

            sober128_add_entropy((const unsigned char*)CLIENT_SERVER_SEED[i].c_str(),CLIENT_SERVER_SEED[i].size(),&this->prng_client[i]);

            sober128_ready(&this->prng_client[i]);

            this->prng_server[i] = new prng_state[NUM_SERVERS];

            for(int j = 0 ; j < NUM_SERVERS;j++)
            {
                this->prng_server[i][j] = prng_state();

                sober128_start(&this->prng_server[i][j]);

                sober128_add_entropy((const unsigned char*)SERVER_SERVER_SEED[i][j].c_str(),SERVER_SERVER_SEED[i][j].size(),&this->prng_server[i][j]);

                sober128_ready(&this->prng_server[i][j]);
            }
        }
    #endif
    //this is for RSSS with 3 servers
    this->client_evict_in = new unsigned char[CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX)];




    random_shares_out = new unsigned char[2*sizeof(TYPE_DATA)];
    random_shares_in = new unsigned char*[NUM_SERVERS-1];
    for(int i = 0; i < NUM_SERVERS - 1; i++)
    {
        random_shares_in[i] = new unsigned char[2*sizeof(TYPE_DATA)];
    }
    retrieval_answer_out = new unsigned char[SERVER_RETRIEVAL_REPLY_LENGTH];

    retrieval_query_in = new unsigned char[CLIENT_RETRIEVAL_OUT_LENGTH];
    lin_rand_com_out = new unsigned char[4*sizeof(TYPE_DATA)];


#if defined(XOR_PIR) && defined(RSSS)
    this->retrieval_query = new unsigned char*[NUM_SHARE_PER_SERVER];
    this->retrieval_path_db = new zz_p**[NUM_SHARE_PER_SERVER];
    this->retrieval_path_mac = new zz_p**[NUM_SHARE_PER_SERVER];

    

    this->retrieval_answer_block = new unsigned char**[NUM_SHARE_PER_SERVER];
    this->retrieval_answer_mac  = new unsigned char**[NUM_SHARE_PER_SERVER];

    for(int i = 0 ; i < NUM_SHARE_PER_SERVER;i++)
    {
        retrieval_query[i] = new unsigned char[CLIENT_RETRIEVAL_QUERY_SIZE];
        this->retrieval_path_db[i] = new zz_p*[PATH_LENGTH];
        this->retrieval_path_mac[i] = new zz_p*[PATH_LENGTH];

        for (TYPE_INDEX j = 0 ; j < PATH_LENGTH; j++)
        {
            this->retrieval_path_db[i][j] = new zz_p[DATA_CHUNKS];
            this->retrieval_path_mac[i][j] = new zz_p[DATA_CHUNKS];
        }

        this->retrieval_answer_block[i] = new unsigned char*[numThreads];
        this->retrieval_answer_mac[i]  = new unsigned char*[numThreads];

        for(int n = 0 ; n < numThreads; n++)
        {
            this->retrieval_answer_block[i][n] = new unsigned char[BLOCK_SIZE];
            this->retrieval_answer_mac[i][n]  = new unsigned char[BLOCK_SIZE];

        }


    }
#else //#if defined(RSSS) or defined(SPDZ)

    #if defined(SPDZ)
        for(int s = 0 ; s < NUM_SERVERS-1; s++)
        {
            retrieval_reshares_in[s] = new unsigned char[PATH_LENGTH*BLOCK_SIZE+PATH_LENGTH*sizeof(TYPE_DATA)];
        }

        this->RetrievalShares_a = new zz_p*[DATA_CHUNKS];
        this->RetrievalShares_b = new zz_p[PATH_LENGTH];
        this->RetrievalShares_c = new zz_p[DATA_CHUNKS];

        this->RetrievalShares_a_mac = new zz_p*[DATA_CHUNKS];
        this->RetrievalShares_b_mac = new zz_p[PATH_LENGTH];
        this->RetrievalShares_c_mac = new zz_p[DATA_CHUNKS];

        for(int i = 0 ; i < DATA_CHUNKS; i++)
        {
            RetrievalShares_a[i] = new zz_p[PATH_LENGTH];
            RetrievalShares_a_mac[i] = new zz_p[PATH_LENGTH];
        }

    #elif defined(RSSS)
        for(int s = 0 ; s < NUM_SERVERS-1; s++)
        {
            retrieval_reshares_in[s] = new unsigned char[2*sizeof(TYPE_DATA)];
        }
    #endif

    this->retrieval_query = new unsigned char*[NUM_SHARE_PER_SERVER];
    this->retrieval_path_db = new zz_p**[NUM_SHARE_PER_SERVER];
    this->retrieval_path_mac = new zz_p**[NUM_SHARE_PER_SERVER];
    this->retrieval_answer_block = new unsigned char*[NUM_SHARE_PER_SERVER];
    this->retrieval_answer_mac = new unsigned char*[NUM_SHARE_PER_SERVER];


    for(int i = 0 ; i < NUM_SHARE_PER_SERVER;i++)
    {
        retrieval_query[i] = new unsigned char[CLIENT_RETRIEVAL_QUERY_SIZE];

        this->retrieval_path_db[i] = new zz_p*[DATA_CHUNKS];
        this->retrieval_path_mac[i] = new zz_p*[DATA_CHUNKS];
        retrieval_answer_block[i] = new unsigned char[BLOCK_SIZE];
        retrieval_answer_mac[i] = new unsigned char[BLOCK_SIZE];

        for (TYPE_INDEX j = 0 ; j < DATA_CHUNKS; j++)
        {
            this->retrieval_path_db[i][j] = new zz_p[PATH_LENGTH];
            this->retrieval_path_mac[i][j] = new zz_p[PATH_LENGTH];
        }
    }

#endif

    //eviction variables

    for(int i = 0 ; i < NUM_CONCURR_EVICT*NUM_MULT; i++) //2* 3
    {
        zz_p** localMatProduct_output = new zz_p*[DATA_CHUNKS];
        zz_p** localMatProduct_output_MAC = new zz_p*[DATA_CHUNKS];

        for (TYPE_INDEX y = 0 ; y < DATA_CHUNKS ; y++)
        {
            localMatProduct_output[y] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
            localMatProduct_output_MAC[y] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
        }
        this->vecLocalMatProduct_output.push_back(localMatProduct_output);
        this->vecLocalMatProduct_output_MAC.push_back(localMatProduct_output_MAC);


        pthread_t* thread_compute = new pthread_t[numThreads];
        pthread_t* thread_compute_MAC = new pthread_t[numThreads];

        this->vecThread_compute.push_back(thread_compute);
        this->vecThread_compute_MAC.push_back(thread_compute_MAC);

        THREAD_COMPUTATION*	thread_comp_arg = new THREAD_COMPUTATION[numThreads];
        THREAD_COMPUTATION*	thread_comp_MAC_arg = new THREAD_COMPUTATION[numThreads];
        this->vecComp_args.push_back(thread_comp_arg);
        this->vecComp_MAC_args.push_back(thread_comp_MAC_arg);

    }
    for(int i = 0 ; i < NUM_CONCURR_EVICT*NUM_SHARE_PER_SERVER; i++)
    {
        zz_p*** evictMatrix = new zz_p**[H+1];
        zz_p*** evictMatrix_MAC = new zz_p**[H+1];

        #if defined(SPDZ)
            zz_p*** shares_b = new zz_p**[H+1];
            zz_p*** shares_b_MAC = new zz_p**[H+1];
        #endif

        for(TYPE_INDEX y = 0 ; y < H+1; y++)
        {
            evictMatrix[y] = new zz_p*[EVICT_MAT_NUM_ROW];
            evictMatrix_MAC[y] = new zz_p*[EVICT_MAT_NUM_ROW];
            #if defined(SPDZ)
                shares_b[y] = new zz_p*[EVICT_MAT_NUM_ROW];
                shares_b_MAC[y] = new zz_p*[EVICT_MAT_NUM_ROW];
            #endif
            for(TYPE_INDEX i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
            {
                evictMatrix[y][i] = new zz_p[EVICT_MAT_NUM_COL];
                evictMatrix_MAC[y][i] = new zz_p[EVICT_MAT_NUM_COL];
                #if defined(SPDZ)
                    shares_b[y][i] = new zz_p[EVICT_MAT_NUM_COL];
                    shares_b_MAC[y][i] = new zz_p[EVICT_MAT_NUM_COL];
                #endif
            }
        }
        this->vecEvictMatrix.push_back(evictMatrix);
        this->vecEvictMatrix_MAC.push_back(evictMatrix_MAC);
        #if defined(SPDZ)
            this->vecShares_b.push_back(shares_b);

            this->vecShares_b_MAC.push_back(shares_b_MAC);

            zz_p **shares_a = new zz_p*[DATA_CHUNKS];
            zz_p **shares_c = new zz_p*[DATA_CHUNKS];

            zz_p **shares_a_MAC = new zz_p*[DATA_CHUNKS];
            zz_p **shares_c_MAC = new zz_p*[DATA_CHUNKS];
        #endif
        zz_p **evictPath_db = new zz_p*[DATA_CHUNKS];
        zz_p **evictPath_MAC = new zz_p*[DATA_CHUNKS];

        for (TYPE_INDEX k = 0 ; k < DATA_CHUNKS; k++)
        {
            evictPath_db[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
            evictPath_MAC[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
            #if defined(SPDZ)
                shares_a[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
                shares_c[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];

                shares_a_MAC[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
                shares_c_MAC[k]  = new zz_p[MAT_PRODUCT_INPUT_DB_LENGTH];
            #endif
        }
        this->vecEvictPath_db.push_back(evictPath_db);
        this->vecEvictPath_MAC.push_back(evictPath_MAC);
        #if defined(SPDZ)
            this->vecShares_a.push_back(shares_a);
            this->vecShares_c.push_back(shares_c);

            this->vecShares_a_MAC.push_back(shares_a_MAC);
            this->vecShares_c_MAC.push_back(shares_c_MAC);
        #endif
    }
    for(int i = 0 ; i < NUM_CONCURR_EVICT ; i++)
    {
        zz_p*** reShares = new zz_p**[NUM_SERVERS];
        zz_p*** reShares_MAC = new zz_p**[NUM_SERVERS];

        for(TYPE_INDEX i = 0 ; i < NUM_SERVERS ;  i++)
        {
            reShares[i] = new zz_p*[DATA_CHUNKS];
            reShares_MAC[i] = new zz_p*[DATA_CHUNKS];
            for(TYPE_INDEX ii = 0 ; ii < DATA_CHUNKS ;  ii++)
            {
                reShares[i][ii] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
                reShares_MAC[i][ii] = new zz_p[MAT_PRODUCT_OUTPUT_LENGTH];
            }
        }

        this->vecReShares.push_back(reShares);
        this->vecReShares_MAC.push_back(reShares_MAC);
    }






    myStoragePath = rootPath + "S" + to_string(serverNo) + "/" ;

	this->CLIENT_ADDR = "tcp://*:" + std::to_string(SERVER_PORT+(serverNo)*NUM_SERVERS+serverNo);



	cout<<endl;
	cout << "=================================================================" << endl;
	cout<< "Starting Server-" << serverNo+1 <<endl;
	cout << "=================================================================" << endl;
//	#if !defined(SEEDING)
//        this->serverNo = serverNo;
//    #endif

	TYPE_INDEX m = 0;
    while (m< NUM_SERVERS-1)
    {
        this->others[m] = (serverNo + m +1 ) % NUM_SERVERS;
        m++;
    }



	this->evict_in = new unsigned char[CLIENT_EVICTION_OUT_LENGTH];


    bucket_data = new unsigned char[BUCKET_DATA_SIZE];




	this->reshares_in = new unsigned char*[NUM_SERVERS-1];
	for (TYPE_INDEX k = 0 ; k < NUM_SERVERS-1; k++)
	{
		this->reshares_in[k] = new unsigned char[SERVER_RESHARE_IN_OUT_LENGTH]; // first 2: MAC, second: RSSS, third: CORAM used for both CORAM & Onion-ORAM
	}

	this->reshares_out = new unsigned char*[NUM_SERVERS-1];
	for (TYPE_INDEX k = 0 ; k < NUM_SERVERS-1; k++)
	{
		this->reshares_out[k] = new unsigned char[SERVER_RESHARE_IN_OUT_LENGTH];
    }




	time_t rawtime = time(0);
	tm *now = localtime(&rawtime);

	if(rawtime != -1)
		strftime(timestamp,16,"%d%m_%H%M",now);


     //socket
    for(int i = 0 ; i < NUM_SERVERS-1;i ++)
    {
        context_send[i] = new zmq::context_t(1);
        socket_send[i] = new zmq::socket_t(*context_send[i],ZMQ_REQ);
        string send_address = SERVER_ADDR[this->others[i]] + ":" + std::to_string(SERVER_PORT+this->others[i]*NUM_SERVERS+this->serverNo);
        cout<<"Opening "<<send_address<<" for sending to " << "S"<<this->others[i]+1<<"...";
        socket_send[i]->connect(send_address);
        cout<<"OK!"<<endl;

        context_recv[i] = new zmq::context_t(2);
        socket_recv[i] = new zmq::socket_t(*context_recv[i],ZMQ_REP);
        string recv_address = "tcp://*:" + std::to_string(SERVER_PORT+(serverNo)*(NUM_SERVERS)+this->others[i]);
        cout<<"Opening "<<recv_address<<" for listening from " << "S"<<this->others[i]+1<<"...";
        socket_recv[i]->bind(recv_address);
        cout<<"OK!"<<endl;

    }

}





/**
 * Function Name: start
 *
 * Description: Starts the server to wait for a command from the client.
 * According to the command, server performs certain subroutines for distributed ORAM operations.
 *
 * @return 0 if successful
 */
int ServerORAM::start()
{
	int ret = 1;
	int CMD;
    unsigned char buffer[sizeof(CMD)];
    zmq::context_t context(1);
    zmq::socket_t socket(context,ZMQ_REP);

	cout<< "[Server] Socket is OPEN on " << this->CLIENT_ADDR << endl;
    socket.bind(this->CLIENT_ADDR.c_str());

	while (true)
	{
		cout<< "[Server] Waiting for a Command..." <<endl;
        socket.recv(buffer,sizeof(CMD));

        memcpy(&CMD, buffer, sizeof(CMD));
		cout<< "[Server] Command RECEIVED!" <<endl;

        socket.send((unsigned char*)CMD_SUCCESS,sizeof(CMD_SUCCESS));

        switch(CMD)
        {
			case CMD_SEND_ORAM_TREE: //inherent
				cout<<endl;
				cout << "=================================================================" << endl;
				cout<< "[Server] Receiving ORAM Data..." <<endl;
				cout << "=================================================================" << endl;
				this->recvORAMTree(socket);
				cout << "=================================================================" << endl;
				cout<< "[Server] ORAM Data RECEIVED!" <<endl;
				cout << "=================================================================" << endl;
				cout<<endl;
				break;
			case CMD_RETRIEVE: //inherent
				cout<<endl;
				cout << "=================================================================" << endl;
				cout<< "[Server] Receiving Logical Vector..." <<endl;
				cout << "=================================================================" << endl;
				this->retrieve(socket);
				cout << "=================================================================" << endl;
				cout<< "[Server] Block Share SENT" <<endl;
				cout << "=================================================================" << endl;
				cout<<endl;
				break;
            case CMD_WRITE_ROOT: //specific
				cout<<endl;
            	cout << "=================================================================" << endl;
				cout<< "[Server] Receiving Block Data..." <<endl;
				cout << "=================================================================" << endl;
				this->writeRoot(socket);
				cout << "=================================================================" << endl;
				cout<< "[Server] Block Data RECEIVED!" <<endl;
				cout << "=================================================================" << endl;
				cout<<endl;
				break;
			case CMD_EVICT: //specific
				cout<<endl;
				cout << "=================================================================" << endl;
				cout<< "Receiving Eviction Matrix..." <<endl;
				cout << "=================================================================" << endl;
				this->evict(socket);
				cout << "=================================================================" << endl;
				cout<< "[Server] EVICTION and DEGREE REDUCTION DONE!" <<endl;
				cout << "=================================================================" << endl;
				cout<<endl;
				break;
			default:
				break;
		}
	}

	ret = 0;
    return ret;
}


int ServerORAM::retrieve(zmq::socket_t& socket)
{
    TYPE_INDEX fullPathIdx[H+1];
    TYPE_INDEX pathID;
    auto start = time_now;
    auto end = time_now;
    unsigned long long tmp;
    int ret = 1;
    int step, startIdx, endIdx;

    #if defined(XOR_PIR)
        step = ceil((double)PATH_LENGTH/(double)numThreads);
    #else
        zz_p** retrieval_query = new zz_p*[NUM_SHARE_PER_SERVER];
        zz_p** retrieval_query_mac = new zz_p*[NUM_SHARE_PER_SERVER];
        for(int i = 0 ; i < NUM_SHARE_PER_SERVER; i++)
        {
            retrieval_query[i] = new zz_p[CLIENT_RETRIEVAL_QUERY_SIZE/sizeof(TYPE_DATA)];
            retrieval_query_mac[i] = new zz_p[CLIENT_RETRIEVAL_QUERY_SIZE/sizeof(TYPE_DATA)];
        }
        zz_p** dotProd_output = new zz_p*[NUM_SHARE_PER_SERVER];
        zz_p** dotProd_mac_output = new zz_p*[NUM_SHARE_PER_SERVER];

        for(int i = 0 ; i < NUM_MULT; i++)
        {
            dotProd_output[i] = new zz_p[DATA_CHUNKS];
            dotProd_mac_output[i] = new zz_p[DATA_CHUNKS];
        }
        step = ceil((double)DATA_CHUNKS/(double)numThreads);
        
        #if defined(RSSS)
            unsigned char* transfer_in = new unsigned char[CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_DATA)];
        #endif
    #endif

    
        start = time_now;
    #if defined(SEEDING)
        #if defined(XOR_PIR) //seeding with XOR-PIR does not save much, so not implemented (use the same code as in non-seeding case
            socket.recv(retrieval_query_in,CLIENT_RETRIEVAL_OUT_LENGTH,0);
            memcpy(&pathID, &retrieval_query_in[CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_ID)], sizeof(pathID));
            for(int i = 0 ; i < 2; i++)
            {
                memcpy(retrieval_query[i],&retrieval_query_in[i*CLIENT_RETRIEVAL_QUERY_SIZE], CLIENT_RETRIEVAL_QUERY_SIZE);
            }
        #else
            if(serverNo==0)
            {
                socket.recv(retrieval_query_in,CLIENT_RETRIEVAL_OUT_LENGTH,0);
                memcpy(&pathID, &retrieval_query_in[CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_DATA)], sizeof(pathID));
            }
            else
            {
                socket.recv(retrieval_query_in,sizeof(TYPE_DATA),0);
                memcpy(&pathID, &retrieval_query_in[0], sizeof(pathID));
            }
            
            if(serverNo==0)
            {
                memcpy(retrieval_query[0], retrieval_query_in, CLIENT_RETRIEVAL_QUERY_SIZE);
                
                    
                #if defined(RSSS)
                    cout<< "	[evict] Creating Threads for Sending..."<< endl;;
                    sendSocket_args[0] = struct_socket(1,  &retrieval_query_in[0], CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_INDEX), NULL, 0, NULL, true);
                    pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);
                    pthread_join(thread_send[0], NULL);
                #else // SPDZ
                    memcpy(retrieval_query_mac[0], &retrieval_query_in[CLIENT_RETRIEVAL_QUERY_SIZE], CLIENT_RETRIEVAL_QUERY_SIZE);
                #endif
            }
            else
            {
                for(int i = 0 ; i < PATH_LENGTH;i++)
                {
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[this->serverNo]);
                    retrieval_query[0][i] = tmp;
                    #if defined(SPDZ)
                        sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[this->serverNo]);
                        retrieval_query_mac[0][i] = tmp;
                    #endif
                }
            }
            #if defined(RSSS)
                if(serverNo==2)
                {
                    cout<< "	[evict] Creating Threads for Receiving..." << endl;
                    recvSocket_args[0] = struct_socket(0, NULL, 0, transfer_in, CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_INDEX), NULL,false);
                    pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);
                    pthread_join(thread_recv[0], NULL);
                    
                    memcpy(retrieval_query[1], transfer_in, CLIENT_RETRIEVAL_QUERY_SIZE);
                }
                else
                {
                    for(int i = 0 ; i < PATH_LENGTH;i++)
                    {
                        sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(this->serverNo+1)%3]);
                        retrieval_query[1][i] = tmp;
                    }
                }
            #endif
        #endif
    #else // NO SEEDING

        start = time_now;
        socket.recv(retrieval_query_in,CLIENT_RETRIEVAL_OUT_LENGTH,0);
        end = time_now;
        cout<< "	[SendBlock] PathID and Logical Vector RECEIVED in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() << " ns" <<endl;
        server_logs[0] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

        memcpy(&pathID, &retrieval_query_in[CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_ID)], sizeof(pathID));
        cout<< "	[SendBlock] PathID is " << pathID <<endl;

        #if defined(XOR_PIR) //XOR-PIR does not implement seeding/non-seeding
            for(int i = 0 ; i < 2; i++)
            {
                memcpy(retrieval_query[i],&retrieval_query_in[i*CLIENT_RETRIEVAL_QUERY_SIZE], CLIENT_RETRIEVAL_QUERY_SIZE);
            }
        #else
            memcpy(retrieval_query[0], retrieval_query_in, CLIENT_RETRIEVAL_QUERY_SIZE);
            #if defined(SPDZ)
                memcpy(retrieval_query_mac[0], &retrieval_query_in[CLIENT_RETRIEVAL_QUERY_SIZE], CLIENT_RETRIEVAL_QUERY_SIZE);
            #endif
            #if defined(RSSS)
                // send client data to other servers (this is due to RSSS)
                cout<< "	[evict] Creating Threads for Receiving..." << endl;
                recvSocket_args[0] = struct_socket(0, NULL, 0, transfer_in, CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_INDEX), NULL,false);
                pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);

                cout<< "	[evict] Creating Threads for Sending..."<< endl;;
                sendSocket_args[0] = struct_socket(1,  retrieval_query_in, CLIENT_RETRIEVAL_OUT_LENGTH-sizeof(TYPE_INDEX), NULL, 0, NULL, true);
                pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);

                cout<< "	[evict] CREATED!" <<endl;
                cout<< "	[evict] Waiting for Threads..." <<endl;

                pthread_join(thread_send[0], NULL);
                pthread_join(thread_recv[0], NULL);

                memcpy(retrieval_query[1], transfer_in, CLIENT_RETRIEVAL_QUERY_SIZE);
            #endif
        #endif
    #endif

    
    
    
    end = time_now;
    cout<< "	[SendBlock] PathID and Logical Vector RECEIVED in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() << " ns" <<endl;
    server_logs[0] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    cout<< "	[SendBlock] PathID is " << pathID <<endl;
    ORAM::getFullPathIdx(fullPathIdx, pathID);

    //read data
    for(int j = 0; j < H+1; j++)
    {
        this->readBucket(fullPathIdx[j], j, serverNo,retrieval_path_db[0],retrieval_path_mac[0]);
        #if defined(RSSS)
            this->readBucket(fullPathIdx[j], j, (serverNo+1)%(3),retrieval_path_db[1],retrieval_path_mac[1]);
        #endif
    }
    //doing computation
    #if defined (XOR_PIR)
        for(int i = 0 ; i < NUM_XOR_QUERY_PER_SERVER;i++)
        {
            memcpy(retrieval_query[i],&retrieval_query_in[i*CLIENT_RETRIEVAL_QUERY_SIZE], CLIENT_RETRIEVAL_QUERY_SIZE);
            for(int j = 0, startIdx = 0 ; j < numThreads; j ++, startIdx+=step)
            {
                if(startIdx+step > PATH_LENGTH)
                    endIdx = PATH_LENGTH;
                else
                    endIdx = startIdx+step;
                vecComp_args[i][j] = THREAD_COMPUTATION(startIdx, endIdx,
                                                            this->retrieval_path_db[i],
                                                            this->retrieval_path_mac[i],
                                                            retrieval_query[i],
                                                            this->retrieval_answer_block[i][j],
                                                            this->retrieval_answer_mac[i][j]);
                pthread_create(&vecThread_compute[i][j], NULL, &ServerORAM::thread_retrieval_by_XOR_func, (void*)&vecComp_args[i][j]);

                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(i, &cpuset);
                pthread_setaffinity_np(vecThread_compute[i][j], sizeof(cpu_set_t), &cpuset);
            }

        }
        for(int i = 0 ; i < NUM_XOR_QUERY_PER_SERVER;i++)
        {
            for(int j = 0 ; j < numThreads; j ++)
            {
                pthread_join(vecThread_compute[i][j],NULL);
            }
        }
        memset(retrieval_answer_out,0,SERVER_RETRIEVAL_REPLY_LENGTH);

        for(int i = 0; i < NUM_XOR_QUERY_PER_SERVER; i++)
        {
            for(int j = 0 ; j < numThreads; j ++)
            {
                for(int c = 0 ; c < BLOCK_SIZE; c+=sizeof(long long))
                {
                    *((unsigned long long*)&retrieval_answer_out[i*BLOCK_SIZE*2+c]) ^= *((unsigned long long*)&this->retrieval_answer_block[i][j][c]);
                    *((unsigned long long*)&retrieval_answer_out[i*BLOCK_SIZE*2+BLOCK_SIZE+c]) ^= *((unsigned long long*)&this->retrieval_answer_mac[i][j][c]);
                }
            }
        }

    #else
        #if defined(RSSS) //RSSS computation
            for(int j = 0 ; j < NUM_MULT; j++)
            {
                for(int i = 0, startIdx = 0 ; i < numThreads; i ++, startIdx+=step)
                {
                    if(startIdx+step > DATA_CHUNKS)
                        endIdx = DATA_CHUNKS;
                    else
                        endIdx = startIdx+step;

                    vecComp_args[j][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_db[RSSS_MULT_ORDER[j][0]], PATH_LENGTH, 1, retrieval_query[RSSS_MULT_ORDER[j][1]], dotProd_output[j]);
                    pthread_create(&vecThread_compute[j][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_args[j][i]);
                    vecComp_MAC_args[j][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_mac[RSSS_MULT_ORDER[j][0]], PATH_LENGTH, 1, retrieval_query[RSSS_MULT_ORDER[j][1]], dotProd_mac_output[j]);
                    pthread_create(&vecThread_compute_MAC[j][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_MAC_args[j][i]);

                    cpu_set_t cpuset;
                    CPU_ZERO(&cpuset);
                    CPU_SET(i, &cpuset);
                    pthread_setaffinity_np(vecThread_compute[j][i], sizeof(cpu_set_t), &cpuset);
                    pthread_setaffinity_np(vecThread_compute_MAC[j][i], sizeof(cpu_set_t), &cpuset);

                }
            }
            for(int j = 0 ; j < NUM_MULT; j++)
            {
                for(int i  = 0 ; i <numThreads ; i++)
                {
                    pthread_join(vecThread_compute[j][i],NULL);

                    pthread_join(vecThread_compute_MAC[j][i],NULL);
                }
            }
            //sum all together
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 1 ; j < NUM_MULT; j++)
                {
                    dotProd_output[0][i] += dotProd_output[j][i];
                    dotProd_mac_output[0][i] += dotProd_mac_output[j][i];
                }
            }
            memcpy(&retrieval_answer_out[0],dotProd_output[0],BLOCK_SIZE);
            memcpy(&retrieval_answer_out[BLOCK_SIZE],dotProd_mac_output[0],BLOCK_SIZE);
            
        #else //SPDZ
            readTriplets(RetrievalShares_a, DATA_CHUNKS, PATH_LENGTH, "/retrieval_triplet_a");
            readTriplets(RetrievalShares_b, PATH_LENGTH, "/retrieval_triplet_b");
            readTriplets(RetrievalShares_c, DATA_CHUNKS, "/retrieval_triplet_c");

            readTriplets(RetrievalShares_a_mac, DATA_CHUNKS, PATH_LENGTH, "/retrieval_triplet_a_mac");
            readTriplets(RetrievalShares_b_mac, PATH_LENGTH, "/retrieval_triplet_b_mac");
            readTriplets(RetrievalShares_c_mac, DATA_CHUNKS, "/retrieval_triplet_c_mac");
            
            unsigned long long currBufferIdx =  0;
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 0 ; j < PATH_LENGTH; j++)
                {
                    this->retrieval_path_db[0][i][j] -= this->RetrievalShares_a[i][j];
                    this->retrieval_path_mac[0][i][j] -= this->RetrievalShares_a_mac[i][j];
                }
                memcpy(&retrieval_reshares_out[currBufferIdx], this->retrieval_path_db[0][i], sizeof(TYPE_DATA)*(PATH_LENGTH));
                currBufferIdx+=sizeof(TYPE_DATA)*(PATH_LENGTH);

            }
            for(int i = 0 ; i < PATH_LENGTH; i++)
            {
                retrieval_query[0][i] -= this->RetrievalShares_b[i];
                retrieval_query_mac[0][i] -= this->RetrievalShares_b_mac[i];

                memcpy(&retrieval_reshares_out[currBufferIdx], &retrieval_query[0][i], sizeof(TYPE_DATA));
                currBufferIdx+=sizeof(TYPE_DATA);
            }

            for(int s = 0 ; s < NUM_SERVERS-1;s++)
            {
                cout<< "	[evict] Creating Threads for Receiving Ports..." << endl;
                recvSocket_args[s] = struct_socket(s, NULL, 0, retrieval_reshares_in[s], PATH_LENGTH*BLOCK_SIZE+PATH_LENGTH*sizeof(TYPE_DATA), NULL,false);
                pthread_create(&thread_recv[s], NULL, &ServerORAM::thread_socket_func, (void*)&recvSocket_args[s]);
                
                cout<< "	[evict] Creating Threads for Sending Shares..."<< endl;;
                sendSocket_args[s] = struct_socket(s, retrieval_reshares_out  , PATH_LENGTH*BLOCK_SIZE+PATH_LENGTH*sizeof(TYPE_DATA), NULL, 0, NULL, true);
                pthread_create(&thread_send[s], NULL, &ServerORAM::thread_socket_func, (void*)&sendSocket_args[s]);
            }
            for(int s = 0 ; s < NUM_SERVERS-1;s++)
            {
                pthread_join(thread_recv[s],NULL);
                pthread_join(thread_send[s],NULL);
            }

            //recover rho & epsilon
            currBufferIdx = 0;
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 0 ; j < PATH_LENGTH; j++)
                {
                    for(int s = 0 ; s < NUM_SERVERS-1;s++)
                    {
                        this->retrieval_path_db[0][i][j] += *((zz_p*)&retrieval_reshares_in[s][currBufferIdx]);
                    }
                    currBufferIdx += sizeof(TYPE_DATA);
                }
            }

            for(int i = 0 ; i < PATH_LENGTH; i++)
            {
                for(int s = 0 ; s < NUM_SERVERS-1; s++)
                {
                    retrieval_query[0][i] += *((zz_p*)&retrieval_reshares_in[s][currBufferIdx]);
                }
                currBufferIdx += sizeof(TYPE_DATA);
            }
            for(int i = 0, startIdx = 0 ; i < numThreads; i ++, startIdx+=step)
            {
                if(startIdx+step > DATA_CHUNKS)
                    endIdx = DATA_CHUNKS;
                else
                    endIdx = startIdx+step;

                vecComp_args[0][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_db[0], PATH_LENGTH, 1, RetrievalShares_b, dotProd_output[0]);
                pthread_create(&vecThread_compute[0][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_args[0][i]);

                vecComp_args[1][i] = THREAD_COMPUTATION(startIdx, endIdx, RetrievalShares_a,  PATH_LENGTH, 1, retrieval_query[0],  dotProd_output[1]);
                pthread_create(&vecThread_compute[1][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_args[1][i]);

                vecComp_args[2][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_db[0], PATH_LENGTH, 1, retrieval_query[0], dotProd_output[2]);
                pthread_create(&vecThread_compute[2][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_args[2][i]);

                vecComp_MAC_args[0][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_db[0], PATH_LENGTH, 1, RetrievalShares_b_mac, dotProd_mac_output[0]);
                pthread_create(&vecThread_compute_MAC[0][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_MAC_args[0][i]);

                vecComp_MAC_args[1][i] = THREAD_COMPUTATION(startIdx, endIdx, RetrievalShares_a_mac,  PATH_LENGTH, 1, retrieval_query[0],  dotProd_mac_output[1]);
                pthread_create(&vecThread_compute_MAC[1][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_MAC_args[1][i]);

                // vecComp_MAC_args[2][i] = THREAD_COMPUTATION(startIdx, endIdx, this->retrieval_path_db[0], PATH_LENGTH, 1, retrieval_query[0], dotProd_mac_output[2]);
                // pthread_create(&vecThread_compute_MAC[2][i], NULL, &ServerORAM::thread_retrieval_by_dotProd_func, (void*)&vecComp_MAC_args[2][i]);

                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(i, &cpuset);


                pthread_setaffinity_np(vecThread_compute[0][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute[1][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute[2][i], sizeof(cpu_set_t), &cpuset);

                pthread_setaffinity_np(vecThread_compute_MAC[0][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute_MAC[1][i], sizeof(cpu_set_t), &cpuset);
                //pthread_setaffinity_np(vecThread_compute_MAC[2][i], sizeof(cpu_set_t), &cpuset);
            }
            for(int i  = 0 ; i <numThreads ; i++)
            {
                    pthread_join(vecThread_compute[0][i],NULL);
                    pthread_join(vecThread_compute[1][i],NULL);
                    pthread_join(vecThread_compute[2][i],NULL);

                    pthread_join(vecThread_compute_MAC[0][i],NULL);
                    pthread_join(vecThread_compute_MAC[1][i],NULL);
                    //pthread_join(vecThread_compute_MAC[2][i],NULL);
            }
            //sum all together
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                dotProd_output[0][i] += dotProd_output[1][i] + RetrievalShares_c[i];
                dotProd_mac_output[0][i] += dotProd_mac_output[1][i] + RetrievalShares_c_mac[i];
                if(this->serverNo == 0)
                {
                    dotProd_output[0][i] += dotProd_output[2][i];
                    dotProd_mac_output[0][i] += MAC_KEY[serverNo]*dotProd_output[2][i];
                }
            }

            // share the random number r
            TYPE_DATA r1, r2;
            share_random_number(r1, r2);               

            zz_p x, y;
            x = 0, y = 0;
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 0 ; j < PATH_LENGTH; j++)
                {
                    x += this->retrieval_path_db[0][i][j] * r1;
                    y += this->retrieval_path_mac[0][i][j] * r1;
                    r1 = (r1 * r1)%P;
                }
            }

            for(int i = 0 ; i < PATH_LENGTH; i++)
            {
                x+= retrieval_query[0][i] * r2;
                y+= retrieval_query_mac[0][i] * r2;
                r2 = ( r2 * r2) % P;
            }
           
            memcpy(&retrieval_answer_out[0],dotProd_output[0],BLOCK_SIZE);
            memcpy(&retrieval_answer_out[BLOCK_SIZE], &x, sizeof(TYPE_DATA));
            memcpy(&retrieval_answer_out[BLOCK_SIZE + sizeof(TYPE_DATA)], &y, sizeof(TYPE_DATA));
            cout<<x<<" "<<y<<endl;
            #endif
        #endif

        start = time_now;
        socket.send(retrieval_answer_out,SERVER_RETRIEVAL_REPLY_LENGTH);
        end = time_now;
        cout<< "	[SendBlock] Block Share SENT in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[3] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        ret = 0;

       #if defined(RSSS) &&!defined(XOR_PIR)
            delete[] transfer_in;
        #endif

        return ret;
}

/**
 * Function Name: recvBlock
 *
 * Description: Receives the share of previosly accessed block from the client
 * with its new index number and stores it into root bucket for later eviction.
 *
 * @param socket: (input) ZeroMQ socket instance for communication with the client
 * @return 0 if successful
 */
int ServerORAM::writeRoot(zmq::socket_t& socket)
{
	cout<< "	[recvBlock] Receiving Block Data..." <<endl;
	auto start = time_now;
    #if defined(SEEDING)
        if(serverNo==0)
            socket.recv(write_root_in, BLOCK_SIZE*2+sizeof(TYPE_INDEX), 0);
        else
            socket.recv(write_root_in, sizeof(TYPE_INDEX), 0);
        auto end = time_now;
        TYPE_INDEX slotIdx;
        memcpy(&slotIdx,&write_root_in[0],sizeof(TYPE_INDEX));

        cout<< "	[recvBlock] Block Data RECV in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[4] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();


        //send to other server (this is due to RSS)
        #if defined(RSSS)
            if(serverNo==2)
            {
                cout<< "	[evict] Creating Threads for Receiving..." << endl;
                recvSocket_args[0] = struct_socket(0, NULL, 0, client_write_root_in, BLOCK_SIZE*2, NULL,false);
                pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);
                pthread_join(thread_recv[0], NULL);
            }
            if(serverNo==0)
            {
                cout<< "	[evict] Creating Threads for Sending..."<< endl;;
                sendSocket_args[0] = struct_socket(1,  &write_root_in[sizeof(TYPE_DATA)], BLOCK_SIZE*2, NULL, 0, NULL, true);
                pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);
                pthread_join(thread_send[0], NULL);
            }
            cout<< "	[evict] CREATED!" <<endl;
            cout<< "	[evict] Waiting for Threads..." <<endl;
        #endif




        start = time_now;
        zz_p tmp2;

        unsigned long long tmp;
        if(serverNo==0)
        {
        }
        else
        {
            for(int i = 0 ; i < DATA_CHUNKS;i++)
            {
                sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo)%3]);
                tmp2  = tmp;
                memcpy(&write_root_in[sizeof(TYPE_DATA)+i*sizeof(TYPE_DATA)],&tmp2,sizeof(TYPE_DATA));

                sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo)%3]);
                tmp2  = tmp;
                memcpy(&write_root_in[sizeof(TYPE_DATA)+i*sizeof(TYPE_DATA)+BLOCK_SIZE],&tmp2,sizeof(TYPE_DATA));
            }
        }
        this->updateRoot(serverNo,slotIdx,&write_root_in[sizeof(TYPE_DATA)],&write_root_in[BLOCK_SIZE+sizeof(TYPE_DATA)]);
        
        #if defined(RSSS)
            if(serverNo==2)
            {

            }
            else
            {
                for(int i = 0 ; i < DATA_CHUNKS;i++)
                {
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo+1)%3]);
                    tmp2  = tmp;
                    memcpy(&client_write_root_in[i*sizeof(TYPE_DATA)],&tmp2,sizeof(TYPE_DATA));

                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_client[(serverNo+1)%3]);
                    tmp2  = tmp;
                    memcpy(&client_write_root_in[i*sizeof(TYPE_DATA)+BLOCK_SIZE],&tmp2,sizeof(TYPE_DATA));
                }
            }
            this->updateRoot((serverNo+1)%3,slotIdx, &client_write_root_in[0], &client_write_root_in[BLOCK_SIZE]);
        #endif
    
        end = time_now;
        cout<< "	[recvBlock] Block STORED in Disk in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

        socket.send((unsigned char*)CMD_SUCCESS,sizeof(CMD_SUCCESS));
        cout<< "	[recvBlock] ACK is SENT!" <<endl;
    #else

        socket.recv(write_root_in, BLOCK_SIZE*2+sizeof(TYPE_INDEX), 0);
        auto end = time_now;
        TYPE_INDEX slotIdx;
        memcpy(&slotIdx,&write_root_in[BLOCK_SIZE*2],sizeof(TYPE_INDEX));

        cout<< "	[recvBlock] Block Data RECV in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[4] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

        #if defined(RSSS)
            //send to other server (this is due to RSS)

            cout<< "	[evict] Creating Threads for Receiving..." << endl;
            recvSocket_args[0] = struct_socket(0, NULL, 0, client_write_root_in, BLOCK_SIZE*2, NULL,false);
            pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);

            cout<< "	[evict] Creating Threads for Sending..."<< endl;;
            sendSocket_args[0] = struct_socket(1,  write_root_in, BLOCK_SIZE*2, NULL, 0, NULL, true);
            pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);

            cout<< "	[evict] CREATED!" <<endl;
            cout<< "	[evict] Waiting for Threads..." <<endl;

            pthread_join(thread_send[0], NULL);
            pthread_join(thread_recv[0], NULL);
        #endif



        start = time_now;
        this->updateRoot(serverNo,slotIdx,&write_root_in[0],&write_root_in[BLOCK_SIZE]);
        #if defined(RSSS)
            this->updateRoot((serverNo+1)%3,slotIdx, &client_write_root_in[0], &client_write_root_in[BLOCK_SIZE]);
        #endif
        end = time_now;
        cout<< "	[recvBlock] Block STORED in Disk in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

        socket.send((unsigned char*)CMD_SUCCESS,sizeof(CMD_SUCCESS));
        cout<< "	[recvBlock] ACK is SENT!" <<endl;
    #endif
    return 0;
}




/**
 * Function Name: thread_socket_func & send & recv
 *
 * Description: Generic threaded socket functions for send and receive operations
 *
 * @return 0 if successful
 */
void *ServerORAM::thread_socket_func(void* args)
{
    struct_socket* opt = (struct_socket*) args;

	if(opt->isSend)
	{
		auto start = time_now;
		send(opt->peer_idx, opt->data_out, opt->data_out_size);
		auto end = time_now;
		if(thread_max < std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count())
			thread_max = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	}
	else
	{
		recv(opt->peer_idx, opt->data_in, opt->data_in_size);
	}
    pthread_exit((void*)opt);
}
int ServerORAM::send(int peer_idx, unsigned char* input, size_t inputSize)
{
    unsigned char buffer_in[sizeof(CMD_SUCCESS)];

    try
    {
		socket_send[peer_idx]->send (input, inputSize);
		cout<< "	[ThreadedSocket] Data SENT!" << endl;

        socket_send[peer_idx]->recv(buffer_in, sizeof(CMD_SUCCESS));
        cout<< "	[ThreadedSocket] ACK RECEIVED!" << endl;
    }
    catch (exception &ex)
    {
        exit(0);
    }

	return 0;
}
int ServerORAM::recv(int peer_idx, unsigned char* output, size_t outputSize)
{
    try
    {
		socket_recv[peer_idx]->recv (output, outputSize);
		cout<< "	[ThreadedSocket] Data RECEIVED! " <<endl;

        socket_recv[peer_idx]->send((unsigned char*)CMD_SUCCESS,sizeof(CMD_SUCCESS));
        cout<< "	[ThreadedSocket] ACK SENT! " <<endl;
    }
    catch (exception &ex)
    {
        cout<<"Socket error!";
        exit(0);
    }

	return 0;
}



int ServerORAM::recvClientEvictData(zmq::socket_t& socket)
{

	cout<< "	[evict] Receiving Evict Matrix..." <<endl;;
	auto start = time_now;
    #if defined(SEEDING)
        if(serverNo==0)
            socket.recv(evict_in, CLIENT_EVICTION_OUT_LENGTH, 0);
        else
            socket.recv(evict_in, sizeof(TYPE_INDEX), 0);
        auto end = time_now;
        cout<< "	[evict] RECEIVED! in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[6] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();


        #if defined(RSSS)
            // send client data to other servers (this is due to RSSS)
            if(serverNo==2)
            {
                cout<< "	[evict] Creating Threads for Receiving..." << endl;
                recvSocket_args[0] = struct_socket(0, NULL, 0, client_evict_in, CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX), NULL,false);
                pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);
                pthread_join(thread_recv[0], NULL);
            }
            if(serverNo==0)
            {
                cout<< "	[evict] Creating Threads for Sending..."<< endl;;
                sendSocket_args[0] = struct_socket(1,  &evict_in[sizeof(TYPE_DATA)], CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX), NULL, 0, NULL, true);
                pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);
                pthread_join(thread_send[0], NULL);

            }
        #endif



        memcpy(&n_evict, &evict_in[0], sizeof(TYPE_INDEX));
        
        cout <<"N evict:" <<n_evict;

    #else
        socket.recv(evict_in, CLIENT_EVICTION_OUT_LENGTH, 0);
        auto end = time_now;
        cout<< "	[evict] RECEIVED! in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() <<endl;
        server_logs[6] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();


        #if defined(RSSS)
            // send client data to other servers (this is due to RSSS)
            cout<< "	[evict] Creating Threads for Receiving..." << endl;
            recvSocket_args[0] = struct_socket(0, NULL, 0, client_evict_in, CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX), NULL,false);
            pthread_create(&thread_recv[0], NULL, &thread_socket_func, (void*)&recvSocket_args[0]);

            cout<< "	[evict] Creating Threads for Sending..."<< endl;;
            sendSocket_args[0] = struct_socket(1,  evict_in, CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX), NULL, 0, NULL, true);
            pthread_create(&thread_send[0], NULL, &thread_socket_func, (void*)&sendSocket_args[0]);

            cout<< "	[evict] CREATED!" <<endl;
            cout<< "	[evict] Waiting for Threads..." <<endl;

            pthread_join(thread_send[0], NULL);
            pthread_join(thread_recv[0], NULL);
        #endif

        memcpy(&n_evict, &evict_in[CLIENT_EVICTION_OUT_LENGTH-sizeof(TYPE_INDEX)], sizeof(TYPE_INDEX));
	#endif

}
/**
 * Function Name: thread_crossProduct_func //inherent
 *
 * Description: Threaded cross-product operation
 *
 */
void *ServerORAM::thread_matProd_func(void* args)
{
    THREAD_COMPUTATION* opt = (THREAD_COMPUTATION*) args;

    for(int l = opt->startIdx ; l < opt->endIdx; l++) //fix this later
    {
        for(int k = 0 ; k < opt->output_length; k++)
        {
            opt->evict_output[l][k] = InnerProd_LL(opt->evict_db_input[l],opt->evict_matrix_input[k],opt->input_length,P,zz_p::ll_red_struct());
        }
    }

    pthread_exit((void*)opt);
}



/**
 * Function Name: sendORAMTree
 *
 * Description: Distributes generated and shared ORAM buckets to servers over network
 *
 * @return 0 if successful
 */

int ServerORAM::recvORAMTree(zmq::socket_t& socket)
{
    int ret = 1;
    for(int i = 0 ; i < NUM_NODES;i++)
    {
        socket.recv(bucket_data,BUCKET_DATA_SIZE,0);
        string path = rootPath + to_string(serverNo) + "/" + to_string(i);

        FILE* file_out = NULL;
        if((file_out = fopen(path.c_str(),"wb+")) == NULL)
        {
            cout<< "	[recvORAMTree] File Cannot be Opened!!" <<endl;
            exit(0);
        }
        fwrite(bucket_data, 1, BUCKET_SIZE*BLOCK_SIZE, file_out);
        fclose(file_out);
        socket.send((unsigned char*)CMD_SUCCESS,sizeof(CMD_SUCCESS),0);

    }
	 cout<< "	[recvORAMTree] ACK is SENT!" <<endl;

	ret = 0;
    return ret ;
}

/*
int ServerORAM::readBucket(TYPE_ID bucketID, int shareID, zz_p** output_data, zz_p** output_mac)
{
    FILE* file_in = NULL;
    string path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID);
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< "	[SendBlock] File cannot be opened!!" <<endl;
        exit;
    }
    for(int i = 0 ; i < BUCKET_SIZE; i++)
    {
        fread(output_data[i], 1, BLOCK_SIZE, file_in);
    }
    fclose(file_in);

    path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID)+ "_mac";
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< "	[SendBlock] File cannot be opened!!" <<endl;
        exit;
    }
    for(int i = 0 ; i < BUCKET_SIZE; i++)
    {
        fread(output_mac[i], 1, BLOCK_SIZE, file_in);
    }
    fclose(file_in);

}*/


//only for SSS Retrieval
int ServerORAM::readBucket(TYPE_ID bucketID, int BucketIdx,  int shareID, zz_p** output_data, zz_p** output_mac)
{
    FILE* file_in = NULL;
    string path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID);
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }
    #if defined (XOR_PIR)
        #if defined (REVERSE_STORAGE_LAYOUT)
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                for(int i = 0 ; i < BUCKET_SIZE; i++)
                {
                    fread(&output_data[BucketIdx*BUCKET_SIZE+i][j], 1, sizeof(TYPE_DATA), file_in);
                }
            }
        #else
            for(int i = 0 ; i < BUCKET_SIZE; i++)
            {
                fread(output_data[BucketIdx*BUCKET_SIZE+i], 1, BLOCK_SIZE, file_in);
            }
        #endif
    #else
        #if !defined (REVERSE_STORAGE_LAYOUT)
            for(int i = 0 ; i < BUCKET_SIZE; i++)
            {
                for(int j = 0 ; j < DATA_CHUNKS; j++)
                {
                    fread(&output_data[j][BucketIdx*BUCKET_SIZE+i], 1, sizeof(TYPE_DATA), file_in);
                }
            }
        #else
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                fread(&output_data[i][BucketIdx*BUCKET_SIZE], 1, BUCKET_SIZE *sizeof(TYPE_DATA), file_in);
            }

        #endif

    #endif
    fclose(file_in);

    path  = myStoragePath + to_string(shareID) + "/" + to_string(bucketID)+ "_mac";
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }

    #if defined (XOR_PIR)
        #if defined (REVERSE_STORAGE_LAYOUT) //for XOR-PIR only
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                for(int i = 0 ; i < BUCKET_SIZE; i++)
                {
                    fread(&output_mac[BucketIdx*BUCKET_SIZE + i][j], 1, sizeof(TYPE_DATA), file_in);
                }
            }
        #else
            for(int i = 0 ; i < BUCKET_SIZE; i++)
            {
                fread(output_mac[BucketIdx*BUCKET_SIZE + i], 1, BLOCK_SIZE, file_in);
            }
        #endif
    #else
        #if !defined (REVERSE_STORAGE_LAYOUT)
            for(int i = 0 ; i < BUCKET_SIZE; i++)
            {
                for(int j = 0 ; j < DATA_CHUNKS; j++)
                {
                    fread(&output_mac[j][BucketIdx*BUCKET_SIZE + i], 1, sizeof(TYPE_DATA), file_in);
                }
            }
        #else
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                fread(&output_mac[i][BucketIdx*BUCKET_SIZE], 1, BUCKET_SIZE*sizeof(TYPE_DATA), file_in);
            }
        #endif
    #endif
    fclose(file_in);
}

int ServerORAM::updateRoot(int shareID, unsigned long long slotIdx, unsigned char* input, unsigned char* mac)
{
    string path = myStoragePath + to_string(shareID) + "/0";
    FILE* f = NULL;
    if((f = fopen(path.c_str(),"r+b")) == NULL)
    {
        cout<< "	[recvBlock] File Cannot be Opened!!" <<endl;
        exit(0);
    }
    #if !defined (REVERSE_STORAGE_LAYOUT)
        fseek(f, slotIdx*BLOCK_SIZE,SEEK_SET);
        fwrite(input,1,BLOCK_SIZE,f);
    #else
        fseek(f, slotIdx*sizeof(TYPE_DATA),SEEK_SET);
        for(int u = 0 ; u < DATA_CHUNKS; u++)
        {
            fwrite(&input[u*sizeof(TYPE_DATA)],1,sizeof(TYPE_DATA),f);
            fseek(f,(BUCKET_SIZE-1)*sizeof(TYPE_DATA),SEEK_CUR);
        }
    #endif
    fclose(f);

    string path_mac  =  path + "_mac";
    FILE* f_mac = NULL;
    if((f_mac = fopen(path_mac.c_str(),"r+b")) == NULL)
    {
        cout<< "	[recvBlock] File Cannot be Opened!!" <<endl;
        exit(0);
    }
    #if !defined (REVERSE_STORAGE_LAYOUT)
        fseek(f_mac, slotIdx*BLOCK_SIZE,SEEK_SET);
        fwrite(mac,1,BLOCK_SIZE,f_mac);
    #else
        fseek(f_mac, slotIdx*sizeof(TYPE_DATA),SEEK_SET);
        for(int u = 0 ; u < DATA_CHUNKS; u++)
        {
            fwrite(&mac[u*sizeof(TYPE_DATA)],1,sizeof(TYPE_DATA),f_mac);
            fseek(f_mac,(BUCKET_SIZE-1)*sizeof(TYPE_DATA),SEEK_CUR);
        }
    #endif
    fclose(f_mac);

}


int ServerORAM::copyBucket(int shareID, TYPE_ID srcBucketID, TYPE_ID destBucketID)
{
    string src_path = myStoragePath + to_string(shareID) + "/" + to_string(srcBucketID);
    string dest_path = myStoragePath + to_string(shareID) + "/" + to_string(destBucketID);

    std::ifstream src(src_path,std::ios::binary);
    std::ofstream dest(dest_path,std::ios::binary);
    dest << src.rdbuf();



    string src_mac_path = src_path + "_mac";
    string dest_mac_path = dest_path + "_mac";

    std::ifstream src_mac(src_mac_path,std::ios::binary);
    std::ofstream dest_mac(dest_mac_path,std::ios::binary);
    dest_mac << src_mac.rdbuf();

    return 0;
}



/*int ServerORAM::writeBucket(TYPE_ID bucketID, int shareID, unsigned char* input, unsigned char* mac)
{
    string path = myStoragePath +  to_string(shareID) + "/" + to_string(bucketID);
    FILE* f = NULL;
    if((f = fopen(path.c_str(),"wb+")) == NULL)
    {
        cout<< "	[evict] File Cannot be Opened!!" <<endl;
        exit(0);
    }
    fwrite(input,1, BUCKET_SIZE*BLOCK_SIZE, f);
    fclose(f);

    #if defined(SPDZ)
        string path_MAC = path + "_mac";
        FILE* f_MAC = NULL;
        if((f_MAC = fopen(path_MAC.c_str(),"wb+")) == NULL)
        {
            cout<< "	[evict] File Cannot be Opened!!" <<endl;
            exit(0);
        }
        fwrite(mac,1, BUCKET_SIZE*BLOCK_SIZE, f_MAC);
        fclose(f_MAC);
    #endif

    return 0;
}*/




/**
 * Function Name: thread_retrieval_func
 *
 * Description: Threaded dot-product operation
 *
 */
void *ServerORAM::thread_retrieval_by_XOR_func(void* args)
{
    THREAD_COMPUTATION* opt = (THREAD_COMPUTATION*) args;
    //XOR
    ORAM::xor_retrieve( opt->retrieval_xor_query,opt->retrieval_db_input, opt->retrieval_mac_input,
                        opt->startIdx, opt->endIdx,
                        opt->retrieval_xor_output, opt->retrieval_xor_mac_output);

}



void *ServerORAM::thread_retrieval_by_dotProd_func(void* args)
{
    THREAD_COMPUTATION* opt = (THREAD_COMPUTATION*) args;

    for(int l = opt->startIdx ; l < opt->endIdx; l++) //fix this later
    {
        opt->retrieval_sss_output[l] = InnerProd_LL(opt->retrieval_db_input[l],opt->retrieval_sss_query,opt->input_length,P,zz_p::ll_red_struct());
    }

}





int ServerORAM::preReSharing(int level, int es, int ee)
{
    // implement the RSSS multiplication first

    #if defined(RSSS)
        int endIdx;
        int step = ceil((double)DATA_CHUNKS/(double)numThreads);
        int nConcurrBucket;

        //computation
        for(int e = es ; e < ee ; e++)
        {
            for(int j = 0 ; j < NUM_MULT; j++)
            {
                for(int i = 0, startIdx = 0 ; i < numThreads; i ++, startIdx+=step)
                {
                    if(startIdx+step > DATA_CHUNKS)
                        endIdx = DATA_CHUNKS;
                    else
                        endIdx = startIdx+step;

                    vecComp_args[e*NUM_MULT+j][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_db[e*NUM_SHARE_PER_SERVER+RSSS_MULT_ORDER[j][0]], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH, this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+RSSS_MULT_ORDER[j][1]][level], this->vecLocalMatProduct_output[e*NUM_MULT+j]);
                    pthread_create(&vecThread_compute[e*NUM_MULT+j][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_args[e*NUM_MULT+j][i]);


                    vecComp_MAC_args[e*NUM_MULT+j][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_MAC[e*NUM_SHARE_PER_SERVER+RSSS_MULT_ORDER[j][0]], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH, this->vecEvictMatrix[e*NUM_SHARE_PER_SERVER+RSSS_MULT_ORDER[j][1]][level], this->vecLocalMatProduct_output_MAC[e*NUM_MULT+j]);
                    pthread_create(&vecThread_compute_MAC[e*NUM_MULT+j][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_MAC_args[e*NUM_MULT+j][i]);

                    cpu_set_t cpuset;
                    CPU_ZERO(&cpuset);
                    CPU_SET(i, &cpuset);
                    pthread_setaffinity_np(vecThread_compute_MAC[e*NUM_MULT+j][i], sizeof(cpu_set_t), &cpuset);

                    pthread_setaffinity_np(vecThread_compute[e*NUM_MULT+j][i], sizeof(cpu_set_t), &cpuset);
                }
            }
        }
        for(int e = es ; e < ee; e ++)
        {
            for(int j = 0 ; j < NUM_MULT; j++)
            {
                for(int i  = 0 ; i <numThreads ; i++)
                {
                    pthread_join(vecThread_compute[e*NUM_MULT+j][i],NULL);

                    pthread_join(vecThread_compute_MAC[e*NUM_MULT+j][i],NULL);
                }
            }
        }
        //sum all together
        for(int e = es ; e < ee; e ++)
        {
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int n = 0 ; n < MAT_PRODUCT_OUTPUT_LENGTH; n++)
                {
                    for(int j = 1 ; j < NUM_MULT; j++)
                    {
                        this->vecLocalMatProduct_output[e*NUM_MULT][i][n] += this->vecLocalMatProduct_output[e*NUM_MULT+j][i][n];

                        //MAC
                        this->vecLocalMatProduct_output_MAC[e*NUM_MULT][i][n] += this->vecLocalMatProduct_output_MAC[e*NUM_MULT+j][i][n];
                    }
                }
            }
        }
    #else // SPDZ
        unsigned long long currBufferIdx =  0;
        for(int e = es ; e < ee ; e++)
        {
            readTriplets(this->vecShares_a[e], DATA_CHUNKS, MAT_PRODUCT_INPUT_DB_LENGTH, "/evict_triplet_a");
            readTriplets(this->vecShares_b[e][level], EVICT_MAT_NUM_ROW, EVICT_MAT_NUM_COL, "/evict_triplet_b");
            readTriplets(this->vecShares_a_MAC[e], DATA_CHUNKS, MAT_PRODUCT_INPUT_DB_LENGTH, "/evict_triplet_a_mac");
            readTriplets(this->vecShares_b_MAC[e][level], EVICT_MAT_NUM_ROW, EVICT_MAT_NUM_COL, "/evict_triplet_b_mac");
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 0 ; j < MAT_PRODUCT_INPUT_DB_LENGTH; j++)
                {
                    //this->vecShares_a[e][i][j] = 0;
                    this->vecEvictPath_db[e][i][j] -= this->vecShares_a[e][i][j];
                    this->vecEvictPath_MAC[e][i][j] -=this->vecShares_a_MAC[e][i][j];
                }
                for(int s = 0 ; s < NUM_SERVERS-1;s++)
                {
                    memcpy(&reshares_out[s][currBufferIdx], this->vecEvictPath_db[e][i], sizeof(TYPE_DATA)*(MAT_PRODUCT_INPUT_DB_LENGTH));
                }
                currBufferIdx+=sizeof(TYPE_DATA)*(MAT_PRODUCT_INPUT_DB_LENGTH);

            }
            for(int i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
            {
                for(int j = 0 ; j < EVICT_MAT_NUM_COL; j++)
                {
                    //this->vecShares_b[e][level][i][j] = 0;
                    this->vecEvictMatrix[e][level][i][j] -= this->vecShares_b[e][level][i][j];
                    this->vecEvictMatrix_MAC[e][level][i][j] -= this->vecShares_b_MAC[e][level][i][j];
                }

                for(int s = 0 ; s < NUM_SERVERS-1;s++)
                {
                    memcpy(&reshares_out[s][currBufferIdx], this->vecEvictMatrix[e][level][i], sizeof(TYPE_DATA)*(EVICT_MAT_NUM_COL));
                }
                currBufferIdx+=sizeof(TYPE_DATA)*(EVICT_MAT_NUM_COL);
            }
        }
    #endif
	return 0;
}

int ServerORAM::reShare(int level, int es, int ee)
{
    #if defined(SEEDING) && defined(RSSS)
        unsigned long long currBufferIdx = 0;
        TYPE_DATA shares_BLOCK[NUM_SERVERS];

        TYPE_DATA shares_MAC[NUM_SERVERS];


        printf("\n");
        for(int e = es ; e < ee; e++)
        {
            int m = 0;
            for(int u = 0 ; u <DATA_CHUNKS; u++)
            {
                for(TYPE_INDEX j = 0; j < MAT_PRODUCT_OUTPUT_LENGTH; j++)
                {
                    ORAM::createShares(this->vecLocalMatProduct_output[e*NUM_MULT][u][j]._zz_p__rep, shares_BLOCK, NULL,prng_server[this->serverNo],this->serverNo); // EACH SERVER CALCULATES AND DISTRIBUTES SHARES

                    //MAC
                    ORAM::createShares(this->vecLocalMatProduct_output_MAC[e*NUM_MULT][u][j]._zz_p__rep,shares_MAC,NULL,prng_server[this->serverNo],this->serverNo);
                    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)
                    {
                        vecReShares[e][k][u][j] = shares_BLOCK[k];
                        //MAC
                        vecReShares_MAC[e][k][u][j] = shares_MAC[k];
                    }


                }
                memcpy(&reshares_out[1][currBufferIdx], vecReShares[e][(this->serverNo)][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));


                //MAC
                memcpy(&reshares_out[1][currBufferIdx], vecReShares_MAC[e][(this->serverNo)][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));

                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));

            }
        }
    #elif defined(RSSS)
        unsigned long long currBufferIdx = 0;
        TYPE_DATA shares_BLOCK[NUM_SERVERS];

        TYPE_DATA shares_MAC[NUM_SERVERS];


        printf("\n");
        for(int e = es ; e < ee; e++)
        {
            int m = 0;
            for(int u = 0 ; u <DATA_CHUNKS; u++)
            {
                for(TYPE_INDEX j = 0; j < MAT_PRODUCT_OUTPUT_LENGTH; j++)
                {
                    ORAM::createShares(this->vecLocalMatProduct_output[e*NUM_MULT][u][j]._zz_p__rep, shares_BLOCK, NULL); // EACH SERVER CALCULATES AND DISTRIBUTES SHARES

                    //MAC
                    ORAM::createShares(this->vecLocalMatProduct_output_MAC[e*NUM_MULT][u][j]._zz_p__rep,shares_MAC,NULL);
                    for(TYPE_INDEX k = 0; k < NUM_SERVERS; k++)
                    {
                        vecReShares[e][k][u][j] = shares_BLOCK[k];
                        //MAC
                        vecReShares_MAC[e][k][u][j] = shares_MAC[k];

                    }
                }
                memcpy(&reshares_out[0][currBufferIdx], vecReShares[e][(this->serverNo+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                memcpy(&reshares_out[1][currBufferIdx], vecReShares[e][(this->serverNo+3-1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));

                memcpy(&reshares_out[0][currBufferIdx], vecReShares[e][((this->serverNo+1)+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                memcpy(&reshares_out[1][currBufferIdx], vecReShares[e][(this->serverNo+3-1+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));


                //MAC
                memcpy(&reshares_out[0][currBufferIdx], vecReShares_MAC[e][(this->serverNo+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                memcpy(&reshares_out[1][currBufferIdx], vecReShares_MAC[e][(this->serverNo+3-1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));

                memcpy(&reshares_out[0][currBufferIdx], vecReShares_MAC[e][((this->serverNo+1)+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                memcpy(&reshares_out[1][currBufferIdx], vecReShares_MAC[e][(this->serverNo+3-1+1)%3][u], sizeof(TYPE_DATA)*(MAT_PRODUCT_OUTPUT_LENGTH));
                currBufferIdx += ((MAT_PRODUCT_OUTPUT_LENGTH)*sizeof(TYPE_DATA));

            }


            //currBufferIdx += ((BUCKET_SIZE+1)*BLOCK_SIZE);
        }
    #else //SPDZ does not need this phase
    #endif
    return 0;

}


int ServerORAM::postReSharing(int level, int es, int ee)
{
    #if (defined(SEEDING) && defined(RSSS))
        unsigned long long currBufferIdx = 0;
        unsigned long long tmp;
        unsigned long long tmp3[MAT_PRODUCT_OUTPUT_LENGTH];
        unsigned long long tmp3_MAC[MAT_PRODUCT_OUTPUT_LENGTH];

        memset(tmp3,0,MAT_PRODUCT_OUTPUT_LENGTH*sizeof(TYPE_DATA));
        zz_p tmp2;
        for(int e = es ; e < ee; e++)
        {
            for(int u = 0 ; u < DATA_CHUNKS; u ++)
            {
                for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
                {
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+1)%3][this->serverNo]);
                    tmp2 = tmp;
                    vecReShares[e][this->serverNo][u][j] += tmp2;

                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+2)%3][this->serverNo]);
                    tmp2 = tmp;
                    vecReShares[e][this->serverNo][u][j] += tmp2;

                    //MAC
                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+1)%3][this->serverNo]);
                    tmp2 = tmp;
                    vecReShares_MAC[e][this->serverNo][u][j] += tmp2;

                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+2)%3][this->serverNo]);
                    tmp2 = tmp;
                    vecReShares_MAC[e][this->serverNo][u][j] += tmp2;

                }

                memcpy((unsigned char*)&tmp3,&reshares_in[0][currBufferIdx],MAT_PRODUCT_OUTPUT_LENGTH*sizeof(TYPE_DATA));
                currBufferIdx+=MAT_PRODUCT_OUTPUT_LENGTH*sizeof(TYPE_DATA);
                memcpy((unsigned char*)&tmp3_MAC,&reshares_in[0][currBufferIdx],MAT_PRODUCT_OUTPUT_LENGTH*sizeof(TYPE_DATA));
                currBufferIdx+=MAT_PRODUCT_OUTPUT_LENGTH*sizeof(TYPE_DATA);
                for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH; j++)
                {
                    tmp2 = tmp3[j];
                    vecReShares[e][(this->serverNo+1)%3][u][j] += tmp2;

                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+2)%3][(this->serverNo+1)%3]);
                    tmp2 = tmp;
                    vecReShares[e][(this->serverNo+1)%3][u][j] += tmp2;


                    //MAC
                    tmp2 = tmp3_MAC[j];
                    vecReShares_MAC[e][(this->serverNo+1)%3][u][j] +=  tmp2;

                    sober128_read((unsigned char*)&tmp,sizeof(TYPE_DATA),&prng_server[(this->serverNo+2)%3][(this->serverNo+1)%3]); // dont have this
                    tmp2 = tmp;
                    vecReShares_MAC[e][(this->serverNo+1)%3][u][j] +=  tmp2;
                }

            }
            TYPE_DATA r1;
            share_random_number(r1);  
            
            for(int u = 0 ; u < DATA_CHUNKS; u ++)
            {
                for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
                {
                    this->X1 += vecReShares[e][this->serverNo][u][j] * r1; 
                    this->Y1 += vecReShares_MAC[e][this->serverNo][u][j] * r1;
                    
                    this->X2 += vecReShares[e][(this->serverNo+1)%3][u][j] * r1; 
                    this->Y2 += vecReShares_MAC[e][(this->serverNo+1)%3][u][j] * r1;
                    r1 = (r1 * r1) % P;
                }
            }
            cout<<X1<<" "<<X2<<endl;
            cin.get();


        }
    #elif defined(RSSS) //only RSSS
    unsigned long long currBufferIdx = 0;
    for(int e = es ; e < ee; e++)
    {
        for(int u = 0 ; u < DATA_CHUNKS; u ++)
        {
            for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
            {
                vecReShares[e][this->serverNo][u][j] += *((zz_p*)&reshares_in[0][currBufferIdx]) + *((zz_p*)&reshares_in[1][currBufferIdx]);
                currBufferIdx += sizeof(zz_p);
            }
            for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH; j++)
            {
                vecReShares[e][(this->serverNo+1)%3][u][j] += *((zz_p*)&reshares_in[0][currBufferIdx]) + *((zz_p*)&reshares_in[1][currBufferIdx]);
                currBufferIdx += sizeof(zz_p);
            }

            //MAC
            for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
            {
                vecReShares_MAC[e][this->serverNo][u][j] += *((zz_p*)&reshares_in[0][currBufferIdx]) + *((zz_p*)&reshares_in[1][currBufferIdx]);
                currBufferIdx += sizeof(zz_p);
            }
            for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
            {
                vecReShares_MAC[e][(this->serverNo+1)%3][u][j] += *((zz_p*)&reshares_in[0][currBufferIdx]) + *((zz_p*)&reshares_in[1][currBufferIdx]);
                currBufferIdx += sizeof(zz_p);
            }
        }

        // share the random number r
        TYPE_DATA r1;
        share_random_number(r1);  
        
        for(int u = 0 ; u < DATA_CHUNKS; u ++)
        {
            for(int j = 0 ; j < MAT_PRODUCT_OUTPUT_LENGTH ; j++)
            {
                this->X1 += vecReShares[e][this->serverNo][u][j] * r1; 
                this->Y1 += vecReShares_MAC[e][this->serverNo][u][j] * r1;
                
                this->X2 += vecReShares[e][(this->serverNo+1)%3][u][j] * r1; 
                this->Y2 += vecReShares_MAC[e][(this->serverNo+1)%3][u][j] * r1;
                r1 = (r1 * r1) % P;
            }
        }
        cin.get();
        cout<<serverNo<<" "<<X1<<" "<<X2<<endl;
    }
    #else // SPDZ with/without-seeding
        //recover rho & epsilon
        unsigned long currBufferIdx = 0;
        for(int e = es ; e < ee ; e++)
        {
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int j = 0 ; j < MAT_PRODUCT_INPUT_DB_LENGTH; j++)
                {
                    for(int s = 0 ; s < NUM_SERVERS-1;s++)
                    {
                        this->vecEvictPath_db[e][i][j] += *((zz_p*)&reshares_in[s][currBufferIdx]);
                    }
                    currBufferIdx += sizeof(TYPE_DATA);
                }
            }

            for(int i = 0 ; i < EVICT_MAT_NUM_ROW; i++)
            {
                for(int j = 0 ; j < EVICT_MAT_NUM_COL; j++)
                {
                    for(int s = 0 ; s < NUM_SERVERS-1; s++)
                    {
                        this->vecEvictMatrix[e][level][i][j] += *((zz_p*)&reshares_in[s][currBufferIdx]);
                    }
                    currBufferIdx += sizeof(TYPE_DATA);
                }

            }
        }
          int endIdx;
        int step = ceil((double)DATA_CHUNKS/(double)numThreads);

        for(int e = es ; e < ee ; e++)
        {
            for(int i = 0, startIdx = 0 ; i < numThreads; i ++, startIdx+=step)
            {
                if(startIdx+step > DATA_CHUNKS)
                    endIdx = DATA_CHUNKS;
                else
                    endIdx = startIdx+step;

                vecComp_args[e*NUM_MULT+0][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_db[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH, this->vecShares_b[e][level], this->vecLocalMatProduct_output[e*NUM_MULT+0]);
                pthread_create(&vecThread_compute[e*NUM_MULT+0][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_args[e*NUM_MULT+0][i]);

                vecComp_args[e*NUM_MULT+1][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecShares_a[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH,  this->vecEvictMatrix[e][level],  this->vecLocalMatProduct_output[e*NUM_MULT+1]);
                pthread_create(&vecThread_compute[e*NUM_MULT+1][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_args[e*NUM_MULT+1][i]);

                vecComp_args[e*NUM_MULT+2][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_db[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH,  this->vecEvictMatrix[e][level], this->vecLocalMatProduct_output[e*NUM_MULT+2]);
                pthread_create(&vecThread_compute[e*NUM_MULT+2][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_args[e*NUM_MULT+2][i]);



                /*vecComp_MAC_args[e*NUM_MULT+j][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_MAC[e*NUM_MULT+RSSS_MULT_ORDER[j][0]], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH, this->vecEvictMatrix[e*NUM_MULT+RSSS_MULT_ORDER[j][1]][level], this->vecLocalMatProduct_output_MAC[e*NUM_MULT+j]);
                pthread_create(&vecThread_compute_MAC[e*NUM_MULT+0][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_MAC_args[e*NUM_MULT+0][i]);*/

                vecComp_MAC_args[e*NUM_MULT+0][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_db[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH, this->vecShares_b_MAC[e][level], this->vecLocalMatProduct_output_MAC[e*NUM_MULT+0]);
                pthread_create(&vecThread_compute_MAC[e*NUM_MULT+0][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_MAC_args[e*NUM_MULT+0][i]);

                vecComp_MAC_args[e*NUM_MULT+1][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecShares_a_MAC[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH,  this->vecEvictMatrix[e][level],  this->vecLocalMatProduct_output_MAC[e*NUM_MULT+1]);
                pthread_create(&vecThread_compute_MAC[e*NUM_MULT+1][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_MAC_args[e*NUM_MULT+1][i]);

                //vecComp_MAC_args[e*NUM_MULT+2][i] = THREAD_COMPUTATION(startIdx, endIdx, this->vecEvictPath_db[e], MAT_PRODUCT_INPUT_DB_LENGTH, MAT_PRODUCT_OUTPUT_LENGTH,  this->vecEvictMatrix[e][level], this->vecLocalMatProduct_output_MAC[e*NUM_MULT+2]);
                //pthread_create(&vecThread_compute_MAC[e*NUM_MULT+2][i], NULL, &ServerORAM::thread_matProd_func, (void*)&vecComp_MAC_args[e*NUM_MULT+2][i]);


                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(i, &cpuset);


                pthread_setaffinity_np(vecThread_compute[e*NUM_MULT+0][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute[e*NUM_MULT+1][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute[e*NUM_MULT+2][i], sizeof(cpu_set_t), &cpuset);

                pthread_setaffinity_np(vecThread_compute_MAC[e*NUM_MULT+0][i], sizeof(cpu_set_t), &cpuset);
                pthread_setaffinity_np(vecThread_compute_MAC[e*NUM_MULT+1][i], sizeof(cpu_set_t), &cpuset);
                //pthread_setaffinity_np(vecThread_compute_MAC[e*NUM_MULT+2][i], sizeof(cpu_set_t), &cpuset);

            }
        }
        for(int e = es ; e < ee; e ++)
        {
            for(int i  = 0 ; i <numThreads ; i++)
            {
                pthread_join(vecThread_compute[e*NUM_MULT+0][i],NULL);
                pthread_join(vecThread_compute[e*NUM_MULT+1][i],NULL);
                //if(this->serverNo==0)
                pthread_join(vecThread_compute[e*NUM_MULT+2][i],NULL);

                pthread_join(vecThread_compute_MAC[e*NUM_MULT+0][i],NULL);
                pthread_join(vecThread_compute_MAC[e*NUM_MULT+1][i],NULL);
                //pthread_join(vecThread_compute_MAC[e*NUM_MULT+2][i],NULL);
            }
        }
        //sum all together
        for(int e = es ; e < ee; e ++)
        {
            readTriplets(this->vecShares_c[e], DATA_CHUNKS, MAT_PRODUCT_OUTPUT_LENGTH, "/evict_triplet_c");
            readTriplets(this->vecShares_c_MAC[e], DATA_CHUNKS, MAT_PRODUCT_OUTPUT_LENGTH, "/evict_triplet_c_mac");
            for(int i = 0 ; i < DATA_CHUNKS; i++)
            {
                for(int n = 0 ; n < MAT_PRODUCT_OUTPUT_LENGTH; n++)
                {
                    //vecShares_c[e][i][n] = 0;
                    this->vecReShares[e][this->serverNo][i][n] = this->vecLocalMatProduct_output[e*NUM_MULT+0][i][n] + this->vecLocalMatProduct_output[e*NUM_MULT+1][i][n] + vecShares_c[e][i][n];

                    if(this->serverNo == 0)
                    {
                        this->vecReShares[e][this->serverNo][i][n] += this->vecLocalMatProduct_output[e*NUM_MULT+2][i][n];
                    }

                    this->vecReShares_MAC[e][this->serverNo][i][n] =  vecShares_c_MAC[e][i][n] + this->vecLocalMatProduct_output_MAC[e*NUM_MULT+0][i][n] + this->vecLocalMatProduct_output_MAC[e*NUM_MULT+1][i][n] +  this->vecLocalMatProduct_output[e*NUM_MULT+2][i][n]*MAC_KEY[this->serverNo] ;
                    //cout<<this->vecReShares_MAC[e][this->serverNo][i][n]<<endl;
                    //cin.get();

                }
            }

            // share the random number r
            TYPE_DATA r1, r2;
            share_random_number(r1, r2);  

            for(int i = 0 ; i < DATA_CHUNKS; i ++)
            {
                for(int j = 0 ; j < MAT_PRODUCT_INPUT_DB_LENGTH ; j++)
                {
                    this->X1 += vecEvictPath_db[e][i][j] * r1; 
                    this->Y1 += vecEvictPath_MAC[e][i][j] * r1;
                    r1 = (r1 * r1) % P; // optimize it later
                }
            }

            for(int i = 0 ; i < EVICT_MAT_NUM_ROW; i ++)
            {
                for(int j = 0 ; j < EVICT_MAT_NUM_COL ; j++)
                {

                    this->X1 += vecEvictMatrix[e][level][i][j] * r2; 
                    this->Y1 += vecEvictMatrix_MAC[e][level][i][j] * r2;
                    r2 = (r2 * r2) % P;
                }
            }
            cout<< this->X1<<" "<<this->Y1<<endl;
        }

        
    #endif
}



int ServerORAM::writeBucket(int bucketID, int shareID, zz_p ** data, zz_p** mac)
{
    string path = myStoragePath + to_string(shareID) + "/" + to_string(bucketID);
    FILE *file_out;
    if((file_out = fopen(path.c_str(),"wb+")) == NULL)
    {
        cout<< path << " cannot be Opened!!" <<endl;
        exit(0);
    }
    string path_MAC =  path + "_mac";
    FILE *file_out_MAC;

    if((file_out_MAC = fopen(path_MAC.c_str(),"wb+")) == NULL)
    {
        cout<< path_MAC << " cannot be Opened!!" <<endl;
        exit(0);
    }
    #if !defined (REVERSE_STORAGE_LAYOUT)
        for ( int u = 0 ; u < BUCKET_SIZE; u++)
        {
            for(int j = 0 ; j < DATA_CHUNKS; j++)
            {
                fwrite(&data[j][u]._zz_p__rep,1,sizeof(TYPE_DATA),file_out);
                //MAC
                fwrite(&mac[j][u]._zz_p__rep,1,sizeof(TYPE_DATA),file_out_MAC);
            }
        }
    #else
        for(int j = 0 ; j < DATA_CHUNKS; j++)
        {
            fwrite(data[j],1,sizeof(TYPE_DATA)*BUCKET_SIZE,file_out);
            //MAC
            fwrite(mac[j],1,sizeof(TYPE_DATA)*BUCKET_SIZE,file_out_MAC);
        }
    #endif
    fclose(file_out);

    //MAC
    fclose(file_out_MAC);

}


int ServerORAM::readTriplets(zz_p** data, int row, int col, string file_name)
{
    FILE* file_in = NULL;
    string path  = myStoragePath + to_string(serverNo) + file_name;
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }

    for(int i = 0 ; i < row; i++)
    {
        for(int j = 0 ; j < col; j++)
        {
            fread(&data[i][j], 1, sizeof(TYPE_DATA), file_in);
        }
    }
    fclose(file_in);
    
    // use a more efficient method to delete first n bytes from file
    //string file_pop_cmd = "dd conv=notrunc status=none if=" + path + " bs=1 skip=" + to_string(row*col*sizeof(TYPE_DATA)) +" of=" + path;
    string file_pop_cmd = "tail -c +" + to_string(row*col*sizeof(TYPE_DATA) + 1) + " " + path + " > tmp" + to_string(serverNo);
    string move_cmd = "mv tmp" + to_string(serverNo)+ " " + path;
    system(file_pop_cmd.c_str());
    system(move_cmd.c_str());
} 


int ServerORAM::readTriplets(zz_p* data, int length, string file_name)
{
    FILE* file_in = NULL;
    string path  = myStoragePath + to_string(serverNo) + file_name;
    if((file_in = fopen(path.c_str(),"rb")) == NULL)
    {
        cout<< path << " cannot be opened!!" <<endl;
        exit;
    }

    for(int i = 0 ; i < length; i++)
    {
        fread(&data[i], 1, sizeof(TYPE_DATA), file_in);
    }
    fclose(file_in);
    // use a more efficient method to delete first n bytes from file
    //string file_pop_cmd = "dd conv=notrunc if=" + path + " bs=1 skip=" + to_string(length*sizeof(TYPE_DATA)) +" of=" + path;
    string file_pop_cmd = "tail -c +" + to_string(length*sizeof(TYPE_DATA) + 1) + " " + path + " > tmp" + to_string(serverNo);
    string move_cmd = "mv tmp" + to_string(serverNo)+ " " + path;
    system(file_pop_cmd.c_str());
    system(move_cmd.c_str());
} 


int ServerORAM::share_random_number(TYPE_DATA& r1, TYPE_DATA& r2)
{
    r1 = rand();
    r2 = rand();
    memcpy(&random_shares_out[0], &r1, sizeof(TYPE_DATA));
    memcpy(&random_shares_out[sizeof(TYPE_DATA)], &r2, sizeof(TYPE_DATA));
    for(int s = 0 ; s < NUM_SERVERS-1;s++)
    {
        cout<< "	[evict] Creating Threads for Receiving Ports..." << endl;
        recvSocket_args[s] = struct_socket(s, NULL, 0, random_shares_in[s], 2*sizeof(TYPE_DATA), NULL,false);
        pthread_create(&thread_recv[s], NULL, &ServerORAM::thread_socket_func, (void*)&recvSocket_args[s]);
        
        cout<< "	[evict] Creating Threads for Sending Shares..."<< endl;;
        sendSocket_args[s] = struct_socket(s, random_shares_out, 2*sizeof(TYPE_DATA), NULL, 0, NULL, true);
        pthread_create(&thread_send[s], NULL, &ServerORAM::thread_socket_func, (void*)&sendSocket_args[s]);
    }
    for(int s = 0 ; s < NUM_SERVERS-1;s++)
    {
        pthread_join(thread_recv[s],NULL);
        pthread_join(thread_send[s],NULL);
    }
    
    for(int s = 0 ; s < NUM_SERVERS -1; s++)
    {
        r1 += *((TYPE_DATA*)&random_shares_in[s][0]);
        r2 += *((TYPE_DATA*)&random_shares_in[s][sizeof(TYPE_DATA)]);
        r1 %= P;
        r2 %= P;
    }
}

int ServerORAM::share_random_number(TYPE_DATA& r1)
{
    r1 = rand();
    memcpy(&random_shares_out[0], &r1, sizeof(TYPE_DATA));
    for(int s = 0 ; s < NUM_SERVERS-1;s++)
    {
        cout<< "	[evict] Creating Threads for Receiving Ports..." << endl;
        recvSocket_args[s] = struct_socket(s, NULL, 0, random_shares_in[s], sizeof(TYPE_DATA), NULL,false);
        pthread_create(&thread_recv[s], NULL, &ServerORAM::thread_socket_func, (void*)&recvSocket_args[s]);
        
        cout<< "	[evict] Creating Threads for Sending Shares..."<< endl;;
        sendSocket_args[s] = struct_socket(s, random_shares_out, sizeof(TYPE_DATA), NULL, 0, NULL, true);
        pthread_create(&thread_send[s], NULL, &ServerORAM::thread_socket_func, (void*)&sendSocket_args[s]);
    }
    for(int s = 0 ; s < NUM_SERVERS-1;s++)
    {
        pthread_join(thread_recv[s],NULL);
        pthread_join(thread_send[s],NULL);
    }
    
    for(int s = 0 ; s < NUM_SERVERS -1; s++)
    {
        r1 += *((TYPE_DATA*)&random_shares_in[s][0]);
        r1 %= P;
    }
}