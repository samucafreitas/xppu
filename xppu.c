/*
 * Copyright 1990, 1998  The Open Group
 * Copyright (c) 2000  The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of The Open Group shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization
 * from The Open Group.
 */

/*
 * X Pied Piper Utility - Xppu
 *
 * Xppu has used snippets from xprop 1.2.0 -> https://opensource.apple.com
 *
 * File              : xppu.c
 * Author            : Sam Uel <samuelfreitas@linuxmail.org>
 * Date              : 01 feb 2017
 * Last Modified Date: 20 jun 2017
 * Last Modified By  : Sam Uel <samuelfreitas@linuxmail.org>
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <glib.h>

#define INIT_NAME prog_name=argv[0]

struct mousePos{
    int x;
    int y;
};

struct dpyRes {
    int dwidth;
    int dheight;
    int dwidthMM;
    int dheightMM;
};

char *prog_name;
void mouseMove(Display *dpy, Window win, int x, int y);
Status getMousePosition(Display *dpy, Window win, struct mousePos *mpos);
void getResolution(Display *dpy, int screen_num, struct dpyRes *res);
Window getWinFocus(Display *dpy);
GString *getWinTitle(Display *dpy, Window win);
Window selectWindow(Display *dpy, Window root);
void criteria(Display *dpy, Window win);

void
usage() {
    static const char help[] =
    "usage: %s [-options ...]\n\n"
    "where options include:\n"
    "    -resolution            print out display resolution\n"
    "    -mouseposition         returns the current mouse position\n"
    "    -mousemove x y         move the mouse to the x,y coordinates\n"
    "    -selectwin             returns (id, class, etc...) of the selected window\n"
    "    -wintitle              returns the title of the current focused window\n\n";

    fprintf (stderr, help, prog_name);
    exit (1);
}

int
main(int argc, char **argv)
{
    int screen_num;
    Display *dpy;
    Window root_win;

    INIT_NAME;

    // To open a connection to the X server
    dpy = XOpenDisplay(NULL);

    if (!dpy) {
        fprintf(stderr, "[ERROR]->unable to connect to display");
        return 7;
    }

    // attr
    screen_num = DefaultScreen(dpy);
    root_win = DefaultRootWindow(dpy);

    // Handle '-' options
    while (argv++, --argc>0 && **argv == '-')
    {
        if (!strcmp(argv[0], "-"))
            continue;

        if (!strcmp(argv[0], "-resolution"))
        {
            struct dpyRes res;
            getResolution(dpy, screen_num, &res);
            printf("px=%dx%d\nmm=%dx%d\n", res.dwidth, res.dheight, res.dwidthMM, res.dheightMM);
            continue;
        }

        if (!strcmp(argv[0], "-mouseposition"))
        {
            struct mousePos mpos;
            if (getMousePosition(dpy, root_win, &mpos))
                printf("X=%d\nY=%d\n", mpos.x, mpos.y);
            continue;
        }

        if (!strcmp(argv[0], "-mousemove"))
        {
            if (argc < 3) usage();
            int x = strtol(argv[1], NULL, 10);
            int y = strtol(argv[2], NULL, 10);
            mouseMove(dpy, root_win, x, y);
            continue;
        }

        if (!strcmp(argv[0], "-wintitle"))
        {
            printf("wintitle=%s\n", getWinTitle(dpy, getWinFocus(dpy))->str);
            continue;
        }

        if (!strcmp(argv[0], "-selectwin"))
        {
            Window target_win = selectWindow(dpy, root_win);
            criteria(dpy, target_win);
            continue;
        }

        usage();
	}

    XCloseDisplay(dpy);
    return 0;
}

void
mouseMove(Display *dpy, Window win, int x, int y)
{
    // moves the pointer by the offsets (x, y)
    XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
}

Status
getMousePosition(Display *dpy, Window win, struct mousePos *mpos)
{
    Window root_return, child_return;
    int win_x_return, win_y_return;
    unsigned int mask_return;
    Status status;

    /*
     * XQueryPointer function returns the root window the pointer is logically on
     * and the pointer coordinates relative to the root window's origin.
     */
    status = XQueryPointer(dpy, win, &root_return, &child_return, &mpos->x, &mpos->y,
                     &win_x_return, &win_y_return, &mask_return);
    return status;
}

void
getResolution(Display *dpy, int screen_num, struct dpyRes *res)
{
    // display width and height in pixels
    res->dwidth = DisplayWidth(dpy, screen_num);
    res->dheight = DisplayHeight(dpy, screen_num);

    // display width and height in millimeters
    res->dwidthMM = DisplayWidthMM(dpy, screen_num);
    res->dheightMM = DisplayHeightMM(dpy, screen_num);
}

Window
getWinFocus(Display *dpy)
{
    Window focus;
    int revert;

    // returns the focus window and the current focus state
    XGetInputFocus(dpy, &focus, &revert);

    return focus;
}

GString
*getWinTitle(Display *dpy, Window win)
{
    /*
     * I was going to use XfetchName(), but it doesn't always return a name
     * See: https://github.com/herbstluftwm/herbstluftwm/issues/64
     * See: https://stackoverflow.com/questions/8825661/xfetchname-always-returns-0
     */

    XTextProperty text_prop;
    char **list = NULL;
    GString *win_title;
    int count = 0;

    if (XGetWMName(dpy, win, &text_prop) != 0 && Xutf8TextPropertyToTextList(dpy, &text_prop, &list, &count) == 0 && count > 0)
        win_title = g_string_new(list[0]);
    else
        return g_string_new(""); //no title

    if (text_prop.value != NULL)
        XFree(text_prop.value);

    if (list != NULL)
        XFreeStringList(list);

    return win_title;
}

Window
selectWindow(Display *dpy, Window root)
{
    int status;
    Cursor cursor;
    XEvent event;
    Window target_win = None;
    int btns = 0;
    int lain_i;
    unsigned int lain_u;

    // Make the cursor
    cursor = XCreateFontCursor(dpy, XC_crosshair);

    status = XGrabPointer(dpy, root, False,
                          ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                          GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess)
        fprintf(stderr, "[ERROR]->Can't grab the mouse.");

    // select a window
    while ((target_win == None) || btns != 0) {
        XAllowEvents(dpy, SyncPointer, CurrentTime);
        XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
        switch (event.type) {
            case ButtonPress:
                if (target_win == None) {
                    if (!event.xbutton.subwindow || !XQueryPointer(dpy, event.xbutton.subwindow, &root, &target_win,
                                      &lain_i, &lain_i, &lain_i, &lain_i, &lain_u))
                        target_win = root;
                }
                btns++;
                break;
            case ButtonRelease:
                if (btns > 0) btns--;
                break;
        }
    }
    XUngrabPointer(dpy, CurrentTime);

    return target_win;
}

void
criteria(Display *dpy, Window win)
{
    XClassHint *wm;
    if (!(wm = XAllocClassHint()))
        fprintf(stderr, "[ERROR]->Insufficient memory!");

    if (XGetClassHint(dpy, win, wm))
        printf("class=\"%s\"\ninstance=\"%s\"\n", wm->res_class, wm->res_name);

    printf("id=%d\n", win);
    printf("title=\"%s\"\n", getWinTitle(dpy, win)->str);

    XFree(wm);
}
