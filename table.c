#include "mail.h"

#define MAIN_LENGTH 730 
#define MAIN_WIDTH 750

enum {
	SEND_MAIL = 1,
	RECV_MAIL,
	HASSEND_MAIL,
	CONNECTOR,
	DRAFT_BOX,
	TRASH_BOX,
	SETTING,
	HELP
};

typedef struct {
	char name[20];
	int type;
	int flag;
	int exist;
	int connect_x, connect_y;
}Picture;

typedef struct {
	int x, y;
}Seat;

static GtkWidget *(button[4][4]), *(image[4][4]), *window;
static Picture picture[4][4];
const gchar *account;
pthread_t tid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *remove_picture(void *arg)
{
	Seat p = { ((int)arg)&0xff, ((int)arg)>>8};
	usleep(300000);
	pthread_mutex_lock(&mutex);
	gtk_container_remove(GTK_CONTAINER(button[p.x][p.y]), image[p.x][p.y]);
	picture[p.x][p.y].flag = 0;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void mail_go(int type)
{
	char *mailbox[] = {
		"Compose", "Inbox", "Sent Box", 
		"Circle", "Draft", "Trash",
		"Setting", "Help"
	};
	GtkWidget *dialog;
	if (type == 7)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
				GTK_BUTTONS_CLOSE, "Easter egg!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
			GTK_BUTTONS_CLOSE, mailbox[type - 1]);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch (type)
	{
		case SEND_MAIL:
			compose(account);
			break;
		case RECV_MAIL:
			recv_mail(account);
			break;
		case HASSEND_MAIL:
			sent_mail(account);
			break;
		case CONNECTOR:
			connector(account);
			break;
		case DRAFT_BOX:
			draft_box(account);
			break;
		case TRASH_BOX:
			trash_box(account);
			break;
		case SETTING:
			//setting(account);
			break;
		case HELP:
			help();
			break;
	}
}

void on_delete(GtkWidget *widget, gpointer data)
{
	exit(0);
}

void kill_player(int signal)
{
	system("killall -9 mplayer");
}

void *music_play(void *arg)
{
	//signal(SIGALRM, kill_player);
	system("mplayer 1.mp3");
	//ualarm(300000, 0);
	return NULL;
}

void play(GtkWidget *button, gpointer data)
{
	Seat p = { ((int)data)&0xff, ((int)data)>>8};
	if (picture[p.x][p.y].flag == 1)
		return;
	image[p.x][p.y] = gtk_image_new_from_file(picture[p.x][p.y].name);
	gtk_container_add(GTK_CONTAINER(button), image[p.x][p.y]);
	gtk_widget_show(image[p.x][p.y]);
	picture[p.x][p.y].flag = 1;
	//pthread_create(&tid, NULL, music_play, (void*)data);
	if (picture[picture[p.x][p.y].connect_x][picture[p.x][p.y].connect_y].flag == 1)
	{
		mail_go(picture[p.x][p.y].type);
	}
	pthread_create(&tid, NULL, remove_picture, (void*)data);
	return;
}

void table(const gchar *user_account)
{
	GtkWidget *table, *frame;
	srand(time(NULL));
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(on_delete), NULL);

	gtk_window_set_title(GTK_WINDOW(window), "Welcome");
	gtk_window_set_default_size(GTK_WINDOW(window), MAIN_LENGTH, MAIN_WIDTH);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	account = user_account;

	gtk_container_set_border_width(GTK_CONTAINER(window), 20);
	char welcome[100];
	sprintf(welcome, "welcome %s, click to pair", account);
	frame = gtk_frame_new(welcome);
	gtk_container_add(GTK_CONTAINER(window), frame);
	table = gtk_table_new(4, 4, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(table), 10);
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_container_add(GTK_CONTAINER(frame), table);
	update_widget_bg(window, "picture/bg.jpg");

	gint i, j;
	bzero(picture, sizeof(picture));
	int num = 1, connect = 0;
	int a = -1, b = -1;
	while (1)
	{
		int x = rand() % 4;
		int y = rand() % 4;
		if (picture[x][y].exist == 0)
		{
			sprintf(picture[x][y].name, "picture/%d.jpg", num);
			picture[x][y].type = num;
			picture[x][y].exist = 1;
			if (connect == 1)
			{
				picture[a][b].connect_x = x;
				picture[a][b].connect_y = y;
				picture[x][y].connect_x = a;
				picture[x][y].connect_y = b;
			}
			a = x; b = y;
			connect++;
			if (connect == 2)
			{
				num++;
				connect = 0;
			}
		}
		if (num == 9)
			break;
	}
	Seat seat;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			button[i][j] = gtk_button_new();
			gtk_table_attach_defaults(GTK_TABLE(table), button[i][j], i, i + 1, j, j + 1);
			seat.x = i; seat.y = j;
			g_signal_connect(G_OBJECT(button[i][j]), "clicked", G_CALLBACK(play),(gpointer)(seat.x+(seat.y<<8)));
		}
	}
	gtk_widget_show_all(window);
	gtk_main();
}

