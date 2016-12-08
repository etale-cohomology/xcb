// http://stackoverflow.com/questions/27745131

#include <mathisart.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/shm.h>


#define W 512
#define H 512


int main(){
  //connect to the X server and get screen
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  //create a window
  uint32 value_mask = XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK;
  uint32 value_list[2] = {screen->black_pixel, XCB_EVENT_MASK_EXPOSURE};
  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window(connection, screen->root_depth, window, screen->root, 0, 0, W, H, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

  //create a graphic context
  value_mask = XCB_GC_FOREGROUND|XCB_GC_GRAPHICS_EXPOSURES;
  value_list[0] = screen->black_pixel;
  value_list[1] = 0;
  xcb_gcontext_t gcontext = xcb_generate_id(connection);
  xcb_create_gc(connection, gcontext, window, value_mask, value_list);

  //map the window onto the screen
  xcb_map_window(connection, window);
  xcb_flush(connection);

  //Shm test
  xcb_shm_segment_info_t info;
  xcb_shm_query_version_reply(connection, xcb_shm_query_version(connection), NULL);

  info.shmid   = shmget(IPC_PRIVATE, W*H*4, IPC_CREAT|0777);
  info.shmaddr = shmat(info.shmid, 0, 0);
  info.shmseg = xcb_generate_id(connection);
  xcb_shm_attach(connection, info.shmseg, info.shmid, 0);
  shmctl(info.shmid, IPC_RMID, 0);
  uint8* data = info.shmaddr;

  xcb_pixmap_t pix = xcb_generate_id(connection);
  xcb_shm_create_pixmap(connection, pix, window, W, H, screen->root_depth, info.shmseg, 0);


  uint8 lala[W*H*4] = {0};
  const xcb_setup_t* setup = xcb_get_setup(connection);
  xcb_format_t* fmt = xcb_setup_pixmap_formats(setup);
  printf("scanline_pad %u  depth %u  bpp %u  byte_order %u\n",
    fmt->scanline_pad, fmt->depth, fmt->bits_per_pixel, setup->image_byte_order);
  xcb_image_t* img = xcb_image_create(W,H, XCB_IMAGE_FORMAT_Z_PIXMAP,
    fmt->scanline_pad, 24, 32, 0, setup->image_byte_order, XCB_IMAGE_ORDER_LSB_FIRST, lala, W*H*4, lala);

  // xcb_image_shm_get(connection, window, img, info, 0,0, XCB_IMAGE_FORMAT_Z_PIXMAP);
  // xcb_image_shm_put(connection, window, img, info, 0,0, XCB_IMAGE_FORMAT_Z_PIXMAP);


  // --------------------------------------------------------------
  uint i = 0;
  while(1){
    data[i++] = 0xff;
    xcb_copy_area(connection, pix, window, gcontext, 0, 0, 0, 0, W, H);
    xcb_flush(connection);
  }

  xcb_shm_detach(connection, info.shmseg);
  shmdt(info.shmaddr);

  xcb_free_pixmap(connection, pix);
  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);
}
