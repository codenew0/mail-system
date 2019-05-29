/*************************************************************************
	> File Name: gtk_login.c
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 13 Oct 2014 06:58:09 PM CST
 ************************************************************************/

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include "mail.h"
#include "userinfo.h"

#define WINDOW_LENGTH 320
#define WINDOW_WIDTH 300
#define LOGIN_WINDOW_PICTURE "picture/login.jpg"
#define LOGIN_WINDOW_ICON "picture/login_icon.jpg"

typedef struct {
	GtkWidget *account_edit, *passwd_edit;
}login_window;
static GtkWidget *window;
void init_network();
void destroy_network();

static void input_verify(GtkWidget* entry, gchar* new_text, int new_text_length, int* position)
{
	gchar c = new_text[0];
	if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@' || c == '.'))
		new_text[0] = '\0';
}

int sfd = -1;

void init_network()
{
	struct sockaddr_in addr;
	int ret;

	sfd = socket(AF_INET, SOCK_STREAM, 0);	
	if ( sfd < 0 ) {
		perror("socket()");
		exit(1);
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	ret = connect(sfd, (struct sockaddr*)&addr, sizeof(addr));
	if ( ret != 0 ) {
		perror("connect()");
		exit(1);
	}
}
void destroy_network()
{
	close(sfd);
}

static void login_verify(GtkWidget *widget, gpointer data)
{
	login_window *lwindow = (login_window*)data;
	const gchar *account, *passwd;
	GtkWidget *dialog;
	account = gtk_entry_get_text(GTK_ENTRY(lwindow->account_edit));
	passwd = gtk_entry_get_text(GTK_ENTRY(lwindow->passwd_edit));
	if (strlen(account) == 0 || strlen(passwd) == 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account/passwd is not completed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	c2s_t snd_msg;
	s2c_login_t rcv_msg;
	bzero(&snd_msg, sizeof(snd_msg));
	snd_msg.type = C2S_LOGIN;
	strcpy(snd_msg.info.account, account);
	strcpy(snd_msg.info.passwd, passwd);

	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	printf("%d\n", __LINE__);
	bzero(&rcv_msg, sizeof(rcv_msg));
	printf("%d\n", __LINE__);
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	printf("%d\n", __LINE__);
	if (rcv_msg.type == S2C_LOGIN_FAIL)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account/passwd is incorrect!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (rcv_msg.type == S2C_LOGIN_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Login success!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_widget_hide_all(window);
	table(account);
}

static void delete_window(GtkWidget *widget, gpointer data)
{
	exit(0);
}

int main(int argc, char *argv[])
{
	GtkWidget *register_button, *login_button, *forgetpwd_button;
	GtkWidget *account_box, *passwd_box, *button_box;
	GtkWidget *account_label, *passwd_label;
	GtkWidget *vbox;
	login_window lwindow;

	//Create window
	gtk_init(&argc, &argv);
	init_network();
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//Close window
	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(delete_window), NULL);
	/*Window attribute*/
	gtk_window_set_title(GTK_WINDOW(window), "Sign in");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 20);
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("picture/login_icon.jpg"));
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_default_size(GTK_WINDOW(window), 
								WINDOW_LENGTH, WINDOW_WIDTH);
	update_widget_bg(window, "picture/login.jpg");

	/*First-floor box*/
	//Set largest box
	vbox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	//Push account box
	account_box = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), account_box, FALSE, FALSE, 40);
	//Push password box
	passwd_box  = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), passwd_box, FALSE, FALSE, 40);
	//Push button box
	button_box  = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 40);

	/*Second-floor box*/
	//Set account box
	gchar *markup = "<span foreground=\"red\">account</span>";
	account_label = gtk_label_new("account");
	gtk_label_set_markup(GTK_LABEL(account_label), markup);
	gtk_box_pack_start(GTK_BOX(account_box), account_label, FALSE, FALSE, 5);
	lwindow.account_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(account_box), lwindow.account_edit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(lwindow.account_edit), "insert_text",
								 G_CALLBACK(input_verify), NULL);
	//Set password box
	markup = "<span foreground=\"red\">password</span>";
	passwd_label  = gtk_label_new("password");
	gtk_label_set_markup(GTK_LABEL(passwd_label), markup);
	gtk_box_pack_start(GTK_BOX(passwd_box), passwd_label, FALSE, FALSE, 5);
	lwindow.passwd_edit = gtk_entry_new_with_max_length(50);
	gtk_entry_set_visibility(GTK_ENTRY(lwindow.passwd_edit), FALSE);
	gtk_box_pack_start(GTK_BOX(passwd_box), lwindow.passwd_edit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(lwindow.passwd_edit), "insert_text",
								 G_CALLBACK(input_verify), NULL);
	//Set button box
	login_button = gtk_button_new_with_label("Login");
	gtk_box_pack_start(GTK_BOX(button_box), login_button, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(login_button), "clicked",
					 G_CALLBACK(login_verify), (gpointer)&lwindow);
	register_button = gtk_button_new_with_label("Regsiter");
	gtk_box_pack_start(GTK_BOX(button_box), register_button, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(register_button), "clicked",
					 G_CALLBACK(register_account), NULL);
	forgetpwd_button = gtk_button_new_with_label("Forget passwd");
	gtk_box_pack_start(GTK_BOX(button_box), forgetpwd_button, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(forgetpwd_button), "clicked",
					 G_CALLBACK(forget_passwd), NULL);
	/*Show*/
	gtk_widget_show_all(window);
	gtk_main();

	destroy_network();
			  
	return 0;
}

