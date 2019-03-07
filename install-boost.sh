#!/bin/sh
set -ex

wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
tar -xzvf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh --prefix=/usr/local
sudo ./b2 install
