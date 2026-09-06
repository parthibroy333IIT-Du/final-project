#include <stdio.h>
#include <stdlib.h>
#include <iup.h>
#include "image.h"
#include "image_processing.h"

static Image *g_current_image = NULL;
static Image *g_undo_image = NULL;
static Ihandle *g_label_status = NULL;
static Ihandle *g_image_display = NULL;

static void update_image_display(void) {
    if (!g_current_image) return;

    int w = g_current_image->width;
    int h = g_current_image->height;

    unsigned char *rgb_pixels = (unsigned char *)malloc(w * h * 3);
    if (!rgb_pixels) return;

    for (int i = 0; i < w * h; i++) {
        rgb_pixels[i * 3 + 0] = g_current_image->data[i].r;
        rgb_pixels[i * 3 + 1] = g_current_image->data[i].g;
        rgb_pixels[i * 3 + 2] = g_current_image->data[i].b;
    }

    Ihandle *iup_img = IupImageRGB(w, h, rgb_pixels);
    free(rgb_pixels);

    static int counter = 0;
    char handle_name[64];
    sprintf(handle_name, "BMP_IMG_HANDLE_%d", ++counter);

    IupSetHandle(handle_name, iup_img);

    char rastersize[32];
    sprintf(rastersize, "%dx%d", w, h);
    IupSetAttribute(g_image_display, "RASTERSIZE", rastersize);
    IupSetAttribute(g_image_display, "IMAGE", handle_name);

    Ihandle *dlg = IupGetDialog(g_image_display);
    if (dlg) {
        IupRefresh(dlg);
        IupRedraw(dlg, 1);
    }
}

static void save_undo(void) {
    if (g_undo_image) free_image(g_undo_image);
    g_undo_image = copy_image(g_current_image);
}

static int cb_open(Ihandle *self) {
    (void)self;
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(filedlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");
    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(filedlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(filedlg, "VALUE");
        Image *loaded = load_bmp(filename);
        if (loaded) {
            if (g_current_image) free_image(g_current_image);
            g_current_image = loaded;
            update_image_display();
            IupSetStrAttribute(g_label_status, "TITLE", "Image loaded successfully.");
        } else {
            IupSetStrAttribute(g_label_status, "TITLE", "Error: Failed to load BMP.");
            IupMessage("Error", "Could not open file as 24-bit BMP.");
        }
    }
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

static int cb_save(Ihandle *self) {
    (void)self;
    if (!g_current_image) {
        IupMessage("Error", "No image to save.");
        return IUP_DEFAULT;
    }
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(filedlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");
    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(filedlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(filedlg, "VALUE");
        if (save_bmp(filename, g_current_image)) {
            IupSetStrAttribute(g_label_status, "TITLE", "Image saved successfully.");
        } else {
            IupMessage("Error", "Failed to save BMP.");
        }
    }
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

static int cb_grayscale(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo(); apply_grayscale(g_current_image); update_image_display();
    IupSetStrAttribute(g_label_status, "TITLE", "Applied Grayscale.");
    return IUP_DEFAULT;
}

static int cb_invert(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo(); apply_invert(g_current_image); update_image_display();
    IupSetStrAttribute(g_label_status, "TITLE", "Applied Invert.");
    return IUP_DEFAULT;
}

static int cb_brightness(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo(); apply_brightness(g_current_image, 30); update_image_display();
    IupSetStrAttribute(g_label_status, "TITLE", "Applied Brightness.");
    return IUP_DEFAULT;
}

static int cb_flip(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo(); apply_flip_horizontal(g_current_image); update_image_display();
    IupSetStrAttribute(g_label_status, "TITLE", "Flipped Horizontally.");
    return IUP_DEFAULT;
}

static int cb_rotate(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo();
    Image *rot = rotate_90_clockwise(g_current_image);
    if (rot) {
        free_image(g_current_image);
        g_current_image = rot;
        update_image_display();
        IupSetStrAttribute(g_label_status, "TITLE", "Rotated 90°.");
    }
    return IUP_DEFAULT;
}

static int cb_blur(Ihandle *self) {
    (void)self; if (!g_current_image) return IUP_DEFAULT;
    save_undo();
    Image *blurred = apply_blur(g_current_image);
    if (blurred) {
        free_image(g_current_image);
        g_current_image = blurred;
        update_image_display();
        IupSetStrAttribute(g_label_status, "TITLE", "Applied Blur.");
    }
    return IUP_DEFAULT;
}

static int cb_undo(Ihandle *self) {
    (void)self; if (!g_undo_image) return IUP_DEFAULT;
    if (g_current_image) free_image(g_current_image);
    g_current_image = g_undo_image;
    g_undo_image = NULL;
    update_image_display();
    IupSetStrAttribute(g_label_status, "TITLE", "Restored previous state.");
    return IUP_DEFAULT;
}

int main(int argc, char **argv) {
    IupOpen(&argc, &argv);

    Ihandle *btn_open   = IupButton("Open BMP", NULL);
    Ihandle *btn_save   = IupButton("Save BMP", NULL);
    Ihandle *btn_gray   = IupButton("Grayscale", NULL);
    Ihandle *btn_invert = IupButton("Invert", NULL);
    Ihandle *btn_bright = IupButton("Brightness", NULL);
    Ihandle *btn_flip   = IupButton("Flip H", NULL);
    Ihandle *btn_rotate = IupButton("Rotate 90°", NULL);
    Ihandle *btn_blur   = IupButton("Blur", NULL);
    Ihandle *btn_undo   = IupButton("Undo", NULL);

    g_label_status  = IupLabel("Click 'Open BMP' to load an image.");
    g_image_display = IupLabel(NULL);

    IupSetCallback(btn_open,   "ACTION", (Icallback)cb_open);
    IupSetCallback(btn_save,   "ACTION", (Icallback)cb_save);
    IupSetCallback(btn_gray,   "ACTION", (Icallback)cb_grayscale);
    IupSetCallback(btn_invert, "ACTION", (Icallback)cb_invert);
    IupSetCallback(btn_bright, "ACTION", (Icallback)cb_brightness);
    IupSetCallback(btn_flip,   "ACTION", (Icallback)cb_flip);
    IupSetCallback(btn_rotate, "ACTION", (Icallback)cb_rotate);
    IupSetCallback(btn_blur,   "ACTION", (Icallback)cb_blur);
    IupSetCallback(btn_undo,   "ACTION", (Icallback)cb_undo);

    Ihandle *hbox1 = IupHbox(btn_open, btn_save, btn_undo, NULL);
    Ihandle *hbox2 = IupHbox(btn_gray, btn_invert, btn_bright, btn_flip, btn_rotate, btn_blur, NULL);
    IupSetAttribute(hbox1, "GAP", "8");
    IupSetAttribute(hbox2, "GAP", "8");

    Ihandle *scrollbox = IupScrollBox(g_image_display);
    IupSetAttribute(scrollbox, "EXPAND", "YES");

    Ihandle *vbox = IupVbox(hbox1, hbox2, scrollbox, g_label_status, NULL);
    IupSetAttribute(vbox, "MARGIN", "10x10");
    IupSetAttribute(vbox, "GAP", "10");

    Ihandle *dialog = IupDialog(vbox);
    IupSetAttribute(dialog, "TITLE", "Image Editor (C & IUP)");
    IupSetAttribute(dialog, "RASTERSIZE", "800x600");

    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
    IupMainLoop();

    if (g_current_image) free_image(g_current_image);
    if (g_undo_image) free_image(g_undo_image);
    IupClose();
    return 0;
}
