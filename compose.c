#include <gtk/gtk.h>
#include "mail.h"

static GtkWidget *to_entry;
static GtkWidget *sub_entry, *mlabel;
static GtkWidget *text;
static GtkWidget *window;
static GtkTextBuffer *buffer;
static const gchar *send_account;
extern int sfd;

void create_message_dialog (GtkMessageType type, gchar* message)
{
	GtkWidget* dialogx;
	dialogx = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			type , GTK_BUTTONS_OK, message);
	gtk_dialog_run(GTK_DIALOG(dialogx));
	gtk_widget_destroy(dialogx);
}

void send_message(GtkWidget *widget, gpointer data)
{
	GtkTextIter start, end;
	GtkWidget *dialog;
	const gchar *sender = gtk_entry_get_text(GTK_ENTRY(to_entry));
	const gchar *title = gtk_entry_get_text(GTK_ENTRY(sub_entry));
	gtk_text_buffer_get_start_iter(buffer,&start);
	gtk_text_buffer_get_end_iter(buffer,&end);
	gchar *buf = gtk_text_buffer_get_text(buffer,&start,&end,FALSE);
	
	time_t t = time(NULL);
	struct tm *d = localtime(&t);
	char date[30];
	sprintf(date, "%d-%d-%d %d:%d:%d", d->tm_year + 1900, d->tm_mon, d->tm_mday,
			d->tm_hour, d->tm_min, d->tm_sec);

	c2s_t snd_msg;
	s2c_sendmail_t rcv_msg;
	bzero(&snd_msg, sizeof(snd_msg));
	bzero(&rcv_msg, sizeof(rcv_msg));

	snd_msg.type = C2S_SENDMAIL;
	strcpy(snd_msg.info.account, send_account);
	strcpy(snd_msg.mail.sender, sender);
	strcpy(snd_msg.mail.title, title);
	strcpy(snd_msg.mail.content, buf);
	strcpy(snd_msg.mail.date, date);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);

	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_SEND_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else if (rcv_msg.type == S2C_SENDER_NOEXIST)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Sender is not exist!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else 
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send fail!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_entry_set_text(GTK_ENTRY(to_entry), "");
	gtk_entry_set_text(GTK_ENTRY(sub_entry), "");
	gtk_text_buffer_set_text(buffer, "", 0);
}

void add_to_draft(GtkWidget *widget, gpointer data)
{
	GtkTextIter start, end;
	GtkWidget *dialog;
	const gchar *sender = gtk_entry_get_text(GTK_ENTRY(to_entry));
	const gchar *title = gtk_entry_get_text(GTK_ENTRY(sub_entry));
	gtk_text_buffer_get_start_iter(buffer,&start);
	gtk_text_buffer_get_end_iter(buffer,&end);
	gchar *buf = gtk_text_buffer_get_text(buffer,&start,&end,FALSE);
	
	time_t t = time(NULL);
	struct tm *d = localtime(&t);
	char date[30];
	sprintf(date, "%d-%d-%d %d:%d:%d", d->tm_year + 1900, d->tm_mon, d->tm_mday,
			d->tm_hour, d->tm_min, d->tm_sec);

	c2s_t snd_msg;
	s2c_sendmail_t rcv_msg;
	bzero(&snd_msg, sizeof(snd_msg));
	bzero(&rcv_msg, sizeof(rcv_msg));

	snd_msg.type = C2S_SAVEDRAFT;
	strcpy(snd_msg.info.account, send_account);
	strcpy(snd_msg.mail.sender, sender);
	strcpy(snd_msg.mail.title, title);
	strcpy(snd_msg.mail.content, buf);
	strcpy(snd_msg.mail.date, date);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_SEND_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else 
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Send fail!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

void compose(const gchar *account)
{
	GtkWidget *vbox, *hbox1, *hbox2;
	GtkWidget *label, *send_button, *draft_button, *view;
	send_account = account;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);
	gtk_window_set_title(GTK_WINDOW(window),"Compose");
	gtk_window_set_default_size(GTK_WINDOW(window),640,600);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(window),10);
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("picture/compose_icon.jpg"));
	update_widget_bg(window, "picture/compose.png");

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	hbox1 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,5);
	gchar *markup = "<span foreground=\"red\">Send to: </span>";
	label = gtk_label_new("Send to:");
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,5);
	to_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox1),to_entry,TRUE,TRUE,5);
	send_button = gtk_button_new_with_label("Send");
	gtk_box_pack_start(GTK_BOX(hbox1),send_button,FALSE,FALSE,5);
	g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_message), NULL);

	hbox2 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox2,FALSE,FALSE,5);
	markup = "<span foreground=\"red\">Topic: </span>";
	label = gtk_label_new("Topic:");
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE,FALSE,5);
	sub_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox2),sub_entry,TRUE,TRUE,5);
	draft_button = gtk_button_new_with_label("Add to draft");
	gtk_box_pack_start(GTK_BOX(hbox1),draft_button,FALSE,FALSE,5);
	g_signal_connect(G_OBJECT(draft_button), "clicked", G_CALLBACK(add_to_draft), NULL);

	view = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(view),
			GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	text = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	gtk_container_add(GTK_CONTAINER(view),text);
	gtk_box_pack_start(GTK_BOX(vbox),view,TRUE,TRUE,5);
	view = gtk_viewport_new(NULL,NULL);
	gtk_box_pack_start(GTK_BOX(vbox),view,FALSE,FALSE,5);
	mlabel = gtk_label_new("Tip: Complete your mail and send it.");
	gtk_container_add(GTK_CONTAINER(view),mlabel);

	gtk_widget_show_all(window);
	gtk_main();
}
