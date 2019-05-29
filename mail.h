/*************************************************************************
	> File Name: mail.h
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 13 Oct 2014 08:37:38 PM CST
 ************************************************************************/

#ifndef _MAIL_H_
#define _MAIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <strings.h>
#include "userinfo.h"
#include "mailinfo.h"
#include "connector.h"

#define SERVER_PORT 8888
#define SERVER_IP "192.168.2.189"
#define err_sys(x) \
	do { \
		perror(x); \
		exit(1); \
	}while (0);

enum c2s_type_t {
	C2S_REGISTER,
	C2S_LOGIN,
	C2S_FORGETPASSWD,
	C2S_SENDMAIL,
	C2S_SHOWMAIL,
	C2S_DELMAIL,
	C2S_PUTTRASH,
	C2S_HASSENDMAIL,
	C2S_LISTSENDMAIL,
	C2S_LISTRECVMAIL,
	C2S_LISTDRAFTMAIL,
	C2S_LISTTRASHMAIL,
	C2S_LISTCONNECTOR,
	C2S_SEARCH,
	C2S_SAVEDRAFT,
	C2S_ADDCONNECTOR,
	C2S_DELCONNECTOR,
	C2S_SHOWCONTENT, 
	C2S_FORWARDMAIL
};

enum mail_box_type_t {
	C2S_SEND_MAIL,
	C2S_RECV_MAIL,
	C2S_DRAFT,
	C2S_TRASH
};

typedef struct {
	enum c2s_type_t type;
	enum mail_box_type_t mailtype;
	int id;
	char search[30];
	connector_t connector;
	mail_t mail;
	userInfo_t info;
}c2s_t;

/*********************************************/
//server to client 's reply for listall,liston,listoff,searchid, searchname, searchage

//server to client's reply for register
enum s2c_register_type_t {
	S2C_REGISTER_REPEAT, S2C_REGISTER_SUCCESS
};
typedef struct {
	enum s2c_register_type_t type;
}s2c_register_t;

//server to client's reply for login
enum s2c_login_type_t {
	S2C_LOGIN_FAIL, S2C_LOGIN_SUCCESS
};
typedef struct {
	enum s2c_login_type_t type;
}s2c_login_t;

//server to client's reply for forgetpasswd
enum s2c_forget_type_t {
	S2C_ACCOUNT_NOEXIST, S2C_QUESTION_ERROR,
	S2C_MODIFY_SUCCESS
};
typedef struct {
	enum s2c_forget_type_t type;
}s2c_forget_t;

enum s2c_view_type_t{
	S2C_VIEW_SUCCESS, S2C_VIEW_FAIL,
	S2C_VIEW_END
};

typedef struct {
	enum s2c_view_type_t type;
	mail_t mail;
}s2c_view_t;

typedef struct {
	enum s2c_view_type_t type;
	connector_t connector;
}s2c_connectorview_t;

enum s2c_send_type_t {
	S2C_SEND_SUCCESS, S2C_SEND_FAIL, 
	S2C_SENDER_NOEXIST
};

typedef struct {
	enum s2c_send_type_t type;
}s2c_sendmail_t;

enum s2c_return_type_t {
	S2C_SUCCESS, S2C_FAIL
};

typedef struct {
	enum s2c_return_type_t type;
}s2c_connector_t;

enum s2c_addconnector_type_t {
	S2C_CONNECTOR_NOEXIST, S2C_ADD_SUCCESS,
	S2C_CONNECTOR_REPEAT
};

typedef struct {
	enum s2c_addconnector_type_t type;
}s2c_addconnector_t;

typedef struct {
	enum s2c_return_type_t type;
}s2c_delete_t;

//server to client 's reply for add
/*
enum s2c_add_type_t {
	S2C_ADD_ACCEPT, S2C_ADD_REJECT
};
typedef struct {
	enum s2c_add_type_t type;
	staff_info_t info;
}s2c_add_t;

//server to client 's reply for del
enum s2c_del_type_t {
	S2C_DEL_ACCEPT, S2C_DEL_REJECT
};
typedef struct {
	enum s2c_del_type_t type;
}s2c_del_t;
*/
void login_account(GtkWidget*, gpointer);
void register_account(GtkWidget*, gpointer);
void forget_passwd(GtkWidget*, gpointer);
void table(const gchar*);
GdkPixbuf *create_pixbuf(const gchar*);
void update_widget_bg(GtkWidget *, const gchar *);
void recv_mail(const gchar*);
void compose(const gchar*);
void sent_mail(const gchar*);
void connector(const gchar*);
void draft_box(const gchar*);
void trash_box(const gchar*);
void setting(const gchar*);
void help();

#endif

