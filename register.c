/*************************************************************************
	> File Name: register.c
	> Author: codenew
	> Mail: codenew@gmail.com 
	> Created Time: Mon 13 Oct 2014 09:25:15 PM CST
 ************************************************************************/

#include <gtk/gtk.h>
#include <string.h>
#include "mail.h"
#include <mysql/mysql.h>

#define LABEL_NUM 8 
#define REGISTER_LENGTH 400
#define REGISTER_WIDTH 600

static GtkWidget *window;
extern int sfd;

typedef struct rWindow{
	GtkWidget *account_edit, *passwd_edit, *repasswd_edit;
	GtkWidget *nickname_edit, *gender_radio_m, *safequestion_combo;
	GtkWidget *birthyear_combo, *birthmonth_combo, *birthday_combo;
	GtkWidget *answer_edit, *gender_radio_w;
}rWindow;

int is_leap_year(int year)
{
	if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
		return 1;
	return 0;
}

int is_birth_correct(int month, int day, int leap)
{
	switch(month)
	{
		case 4:
		case 6:
		case 9:
		case 11:
			if (day > 30)
				return 0;
			break;
		case 2:
			if (day > leap + 28)
				return 0;
	}
	return 1;
}

static void input_verify(GtkWidget* entry, gchar* new_text, int new_text_length, int* position)
{
	gchar c = new_text[0];
	if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		new_text[0] = '\0';
}

static void verify(GtkWidget *widget, gpointer data)
{
	rWindow *p = (rWindow*)data;
	typedef const gchar* pgchar;
	pgchar account, passwd, repasswd, nickname, answer;
	GtkWidget *dialog;
	gint year, month, day, safequestion, gender, birthday;
	account = gtk_entry_get_text(GTK_ENTRY(p->account_edit));
	passwd = gtk_entry_get_text(GTK_ENTRY(p->passwd_edit));
	repasswd = gtk_entry_get_text(GTK_ENTRY(p->repasswd_edit));
	nickname = gtk_entry_get_text(GTK_ENTRY(p->nickname_edit));
	answer = gtk_entry_get_text(GTK_ENTRY(p->answer_edit));
	if (!strlen(account) || !strlen(passwd) || !strlen(repasswd) || !strlen(nickname) || !strlen(answer))
	{
		//MessageBox not complete
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Information doesn't complete!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (strcmp(passwd, repasswd) != 0)
	{
		//MessageBox passwd not pair
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Password doesn't pair!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	year = gtk_combo_box_get_active(GTK_COMBO_BOX(p->birthyear_combo)) + 1900;
	month = gtk_combo_box_get_active(GTK_COMBO_BOX(p->birthmonth_combo)) + 1;
	day = gtk_combo_box_get_active(GTK_COMBO_BOX(p->birthday_combo)) + 1;
	birthday = year * 10000 + month * 100 + day;
	safequestion = gtk_combo_box_get_active(GTK_COMBO_BOX(p->safequestion_combo)) + 1;
	int rt = is_birth_correct(month, day, is_leap_year(year));
	if (rt == 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Birthday is choosed error!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->gender_radio_m)))
		gender = SEX_MALE;
	else
		gender = SEX_FEMALE;
	c2s_t snd_msg;
	s2c_register_t rcv_msg;
	snd_msg.type = C2S_REGISTER;
	strcpy(snd_msg.info.account, account);
	strcpy(snd_msg.info.passwd, passwd);
	strcpy(snd_msg.info.nickname, nickname);
	strcpy(snd_msg.info.answer, answer);
	snd_msg.info.birthday = birthday;
	snd_msg.info.gender = gender;
	snd_msg.info.safequestion = safequestion;
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	bzero(&rcv_msg, sizeof(rcv_msg));
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_REGISTER_REPEAT)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account has been registered!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}

	if (rcv_msg.type == S2C_REGISTER_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Register successed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_widget_hide_all(window);
}

void register_account(GtkWidget *button, gpointer data)
{
	int i;
	GtkWidget *vbox;
	GtkWidget *widget_label[LABEL_NUM];
	GtkWidget *hbox[LABEL_NUM + 1], *submit_button;
	rWindow rwindow;
	char *tempquestion[6] = {
		"Where is your home?", "What's your father's name?",
		"What's your mother's name", "Your phone number?",
		"What's your home address", "What's your favorite food"
	};
	char *label[LABEL_NUM] = {
		"Account", "Password", "Confirm password",
		"Nick name", "Gender", 	"Birthday",
		"Safe question", "Answer"
	};

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_title(GTK_WINDOW(window), "register");
	gtk_window_set_default_size(GTK_WINDOW(window), 
				REGISTER_LENGTH, REGISTER_WIDTH);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 20);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("register_icon.jpg"));
	update_widget_bg(window, "picture/register.jpg");

	vbox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	for (i = 0; i < LABEL_NUM + 1; i++)
	{
		hbox[i] = gtk_hbox_new(TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox[i], FALSE, FALSE, 5);
	}

	for (i = 0; i < LABEL_NUM; i++)
	{
		widget_label[i] = gtk_label_new(label[i]);
		gtk_box_pack_start(GTK_BOX(hbox[i]), widget_label[i], FALSE, FALSE, 5);
	}
	//account
	rwindow.account_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[0]), rwindow.account_edit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(rwindow.account_edit), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//passwd
	rwindow.passwd_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[1]), rwindow.passwd_edit, FALSE, FALSE, 5);
	gtk_entry_set_visibility(GTK_ENTRY(rwindow.passwd_edit), FALSE);
	g_signal_connect(G_OBJECT(rwindow.passwd_edit), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//confirm passwd
	rwindow.repasswd_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[2]), rwindow.repasswd_edit, FALSE, FALSE, 5);
	gtk_entry_set_visibility(GTK_ENTRY(rwindow.repasswd_edit), FALSE);
	g_signal_connect(G_OBJECT(rwindow.repasswd_edit), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//nick name
	rwindow.nickname_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[3]), rwindow.nickname_edit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(rwindow.nickname_edit), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//gender
	rwindow.gender_radio_m = gtk_radio_button_new_with_label(NULL, "male");
	gtk_box_pack_start(GTK_BOX(hbox[4]), rwindow.gender_radio_m, FALSE, FALSE, 5);
	rwindow.gender_radio_w = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rwindow.gender_radio_m), "female");
	gtk_box_pack_start(GTK_BOX(hbox[4]), rwindow.gender_radio_w, FALSE, FALSE, 5);
	//birthday
	rwindow.birthyear_combo = gtk_combo_box_new_text();
	for (i = 1900; i <= 2013; i++)
	{
		char year[5];
		sprintf(year, "%d", i);
		gtk_combo_box_append_text(GTK_COMBO_BOX(rwindow.birthyear_combo), year);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(rwindow.birthyear_combo), 93);
	gtk_box_pack_start(GTK_BOX(hbox[5]), rwindow.birthyear_combo, FALSE, FALSE, 5);
	rwindow.birthmonth_combo = gtk_combo_box_new_text();
	for (i = 1; i <= 12; i++)
	{
		char month[3];
		sprintf(month, "%d", i);
		gtk_combo_box_append_text(GTK_COMBO_BOX(rwindow.birthmonth_combo), month);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(rwindow.birthmonth_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox[5]), rwindow.birthmonth_combo, FALSE, FALSE, 5);
	rwindow.birthday_combo = gtk_combo_box_new_text();
	for (i = 1; i <= 31; i++)
	{
		char day[3];
		sprintf(day, "%d", i);
		gtk_combo_box_append_text(GTK_COMBO_BOX(rwindow.birthday_combo), day);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(rwindow.birthday_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox[5]), rwindow.birthday_combo, FALSE, FALSE, 5);
	//safe question
	rwindow.safequestion_combo = gtk_combo_box_new_text();
	for (i = 0; i < 6; i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(rwindow.safequestion_combo), 
									tempquestion[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(rwindow.safequestion_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox[6]), rwindow.safequestion_combo, FALSE, FALSE, 5);
	//answer
	rwindow.answer_edit = gtk_entry_new_with_max_length(50);
	gtk_box_pack_start(GTK_BOX(hbox[7]), rwindow.answer_edit, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(rwindow.answer_edit), "insert_text",
					 G_CALLBACK(input_verify), NULL);
	//submit
	submit_button = gtk_button_new_with_label("submit");
	gtk_box_pack_start(GTK_BOX(hbox[8]), submit_button, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(submit_button), "clicked",
							  G_CALLBACK(verify), (gpointer)&rwindow);

	gtk_widget_show_all(window);
	gtk_main();
}

