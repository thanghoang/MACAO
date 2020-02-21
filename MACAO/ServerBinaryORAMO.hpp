/*
 * ServerBinaryORAMO.hpp
 *
 *      Author: thanghoang
 */

#ifndef SERVERBINARYORAMO_HPP
#define SERVERBINARYORAMO_HPP

#include "ServerORAM.hpp"

#include "config.h"
#include <zmq.hpp>
#include <pthread.h>

class ServerBinaryORAMO : public ServerORAM
{
private:
public:
    ServerBinaryORAMO();
    ServerBinaryORAMO(TYPE_INDEX serverNo, int selectedThreads);
    ~ServerBinaryORAMO();

    // main functions
    int evict(zmq::socket_t &socket);

    // eviction subroutine
    int readBucket_evict(TYPE_INDEX BucketIDs[], int shareID, zz_p **output_data, zz_p **output_mac);

    int writeRoot(zmq::socket_t &socket);
    int prepareEvictComputation();
};

#endif // SERVERORAM_HPP
