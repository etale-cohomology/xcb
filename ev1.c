// gcc ev1.c -o ev1 -lxcb && ./ev1

#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>


void print_modifiers(uint32_t mask){
  const char** mod, * mods[] = {"Shift", "Lock", "Ctrl", "Alt",
                               "Mod2", "Mod3", "Mod4", "Mod5",
                               "Button1", "Button2", "Button3", "Button4", "Button5"
  };
  printf("Modifier mask: ");
  for(mod = mods ; mask; mask >>= 1, mod++)
    if(mask & 1)
      printf("%s\n", *mod);
  putchar('\n');
}


// ------------------------------------------------------------------------------------------------
int main (){
  xcb_connection_t* c;
  xcb_screen_t* screen;
  xcb_window_t win;
  xcb_generic_event_t* g_event;
  uint32_t mask;
  uint32_t values[2];

  c = xcb_connect(NULL, NULL);  /* Open the connection to the X server */
  screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;  /* Get the first screen */
  win = xcb_generate_id(c);  /* Ask for our window's Id */

  /* Create the window */
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;

  // connection  depth  window_ID  parent  x,y  WxH  border_width  class  visual  masks
  xcb_create_window(c, 0, win, screen->root, 0,0, 150,150, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, mask, values);

  xcb_map_window(c, win);
  xcb_flush(c);

  // ----------------------------------------------------------------------------------------------
  while(g_event = xcb_wait_for_event(c)){

    switch (g_event->response_type & ~0x80){

      case XCB_EXPOSE:{
        xcb_expose_event_t* ev = (xcb_expose_event_t*)g_event;
        printf("window %d exposed. x,y %d,%d  WxH %dx%d\n", ev->window, ev->x, ev->y, ev->width, ev->height);
        break;
      }
      case XCB_KEY_PRESS:{
        xcb_key_press_event_t* ev = (xcb_key_press_event_t*)g_event;
        // print_modifiers(ev->state);
        printf("Press!  window %d\n", ev->event);
        break;
      }
      case XCB_KEY_RELEASE:{
        xcb_key_release_event_t* ev = (xcb_key_release_event_t*)g_event;
        // print_modifiers(ev->state);
        printf("Release!  window %d\n", ev->event);
        break;
      }
      default:
        printf("Unknown event: %d\n", g_event->response_type);
        break;

    }  // END switch!

    free(g_event);  /* Free the Generic Event */
  }  // END while!

  return 0;
}