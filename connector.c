#include <gtk/gtk.h>
#include "mail.h"

void refresh_con_list();
void init_mainwindow();
void init_addwindow();
void clear_addwindow();

typedef struct {
	GtkWidget *window;
	GtkWidget *info_clist;
	GtkWidget *add_button, *del_button;
	GtkWidget *main_box, *button_box;
}ConWindow;
typedef struct {
	GtkWidget *window;
	GtkWidget *ok_button, *cancel_button;
	GtkWidget *account_label,*nickname_label;
	GtkWidget *account_entry, *nickname_entry;
	GtkWidget *table;
}AddWindow;

extern int sfd;
static int connect_row;
static const gchar *con_account;
static ConWindow conWindow;
static AddWindow addWindow;

void connector(const gchar *account)
{	
	con_account = account;
	init_mainwindow();
	gtk_widget_show_all(conWindow.window);
	init_addwindow();
	refresh_con_list();
	gtk_main();
}

void conWindow_clist_select_row(GtkCList *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
	connect_row = row;
}
void conWindow_clist_unselect_row(GtkList *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
	if ( connect_row == row ) {
		connect_row = -1;
	}
}
void conWindow_add_button_clicked(GtkButton *widget, gpointer data)
{
	if ( GTK_WIDGET_VISIBLE(addWindow.window) ) {
		gtk_window_present(GTK_WINDOW(addWindow.window));
		return;
	}
	clear_addwindow();
	gtk_widget_show_all(addWindow.window);
}
void refresh_con_list()
{
	gchar tmpstr1[64], tmpstr2[64], tmpstr3[64], tmpstr4[64];
	gchar *infostr[4] = {tmpstr1, tmpstr2, tmpstr3, tmpstr4};
	c2s_t snd_msg;
	s2c_connectorview_t rcv_msg;

	snd_msg.type = C2S_LISTCONNECTOR;
	strcpy(snd_msg.info.account, con_account);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);

	gtk_clist_freeze(GTK_CLIST(conWindow.info_clist));
	gtk_clist_unselect_all(GTK_CLIST(conWindow.info_clist));
	gtk_clist_clear(GTK_CLIST(conWindow.info_clist));
	while (1) {
		bzero(&rcv_msg, sizeof(rcv_msg));
		recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
		if ( rcv_msg.type == S2C_VIEW_END ) {
			break;
		}
		sprintf(infostr[0], "%s", rcv_msg.connector.connector);
		sprintf(infostr[1], "%s", rcv_msg.connector.nickname);
		gtk_clist_append(GTK_CLIST(conWindow.info_clist), infostr);
	}
	gtk_clist_thaw(GTK_CLIST(conWindow.info_clist));
}

void conWindow_del_button_clicked(GtkButton *widget, gpointer data)
{
	if (connect_row == -1) {
		return;
	}
	c2s_t snd_msg;
	s2c_delete_t rcv_msg;
	gchar *connector;
	GtkWidget *dialog;
	gtk_clist_get_text(GTK_CLIST(conWindow.info_clist), connect_row, 0, &connector);
	snd_msg.type = C2S_DELCONNECTOR;
	strcpy(snd_msg.info.account, con_account);
	strcpy(snd_msg.connector.connector, connector);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	bzero(&rcv_msg, sizeof(rcv_msg));
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(conWindow.window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Delete succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	refresh_con_list();
}

void init_mainwindow()
{
	gchar *titles[] = {
		"Account", "Nickname"
	};
	
	connect_row = -1;
	
	conWindow.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(conWindow.window, 450, 600);
	gtk_window_set_title(GTK_WINDOW(conWindow.window), "Connector");
	gtk_widget_set_uposition(conWindow.window, 300, 100);
	gtk_window_set_position(GTK_WINDOW(conWindow.window), GTK_WIN_POS_CENTER);

	conWindow.info_clist = gtk_clist_new_with_titles(2, titles);

	conWindow.add_button = gtk_button_new_with_label("Add");
	conWindow.del_button = gtk_button_new_with_label("Del");

	conWindow.button_box = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(conWindow.button_box), 5);
	gtk_box_pack_start(GTK_BOX(conWindow.button_box), conWindow.add_button,  FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(conWindow.button_box), conWindow.del_button,  FALSE, FALSE, 0);

	conWindow.main_box = gtk_hbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(conWindow.main_box), 10);
	gtk_box_pack_start(GTK_BOX(conWindow.main_box), conWindow.info_clist, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(conWindow.main_box), conWindow.button_box, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(conWindow.window), conWindow.main_box);

	gtk_clist_set_selection_mode(GTK_CLIST(conWindow.info_clist), GTK_SELECTION_SINGLE);

	g_signal_connect(GTK_OBJECT(conWindow.window), "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_OBJECT(conWindow.info_clist), "select-row", G_CALLBACK(conWindow_clist_select_row), NULL);
	g_signal_connect(GTK_OBJECT(conWindow.info_clist), "unselect-row", G_CALLBACK(conWindow_clist_unselect_row), NULL);

	g_signal_connect(GTK_OBJECT(conWindow.add_button), "clicked", 
									G_CALLBACK(conWindow_add_button_clicked), NULL);
	g_signal_connect(GTK_OBJECT(conWindow.del_button), "clicked", 
									G_CALLBACK(conWindow_del_button_clicked), (gpointer)&conWindow);
}

void addWindow_cancel_button_clicked(GtkWidget *widget)
{
	gtk_widget_hide_all(addWindow.window);
}
void addWindow_ok_button_clicked(GtkWidget *widget)
{
	GtkWidget *dialog;
	const gchar *conaccount = gtk_entry_get_text(GTK_ENTRY(addWindow.account_entry));
	if (!strlen(conaccount))
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(addWindow.window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account is null!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	c2s_t snd_msg;
	snd_msg.type = C2S_ADDCONNECTOR;
	int i = 0;
	while (1)
	{
		gchar *connector="";
		int rt = gtk_clist_get_text(GTK_CLIST(conWindow.info_clist), i, 0, &connector);
		i++;
		if (rt == 0)
			break;
		if (!strcmp(connector,conaccount))
		{
			dialog = gtk_message_dialog_new(GTK_WINDOW(addWindow.window), 
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE, "Connector repeat!");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			gtk_widget_hide_all(addWindow.window);
			return;
		}
	}
	strcpy(snd_msg.info.account, con_account);
	strcpy(snd_msg.connector.connector, conaccount);
	send(sfd, &snd_msg, sizeof(snd_msg), 0);
	s2c_addconnector_t rcv_msg;
	bzero(&rcv_msg, sizeof(rcv_msg));
	recv(sfd, &rcv_msg, sizeof(rcv_msg), 0);
	if (rcv_msg.type == S2C_ADD_SUCCESS)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(addWindow.window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Add succeed!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else if (rcv_msg.type == S2C_CONNECTOR_NOEXIST)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(addWindow.window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Account is not exist!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_widget_hide_all(addWindow.window);
	refresh_con_list();
}

void init_addwindow()
{
	gboolean addWindow_delete_clicked();

	addWindow.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(addWindow.window), "Add conncetor");
	gtk_widget_set_usize(addWindow.window, 300, 200);
	gtk_window_set_position(GTK_WINDOW(addWindow.window), GTK_WIN_POS_CENTER);

	addWindow.ok_button = gtk_button_new_with_label("Yes");
	addWindow.cancel_button = gtk_button_new_with_label("No");

	addWindow.account_label = gtk_label_new("Account:");
	addWindow.account_entry = gtk_entry_new();

	addWindow.table = gtk_table_new(4, 4, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(addWindow.table), 10);
	gtk_table_attach_defaults(GTK_TABLE(addWindow.table), addWindow.account_label, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(addWindow.table), addWindow.account_entry, 1, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(addWindow.table), addWindow.ok_button, 2, 3, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(addWindow.table), addWindow.cancel_button, 3, 4, 3, 4);
	gtk_container_add(GTK_CONTAINER(addWindow.window), addWindow.table);

	g_signal_connect(GTK_OBJECT(addWindow.ok_button), "clicked", 
									G_CALLBACK(addWindow_ok_button_clicked), NULL);
	g_signal_connect(GTK_OBJECT(addWindow.cancel_button), "clicked", 
									G_CALLBACK(addWindow_cancel_button_clicked), NULL);
	g_signal_connect(GTK_OBJECT(addWindow.window), "delete-event",
			G_CALLBACK(addWindow_delete_clicked), NULL);
}

void clear_addwindow()
{
	gtk_entry_set_text(GTK_ENTRY(addWindow.account_entry), "");
}

gboolean addWindow_delete_clicked(GtkWidget *Widget,GdkEvent *event,gpointer func_data )
{
	gtk_widget_hide_all(Widget);
	return TRUE;	
}

