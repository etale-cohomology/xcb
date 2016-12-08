// sudo apt-get install libxcb-util0-dev

// XCB and Xlib CAN'T handle gif/png/jpeg/tiff. For display in X, these must be converted into Xbitmaps/Xpixmaps using by the user!

// Pixmap is the ONLY format supported by the X protocol. ANYTHING to be DISPLAYED must be in this format!

// struct xcb_image_t
// A structure that describes an xcb_image_t. Definition @ line 92 of xcb_image.h

// void* base  Malloced block of storage that will be freed by xcb_image_destroy() if non-null
// xcb_image_order_t bit_order Bit order of scanline unit for xy-bitmap and xy-pixmap
// uint8_t bpp Storage per pixel in bits. Must be >= depth. Valid bpp are 1, 4, 8, 16, 24, 32 for zpixmap, 1 for xybitmap, anything for xypixmap
// xcb_image_order_t byte_order  Component byte order for z-pixmap, byte order of scanline unit for xybitmap/xypixmap. Nybble order for z-pixmap when bpp==4
// uint8_t * data  The actual image
// uint8_t   depth Depth in bits. Valid depths are 1,4,8,16,24 for z format, 1 for xy-bitmap-format, anything for xy-pixmap-format
// xcb_image_format_t  format
// uint16_t  height  Height in pixels
// uint32_t  plane_mask  When xypixmap of depth>1, this says which planes are "valid". Used by xcb_image_get/put_pixel, set by xcb_image_get
// uint8_t   scanline_pad  Right pad in bits. Valid pads are 8, 16, 32
// uint32_t  size  Size of image data in bytes. Computable from other data, but cached for convenience
// uint32_t  stride  Bytes per image row. Computable from other data, but cached for convenience
// uint8_t   unit  Scanline unit in bits for xypixmap and bpp==1: valid values are 8,16,32. Else will be max(8, bpp). Must be >= bpp
// uint16_t  width Width in pixels, excluding pads etc

#include <mathisart.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#define W 256
#define H 256
#define C 4

#define F XCB_IMAGE_FORMAT_Z_PIXMAP  // 2
#define S 32  // 32
#define D 24  // 24
#define B 32  // 32
#define U 32  // 0
#define O XCB_IMAGE_ORDER_LSB_FIRST  // 0

#define W_INPUT XCB_WINDOW_CLASS_INPUT_OUTPUT
#define W_EVENT XCB_CW_EVENT_MASK

void paint(uint8* buffer, uint w, uint h){
  printf("painting... %ux%u\n", w, h);
  for(uint i=0; i<w; i++){
    for(uint j=0; j<h; j++){
      *buffer++ = rand()%256;  // b
      // *buffer++ = rand()%256;  // g
      // *buffer++ = 0;  // r
      // *buffer++;  // a
    }
  }
}


// ------------------------------------------------------------------------------------------------
int main(){
  xcb_generic_event_t* event;
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup(connection)).data;
  const xcb_setup_t* setup = xcb_get_setup(connection);
  uint window_val = XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS;
  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window(connection, D, window, screen->root, 10,10, W,H, 0, W_INPUT, 0, W_EVENT, &window_val);
  printf("screen->root %u\n", screen->root);

  // ----------------------------------------------------------------------------------------------
  uint8 buffer[W*H*C];  // Can be bigger than actual size, but not smaller!

  xcb_image_format_t image_format = F;
  uint8 spad = S;   // Valids: 8,16,32
  uint8 depth = D;  // Valids: 1,4,8,16,24 zpixmap, 1 xybitmap, anything xypixmap
  uint8 bpp = B;    // Valids: 1,4,8,16,24,32 zpixmap, 1 xybitmap, anything xypixmap
  uint8 unit = U;   // Valids: 8,16,32
  xcb_image_order_t byte_order = setup->image_byte_order;
  xcb_image_order_t bit_order = O;
  uint size = W*H*C;  // In bytes! Can be bigger than actual size, but not smaller!

  xcb_image_t* image = xcb_image_create(W, H, image_format, spad, depth, bpp, unit, byte_order, bit_order, buffer, size, buffer);
  printf("%ux%u  format %u  pad %u  d %u  bpp %u  unit %u  byte_order %u bit_order %u  size %u\n",
    W, H, image_format, spad, depth, bpp, unit, byte_order, bit_order, size);

  // ----------------------------------------------------------------------------------------------
  xcb_pixmap_t pixmap = xcb_generate_id(connection);
  xcb_create_pixmap(connection, depth, pixmap, window, W, H);

  xcb_gcontext_t gc = xcb_generate_id(connection);
  xcb_create_gc(connection, gc, pixmap, 0, 0);

  // ----------------------------------------------------------------------------------------------
  xcb_map_window(connection, window);
  xcb_flush(connection);

  while(event = xcb_wait_for_event(connection)){
    switch (event->response_type) {
      case XCB_EXPOSE:
        paint(buffer, W, H);
        xcb_image_put(connection, pixmap, gc, image, 0, 0, 0);
        xcb_copy_area(connection, pixmap, window, gc, 0,0,0,0, image->width,image->height);  // src, dst
        xcb_flush(connection);
       break;
      case XCB_KEY_PRESS:
        paint(image->data, image->width, image->height);
        xcb_image_put(connection, pixmap, gc, image, 0, 0, 0);
        xcb_copy_area(connection, pixmap, window, gc, 0,0,0,0, image->width,image->height);  // src, dst
        xcb_flush (connection);
        break;
    }
  }

  xcb_free_pixmap(connection, pixmap);
  xcb_disconnect (connection);
  m_exit_success();
}
