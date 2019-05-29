#include <gtk/gtk.h>
#include <strings.h>

#define HELP_LENGTH 640
#define HELP_WIDTH 600

void help()
{
	GtkWidget *window, *vbox;
	GtkWidget *hbox[4], *image[8], *label[8];
	char *label_str[8] = {
		"\tCompose\nTo send your mail\n by this",
		"\tInbox\nView the mail that \nyou have received",
		"\tSent mail\nView the mail \nthat you have sent",
		"\tCircle\nView your friends \nthat you can eazily\n connect them",
		"\tDraft\nView the mail that \nyou haven't sent\n or need to save",
		"\tTrash\nView the mail that \nyou don't like",
		"   Easter egg!\n",
		"   Help\nIt's me!"
	};
	int i;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
			G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_title(GTK_WINDOW(window), "help");
	gtk_window_set_default_size(GTK_WINDOW(window), 
			HELP_LENGTH, HELP_WIDTH);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);

	vbox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	for (i = 0; i < 4; i++)
	{
		hbox[i] = gtk_hbox_new(TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox[i], FALSE, FALSE, 7);
	}

	char buf[20];
	for (i = 0; i < 8; i++)
	{
		bzero(buf, sizeof(buf));
		sprintf(buf, "picture/%d.jpg", i + 1);
		image[i] = gtk_image_new_from_file(buf);
		gtk_box_pack_start(GTK_BOX(hbox[i / 2]), image[i], FALSE, FALSE, 0);
		label[i] = gtk_label_new(label_str[i]);
		gtk_box_pack_start(GTK_BOX(hbox[i / 2]), label[i], FALSE, FALSE, 3);
		i++;
		sprintf(buf, "picture/%d.jpg", i + 1);
		label[i] = gtk_label_new(label_str[i]);
		gtk_box_pack_end(GTK_BOX(hbox[i / 2]), label[i], FALSE, FALSE, 0);
		image[i] = gtk_image_new_from_file(buf);
		gtk_box_pack_end(GTK_BOX(hbox[i / 2]), image[i], FALSE, FALSE, 3);
	}

	gtk_widget_show_all(window);
	gtk_main();
}

