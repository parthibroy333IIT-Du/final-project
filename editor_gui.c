#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

GtkWidget *window;
GtkWidget *image_widget;
GtkWidget *status_label;
GtkWidget *entry_brightness;
GtkWidget *entry_crop_x, *entry_crop_y, *entry_crop_w, *entry_crop_h;

GdkPixbuf *current_pixbuf = NULL;
GdkPixbuf *undo_pixbuf = NULL;
char loaded_filename[256] = "";

void save_undo() {
    if (current_pixbuf) {
        if (undo_pixbuf) g_object_unref(undo_pixbuf);
        undo_pixbuf = gdk_pixbuf_copy(current_pixbuf);
    }
}

void update_image_display() {
    if (current_pixbuf) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), current_pixbuf);
        char status[512];
        snprintf(status, sizeof(status), "<span foreground='#a6e3a1'><b>Image: %s (%dx%d)</b></span>",
                 loaded_filename[0] ? loaded_filename : "New Image", 
                 gdk_pixbuf_get_width(current_pixbuf), 
                 gdk_pixbuf_get_height(current_pixbuf));
        gtk_label_set_markup(GTK_LABEL(status_label), status);
    }
}

void on_file_menu_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Image",
                                                     GTK_WINDOW(window),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Open", GTK_RESPONSE_ACCEPT,
                                                     NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GError *error = NULL;
        GdkPixbuf *new_buf = gdk_pixbuf_new_from_file(filename, &error);
        if (new_buf) {
            if (current_pixbuf) g_object_unref(current_pixbuf);
            current_pixbuf = new_buf;
            strncpy(loaded_filename, filename, 255);
            save_undo();
            update_image_display();
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_save_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(window),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_OK,
                                                "No image loaded to save!");
        gtk_dialog_run(GTK_DIALOG(msg));
        gtk_widget_destroy(msg);
        return;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Image",
                                                     GTK_WINDOW(window),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Save", GTK_RESPONSE_ACCEPT,
                                                     NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "output.bmp");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GError *error = NULL;
        
        const char *type = "bmp";
        if (g_str_has_suffix(filename, ".png")) type = "png";
        else if (g_str_has_suffix(filename, ".jpg") || g_str_has_suffix(filename, ".jpeg")) type = "jpeg";

        if (gdk_pixbuf_save(current_pixbuf, filename, type, &error, NULL)) {
            strncpy(loaded_filename, filename, 255);
            update_image_display();
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_grayscale_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    int width = gdk_pixbuf_get_width(current_pixbuf);
    int height = gdk_pixbuf_get_height(current_pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(current_pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(current_pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(current_pixbuf);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            guchar gray = (guchar)(0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2]);
            p[0] = p[1] = p[2] = gray;
        }
    }
    update_image_display();
}

void on_invert_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    int width = gdk_pixbuf_get_width(current_pixbuf);
    int height = gdk_pixbuf_get_height(current_pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(current_pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(current_pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(current_pixbuf);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            p[0] = 255 - p[0];
            p[1] = 255 - p[1];
            p[2] = 255 - p[2];
        }
    }
    update_image_display();
}

void on_hflip_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    GdkPixbuf *flipped = gdk_pixbuf_flip(current_pixbuf, TRUE);
    g_object_unref(current_pixbuf);
    current_pixbuf = flipped;
    update_image_display();
}

void on_vflip_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    GdkPixbuf *flipped = gdk_pixbuf_flip(current_pixbuf, FALSE);
    g_object_unref(current_pixbuf);
    current_pixbuf = flipped;
    update_image_display();
}

void on_rotate_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    GdkPixbuf *rotated = gdk_pixbuf_rotate_simple(current_pixbuf, GDK_PIXBUF_ROTATE_CLOCKWISE);
    g_object_unref(current_pixbuf);
    current_pixbuf = rotated;
    update_image_display();
}

void on_blur_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    GdkPixbuf *copy = gdk_pixbuf_copy(current_pixbuf);
    int width = gdk_pixbuf_get_width(current_pixbuf);
    int height = gdk_pixbuf_get_height(current_pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(current_pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(current_pixbuf);
    guchar *src = gdk_pixbuf_get_pixels(copy);
    guchar *dst = gdk_pixbuf_get_pixels(current_pixbuf);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        sum += src[(y + dy) * rowstride + (x + dx) * n_channels + c];
                    }
                }
                dst[y * rowstride + x * n_channels + c] = sum / 9;
            }
        }
    }
    g_object_unref(copy);
    update_image_display();
}

void on_undo_clicked(GtkWidget *widget, gpointer data) {
    if (undo_pixbuf) {
        if (current_pixbuf) g_object_unref(current_pixbuf);
        current_pixbuf = gdk_pixbuf_copy(undo_pixbuf);
        update_image_display();
    }
}

void on_brightness_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    save_undo();
    int val = atoi(gtk_entry_get_text(GTK_ENTRY(entry_brightness)));
    int width = gdk_pixbuf_get_width(current_pixbuf);
    int height = gdk_pixbuf_get_height(current_pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(current_pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(current_pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(current_pixbuf);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            for (int c = 0; c < 3; c++) {
                int v = p[c] + val;
                p[c] = (v > 255) ? 255 : ((v < 0) ? 0 : v);
            }
        }
    }
    update_image_display();
}

void on_crop_clicked(GtkWidget *widget, gpointer data) {
    if (!current_pixbuf) return;
    int cx = atoi(gtk_entry_get_text(GTK_ENTRY(entry_crop_x)));
    int cy = atoi(gtk_entry_get_text(GTK_ENTRY(entry_crop_y)));
    int cw = atoi(gtk_entry_get_text(GTK_ENTRY(entry_crop_w)));
    int ch = atoi(gtk_entry_get_text(GTK_ENTRY(entry_crop_h)));
    int img_w = gdk_pixbuf_get_width(current_pixbuf);
    int img_h = gdk_pixbuf_get_height(current_pixbuf);

    if (cw <= 0 || ch <= 0 || cx < 0 || cy < 0 || (cx + cw) > img_w || (cy + ch) > img_h) return;

    save_undo();
    GdkPixbuf *sub = gdk_pixbuf_new_subpixbuf(current_pixbuf, cx, cy, cw, ch);
    GdkPixbuf *cropped = gdk_pixbuf_copy(sub);
    g_object_unref(sub);
    g_object_unref(current_pixbuf);
    current_pixbuf = cropped;
    update_image_display();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "BMP Image Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 650);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 🎨 নতুন কালার স্কিম (CSS Theme)
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "window { background-color: #1e1e2e; }\n"
        "button { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; margin: 2px; padding: 4px 8px; font-weight: bold; }\n"
        "button:hover { background-color: #45475a; color: #89b4fa; border-color: #89b4fa; }\n"
        "entry { background-color: #181825; color: #a6e3a1; border: 1px solid #45475a; border-radius: 3px; min-width: 35px; font-size: 12px; font-weight: bold; }\n", -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 1. Header Title
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span foreground='#cba6f7' font='14'><b>Image Manipulation</b></span>");
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 0);

    // 2. File Operations Row (Open File & Save Image)
    GtkWidget *file_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_set_homogeneous(GTK_BOX(file_hbox), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), file_hbox, FALSE, FALSE, 0);

    GtkWidget *btn_file = gtk_button_new_with_label("File Menu (Open)");
    g_signal_connect(btn_file, "clicked", G_CALLBACK(on_file_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(file_hbox), btn_file, TRUE, TRUE, 0);

    GtkWidget *btn_save = gtk_button_new_with_label("Save Image");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(file_hbox), btn_save, TRUE, TRUE, 0);

    // 3. Action Buttons Row
    GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(hbox1), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);

    struct { const char *label; void (*cb)(GtkWidget*, gpointer); } btns[] = {
        {"Grayscale", on_grayscale_clicked},
        {"Inversion", on_invert_clicked},
        {"Horizontal Flip", on_hflip_clicked},
        {"Vertical Flip", on_vflip_clicked},
        {"Rotate 90 deg", on_rotate_clicked},
        {"Blur", on_blur_clicked},
        {"Undo", on_undo_clicked}
    };
    for (int i = 0; i < 7; i++) {
        GtkWidget *b = gtk_button_new_with_label(btns[i].label);
        g_signal_connect(b, "clicked", G_CALLBACK(btns[i].cb), NULL);
        gtk_box_pack_start(GTK_BOX(hbox1), b, TRUE, TRUE, 0);
    }

    // 4. Controls Row (Brightness & Crop)
    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);

    GtkWidget *lbl_b = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_b), "<span foreground='#f9e2af'>Brightness:</span>");
    gtk_box_pack_start(GTK_BOX(hbox2), lbl_b, FALSE, FALSE, 0);

    entry_brightness = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_brightness), "0");
    gtk_box_pack_start(GTK_BOX(hbox2), entry_brightness, FALSE, FALSE, 0);

    GtkWidget *btn_app_b = gtk_button_new_with_label("Apply Brightness");
    g_signal_connect(btn_app_b, "clicked", G_CALLBACK(on_brightness_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), btn_app_b, FALSE, FALSE, 0);

    GtkWidget *sep = gtk_label_new("|");
    gtk_box_pack_start(GTK_BOX(hbox2), sep, FALSE, FALSE, 4);

    GtkWidget *btn_app_c = gtk_button_new_with_label("Apply Crop");
    g_signal_connect(btn_app_c, "clicked", G_CALLBACK(on_crop_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), btn_app_c, FALSE, FALSE, 0);

    entry_crop_x = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(entry_crop_x), "0");
    entry_crop_y = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(entry_crop_y), "0");
    entry_crop_w = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(entry_crop_w), "100");
    entry_crop_h = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(entry_crop_h), "100");

    gtk_box_pack_start(GTK_BOX(hbox2), entry_crop_x, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), entry_crop_y, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), entry_crop_w, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), entry_crop_h, FALSE, FALSE, 0);

    // 5. Status Label
    status_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(status_label), "<span foreground='#a6e3a1'><b>Image: (No file loaded)</b></span>");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    // 6. Image Container Area
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    image_widget = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(scroll), image_widget);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
