/*************************************************************************
	> File Name: connector.h
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Sun 19 Oct 2014 09:25:52 PM CST
 ************************************************************************/

#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#define CONNECTOR_LEN 60

typedef struct {
	char account[CONNECTOR_LEN];
	char connector[CONNECTOR_LEN];
	char nickname[CONNECTOR_LEN];
}connector_t;

#endif

