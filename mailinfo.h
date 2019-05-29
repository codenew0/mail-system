/*************************************************************************
	> File Name: mailinfo.h
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 20 Oct 2014 06:22:42 PM CST
 ************************************************************************/

#ifndef _MAILINFO_H_
#define _MAILINFO_H_

#define MAIL_LEN 60
#define MAIL_CONTENT_LEN 2000
#define DATE_LEN 30

typedef struct {
	char title[MAIL_LEN];
	char sender[MAIL_LEN];
	char content[MAIL_CONTENT_LEN];
	char date[DATE_LEN];
}mail_t;

#endif

