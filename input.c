// gcc input.c -o input -lxcb -lxcb-keysyms && ./input

#include <mathisart.h>
#include <xcb/xcb_keysyms.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#define XK_LATIN1
#include <X11/keysymdef.h>


#define W 1070
#define H 1070
#define PAD_X 1
#define PAD_Y 0

#define RGB_BG 0x0a0a0a
#define RGB_FG 0xf0f0f0

#define FONT_W 0
#define FONT_H 8


// ------------------------------------------------------------------------------------------------
// A few global structs to hold the global state of the app!

struct App{
  xcb_connection_t* connection;
  xcb_screen_t* screen;
  xcb_window_t window;
  xcb_gcontext_t gc;
  xcb_font_t font;
  xcb_generic_event_t* event;
  xcb_key_symbols_t* key_symbols;
  xcb_keysym_t keysym;  // This holds the input!

  uint_fast8 buffer[1<<10];
  uint64_t buffer_pos;
  uint64_t buffer_len;

  uint canvas_x;
  uint canvas_y;

  // 0 or 1 flags!
  uint key_printable, key_newline, key_erase, key_arrow, key_exit, key_bol, key_eol;
  uint key_control_s, key_control_o, key_control_shift_s;
};
struct App app;

void app_flush(){
  xcb_flush(app.connection);
}

void app_clear(){
  xcb_clear_area(app.connection, 0, app.window, 0,0, W,H);
}

void app_draw(){
  xcb_image_text_8(app.connection, app.buffer_len, app.window, app.gc, PAD_X,
    (FONT_H+PAD_Y)*app.canvas_y, app.buffer);
  app_flush();
}


void app_init(){
  app.connection = xcb_connect (NULL, NULL);
  app.screen = xcb_setup_roots_iterator(xcb_get_setup(app.connection)).data;
  app.window = xcb_generate_id(app.connection);
  app.gc = xcb_generate_id(app.connection);
  app.font = xcb_generate_id(app.connection);

  uint32 window_mask = XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK;
  uint32 gc_mask = XCB_GC_FOREGROUND|XCB_GC_BACKGROUND|XCB_GC_FONT;
  uint32 window_attrs[2] = {RGB_BG, XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS};
  uint32 gc_attrs[3] = {RGB_FG, RGB_BG, app.font};

  xcb_create_window(app.connection, XCB_COPY_FROM_PARENT, app.window, app.screen->root, 0,0, W,H, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, app.screen->root_visual, window_mask, window_attrs);
  xcb_map_window(app.connection, app.window);

  xcb_open_font(app.connection, app.font, 4, "6x10");
  xcb_create_gc(app.connection, app.gc, app.window, gc_mask, gc_attrs);
  xcb_close_font(app.connection, app.font);

  app.key_symbols = xcb_key_symbols_alloc(app.connection);

  app_flush();
}

void buffer_init(){
  app.buffer_pos = 0;
  app.buffer_len = 0;
  app.canvas_x = 1;
  app.canvas_y = 1;
}

void app_exit(){
  free(app.event);
  xcb_free_gc(app.connection, app.gc);
  xcb_disconnect(app.connection);
  m_exit_success();
}


// ------------------------------------------------------------------------------------------------
// Input commands!

void app_key_conditions(){
  app.key_printable = app.keysym>=XK_space && app.keysym<=XK_asciitilde;
  app.key_newline = app.keysym==XK_Return;
  app.key_exit = app.keysym == XK_Escape;
}

  
// ------------------------------------------------------------------------------------------------
int main(){
  app_init();
  buffer_init();

  // ----------------------------------------------------------------------------------------------
  while(1){
    app.event = xcb_poll_for_event(app.connection);  // xcb_wait_for_event

    if(app.event){

      switch(app.event->response_type){

        case XCB_EXPOSE:
          puts("XCB_EXPOSE");
          break;

        case XCB_KEY_PRESS:
          app.keysym = xcb_key_press_lookup_keysym(app.key_symbols, (xcb_key_press_event_t*)app.event, 0);

          // ---------------------
          app_key_conditions();
          if(app.key_printable){
            app_clear();
            app.buffer[app.buffer_pos] = app.keysym;
            app.buffer_pos++;
            app.buffer_len++;
            app_draw();
          }else if(app.key_newline){
            app_clear();
            app.buffer[app.buffer_pos] = app.keysym;
            app.buffer_pos++;
            app.buffer_len++;
            app.canvas_y++;
            app_draw();
          }else if(app.key_exit){
            app_exit();
          }else{
            puts("Unsupported key!");
          }

      }  // END main switch!
    }  // END if!
  } // END while!

}
