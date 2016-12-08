#include <mathisart.h>

#include <X11/Xlib-xcb.h>  // Includes Xlib.h and xcb.h!
#include <GL/glx.h>

#define W 1024
#define H 1024

void draw(){
  glClearColor(0.2, 0.4, 0.9, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void main_loop(Display *display, xcb_connection_t *connection, xcb_window_t window, GLXDrawable drawable){
  int running = 1;
  while(running){
    xcb_generic_event_t* event = xcb_wait_for_event(connection);

    switch(event->response_type){
      case XCB_KEY_PRESS:
        running = 0;
        break;
      case XCB_EXPOSE:  // Draw and swap buffers!
        draw();
        glXSwapBuffers(display, drawable);
        break;
    }
  }
}


// ---------------------------------------------------------------------------------------
int main(){
  Display* display = XOpenDisplay(0);
  int default_screen = DefaultScreen(display);
  xcb_connection_t* connection = XGetXCBConnection(display);  /* Get the XCB connection from the display */
  XSetEventQueueOwner(display, XCBOwnsEventQueue);  /* Acquire event queue ownership */
  xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  // ----------------------------------------------------------------------------------------------
  int visualID = 0;

  GLXFBConfig *fb_configs = 0;
  int num_fb_configs = 0;
  fb_configs = glXGetFBConfigs(display, default_screen, &num_fb_configs);

  /* Select first framebuffer config and query visualID */
  GLXFBConfig fb_config = fb_configs[0];
  glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

  GLXContext context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);

  xcb_colormap_t colormap = xcb_generate_id(connection);
  xcb_window_t window = xcb_generate_id(connection);

  xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visualID);

  uint32 eventmask = XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS;
  uint32 valuelist[] = { eventmask, colormap, 0 };
  uint32 valuemask = XCB_CW_EVENT_MASK|XCB_CW_COLORMAP;

  xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0,0, W, H, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, visualID, valuemask, valuelist);
  xcb_map_window(connection, window);  // NOTE: window must be mapped before glXMakeContextCurrent

  GLXDrawable drawable = 0;
  GLXWindow glxwindow = glXCreateWindow(display, fb_config, window, 0);  /* Create GLX Window */

  drawable = glxwindow;

  // -----------------------------------------------------------------------
  // THIS is the slow part! Every until this part is FAST!
  glXMakeContextCurrent(display, drawable, drawable, context);  /* make OpenGL context current */

  // -----------------------------------------------------------------------
  main_loop(display, connection, window, drawable);  /* run main loop */

  // ----------------------------------------------------------------------------------------------
  glXDestroyWindow(display, glxwindow);
  xcb_destroy_window(connection, window);
  glXDestroyContext(display, context);
  XCloseDisplay(display);
}

// xcb_glx_make_current (xcb_connection_t      *c  /**< */,
//                       xcb_glx_drawable_t     drawable  /**< */,
//                       xcb_glx_context_t      context  /**< */,
//                       xcb_glx_context_tag_t  old_context_tag  /**< */);
