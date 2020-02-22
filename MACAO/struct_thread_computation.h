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

/**
 * FOR XOR_PIR retrieval
 * 
 * @param startIdx: starting index to be processed (for multi-thread)
 * @param endIdx: ending index to be processed (for multi-thread)
 * @param retrieval_db_input: database input for xor-pir 
 * @param retrieval_mac_input: mac of database input for xor-pir
 * @param retrieval_xor_query: xor-pir query
 * @param retrieval_xor_output: xor-pir computation result on database
 * @param retrieval_xor_mac_output: xor-pir computation result on mac of database
 *
 * @return 0 if successful
 */
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

/**
 * FOR PIR retrieval based on additive secret sharing
 * 
 * @param startIdx: starting index to be processed (for multi-thread)
 * @param endIdx: ending index to be processed (for multi-thread)
 * @param retrieval_db_input: database input for xor-pir 
 * @param input_length: length of database input
 * @param output_length: length of computation output
 * @param retrieval_sss_query: pir query (created by additive secret sharing)
 * @param retrieval_output: output of asss-pir computation
 *
 * @return 0 if successful
 */    
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
    
/**
 * FOR eviction  based on additive secret sharing
 * 
 * @param startIdx: starting index to be processed (for multi-thread)
 * @param endIdx: ending index to be processed (for multi-thread)
 * @param evict_db_input: database input for eviction
 * @param input_length: length of database input
 * @param output_length: length of computation output
 * @param evict_matrix_input: permutation matrix input
 * @param evict_output: output of eviction muliplitcation 
 *
 * @return 0 if successful
 */  
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