// gcc test.c -o test -lxcb -lX11 && ./test

// A property is a collection of named, typed data. The window system has a set of predefined properties (for example, the name of a window, size hints, and so on), and users can define any other arbitrary information and associate it with windows. Each property has a name, which is an ISO Latin-1 string. For each named property, a unique identifier (atom) is associated with it. A property also has a type, for example, string or integer. These types are also indicated using atoms, so arbitrary new types can be defined. Data of only one type may be associated with a single property name. Clients can store and retrieve properties associated with windows. For efficiency reasons, an atom is used rather than a character string. XInternAtom() can be used to obtain the atom for property names.


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

#define N_NAMES (1<<14)


char* strdup(const char* s) {
  int n = strlen(s) + 1;
  char* dup = malloc(n);
  strcpy(dup, s);
  return dup;
}

double get_time (void) {
  struct timeval timev;           
  gettimeofday(&timev, NULL);
  return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}


// ------------------------------------------------------------------------------------------------
// Request atoms one by one.
void useXlib(char** names, Display* display){
  Atom atoms[N_NAMES];
  for(int i = 0; i < N_NAMES; ++i)
    atoms[i] = XInternAtom(display, names[i], 0);
}

// Request all atoms at once.
void useXlibProperly(char **names, Display *display ){
  Atom atoms[N_NAMES];
  XInternAtoms(display, names, N_NAMES, 0, atoms);
}

void useXCBPoorly(char** names, xcb_connection_t* conn){
  xcb_atom_t atoms[N_NAMES];
  // Bad use of xcb: we use the cookie immediately after posting the request with xcb_intern_atom
  for(int i=0; i<N_NAMES; ++i){
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 0, strlen(names[i]), names[i]);  // Do query
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(conn, cookie, NULL);  // Get reply

    atoms[i] = reply->atom;
    free(reply);
  }
  // now we have our atoms (replies), but this is just a demo, so we do nothing with them
}

void useXCBProperly(char** names, xcb_connection_t* conn){
  xcb_atom_t atoms[N_NAMES];
  xcb_intern_atom_cookie_t cookies[N_NAMES];
  // Make all requests first, THEN check for replies!

  for(int i=0; i<N_NAMES; ++i)  // Do all queries first!
    cookies[i] = xcb_intern_atom(conn, 0, strlen(names[i]), names[i]);

  for(int i = 0; i < N_NAMES; ++i){  // THEN get replies!
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(conn, cookies[i], NULL);

    atoms[i] = reply->atom;
    free(reply);
  }
  // now we have our atoms (replies), but this is just a demo, so we do nothing with them
}


// ------------------------------------------------------------------------------------------------
int main () {
  char(**names) = malloc(N_NAMES * sizeof(*names));

  for(int i=0; i<N_NAMES; ++i){
    char buf[100];
    sprintf(buf, "NAME%d", i);
    names[i] = strdup(buf);
  }

  double start, XlibTime, XlibGoodTime, XCBBadTime, XCBGoodTime;

  /* test Xlib */
  Display* display = XOpenDisplay(NULL);
  start = get_time();
  useXlib(names, display);
  XlibTime = get_time() - start;
  start = get_time();
  useXlibProperly(names, display);
  XlibGoodTime = get_time() - start;
  XCloseDisplay(display);

  /* test XCB */
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  start = get_time();
  useXCBPoorly (names, connection);
  XCBBadTime = get_time() - start;   
  start = get_time ();
  useXCBProperly(names, connection);
  XCBGoodTime = get_time() - start;
  xcb_disconnect(connection);

  printf("bad  Xlib time  %.3f\n", XlibTime);
  printf("bad  xcb time   %.3f\n", XCBBadTime);
  printf("good Xlib time  %.3f\n", XlibGoodTime);
  printf("good xcb time   %.3f\n", XCBGoodTime);
  printf("good Xlib / bad  Xlib  %.3f\n", XlibGoodTime / XlibTime);
  printf("good xcb  / bad  xcb   %.3f\n", XCBGoodTime / XCBBadTime);
  puts("");
  printf("bad  Xlib / bad  xcb   %.3f\n", XlibTime / XCBBadTime);
  printf("good Xlib / good xcb   %.3f\n", XlibGoodTime / XCBGoodTime);
}    
