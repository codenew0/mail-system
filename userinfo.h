/*************************************************************************
	> File Name: userinfo.h
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 13 Oct 2014 09:31:57 PM CST
 ************************************************************************/

#ifndef _USERINFO_H_
#define _USERINFO_H_

#include <string.h>

#define COMMON_LEN 60 

enum {
	SEX_MALE = 1,
	SEX_FEMALE
};

typedef struct {
	char account[COMMON_LEN];
	char passwd[COMMON_LEN];
	char new_passwd[COMMON_LEN];
	char nickname[COMMON_LEN];
	int birthday;
	int gender;
	int safequestion;
	char answer[COMMON_LEN];
}userInfo_t;

#endif

