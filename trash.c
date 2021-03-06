/**************************
**********Trash****************
***************************/
#include <gtk/gtk.h>
#include "mail.h"

#define CONTENT_LENGTH 500
#define CONTENT_WIDTH  480

static int selected_row = -1;
static const gchar *trash_account;
extern int sfd;

typedef struct {
	GtkWidget *outbox_window;
	GtkWidget *outbox_delete_button,*outbox_forward_button,*outbox_search_button,*outbox_show_button;
	GtkWidget *outbox_search_entry,*outbox_forward_entry,*outbox_hbox3,*outbox_hbox4;
	GtkWidget *outbox_hbox1,*outbox_hbox2,*outbox_vbox1,*outbox_vbox2,*outbox_vbox3;
	GtkWidget *outbox_clist,*outbox_user_label,*outbox_scrolled,*outbox_hbox5,*outbox_hbox6;
}trashWindow;

void refresh_trash_list(trashWindow *p)
{
	gchar tmpstr1[64], tmpstr2[64], tmpstr3[64], tmpstr4[64];
	gchar *infostr[4] = {tmpstr1, tmpstr2, tmpstr3, tmpstr4};
	c2s_t snd_msg;
	s2c_view_t rcv_msg;

	snd_msg.type = C2S_LISTTRASHMAIL;
	strcpy(snd_msg.info.account, trash_account);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);

	//gtk_clist_freeze(GTK_CLIST(p->outbox_clist));
	gtk_clist_unselect_all(GTK_CLIST(p->outbox_clist));
	gtk_clist_clear(GTK_CLIST(p->outbox_clist));
	while (1) {
		bzero(&rcv_msg, sizeof(rcv_msg));
		recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
		if ( rcv_msg.type == S2C_VIEW_END ) {
			break;
		}
		sprintf(infostr[0], "%s", rcv_msg.mail.title);
		sprintf(infostr[1], "%s", rcv_msg.mail.sender);
		sprintf(infostr[2], "%s", rcv_msg.mail.date);
		gtk_clist_append(GTK_CLIST(p->outbox_clist), infostr);
	}
	//gtk_clist_thaw(GTK_CLIST(p->outbox_clist));
}

void show_trash_content(GtkWidget *widget, gpointer data)
{
	if (selected_row == -1 )
		return;
	gchar *title,*sender,*date,*content;	
	GtkWidget *window,*label1,*vbox,*label2,*label3,*label4;
	trashWindow *p = (trashWindow*)data;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW (window),"Mail content");
	gtk_window_set_default_size(GTK_WINDOW(window), CONTENT_LENGTH, CONTENT_WIDTH);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_widget_show(window);

	gtk_clist_get_text(GTK_CLIST(p->outbox_clist),selected_row,0,&title);
	gtk_clist_get_text(GTK_CLIST(p->outbox_clist),selected_row,1,&sender);
	gtk_clist_get_text(GTK_CLIST(p->outbox_clist),selected_row,2,&date);

	c2s_t snd_msg;
	s2c_view_t rcv_msg;
	snd_msg.type = C2S_SHOWCONTENT;
	snd_msg.mailtype = C2S_TRASH;
	strcpy(snd_msg.info.account, trash_account);
	strcpy(snd_msg.mail.date, date);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	content = rcv_msg.mail.content;

	label1 = gtk_label_new(title);
	label2 = gtk_label_new(sender);
	label3 = gtk_label_new(date);
	label4 = gtk_label_new(content);

	vbox = gtk_vbox_new(FALSE,20);
	gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label4, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
	gtk_widget_show_all(window);
	gtk_main();
}

void delete_trash_mail(GtkWidget *widget, gpointer data)
{
	if (selected_row == -1)
		return;
	trashWindow *p = (trashWindow*)data;
	c2s_t snd_msg;
	s2c_delete_t rcv_msg;
	gchar *date;
	GtkWidget *dialog;
	gtk_clist_get_text(GTK_CLIST(p->outbox_clist), selected_row, 2, &date);
	snd_msg.type = C2S_DELMAIL;
	snd_msg.mailtype = C2S_TRASH;
	strcpy(snd_msg.info.account, trash_account);
	strcpy(snd_msg.mail.date, date);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	bzero(&rcv_msg, sizeof(rcv_msg));
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Delete succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	refresh_trash_list(p);
}

void search_trash_mail(GtkWidget *widget, gpointer data)
{
	trashWindow *p = (trashWindow*)data;
	GtkWidget *dialog;
	const gchar *search = gtk_entry_get_text(GTK_ENTRY(p->outbox_search_entry));
	if (!strlen(search))
	{
		refresh_trash_list(p);
		return;
	}
	c2s_t snd_msg;
	s2c_view_t rcv_msg;
	gchar tmpstr1[64], tmpstr2[64], tmpstr3[64], tmpstr4[64];
	gchar *infostr[4] = {tmpstr1, tmpstr2, tmpstr3, tmpstr4};

	bzero(&snd_msg, sizeof(snd_msg));
	strcpy(snd_msg.search, search);
	strcpy(snd_msg.info.account, trash_account);
	snd_msg.mailtype = C2S_TRASH;
	snd_msg.type = C2S_SEARCH;
	send(sfd, &snd_msg, sizeof(snd_msg), 0);

	//gtk_clist_freeze(GTK_CLIST(p->outbox_clist));
	gtk_clist_unselect_all(GTK_CLIST(p->outbox_clist));
	gtk_clist_clear(GTK_CLIST(p->outbox_clist));
	while (1) {
		bzero(&rcv_msg, sizeof(rcv_msg));
		recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
		if ( rcv_msg.type == S2C_VIEW_END ) {
			break;
		}
		if (rcv_msg.type == S2C_VIEW_FAIL)
		{
			dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE, "No result!");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			break;
		}
		sprintf(infostr[0], "%s", rcv_msg.mail.title);
		sprintf(infostr[1], "%s", rcv_msg.mail.sender);
		sprintf(infostr[2], "%s", rcv_msg.mail.date);
		gtk_clist_append(GTK_CLIST(p->outbox_clist), infostr);
	}
	//gtk_clist_thaw(GTK_CLIST(p->outbox_clist));
}

void forward_trash_mail(GtkWidget *widget, gpointer data)
{
	trashWindow *p = (trashWindow*)data;
	GtkWidget *dialog;
	const gchar *forward = gtk_entry_get_text(GTK_ENTRY(p->outbox_forward_entry));
	if (!strlen(forward))
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Receiver is null!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	c2s_t snd_msg;
	snd_msg.type = C2S_FORWARDMAIL;
	gchar *date;
	gtk_clist_get_text(GTK_CLIST(p->outbox_clist), selected_row, 2, &date);
	strcpy(snd_msg.info.account, trash_account);
	strcpy(snd_msg.mail.sender, forward);
	strcpy(snd_msg.mail.date, date);
	snd_msg.mailtype = C2S_TRASH;
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	s2c_sendmail_t rcv_msg;
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_SEND_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else if (rcv_msg.type == S2C_SENDER_NOEXIST)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Sender is not exist!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else 
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(p->outbox_window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send fail!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

void unget_trash_row(GtkCList *clist,gint row,gint column,GdkEventButton *event,gpointer user_data)
{
	selected_row = -1;	
}

void get_trash_row(GtkCList *clist,gint row,gint column,GdkEventButton *event,gpointer user_data){
	selected_row = row;
}

gboolean delete_trashwindow(GtkWidget *widget, gpointer data)
{
	return FALSE;
}

void trash_box(const gchar *account)
{
	trashWindow swindow;
	gchar *title[] = {"Title","Sender","Date"};
	trash_account = account;
	
	swindow.outbox_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (swindow.outbox_window),"Trash");
	gtk_widget_set_usize((swindow.outbox_window),800,650);
	gtk_widget_set_uposition((swindow.outbox_window),200,200);
	gtk_window_set_modal(GTK_WINDOW(swindow.outbox_window), TRUE);
	g_signal_connect(G_OBJECT(swindow.outbox_window), "delete_event",
					 G_CALLBACK(gtk_main_quit), NULL);

	gchar welcome[60];
	sprintf(welcome, "Hello, %s", account);
	swindow.outbox_user_label = gtk_accel_label_new (welcome);
	swindow.outbox_delete_button = gtk_button_new_with_label("Delete");
	swindow.outbox_forward_button = gtk_button_new_with_label("Forward");
	swindow.outbox_search_entry = gtk_entry_new();
	swindow.outbox_forward_entry = gtk_entry_new();
	swindow.outbox_search_button = gtk_button_new_with_label("Search");
	swindow.outbox_show_button = gtk_button_new_with_label("Show");
	swindow.outbox_scrolled = gtk_scrolled_window_new(NULL,NULL);
	swindow.outbox_hbox1 = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox1), swindow.outbox_user_label, FALSE, FALSE, 0);

	swindow.outbox_hbox2 = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox2), swindow.outbox_search_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox2), swindow.outbox_search_button, FALSE, FALSE, 0);
	     
	swindow.outbox_hbox3 = gtk_hbox_new(TRUE,20);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox3), swindow.outbox_delete_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox3), swindow.outbox_show_button, FALSE, FALSE, 0);

	swindow.outbox_hbox4 = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox4), swindow.outbox_forward_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox4), swindow.outbox_forward_button, FALSE, FALSE, 0);
	
	swindow.outbox_hbox5 = gtk_hbox_new(TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(swindow.outbox_hbox5), 20);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox5),swindow.outbox_hbox1,FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox5),swindow.outbox_hbox2,TRUE,TRUE,0);
	
	swindow.outbox_hbox6 = gtk_hbox_new(TRUE,0);	
	gtk_container_set_border_width(GTK_CONTAINER(swindow.outbox_hbox6), 5);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox6),swindow.outbox_hbox3,FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_hbox6),swindow.outbox_hbox4,TRUE,TRUE,0);

	swindow.outbox_vbox1 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_vbox1),swindow.outbox_hbox5,FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_vbox1),swindow.outbox_hbox6,TRUE,TRUE,0);

	swindow.outbox_vbox2 = gtk_vbox_new(FALSE,0);
	swindow.outbox_clist = gtk_clist_new_with_titles(3,title);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swindow.outbox_scrolled),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(swindow.outbox_scrolled),swindow.outbox_clist);
	gtk_clist_set_column_width (GTK_CLIST(swindow.outbox_clist),0,150);
	gtk_clist_set_column_width (GTK_CLIST(swindow.outbox_clist),1,150);
	gtk_clist_set_column_width (GTK_CLIST(swindow.outbox_clist),2,150);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_vbox2),swindow.outbox_scrolled,TRUE,TRUE,20);
	
	swindow.outbox_vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_vbox3),swindow.outbox_vbox1,FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(swindow.outbox_vbox3),swindow.outbox_vbox2,TRUE,TRUE,0);
	gtk_container_add(GTK_CONTAINER(swindow.outbox_window), GTK_WIDGET(swindow.outbox_vbox3));

	g_signal_connect(G_OBJECT(swindow.outbox_show_button),"clicked",G_CALLBACK(show_trash_content), (gpointer)&swindow);
	g_signal_connect(G_OBJECT(swindow.outbox_delete_button), "clicked", G_CALLBACK(delete_trash_mail), (gpointer)&swindow);
	g_signal_connect(G_OBJECT(swindow.outbox_forward_button), "clicked", G_CALLBACK(forward_trash_mail), (gpointer)&swindow);
	g_signal_connect(G_OBJECT(swindow.outbox_search_button), "clicked", G_CALLBACK(search_trash_mail), (gpointer)&swindow);
	g_signal_connect(G_OBJECT(swindow.outbox_clist),"select_row",G_CALLBACK(get_trash_row), NULL);
	g_signal_connect(G_OBJECT(swindow.outbox_clist), "unselect_row", G_CALLBACK(unget_trash_row), NULL);
	refresh_trash_list(&swindow);

	gtk_widget_show_all(swindow.outbox_window);
	gtk_main();
}

