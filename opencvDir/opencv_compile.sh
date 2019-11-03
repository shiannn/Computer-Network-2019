#! /bin/bash

g++ opencv_sample.cpp -o opencv_sample `pkg-config --cflags --libs opencv`