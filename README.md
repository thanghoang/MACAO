# MACAO: A Maliciously-Secure and Client-Efficient Active ORAM Framework

This repository  contains the implementation of the [**MACAO framework**](https://eprint.iacr.org/2020/203) to appear in NDSS'20.
This project is built on [CodeLite IDE](http://codelite.org). It is recommended to install CodeLite to load the full MACAO workspace. 

## Updates

* 2020/02/21: Version 1.0 released


# Required Libraries
1. [NTL v9.10.0](http://www.shoup.net/ntl/download.html)

2. [ZeroMQ](http://zeromq.org/intro:get-the-software)

3. [Tomcrypt](https://github.com/libtom/libtomcrypt)

# Configuration
All MACAO Framework configurations are located in ```MACAO/config.h```. 


## Using eviction paradigms
In ```MACAO/config.h```, enable either the macro ``#define CORAM_LAYOUT``  to use Circuit-ORAM eviction principle, or macro ``#define TRIPLET_EVICTION`` to use triplet eviction principle

## Using different MACAO schemes
In ```MACAO/config.h```, enable the macros ``#define XOR_PIR`` and  ``#define RSSS`` (and disable ``#define SPDZ``) to use *XOR-PIR for retrieval*, and *replicated sercret sharing scheme (RSSS)'s multiplication for eviction*. 

Enable macro  ``#define RSSS`` (and disable ``#define SPDZ`` and ``#define XOR_PIR``) to use *RSSS for both retrieval and eviction*.

Enable macro ``#define SPDZ`` (and disable ``#define RSSS`` and ``#define XOR_PIR``) to use *SPDZ for both retrieval and eviction*.

Enable macro ``#define SEEDING`` to use PRF to generate additive shares locally in each server machine.







## Some Important  Parameters:
```

#define BLOCK_SIZE 128                                -> Data block size (in bytes)

#define HEIGHT 4                                      -> Height of MACAO Tree structure

static const unsigned long long P = 1073742353;       -> Prime field (size should be equal to the defined TYPE_DATA)

#define NUM_SERVERS 3                                 -> Number of servers \ell (only change this if using SPDZ)


const std::string SERVER_ADDR[NUM_SERVERS]            -> Server IP addresses
#define SERVER_PORT 5555                              -> Define the first port for incremental to generate other ports for client-server / server-server communications

const TYPE_DATA GLOBAL_MAC_KEY                        -> Define global MAC key

const TYPE_DATA MAC_KEY[NUM_SERVERS]                  -> Define shares of global MAC key (sum of shares_mac_key = global MAC key)

static string CLIENT_SERVER_SEED[NUM_SERVERS]         -> seed being shared between the cliet and each server for additive secret share generation by PRF
static string SERVER_SERVER_SEED[NUM_SERVER][NUM_SERVERS] -> seed being shared between the servers for additive secret share generation by PRF (used in RSSS)

```




# Build & Compile
Goto folder ``MACAO/`` and execute
``` 
make
```

, which produces the binary executable file named ```MACAO``` in ``MACAO/Debug/``.

# ORAM Data Location
All necessary data by the client setup phase are output to the ``MACAO/data`` folder (by default, which can be changed by editing the ``config.h`` under ``PATHS`` section). WE refer to ``MACAO/data.zip`` as an example of the structure of the folder.

# Usage

Run the binary executable file ```MACAO```, which will ask for either Client or Server mode. The MACAO implementation can be tested using either **single** machine or **multiple** machines:


## Local Testing:
1. Set ``SERVER_ADDR`` in ``MACAO/config.h`` to ``localhost``. 
2. Set ``SERVER_PORT``
3. Compile the code with ``make`` in the ``MACAO/`` folder. 
4. Go to ``MACAO/Debug`` and run the compiled ``MACAO`` file in different Terminals, each playing the client/server role.

## Real Network Testing:
1. Set ``SERVER_ADDR`` in ``MACAO/config.h`` to the real IP address of each server machine. 
1. Copy the binary file ``MACAO`` compiled under the same configuration to running machines. 
2. For first time usage, run the ``MACAO/Debug/MACAO`` file on the *client* machine to initialize the MACAO structure first.
3. Copy the folder the data generated in ``MACAO/data/xxx/`` to corresponding server
4. For each server, run the compiled file ``MACAO`` and select the server role (option 2) and the corresponding ID ``i``.



## Citing

If the paper and the code is found useful, we would be appreciated if our paper can be cited with the following bibtex format 

```
@inproceedings{hoang20:MACAO, 
author = {Hoang, Thang and Guajardo, Jorge and Yavuz, Attila A.}, 
title = {MACAO: A Maliciously-Secure and Client-Efficient Active ORAM Framework}, 
year = {2020}, 
issue_date = {February 2020}, 
url = {https://doi.org/10.14722/ndss.2020.24313}, doi = {10.14722/ndss.2020.24313}, 
booktitle = {Network and Distributed Systems Security (NDSS) Symposium 2020}, 
}
```


# Further Information
For any inquiries, bugs, and assistance on building and running the code, please contact me at [hoangm@mail.usf.edu](mailto:hoangm@mail.usf.edu?Subject=[Extended%20S3ORAM]%20Inquiry).
