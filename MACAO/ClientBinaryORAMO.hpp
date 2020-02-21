/*
 * ClientBinaryORAMO.hpp
 *
 *      Author: thanghoang
 */

#ifndef CLIENTBINARYORAMO_HPP
#define CLIENTBINARYORAMO_HPP

#include "ClientORAM.hpp"
#include "config.h"

class ClientBinaryORAMO : public ClientORAM
{
protected:
    unsigned char **write_root_out;

public:
    ClientBinaryORAMO();
    ~ClientBinaryORAMO();

    int access(TYPE_INDEX blockID);
    //eviction_matrix
    int getEvictMatrix();

    int updatePosMap(TYPE_INDEX blockID);

    int writeRoot();
    int evict();
};

#endif // CLIENTORAM_HPP
