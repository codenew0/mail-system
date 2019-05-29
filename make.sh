#########################################################################
# File Name: make.sh
# Author: codenew
# mail: codenew0@gmail.com
# Created Time: Tue 21 Oct 2014 06:24:02 PM CST
#########################################################################
#!/bin/bash

exec gcc -o 1 -Wall `pkg-config --libs --cflags gtk+-2.0` -lpthread -lm gtk_login.c table.c register.c forgetpwd.c connector.c recvmail.c update.c help.c compose.c sentmail.c draft.c trash.c 

