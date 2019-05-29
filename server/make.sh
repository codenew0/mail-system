#########################################################################
# File Name: make.sh
# Author: codenew
# mail: codenew0@gmail.com
# Created Time: Wed 22 Oct 2014 08:28:16 PM CST
#########################################################################
#!/bin/bash

gcc -o 1 `pkg-config --libs --cflags gtk+-2.0` server.c -L/usr/lib64/mysql -lmysqlclient

#gcc -o 1 `pkg-config --libs --cflags gtk+-2.0` server.c -L/usr/lib/mysql -lmysqlclient
