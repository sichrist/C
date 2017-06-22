#include <stdio.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

void show_clip(Display *display, Window target_window, Atom target_property)
{
	Atom da, incr, type;
	int di;
	unsigned long size, dul;
	unsigned char *prop_ret = NULL;


	XGetWindowProperty(display, target_window, target_property, 0, 0, False, AnyPropertyType,
                       &type, &di, &dul, &size, &prop_ret);
    	XFree(prop_ret);

	incr = XInternAtom(display, "INCR", False);

	if (type == incr) goto too_large;

	XGetWindowProperty(display, target_window, target_property, 0, size, False, AnyPropertyType,
                       &da, &di, &dul, &dul, &prop_ret);

	printf("Inhalt: %s \n",prop_ret);
	fflush(stdout);
	XFree(prop_ret);
	XDeleteProperty(display, target_window, target_property);

	return;

	too_large:
	        printf("Data too large and INCR mechanism not implemented\n");
        	return;
}

/* https://www.uninformativ.de/blog/postings/2017-04-02/0/POSTING-de.html */

int main()
{
	Display *display;
	Window owner, target_window, root;
	Atom sel, target_property, utf8;
	XEvent ev;
	XSelectionEvent *sev;
	int screen;

	display = XOpenDisplay(NULL);
		if(!display) goto disp_error;

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	target_window = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);
    	owner = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);

	sel = XInternAtom(display, "CLIPBOARD", False);
	utf8 = XInternAtom(display, "UTF8_STRING", False);
	target_property = XInternAtom(display, "TARGET_", False);
	
	XSelectInput(display, owner, SelectionClear | SelectionRequest);
	XSelectInput(display, target_window, SelectionNotify);
	
	XSetSelectionOwner(display, sel, owner, CurrentTime);

	for(;;)
	{

		XNextEvent(display, &ev);

		switch(ev.type)
		{

			/* If owner of CLIPBOARD changed, request it's content */
			case SelectionClear:
				XConvertSelection(display,sel,utf8,target_property ,target_window, CurrentTime);
				break;

			/* If CLIPBOARD is owned and we get a request ( paste ) we need to handle it (see link above main)*/
			case SelectionRequest:
				//TODO: Not implemented yet
				break;

			/* Content arrived at target_property */
			case SelectionNotify:
				sev = (XSelectionEvent*)&ev.xselection;

                		if (sev->property != None)
					show_clip(display, target_window, target_property);

				/* Claim the Clipboard to make sure that we are notified when someone want to change the CLIPBOARD */
				XSetSelectionOwner(display, sel, owner, CurrentTime);
				break;
		}
	}

	disp_error:
		fprintf(stderr, "Could not open X display\n");
		return 1;
}
