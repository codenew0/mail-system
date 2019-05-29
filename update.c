#include "mail.h"
#include <gtk/gtk.h>
//Set window icon
GdkPixbuf *create_pixbuf(const gchar* filename)  
{  
	GdkPixbuf *pixbuf;  
	GError *error = NULL;  
	pixbuf = gdk_pixbuf_new_from_file(filename, &error);  
	if(!pixbuf) {  
		fprintf(stderr, "%s\n", error->message);  
		g_error_free(error);  
	}  
	return pixbuf;  
}  


void update_widget_bg(GtkWidget *widget, const gchar *img_file)  
{          
	GtkStyle *style;        
	GdkPixbuf *pixbuf;          
	GdkPixmap *pixmap;        
	gint width, height;       

	pixbuf = gdk_pixbuf_new_from_file(img_file, NULL);       
	width = gdk_pixbuf_get_width(pixbuf);      
	height = gdk_pixbuf_get_height(pixbuf);       
	pixmap = gdk_pixmap_new(NULL, width, height, 24);      
	gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, NULL, 0);  
	style = gtk_style_copy(GTK_WIDGET (widget)->style);       

	if (style->bg_pixmap[GTK_STATE_NORMAL])           
		g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);       

	style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(pixmap);       
	style->bg_pixmap[GTK_STATE_ACTIVE] = g_object_ref(pixmap);  
	style->bg_pixmap[GTK_STATE_PRELIGHT] = g_object_ref(pixmap);  
	style->bg_pixmap[GTK_STATE_SELECTED] = g_object_ref(pixmap);  
	style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap);  
	gtk_widget_set_style(GTK_WIDGET (widget), style);  
	g_object_unref(style);  
} 

