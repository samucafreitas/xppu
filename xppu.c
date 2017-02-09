/*
 * X Pied Piper Utility - Xppu
 *
 * Written by Sam Uel -> https://github.com/samucafreitas
 *
 * Last updated 06/02/2017
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdlib.h>
#include <stdio.h>
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
GString *getWinName(Display *dpy);

void usage() {
    static const char help[] =
    "usage: %s [-options ...]\n\n"
    "where options include:\n"
    "    -resolution                    print out display resolution\n"
    "    -mouseposition                 returns the current mouse position\n"
    "    -mousemove x y                 move the mouse to the x,y coordinates\n"
    "    -winfocus                      returns the title of the current focused window\n\n";

    fprintf (stderr, help, prog_name);
    exit (1);
}

int
main(int argc, char ** argv)
{
    int screen_num;
    Display *dpy;
    Window root_win;

    INIT_NAME;

    // To open a connection to the X server
    dpy = XOpenDisplay(NULL); 

    if (!dpy) {
        fprintf(stderr, "unable to connect to display");
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
            printf("mm=%dx%d\npx=%dx%d\n", res.dwidth, res.dheight, res.dwidthMM, res.dheightMM);
            continue;
        }

        if (!strcmp(argv[0], "-mouseposition"))
        {
            struct mousePos mpos;
            if(getMousePosition(dpy, root_win, &mpos))
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

        if (!strcmp(argv[0], "-winfocus"))
        {
            printf("winfocus=%s\n", getWinName(dpy)->str);
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

GString *
getWinName(Display *dpy)
{
    /*
     * I was going to use XfetchName(), but it doesn't always return a name
     * See: https://github.com/herbstluftwm/herbstluftwm/issues/64
     * See: https://stackoverflow.com/questions/8825661/xfetchname-always-returns-0
     */

    XTextProperty text_prop;
    char **list = NULL;
    GString *win_name;
    int count = 0;
    Window focus = getWinFocus(dpy); 

    if (XGetWMName(dpy, focus, &text_prop) != 0 && Xutf8TextPropertyToTextList(dpy, &text_prop, &list, &count) == 0 && count > 0)
        win_name = g_string_new(list[0]);
    else
        return g_string_new(""); //no title

    if (text_prop.value != NULL)
        XFree(text_prop.value);

    if (list != NULL)
        XFreeStringList(list);

    return win_name;
}
