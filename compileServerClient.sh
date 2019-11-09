#! /bin/bash
g++ ClientHome/client_sample.cpp -o ClientHome/client `pkg-config --cflags --libs opencv`
g++ ServerHome/selectSock.cpp -o ServerHome/server `pkg-config --cflags --libs opencv`