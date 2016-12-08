// gcc font.c -o font -lxcb && ./font

#include <mathisart.h>
#include <stdlib.h>

#include <xcb/xcb.h>

#define W 1077
#define H 1077
#define PAD_X 1
#define PAD_Y 0

#define RGB_BG 0x0a0a0a
#define RGB_FG 0xf0f0f0

#define FONT_W 0
#define FONT_H 8


// ------------------------------------------------------------------------------------------------
int main () {
  xcb_connection_t* c = xcb_connect (NULL, NULL);
  xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;
  xcb_window_t window = xcb_generate_id(c);
  uint32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32 type = XCB_WINDOW_CLASS_INPUT_OUTPUT;
  uint32 values[2] = {RGB_BG, XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_EXPOSURE};
  xcb_create_window(c, XCB_COPY_FROM_PARENT, window, screen->root, 0,0, W,H, 0, type, screen->root_visual, mask, values);
  xcb_map_window(c, window);
  xcb_flush(c);

  xcb_gcontext_t gc = xcb_generate_id(c);
  xcb_font_t font = xcb_generate_id(c);
  xcb_open_font(c, font, 4, "6x10");
  uint32 values2[3] = {RGB_FG, RGB_BG, font};
  xcb_create_gc(c, gc, window, XCB_GC_FOREGROUND|XCB_GC_BACKGROUND|XCB_GC_FONT, values2);
  xcb_close_font(c, font);

  xcb_flush(c);

  // ----------------------------------------------------------------------------------------------
  uint_fast8 buffer[1<<10] = {0};
  uint64_t buffer_pos = 0;
  uint64_t buffer_len = 0;
  uint canvas_x = 1, canvas_y = 1;

  char* txt = "Press x!";

  while (1) {

    xcb_generic_event_t* event = xcb_poll_for_event(c);  // xcb_wait_for_event

    if(event){

      switch(event->response_type){

        case XCB_EXPOSE:
          puts("XCB_EXPOSE");
          break;

        case XCB_KEY_PRESS:{
          xcb_key_press_event_t* key = (xcb_key_press_event_t*)event;
          xcb_keycode_t keycode = key->detail;
          printf("keycode %u\n", keycode);

          uint printable = (23<keycode) && (keycode<34);
          uint quit = keycode == 9;  // esc

          if(printable){
            buffer[buffer_pos] = keycode;
            buffer_pos++;
            buffer_len++;
            // canvas_y++;

            xcb_image_text_8(c, buffer_len, window, gc, PAD_X, (FONT_H+PAD_Y)*canvas_y, buffer);
            xcb_flush(c);

          }else if(quit){
            free(event);
            xcb_free_gc(c, gc);
            xcb_disconnect(c);
            return 0;
          }

        }  // END case KEY_PRESS!

      }  // END main switch!
    }

  } // END while!

}
