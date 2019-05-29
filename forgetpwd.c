/*************************************************************************
	> File Name: forgetpwd.c
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 13 Oct 2014 11:34:12 PM CST
 ************************************************************************/

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <string.h>
#include "mail.h"

#define FORGETPWD_LENGTH 320
#define FORGETPWD_WIDTH 300

extern int sfd;

typedef struct fWindow
{
	GtkWidget *account, *question, *answer;
	GtkWidget *new_passwd, *renew_passwd;
}fWindow;

static GtkWidget *window;

static void input_verify(GtkWidget* entry, gchar* new_text, int new_text_length, int* position)
{
	gchar c = new_text[0];
	if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		new_text[0] = '\0';
}

static void verify(GtkWidget *widget, gpointer data)
{
	fWindow *p = (fWindow*)data;
	typedef const gchar* pgchar;
	pgchar account, answer, new_passwd, renew_passwd;
	gint question;
	GtkWidget *dialog;
	account = gtk_entry_get_text(GTK_ENTRY(p->account));
	new_passwd = gtk_entry_get_text(GTK_ENTRY(p->new_passwd));
	renew_passwd = gtk_entry_get_text(GTK_ENTRY(p->renew_passwd));
	answer = gtk_entry_get_text(GTK_ENTRY(p->answer));
	if (!strlen(account) || !strlen(new_passwd) || !strlen(renew_passwd) || !strlen(answer))
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Information doesn't complete!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (strcmp(new_passwd, renew_passwd))
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Password doesn't pair!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	question = gtk_combo_box_get_active(GTK_COMBO_BOX(p->question)) + 1;
	c2s_t snd_msg;
	s2c_forget_t rcv_msg;
	snd_msg.type = C2S_FORGETPASSWD;
	strcpy(snd_msg.info.account, account);
	strcpy(snd_msg.info.new_passwd, new_passwd);
	snd_msg.info.safequestion = question;
	strcpy(snd_msg.info.answer, answer);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	bzero(&rcv_msg, sizeof(rcv_msg));
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_ACCOUNT_NOEXIST)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account doesn't exist!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (rcv_msg.type == S2C_QUESTION_ERROR)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Question/answer is incorrect!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (rcv_msg.type == S2C_MODIFY_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Modify succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_widget_hide_all(window);
}

void forget_passwd(GtkWidget *widget, gpointer data)
{
	GtkWidget *submit;
	GtkWidget *vbox, *label[5];
	GtkWidget *hbox[6];
	fWindow fwindow;
	char *labels[] = {"Account", "new password", "confirm password", "Safe question", "answer"};
	char *tempquestion[6] = {
		"Where is your home?", "What's your father's name?",
		"What's your mother's name", "Your phone number?",
		"What's your home address", "What's your favorite food"
	};
	int i;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_title(GTK_WINDOW(window), "forget password");
	gtk_window_set_default_size(GTK_WINDOW(window), 
					FORGETPWD_LENGTH, FORGETPWD_WIDTH);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 20);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("picture/forgetpwd_icon.jpg"));
	update_widget_bg(window, "picture/forgetpwd.jpg");

	vbox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	for (i = 0; i < 6; i++)
	{
		hbox[i] = gtk_hbox_new(TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox[i], FALSE, FALSE, 5);
	}
	
	for (i = 0; i < 5; i++)
	{
		label[i] = gtk_label_new(labels[i]);
		gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 5);
	}

	//acount
	fwindow.account = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[0]), fwindow.account, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(fwindow.account), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//new passwd
	fwindow.new_passwd = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[1]), fwindow.new_passwd, FALSE, FALSE, 5);
	gtk_entry_set_visibility(GTK_ENTRY(fwindow.new_passwd), FALSE);
	g_signal_connect(G_OBJECT(fwindow.new_passwd), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//confirm new passwd
	fwindow.renew_passwd = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[2]), fwindow.renew_passwd, FALSE, FALSE, 5);
	gtk_entry_set_visibility(GTK_ENTRY(fwindow.renew_passwd), FALSE);
	g_signal_connect(G_OBJECT(fwindow.renew_passwd), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//question
	fwindow.question = gtk_combo_box_new_text();
	for (i = 0; i < 6; i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(fwindow.question), tempquestion[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(fwindow.question), 0);
	gtk_box_pack_start(GTK_BOX(hbox[3]), fwindow.question, FALSE, FALSE, 5);
	//answer
	fwindow.answer = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[4]), fwindow.answer, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(fwindow.answer), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//submit
	submit = gtk_button_new_with_label("submit");
	gtk_box_pack_start(GTK_BOX(hbox[5]), submit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(submit), "clicked",
					 G_CALLBACK(verify), (gpointer)&fwindow);

	gtk_widget_show_all(window);
	gtk_main();
}

