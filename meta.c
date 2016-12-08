// gcc meta.c -o meta -lxcb && ./meta

#include <mathisart.h>
#include <xcb/xcb.h>


// ------------------------------------------------------------------------------------------------
struct App{
  xcb_connection_t* connection;
  xcb_screen_iterator_t screen_iter;
};


void m_app_init(struct App* app){
  puts("Connecting to the X server through XCB!");
  app->connection = xcb_connect(NULL, NULL);
  app->screen_iter = xcb_setup_roots_iterator(xcb_get_setup(app->connection));

  xcb_screen_t* screen = app->screen_iter.data;
  printf("root %u  %ux%u:%u  b %x  w %x  allowed_depths %u\n",
    screen->root, screen->width_in_pixels, screen->height_in_pixels, screen->root_depth,
    screen->black_pixel, screen->white_pixel,screen->allowed_depths_len);
}


void m_app_exit(struct App* app){
  xcb_disconnect(app->connection);
  m_exit_success();
}


// ------------------------------------------------------------------------------------------------
int main(){
  struct App app;
  m_app_init(&app);

  m_app_exit(&app);
}
