// A minimal, fast, hackable text editor! IN DEVELOPMENT

#include <mathisart.h>
#include <xcb/xcb_keysyms.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#define XK_LATIN1
#include <X11/keysymdef.h>


// ------------------------------------------------------------------------------------------------
// Customize these at will! :D
#define W   1070
#define H   1070
#define DX  0  // Pad for x
#define DY  0  // Pad for y

#define RGB_BG 0x0a0a0a
#define RGB_FG 0xf0f0f0

#define FONT_H 8

#define MAX_ROWS 130
#define MAX_COLS 160

// ------------------------------------------------------------------------------------------------
// A few global structs to hold the global state of the app!

struct App{
  xcb_connection_t*     connection;
  const xcb_setup_t*    setup;
  xcb_screen_iterator_t screen_iter;
  int                   rem;
  int                   index;
  xcb_screen_t*         screen;
  xcb_window_t          root_window;
  xcb_visualid_t        root_visual;

  xcb_window_t          window;
  xcb_gcontext_t        gcontext;
  xcb_font_t            font;

  xcb_generic_event_t*  event;
  xcb_key_symbols_t*    key_symbols;
  xcb_keysym_t keysym;  // This holds the input!
};
struct App app;

struct Key{  // 0 or 1 flags for keyboard input!
  uint printable, newline, exit, erase, arrow, bol, eol;
  uint control_s, control_o, control_shift_s;
};
struct Key key;

struct Buffer{  // An abstraction for the text! We'll embed this in a 2D space called a Canvas!
  uint_fast8 data[1<<20];
  uint64_t x;
  uint64_t len;  // Total number of chars, INCLUDING non-printable ones!
  uint64_t h;  // Number of linefeeds!
};
struct Buffer buffer;

struct Canvas{  // An abstraction for the viewport!
  uint x, y;
};
struct Canvas canvas;

void app_init(){
  app.connection  = xcb_connect(NULL, NULL);  // Given
  app.setup       = xcb_get_setup(app.connection);  // Given
  app.screen_iter = xcb_setup_roots_iterator(app.setup);  // Given
  app.screen      = app.screen_iter.data;  // Given
  app.rem         = app.screen_iter.rem;  // Given
  app.index       = app.screen_iter.index;  // Given
  app.root_window = app.screen->root;  // Given
  app.root_visual = app.screen->root_visual;  // Given

  app.window      = xcb_generate_id(app.connection);  // To create
  app.gcontext    = xcb_generate_id(app.connection);  // To create
  app.font        = xcb_generate_id(app.connection);  // To create

  app.key_symbols = xcb_key_symbols_alloc(app.connection);  // To receive from user
}

void app_exit(){
  free(app.event);
  xcb_free_gc(app.connection, app.gcontext);
  xcb_key_symbols_free(app.key_symbols);
  xcb_disconnect(app.connection);
  m_exit_success();
}

void app_flush(){ xcb_flush(app.connection); }
void app_clear(){ xcb_clear_area(app.connection, 0, app.window, 0,0, W,H); }

void app_create_window(){
  uint32 window_mask        = XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK;
  uint32 gcontext_mask      = XCB_GC_FOREGROUND|XCB_GC_BACKGROUND|XCB_GC_FONT;
  uint32 window_attrs[2]    = {RGB_BG, XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS};
  uint32 gcontext_attrs[3]  = {RGB_FG, RGB_BG, app.font};

  xcb_create_window(app.connection, XCB_COPY_FROM_PARENT, app.window, app.root_window, 0,0, W,H, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, app.root_visual, window_mask, window_attrs);
  xcb_map_window(app.connection, app.window);

  xcb_open_font(app.connection, app.font, 4, "6x10");
  xcb_create_gc(app.connection, app.gcontext, app.window, gcontext_mask, gcontext_attrs);
  xcb_close_font(app.connection, app.font);

  app_flush();
}

void app_draw(){
  for(uint i=1; i<=canvas.y; ++i)
    xcb_image_text_8(app.connection, m_min(buffer.len, MAX_COLS), app.window, app.gcontext, DX, (FONT_H+DY)*i, buffer.data);
  app_flush();
}

void buffer_init(){
  buffer.x = 0;
  buffer.len = 0;
  buffer.h = 0;
  canvas.x = 1;
  canvas.y = 1;
}

// Input commands!
void app_key_conditions(){
  app.keysym = xcb_key_press_lookup_keysym(app.key_symbols, (xcb_key_press_event_t*)app.event, 0);
  key.printable = app.keysym>=XK_space && app.keysym<=XK_asciitilde;
  key.newline = app.keysym==XK_Return;
  key.exit = app.keysym == XK_Escape;
}


// ------------------------------------------------------------------------------------------------
void app_main_loop(){
  while(1){
    app.event = xcb_poll_for_event(app.connection);  // NO BLOCKING! Return NULL if no event arrives!
    // app.event = xcb_wait_for_event(app.connection);  // Blocks till an event/error arrives!
    if(app.event){

      switch(app.event->response_type){

        case XCB_EXPOSE:
          puts("XCB_EXPOSE");
          break;

        case XCB_KEY_PRESS:
          app_key_conditions();

          if(key.printable){
            app_clear();
            buffer.data[buffer.x] = app.keysym;
            buffer.x   += 1;
            buffer.len += 1;
            buffer.h   += 0;
            canvas.x   += 1;
            canvas.y = m_min(canvas.y+0, MAX_ROWS);
            app_draw();
          }else if(key.newline){
            app_clear();
            buffer.data[buffer.x] = app.keysym;
            buffer.x   += 1;
            buffer.len += 1;
            buffer.h   += 1;
            canvas.x   += 0;
            canvas.y = m_min(canvas.y+1, MAX_ROWS);
            app_draw();
          }else if(key.exit)  return;
          else                puts("Unsupported key!");

      }  // END main switch!

    }  // END if!
  } // END while!
}


// ------------------------------------------------------------------------------------------------
int main(){
  app_init();
  buffer_init();
  app_create_window();
  app_main_loop();
  app_exit();
}
