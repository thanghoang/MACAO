/*
 * ServerBinaryORAMO.cpp
 *
 *      Author: thanghoang
 */

#include "ServerBinaryORAMO.hpp"
#include "Utils.hpp"
#include "struct_socket.h"

#include "ORAM.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

#include "struct_thread_computation.h"
#include "struct_thread_loadData.h"

ServerBinaryORAMO::ServerBinaryORAMO(TYPE_INDEX serverNo, int selectedThreads) : ServerORAM(serverNo, selectedThreads)
{
    long long n = BLOCK_SIZE * 2;
#if defined(RSSS) && !defined(SEEDING)
    n = n * 2;
#endif
    this->write_root_in = new unsigned char[n + sizeof(TYPE_INDEX)];
}

ServerBinaryORAMO::ServerBinaryORAMO()
{
}

ServerBinaryORAMO::~ServerBinaryORAMO()
{
}

int ServerBinaryORAMO::prepareEvictComputation()
{
#if defined(SEEDING)
    unsigned long long tmp = 0;
    unsigned long long currBufferIdx = sizeof(TYPE_DATA);
#else
    unsigned long long currBufferIdx = 0;
#endif
    //evict matrix
    for (TYPE_INDEX y = 0; y < H + 1; y++)
    {
        for (TYPE_INDEX i = 0; i < EVICT_MAT_NUM_ROW; i++)
        {
#if defined(SEEDING)
            if (serverNo == 0)
            {
                memcpy(this->vecEvictMatrix[0][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
#if defined(SPDZ)
                memcpy(this->vecEvictMatrix_MAC[0][y][i], &evict_in[currBufferIdx + (H + 1) * evictMatSize * sizeof(TYPE_DATA)], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
#endif
            }
            else
            {
                for (int j = 0; j < EVICT_MAT_NUM_COL; j++)
                {
                    sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[serverNo]);
                    this->vecEvictMatrix[0][y][i][j] = tmp;

#if defined(SPDZ)
                    sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[serverNo]);
                    this->vecEvictMatrix_MAC[0][y][i][j] = tmp;
#endif
                }
            }
#if defined(RSSS)
            if (serverNo == 2)
            {
                memcpy(this->vecEvictMatrix[1][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
            }
            else
            {
                for (int j = 0; j < EVICT_MAT_NUM_COL; j++)
                {
                    sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[(serverNo + 1) % 3]);
                    this->vecEvictMatrix[1][y][i][j] = tmp;
                }
            }
#endif
#else
            memcpy(this->vecEvictMatrix[0][y][i], &evict_in[currBufferIdx], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
#if defined(RSSS)
            memcpy(this->vecEvictMatrix[1][y][i], &evict_in[(CLIENT_EVICTION_OUT_LENGTH - sizeof(TYPE_INDEX)) / 2 + currBufferIdx], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
#endif

#if defined(SPDZ)
            memcpy(this->vecEvictMatrix_MAC[0][y][i], &evict_in[currBufferIdx + (H + 1) * evictMatSize * sizeof(TYPE_DATA)], EVICT_MAT_NUM_COL * sizeof(TYPE_DATA));
#endif
#endif
            currBufferIdx += EVICT_MAT_NUM_COL * sizeof(TYPE_DATA);
        }
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
int ServerBinaryORAMO::evict(zmq::socket_t &socket)
{

    recvClientEvictData(socket);

    prepareEvictComputation();

    TYPE_INDEX srcIdx[H];
    TYPE_INDEX destIdx[H];
    TYPE_INDEX siblIdx[H];

    string evict_str = ORAM::getEvictString(n_evict);
    ORAM::getEvictIdx(srcIdx, destIdx, siblIdx, evict_str);

    for (int h = 0; h < H + 1; h++)
    {
        cout << endl;
        cout << "	==============================================================" << endl;
        cout << "	[evict] Starting TripletEviction-" << h + 1 << endl;

        //== THREADS FOR LISTENING =======================================================================================
        cout << "	[evict] Creating Threads for Receiving Ports..." << endl;

#if defined(SEEDING) && defined(RSSS)
        recvSocket_args[0] = struct_socket(0, NULL, 0, reshares_in[0], SERVER_RESHARE_IN_OUT_LENGTH, NULL, false);
        pthread_create(&thread_recv[0], NULL, &ServerORAM::thread_socket_func, (void *)&recvSocket_args[0]);
#else
        for (TYPE_INDEX k = 0; k < NUM_SERVERS - 1; k++)
        {
            recvSocket_args[k] = struct_socket(k, NULL, 0, reshares_in[k], SERVER_RESHARE_IN_OUT_LENGTH, NULL, false);
            pthread_create(&thread_recv[k], NULL, &ServerORAM::thread_socket_func, (void *)&recvSocket_args[k]);
        }
#endif

        TYPE_INDEX curSrcIdx = srcIdx[h];
        TYPE_INDEX curDestIdx = destIdx[h];
        if (h == H) //for src-to-sibling bucket operation at leaf level
        {
            curSrcIdx = srcIdx[H - 1];
            curDestIdx = siblIdx[H - 1];
        }
        // LOAD BUCKETS FROM DISK
        auto start = time_now;

        TYPE_INDEX readBucketIDS[2] = {curSrcIdx, curDestIdx};

        this->readBucket_evict(readBucketIDS, this->serverNo, this->vecEvictPath_db[0], this->vecEvictPath_MAC[0]);
#if defined(RSSS)
        this->readBucket_evict(readBucketIDS, (this->serverNo + 1) % 3, this->vecEvictPath_db[1], this->vecEvictPath_MAC[1]);
#endif
        auto end = time_now;

        long load_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        cout << "	[evict] Evict Nodes READ from Disk in " << load_time << endl;
        server_logs[7] += load_time;

        //perform matrix product
        cout << "	[evict] Multiplying Evict Matrix..." << endl;
        start = time_now;
        this->preReSharing(h, 0, 1); // SERVER SIDE COMPUTATION
        end = time_now;
        cout << "	[evict] Multiplied in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
        server_logs[8] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        start = time_now;
        cout << "	[evict] ReSharing..." << endl;
        this->reShare(h, 0, 1);
        end = time_now;
        cout << "	[evict] Reshared CREATED in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
        server_logs[9] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        //== THREADS FOR SENDING ============================================================================================
        cout << "	[evict] Creating Threads for Sending Shares..." << endl;

#if defined(SEEDING) && defined(RSSS)
        sendSocket_args[1] = struct_socket(1, reshares_out[1], SERVER_RESHARE_IN_OUT_LENGTH, NULL, 0, NULL, true);
        pthread_create(&thread_send[1], NULL, &ServerORAM::thread_socket_func, (void *)&sendSocket_args[1]);
        cout << "	[evict] CREATED!" << endl;
        //=================================================================================================================
        cout << "	[evict] Waiting for Threads..." << endl;
        pthread_join(thread_send[1], NULL);
        pthread_join(thread_recv[0], NULL);
#else
        for (int i = 0; i < NUM_SERVERS - 1; i++)
        {
            sendSocket_args[i] = struct_socket(i, reshares_out[i], SERVER_RESHARE_IN_OUT_LENGTH, NULL, 0, NULL, true);
            pthread_create(&thread_send[i], NULL, &ServerORAM::thread_socket_func, (void *)&sendSocket_args[i]);
        }
        cout << "	[evict] CREATED!" << endl;
        //=================================================================================================================
        cout << "	[evict] Waiting for Threads..." << endl;
        for (int i = 0; i < NUM_SERVERS - 1; i++)
        {
            pthread_join(thread_send[i], NULL);
            pthread_join(thread_recv[i], NULL);
        }
#endif
        cout << "	[evict] DONE!" << endl;
        server_logs[10] += thread_max;
        thread_max = 0;

        start = time_now;
        cout << "	[evict] Post Resharing Computation..." << endl;

        this->postReSharing(h, 0, 1);

        end = time_now;
        server_logs[11] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        //write to file
        start = time_now;
        FILE *file_out = NULL;
        string path;

        //overwrite (non-leaf) sibling bucket with source bucket
        if (h < H - 1)
        {
            this->copyBucket(this->serverNo, curSrcIdx, siblIdx[h]);
#if defined(RSSS)
            this->copyBucket((this->serverNo + 1) % 3, curSrcIdx, siblIdx[h]);
#endif
        }

        this->writeBucket(curDestIdx, serverNo, vecReShares[0][serverNo], vecReShares_MAC[0][serverNo]);

#if defined(RSSS)
        this->writeBucket(curDestIdx, (serverNo + 1) % 3, vecReShares[0][(serverNo + 1) % 3], vecReShares_MAC[0][(serverNo + 1) % 3]);
#endif
        end = time_now;
        server_logs[12] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        cout << "	[evict] Reduction DONE in " << server_logs[11] << endl;
        cout << "	[evict] Written to Disk in " << server_logs[12] << endl;
        cout << "	[evict] TripletEviction-" << h + 1 << " COMPLETED!" << endl;
    }

    prepareRandLinComb(lin_rand_com_out);

#if defined(SPDZ)
    socket.send(lin_rand_com_out, 2 * sizeof(TYPE_DATA));
#else // RSSS
    socket.send(lin_rand_com_out, 4 * sizeof(TYPE_DATA));
#endif

    cout << "	[evict] ACK is SENT!" << endl;

    return 0;
}

int ServerBinaryORAMO::readBucket_evict(TYPE_INDEX bucketIDs[], int shareID, zz_p **output_data, zz_p **output_mac)
{

    FILE *file_in = NULL;
    FILE *file_in_mac = NULL;

    string path, path_mac;
    for (int s = 0; s < 2; s++)
    {
        path = myStoragePath + to_string(shareID) + "/" + to_string(bucketIDs[s]);
        if ((file_in = fopen(path.c_str(), "rb")) == NULL)
        {
            cout << path << " cannot be opened!!" << endl;
            exit;
        }
#if !defined(REVERSE_STORAGE_LAYOUT)
        for (int i = 0; i < BUCKET_SIZE; i++)
        {
            for (int j = 0; j < DATA_CHUNKS; j++)
            {
                fread(&output_data[j][s * BUCKET_SIZE + i], 1, sizeof(TYPE_DATA), file_in);
            }
        }
#else
        for (int j = 0; j < DATA_CHUNKS; j++)
        {
            fread(&output_data[j][s * BUCKET_SIZE], 1, BUCKET_SIZE * sizeof(TYPE_DATA), file_in);
        }
#endif
        fclose(file_in);

        path_mac = path + "_mac";
        if ((file_in_mac = fopen(path_mac.c_str(), "rb")) == NULL)
        {
            cout << path_mac << " cannot be opened!!" << endl;
            exit;
        }
#if !defined(REVERSE_STORAGE_LAYOUT)
        for (int i = 0; i < BUCKET_SIZE; i++)
        {
            for (int j = 0; j < DATA_CHUNKS; j++)
            {
                fread(&output_mac[j][s * BUCKET_SIZE + i], 1, sizeof(TYPE_DATA), file_in_mac);
            }
        }
#else
        for (int j = 0; j < DATA_CHUNKS; j++)
        {
            fread(&output_mac[j][s * BUCKET_SIZE], 1, BUCKET_SIZE * sizeof(TYPE_DATA), file_in_mac);
        }
#endif
        fclose(file_in_mac);
    }
}

int ServerBinaryORAMO::writeRoot(zmq::socket_t &socket)
{
    cout << "	[recvBlock] Receiving Block Data..." << endl;
    auto start = time_now;
    long long n = 2 * BLOCK_SIZE;
#if defined(RSSS) && !defined(SEEDING)
    n = n * 2;
#endif;
    long long m = sizeof(TYPE_INDEX);
#if defined(SEEDING)
    if (serverNo == 0)
        socket.recv(write_root_in, n + m, 0);
    else
    {
#if defined(RSSS)
        if (serverNo == 2)
        {
            m = m + n;
        }
#endif
        socket.recv(write_root_in, m, 0);
    }
    auto end = time_now;
    TYPE_INDEX slotIdx;
    memcpy(&slotIdx, &write_root_in[0], sizeof(TYPE_INDEX));

    cout << "	[recvBlock] Block Data RECV in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
    server_logs[4] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    start = time_now;
    zz_p tmp2;

    unsigned long long tmp;

#if defined(RSSS)
    if (serverNo == 2)
    {
        this->updateRoot((serverNo + 1) % 3, slotIdx, &write_root_in[sizeof(TYPE_INDEX)], &write_root_in[sizeof(TYPE_INDEX) + BLOCK_SIZE]);
    }
    else
    {
        unsigned char *tmp_write_root_in = new unsigned char[BLOCK_SIZE * 2];
        memset(tmp_write_root_in, 0, BLOCK_SIZE * 2);
        for (int i = 0; i < DATA_CHUNKS; i++)
        {
            sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[(serverNo + 1) % 3]);
            tmp2 = tmp;
            memcpy(&tmp_write_root_in[i * sizeof(TYPE_DATA)], &tmp2, sizeof(TYPE_DATA));

            sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[(serverNo + 1) % 3]);
            tmp2 = tmp;
            memcpy(&tmp_write_root_in[i * sizeof(TYPE_DATA) + BLOCK_SIZE], &tmp2, sizeof(TYPE_DATA));
        }
        this->updateRoot((serverNo + 1) % 3, slotIdx, &tmp_write_root_in[0], &tmp_write_root_in[BLOCK_SIZE]);
        delete[] tmp_write_root_in;
    }

#endif

    if (serverNo == 0)
    {
    }
    else
    {
        for (int i = 0; i < DATA_CHUNKS; i++)
        {
            sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[(serverNo) % 3]);
            tmp2 = tmp;
            memcpy(&write_root_in[sizeof(TYPE_DATA) + i * sizeof(TYPE_DATA)], &tmp2, sizeof(TYPE_DATA));

            sober128_read((unsigned char *)&tmp, sizeof(TYPE_DATA), &prng_client[(serverNo) % 3]);
            tmp2 = tmp;
            memcpy(&write_root_in[sizeof(TYPE_DATA) + i * sizeof(TYPE_DATA) + BLOCK_SIZE], &tmp2, sizeof(TYPE_DATA));
        }
    }
    this->updateRoot(serverNo, slotIdx, &write_root_in[sizeof(TYPE_INDEX)], &write_root_in[BLOCK_SIZE + sizeof(TYPE_INDEX)]);

    end = time_now;
    cout << "	[recvBlock] Block STORED in Disk in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
    server_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    socket.send((unsigned char *)CMD_SUCCESS, sizeof(CMD_SUCCESS));
    cout << "	[recvBlock] ACK is SENT!" << endl;
#else

    socket.recv(write_root_in, n + sizeof(TYPE_INDEX), 0);
    auto end = time_now;
    TYPE_INDEX slotIdx;
    memcpy(&slotIdx, &write_root_in[n], sizeof(TYPE_INDEX));

    cout << "	[recvBlock] Block Data RECV in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
    server_logs[4] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    start = time_now;
    this->updateRoot(serverNo, slotIdx, &write_root_in[0], &write_root_in[BLOCK_SIZE]);
#if defined(RSSS)
    this->updateRoot((serverNo + 1) % 3, slotIdx, &write_root_in[n / 2], &write_root_in[n / 2 + BLOCK_SIZE]);
#endif
    end = time_now;
    cout << "	[recvBlock] Block STORED in Disk in " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << endl;
    server_logs[5] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    socket.send((unsigned char *)CMD_SUCCESS, sizeof(CMD_SUCCESS));
    cout << "	[recvBlock] ACK is SENT!" << endl;
#endif
    return 0;
}
