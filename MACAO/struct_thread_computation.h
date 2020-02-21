/*
 * struct_thread_crossProd.h
 *
 *      Author: thanghoang
 */

#ifndef STRUCT_THREAD_CROSSPRODUCT_H
#define STRUCT_THREAD_CROSSPRODUCT_H

#include "config.h"
typedef struct struct_thread_computation
{
    //thread
    TYPE_INDEX startIdx, endIdx;

    //retrieval

    zz_p **retrieval_db_input;
    zz_p **retrieval_mac_input;

    unsigned char *retrieval_xor_query;
    unsigned char *retrieval_xor_output;
    unsigned char *retrieval_xor_mac_output;

    zz_p *retrieval_sss_query;
    zz_p *retrieval_sss_output;

    zz_p *retrieval_query;
    zz_p *output;

    // eviction
    zz_p **evict_db_input;
    zz_p **evict_matrix_input; // cross product matrix

    zz_p **evict_output; // cross product output

    int input_length;
    int output_length;

    //FOR XOR_PIR retrieval
    struct_thread_computation(TYPE_INDEX startIdx, TYPE_INDEX endIdx, zz_p **retrieval_db_input, zz_p **retrieval_mac_input, unsigned char *retrieval_xor_query, unsigned char *retrieval_xor_output, unsigned char *retrieval_xor_mac_output)
    {
        this->startIdx = startIdx;
        this->endIdx = endIdx;
        this->retrieval_db_input = retrieval_db_input;
        this->retrieval_mac_input = retrieval_mac_input;
        this->retrieval_xor_query = retrieval_xor_query;

        this->retrieval_xor_output = retrieval_xor_output;
        this->retrieval_xor_mac_output = retrieval_xor_mac_output;
    }

    //FOR SSS retrieval
    struct_thread_computation(TYPE_INDEX startIdx, TYPE_INDEX endIdx, zz_p **retrieval_db_input, int input_length, int output_length, zz_p *retrieval_sss_query, zz_p *retrieval_output)
    {
        this->startIdx = startIdx;
        this->endIdx = endIdx;

        this->retrieval_sss_query = retrieval_sss_query;

        this->retrieval_db_input = retrieval_db_input;

        this->retrieval_sss_output = retrieval_output;

        this->output_length = output_length;
        this->input_length = input_length;
    }

    struct_thread_computation(TYPE_INDEX startIdx, TYPE_INDEX endIdx, zz_p **evict_db_input, int input_length, int output_length, zz_p **evict_matrix_input, zz_p **evict_output)
    {
        this->evict_matrix_input = evict_matrix_input;
        this->evict_db_input = evict_db_input;

        this->startIdx = startIdx;
        this->endIdx = endIdx;

        this->input_length = input_length;
        this->evict_output = evict_output;
        this->output_length = output_length;
    }

    struct_thread_computation()
    {
    }
    ~struct_thread_computation()
    {
    }

} THREAD_COMPUTATION;

#endif