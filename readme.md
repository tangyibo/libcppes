cppes
=================

C++ Client for Elasticsearch
----------------------------

  cppes is a small C++ API for elasticsearch. 


## Get Source and Build ##
-----------

```
git clone https://github.com/tangyibo/libcppes.git
cd libcppes/
```

Dependencies
------------
- 1. The current version works on Linux platform;
- 2. jsoncpp which is JSON parser library(already contained in this source code)
- 3. libcurl library(which depend on libz and openssl) for HTTP connection

Build
-------

```
cd libcppes/deps
make clean
make all

cd ../
make clean
make all
```

