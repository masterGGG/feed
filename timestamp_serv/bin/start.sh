#!/bin/sh
export LD_LIBRARY_PATH=../lib/ini-file-1.1.1
./timestamp_serv ../conf/bench.conf ./libtimestamp_serv.so ../conf/timestamp_serv.conf
