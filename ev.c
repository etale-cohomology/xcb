#include <mathisart.h>
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
  uint32_t values[2];
  xcb_generic_event_t* g_event;
  xcb_connection_t* c = xcb_connect(NULL, NULL);  /* Open the connection to the X server */
  xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;  /* Get the first screen */
  xcb_window_t win = xcb_generate_id(c);  /* Ask for our window's Id */

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE      |XCB_EVENT_MASK_BUTTON_PRESS  |
              XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_POINTER_MOTION|
              XCB_EVENT_MASK_ENTER_WINDOW  |XCB_EVENT_MASK_LEAVE_WINDOW  |
              XCB_EVENT_MASK_KEY_PRESS     |XCB_EVENT_MASK_KEY_RELEASE;
  xcb_create_window(c, 0, win, screen->root, 0,0, 150,150, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);
  xcb_map_window(c, win);
  xcb_flush(c);

  // ----------------------------------------------------------------------------------------------
  while((g_event = xcb_wait_for_event(c))){

    switch (g_event->response_type & ~0x80){

      case XCB_EXPOSE: {
        xcb_expose_event_t* ev = (xcb_expose_event_t*)g_event;

        printf("Window %d exposed. Region to be redrawn at location (%d,%d), with dimension (%d,%d)\n",
                ev->window, ev->x, ev->y, ev->width, ev->height);
        break;
      }
      case XCB_BUTTON_PRESS: {
        xcb_button_press_event_t* ev = (xcb_button_press_event_t*)g_event;
        print_modifiers(ev->state);

        switch (ev->detail) {
        case 4:
          printf("Wheel Button up in window %d, at coordinates (%d,%d)\n",
                  ev->event, ev->event_x, ev->event_y);
          break;
        case 5:
          printf("Wheel Button down in window %d, at coordinates (%d,%d)\n",
                  ev->event, ev->event_x, ev->event_y);
          break;
        default:
          printf("Button %d pressed in window %d, at coordinates (%d,%d)\n",
                  ev->detail, ev->event, ev->event_x, ev->event_y);
        }
        break;
      }
      case XCB_BUTTON_RELEASE: {
        xcb_button_release_event_t* ev = (xcb_button_release_event_t*)g_event;
        print_modifiers(ev->state);

        printf("Button %d released in window %d, at coordinates (%d,%d)\n",
                ev->detail, ev->event, ev->event_x, ev->event_y);
        break;
      }
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*)g_event;

        printf("Mouse moved in window %d, at coordinates (%d,%d)\n",
                ev->event, ev->event_x, ev->event_y);
        break;
      }
      case XCB_ENTER_NOTIFY: {
        xcb_enter_notify_event_t* ev = (xcb_enter_notify_event_t*)g_event;

        printf("Mouse entered window %d, at coordinates (%d,%d)\n",
                ev->event, ev->event_x, ev->event_y);
        break;
      }
      case XCB_LEAVE_NOTIFY: {
        xcb_leave_notify_event_t* ev = (xcb_leave_notify_event_t*)g_event;

        printf("Mouse left window %d, at coordinates (%d,%d)\n",
                ev->event, ev->event_x, ev->event_y);
        break;
      }
      case XCB_KEY_PRESS: {
        xcb_key_press_event_t* ev = (xcb_key_press_event_t*)g_event;
        print_modifiers(ev->state);

        printf("Key pressed in window %d\n",
                ev->event);
        break;
      }
      case XCB_KEY_RELEASE: {
        xcb_key_release_event_t* ev = (xcb_key_release_event_t*)g_event;
        print_modifiers(ev->state);

        printf("Key released in window %d\n",
                ev->event);
        break;
      }
      default:
        printf("Unknown event: %d\n", g_event->response_type);
        break;

    }  // END switch!
    free(g_event);
  }  // END while!

  return 0;
}