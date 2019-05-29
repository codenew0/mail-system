#include "../mail.h"
#include <mysql/mysql.h>

void init_network(int);
void destroy_network();
void server_accept();

#define DB_NAME				"user"
#define USER_TABLE			"userinfo"

int sfd = -1;	//TCP网络套接字
MYSQL *mysql = NULL;
pthread_rwlock_t rwlock;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Need 2 arguments\n");
		exit(1);
	}
	init_network(atoi(argv[1]));

	mysql = mysql_init(NULL);
	MYSQL *ret = mysql_real_connect(mysql, "localhost", "root", "123456", DB_NAME, 0, 0, 0);
	MYSQL_RES *res;
	MYSQL_ROW row;
	if (ret == NULL)
	{
		fprintf(stderr, "Connect database failed\n");
		return;
	}
	
	pthread_rwlock_init(&rwlock, NULL);

	while (1) {
		server_accept();
	}	

	mysql_close(mysql);
	destroy_network();
	return 0;
}

void init_network(int num)
{
	struct sockaddr_in addr;
	int ret;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sfd < 0 ) {
		perror("socket()");
		exit(1);
	}
	ret = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(ret));
	
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(sfd, (struct sockaddr*)&addr, sizeof(addr));
	if ( ret < 0 ) {
		perror("bind()");
		exit(1);
	}

	listen(sfd, num);
}
void destroy_network()
{
	close(sfd);
}

void * thr_func(void*);
void server_accept()
{
	int afd;
	struct sockaddr_in addr;
	pthread_t tid;
	socklen_t addr_len;
	bzero(&addr, sizeof(addr));
	addr_len = sizeof(addr);
	afd = accept(sfd, (struct sockaddr*)&addr, &addr_len);	
	if ( afd < 0 ) {
		perror("accept()");
		exit(1);
	}
	fprintf(stderr, "connect with %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	pthread_create(&tid, NULL, thr_func, (void*)afd);
}

int search_mail(char *row, char *str)
{
	int n = strlen(row);
	int m = strlen(str);
	int found = 0, i, j;

	int td[128];
	for (i = 0; i < 128; i++)
		td[i] = m + 1;
	char *p;
	for (p = str; *p; p++)
		td[*p] = m - (p - str);
	const char *t, *tx = row;
	while (tx + m <= row + n){
		for (p = str, t = tx; *p; ++p, ++t)
		{
			if (*p != *t)
				break;
		}
		if (*p == 0)
		{
			found = 1;
			break;
		}
		tx += td[tx[m]];
	}

	return found;
}

void * thr_func(void *arg)
{
	c2s_t rcv_msg;
	int rt;
	int afd = (int)arg;
	char sql_cmd[128];
	int len;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	while (1) {
		bzero(&rcv_msg, sizeof(rcv_msg));
		rt = recv(afd, &rcv_msg, sizeof(rcv_msg), 0); 
		if ( rt <= 0 ) {
			break;
		}
		switch ( rcv_msg.type ) {
		case C2S_REGISTER:
			{
				s2c_register_t snd_msg;
				char query[128];
				pthread_rwlock_wrlock(&rwlock);
				sprintf(query, "select * from %s where account='%s'", USER_TABLE, rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, strlen(query));
				if (rt != 0)
				{
					fprintf(stderr, "Select err\n");
					exit(1);
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					exit(1);
				}
				if (row = mysql_fetch_row(res))
				{
					snd_msg.type = S2C_REGISTER_REPEAT;
					pthread_rwlock_unlock(&rwlock);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				memset(query, 0, sizeof(query));
				len = sprintf(query, "insert into %s values('%s','%s','%s',%d,%d,%d,'%s')", 
						USER_TABLE, rcv_msg.info.account, rcv_msg.info.passwd, rcv_msg.info.nickname,
						rcv_msg.info.gender, rcv_msg.info.birthday, rcv_msg.info.safequestion, 
						rcv_msg.info.answer);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Insert error\n");
					return;
				}
				len = sprintf(query, "create table %s_recv_table (sender varchar(60),"
					  		"title varchar(60), content varchar(1000), date varchar(30))", 
							rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "create recv table failed\n");
					return;
				}
				len = sprintf(query, "create table %s_send_table (sender varchar(60),"
					  		"title varchar(60), content varchar(1000), date varchar(30))", 
							rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "create send table failed\n");
					return;
				}
				len = sprintf(query, "create table %s_draft_table (sender varchar(60),"
					  		"title varchar(60), content varchar(1000), date varchar(30))", 
							rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "create draft table failed\n");
					return;
				}
				len = sprintf(query, "create table %s_trash_table (sender varchar(60),"
					  		"title varchar(60), content varchar(1000), date varchar(30))", 
							rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "create trash table failed\n");
					return;
				}
				len = sprintf(query, "create table %s_connector_table ("
					  		"connector varchar(60), nickname varchar(60))", 
							rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "create connector table failed\n");
					return;
				}
				snd_msg.type = S2C_REGISTER_SUCCESS;
				pthread_rwlock_unlock(&rwlock);
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_LOGIN:
			{
				s2c_login_t snd_msg;
				char query[128];
				pthread_rwlock_rdlock(&rwlock);
				len = sprintf(query, "select * from %s where account='%s' and passwd='%s'", 
							USER_TABLE, rcv_msg.info.account, rcv_msg.info.passwd);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Select error\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (!(row = mysql_fetch_row(res)))
				{
					snd_msg.type = S2C_LOGIN_FAIL;
					pthread_rwlock_unlock(&rwlock);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_LOGIN_SUCCESS;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_FORGETPASSWD:
			{
				s2c_forget_t snd_msg;
				pthread_rwlock_wrlock(&rwlock);
				char query[128];
				len = sprintf(query, "select * from %s where account='%s'", USER_TABLE, rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Account query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (!(row = mysql_fetch_row(res)))
				{
					pthread_rwlock_unlock(&rwlock);
					snd_msg.type = S2C_ACCOUNT_NOEXIST;
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				memset(query, 0, sizeof(query));
				len = sprintf(query, "select * from %s where account='%s' and safequestion=%d and answer='%s'", 
								USER_TABLE, rcv_msg.info.account, rcv_msg.info.safequestion, rcv_msg.info.answer);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "question query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (!(row = mysql_fetch_row(res)))
				{
					pthread_rwlock_unlock(&rwlock);
					snd_msg.type = S2C_QUESTION_ERROR;
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				memset(query, 0, sizeof(query));
				len = sprintf(query, "update %s set passwd='%s' where account='%s'", 
							 USER_TABLE, rcv_msg.info.new_passwd, rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Passwd modify failed\n");
					return;
				}
				pthread_rwlock_unlock(&rwlock);
				snd_msg.type = S2C_MODIFY_SUCCESS;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_SENDMAIL:
			{
				s2c_sendmail_t snd_msg;
				char query[128];
				char table[64];
				pthread_rwlock_wrlock(&rwlock);
				//store mail
				sprintf(query, "select * from %s where account='%s'", USER_TABLE, rcv_msg.mail.sender);
				rt = mysql_real_query(mysql, query, strlen(query));
				if (rt != 0)
				{
					fprintf(stderr, "Select error\n");
					exit(1);
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					exit(1);
				}
				if (!(row = mysql_fetch_row(res)))
				{
					snd_msg.type = S2C_SENDER_NOEXIST;
					pthread_rwlock_unlock(&rwlock);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				sprintf(table, "%s_recv_table", rcv_msg.mail.sender);
				len = sprintf(query, "insert into %s(title, sender, content, date) values('%s', '%s', '%s', '%s')", 
						table, rcv_msg.mail.title, rcv_msg.info.account, 
						rcv_msg.mail.content, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Insert %s error\n", table);
					exit(1);
				}
				memset(query, 0, sizeof(query));
				memset(table, 0, sizeof(table));
				sprintf(table, "%s_send_table", rcv_msg.info.account);
				len = sprintf(query, "insert into %s(title, sender, content, date) values('%s', '%s', '%s', '%s')", 
						table, rcv_msg.mail.title, rcv_msg.mail.sender, 
						rcv_msg.mail.content, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					snd_msg.type = S2C_SEND_FAIL;
					fprintf(stderr, "Insert %s error\n", table);
					return;
				}
				else
					snd_msg.type = S2C_SEND_SUCCESS;
				pthread_rwlock_unlock(&rwlock);
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_DELMAIL:
			{
				pthread_rwlock_wrlock(&rwlock);
				char table[64];
				char type[10];
				char query[128];
				s2c_delete_t snd_msg;

				switch (rcv_msg.mailtype)
				{
					case C2S_SEND_MAIL:
						strcpy(type, "send");
						break;
					case C2S_RECV_MAIL:
						strcpy(type, "recv");
						break;
					case C2S_DRAFT:
						strcpy(type, "draft");
						break;
					case C2S_TRASH:
						strcpy(type, "trash");
						break;
				}
				sprintf(table, "%s_%s_table", rcv_msg.info.account, type);
				len = sprintf(query, "delete from %s where date='%s'", 
							  table, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "delete %s failed\n", table);
					return;
				}
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_SUCCESS;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_SEARCH:
			{
				pthread_rwlock_rdlock(&rwlock);
				char table[64];
				char type[10];
				char query[128];
				int i, j, found = 0;
				s2c_view_t snd_msg;

				switch (rcv_msg.mailtype)
				{
					case C2S_SEND_MAIL:
						strcpy(type, "send");
						break;
					case C2S_RECV_MAIL:
						strcpy(type, "recv");
						break;
					case C2S_DRAFT:
						strcpy(type, "draft");
						break;
					case C2S_TRASH:
						strcpy(type, "trash");
						break;
				}
				sprintf(table, "%s_%s_table", rcv_msg.info.account, type);
				len = sprintf(query, "select title, sender, content, date from %s", table);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				int search_len = strlen(rcv_msg.search);
				int flag = 0;
				while (row = mysql_fetch_row(res))
				{
					if (!search_mail(row[0], rcv_msg.search) && !search_mail(row[1], rcv_msg.search)\
						&& !search_mail(row[2], rcv_msg.search) && !search_mail(row[3], rcv_msg.search))
						continue;
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.mail.title, row[0]);
					strcpy(snd_msg.mail.sender, row[1]);
					strcpy(snd_msg.mail.content, row[2]);
					strcpy(snd_msg.mail.date, row[3]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					flag = 1;
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				if (flag == 0)
					snd_msg.type = S2C_VIEW_FAIL;
				else
					snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_PUTTRASH:
			{
				pthread_rwlock_wrlock(&rwlock);
				char table[64];
				char type[10];
				char query[128];
				s2c_delete_t snd_msg;
				//Modify
				switch (rcv_msg.mailtype)
				{
					case C2S_SEND_MAIL:
						strcpy(type, "send");
						break;
					case C2S_RECV_MAIL:
						strcpy(type, "recv");
						break;
					case C2S_DRAFT:
						strcpy(type, "draft");
						break;
					case C2S_TRASH:
						strcpy(type, "trash");
						break;
				}
				sprintf(table, "%s_%s_table", rcv_msg.info.account, type);
				len = sprintf(query, "select title, sender, content from %s where date='%s'", 
							  table, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (row = mysql_fetch_row(res))
				{
					len = sprintf(query, "insert into %s_trash_table(title, sender, content, date)"
							"values('%s', '%s', '%s', '%s')", 
							rcv_msg.info.account, row[0], row[1], row[2], rcv_msg.mail.date);
					rt = mysql_real_query(mysql, query, len);
					if (rt != 0)
					{
						fprintf(stderr, "delete %s failed\n", table);
						return;
					}
				}
				len = sprintf(query, "delete from %s where date='%s'", 
							  table, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "delete %s failed\n", table);
					return;
				}
				snd_msg.type = S2C_SUCCESS;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				pthread_rwlock_unlock(&rwlock);
				break;
			}
		case C2S_LISTSENDMAIL:
			{
				s2c_view_t snd_msg;
				pthread_rwlock_rdlock(&rwlock);
				char query[128];
				len = sprintf(query, "select title, sender, content, date from %s_send_table", 
							  rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				while (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.mail.title, row[0]);
					strcpy(snd_msg.mail.sender, row[1]);
					strcpy(snd_msg.mail.content, row[2]);
					strcpy(snd_msg.mail.date, row[3]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_LISTRECVMAIL:
			{
				s2c_view_t snd_msg;
				pthread_rwlock_rdlock(&rwlock);
				char query[128];
				len = sprintf(query, "select title, sender, content, date from %s_recv_table", 
							  rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				while (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.mail.title, row[0]);
					strcpy(snd_msg.mail.sender, row[1]);
					strcpy(snd_msg.mail.content, row[2]);
					strcpy(snd_msg.mail.date, row[3]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_LISTDRAFTMAIL:
			{
				s2c_view_t snd_msg;
				pthread_rwlock_rdlock(&rwlock);
				char query[128];
				len = sprintf(query, "select title, sender, content, date from %s_draft_table", 
							  rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				while (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.mail.title, row[0]);
					strcpy(snd_msg.mail.sender, row[1]);
					strcpy(snd_msg.mail.content, row[2]);
					strcpy(snd_msg.mail.date, row[3]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_LISTTRASHMAIL:
			{
				s2c_view_t snd_msg;
				pthread_rwlock_rdlock(&rwlock);
				char query[128];
				len = sprintf(query, "select title, sender, content, date from %s_trash_table", 
							  rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				while (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					//Modify
					strcpy(snd_msg.mail.title, row[0]);
					strcpy(snd_msg.mail.sender, row[1]);
					strcpy(snd_msg.mail.content, row[2]);
					strcpy(snd_msg.mail.date, row[3]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_LISTCONNECTOR:
			{
				s2c_connectorview_t snd_msg;
				pthread_rwlock_rdlock(&rwlock);
				char query[128];
				len = sprintf(query, "select connector, nickname from %s_connector_table", 
							  rcv_msg.info.account);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				while (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.connector.connector, row[0]);
					strcpy(snd_msg.connector.nickname, row[1]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				mysql_free_result(res);
				pthread_rwlock_unlock(&rwlock);
				bzero(&snd_msg, sizeof(snd_msg));
				snd_msg.type = S2C_VIEW_END;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_FORWARDMAIL:
			{
				s2c_sendmail_t snd_msg;
				char query[128];
				char table[64];
				char type[10];
				pthread_rwlock_rdlock(&rwlock);
				sprintf(query, "select * from %s where account='%s'", USER_TABLE, rcv_msg.mail.sender);
				rt = mysql_real_query(mysql, query, strlen(query));
				if (rt != 0)
				{
					fprintf(stderr, "Select error\n");
					exit(1);
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					exit(1);
				}
				if (!(row = mysql_fetch_row(res)))
				{
					snd_msg.type = S2C_SENDER_NOEXIST;
					pthread_rwlock_unlock(&rwlock);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					break;
				}
				switch (rcv_msg.mailtype)
				{
					case C2S_SEND_MAIL:
						strcpy(type, "send");
						break;
					case C2S_RECV_MAIL:
						strcpy(type, "recv");
						break;
					case C2S_DRAFT:
						strcpy(type, "draft");
						break;
					case C2S_TRASH:
						strcpy(type, "trash");
						break;
				}
				if (!strcmp(type, "send") && !strcmp(rcv_msg.mail.sender, rcv_msg.info.account))
				{
					snd_msg.type = S2C_SEND_FAIL;
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					pthread_rwlock_unlock(&rwlock);
					break;
				}
				sprintf(table, "%s_%s_table", rcv_msg.info.account, type);
				len = sprintf(query, "select title, content from %s where date='%s'", 
							  table, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					bzero(query, sizeof(query));
					len = sprintf(query, "insert into %s_recv_table(title, sender, content, date) "
									"values('%s', '%s', '%s', '%s')",
									rcv_msg.mail.sender, row[0], rcv_msg.info.account, row[1], rcv_msg.mail.date);
					rt = mysql_real_query(mysql, query, len);
					if (rt != 0)
					{
						fprintf(stderr, "Query failed\n");
						return;
					}
					snd_msg.type = S2C_SEND_SUCCESS;
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				pthread_rwlock_unlock(&rwlock);
				break;
			}
		case C2S_SHOWCONTENT:
			{
				s2c_view_t snd_msg;
				char query[128];
				char table[64];
				char type[10];
				pthread_rwlock_rdlock(&rwlock);
				switch (rcv_msg.mailtype)
				{
					case C2S_SEND_MAIL:
						strcpy(type, "send");
						break;
					case C2S_RECV_MAIL:
						strcpy(type, "recv");
						break;
					case C2S_DRAFT:
						strcpy(type, "draft");
						break;
					case C2S_TRASH:
						strcpy(type, "trash");
						break;
				}
				sprintf(table, "%s_%s_table", rcv_msg.info.account, type);
				len = sprintf(query, "select content from %s where date='%s'", 
							  table, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Query failed\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (row = mysql_fetch_row(res))
				{
					bzero(&snd_msg, sizeof(snd_msg));
					strcpy(snd_msg.mail.content, row[0]);
					send(afd, &snd_msg, sizeof(snd_msg), 0);
				}
				pthread_rwlock_unlock(&rwlock);
				break;
			}
		case C2S_SAVEDRAFT:
			{
				pthread_rwlock_wrlock(&rwlock);
				s2c_sendmail_t snd_msg;
				char query[128];
				//Modify
				len = sprintf(query, "insert into %s_draft_table(title, sender, content, date) values('%s', '%s', '%s', '%s')", 
						rcv_msg.info.account, rcv_msg.mail.title, rcv_msg.mail.sender, 
						rcv_msg.mail.content, rcv_msg.mail.date);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "query failed\n");
					return;
				}
				snd_msg.type = S2C_SEND_SUCCESS;
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				pthread_rwlock_unlock(&rwlock);
				break;
			}
		case C2S_ADDCONNECTOR:
			{
				s2c_addconnector_t snd_msg;
				char query[128];
				pthread_rwlock_wrlock(&rwlock);
				//store mail
				len = sprintf(query, "select nickname from userinfo where account='%s'", 
						rcv_msg.connector.connector);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Insert error\n");
					return;
				}
				if (!(res = mysql_store_result(mysql)))
				{
					fprintf(stderr, "store error\n");
					return;
				}
				if (!(row = mysql_fetch_row(res)))
				{
					snd_msg.type = S2C_CONNECTOR_NOEXIST;
					send(afd, &snd_msg, sizeof(snd_msg), 0);
					pthread_rwlock_unlock(&rwlock);
					break;
				}
				len = sprintf(query, "insert into %s_connector_table(connector, nickname) values('%s', '%s')", 
						rcv_msg.info.account, rcv_msg.connector.connector, row[0]);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Insert error\n");
					return;
				}
				snd_msg.type = S2C_ADD_SUCCESS;
				pthread_rwlock_unlock(&rwlock);
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		case C2S_DELCONNECTOR:
			{
				s2c_connector_t snd_msg;
				char query[128];
				pthread_rwlock_wrlock(&rwlock);
				len = sprintf(query, "delete from %s_connector_table where connector='%s'",
							rcv_msg.info.account, rcv_msg.connector.connector);
				rt = mysql_real_query(mysql, query, len);
				if (rt != 0)
				{
					fprintf(stderr, "Delete error!\n");
					return;
				}
				snd_msg.type = S2C_SUCCESS;
				pthread_rwlock_unlock(&rwlock);
				send(afd, &snd_msg, sizeof(snd_msg), 0);
				break;
			}
		}
	}
	{
		struct sockaddr_in peer_addr;
		socklen_t addr_len = sizeof(peer_addr);
		bzero(&peer_addr, sizeof(peer_addr));
		getpeername(afd, (struct sockaddr*)&peer_addr, &addr_len);
		fprintf(stderr, "disconnect with %s %d\n", 
				inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
	}
	close(afd);
	return NULL;
}

