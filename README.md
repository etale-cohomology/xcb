# XCB examples

XCB, the X-protocol C Binding, is the lowest level API to talk to the X server. It's a replacement for the decades-old __Xlib__ API.

This repo contains a few simple examples of XCB usage.


## XCB notes

__Xlib__ leads to __high latency__. Requests needing a reply are __synchronous__, __blocking__ till the reply comes  
XCB is a direct binding of X protocol operations to C functions  
The XCB API has 2 layers: `XCB_Connection` (lower layer), and `XCB_Protocol` (higher layer)  
`XCB_connection`: connection to the X server, buffering, batching of requests/replies  
`XCB_Protocol`: direct C API to the X protocol  
XCB is __thread safe__ (lots of Pthreads __locking__): each connection is locked against concurrent access with a `pthread_mutex`, there's blocking during API calls through `pthread_cond` conditional variables  
XCB is best w/ __2 threads__: one for __requests/replies__, one to __process events__!  
Unlike Xlib, there can only be __one connection per display__!  
There's __no notion of a screen__  


# X and Xlib notes

__Xlib__ and __XCB__ are the lowest level programming interfaces to X  
__X11R6__ is the __protocol__, __Xlib/XCB__ is the __API__ to the protocol  
X is network-based: apps can run on a remote computer, and the results displayed on a local computer  
X is a window system for __bitmapped graphics displays__ (aka __memory-mapped graphics__)  
In __bitmapped graphics__, each __dot/pixel__ on the screen maps to one or more __bits in memory__. Apps __alter the display__ simply by __writing to display memory__  
The __X protocol__ has about __120 requests__!  

A __display__ is a computer with a keyboard, a mouse, and one or more screens. Multiple screens can work together as __a single display__, with mouse movement allowed to cross physical screen boundaries
The program that __controls each display__ is known as a __server__
Graphics are performed by the display server rather than by the client
The server maintains complex data structures, including windows, cursors, fonts, and "graphics contexts," as resources that can
be shared between clients and referred to simply by resource IDs. 

__Xlib calls__ are translated to __protocol requests__ sent via __TCP/IP__ or over __Unix domain protocol__ (IPC?), either to the local server or to another server across the network  

X was designed to allow __any__ style of user interface  



A single X server can provide display services for any number of screens. A set of screens for a single user with one keyboard and one pointer (usually a mouse) is called a display.
X also provides off-screen storage of graphics objects, called pixmaps. Single plane (depth 1) pixmaps are sometimes referred to as bitmaps.
Most of the functions in Xlib just add requests to an output buffer. These later execute asynchronously on the X server.
Functions that return values of information stored in the server do not return (that is, they __block__) until an explicit reply is received or an error occurs.
Input events (for example, a key pressed or the pointer moved) arrive asynchronously from the server and are queued until they are requested by an explicit call (for example, XNextEvent or XWindowEvent).

A host may start several X server at the same time from different terminals (TTY in GNU/LINUX), this way it allows several displays to be used.



X was designed to provide windows on bitmapped terminals. The code was designed to stress easy portability  
the developers of X decided that it should not imply a particular style of user interface  
To work over a network, there had to be programs running at both ends of the connection to send and receive the information and to interpret it. The end that controls the display and input devices was named the server. At the other end are clients−−programs written using Xlib to interface with the X protocol


The __win_gravity__ attribute controls the repositioning of subwindows when a parent window is resized.
__Window gravity__ is only useful for children placed against or very near the outside edges of the parent or directly in its
center: it'll be value for implementing __tabs__!



GC stores data about how to interpret graphics requests so that the same information does not have to be sent with every request
Since GC's store data in the __server__, they also __reduce network traffic__
you can create several GCs with the different characteristics you need and then switch between them. 
Data like windows, pixmaps and graphic context are resources stored server-side

__Drawing__ is done by the __server__! The __client__ merely sends __requests__, which are placed in a queue!
X has a __graphics pipeline__ that consists of __4 stages__: pixel selection, coloring (at most 2), plane masking, other masking
_Most graphics_ are __drawn__ by simply __overwriting the existing graphics__!s

XClearWindow clears an entire window. If the window has a background_ pixmap attribute, then the window is redrawn with this tile. If the window has `background_pixmap` or `background_pixel` attribute `None`, then the contents of the window are not changed
XClearWindow is not needed to clear a window before redrawing it due to an Expose event, because the server automatically draws the exposed area with the background pixel value or pixmap



Xlib stores more than a kilobyte of data about each X server connection in a structure named Display

Requests always have a “major opcode” identifying the kind of request, and a length field that measures the number of four-byte words needed to contain the entire packet. Responses come in three forms: replies, events, and errors. Replies and errors are sent in response to requests, while events are sent spontaneously.



xim/xom/xlc make up about 44% of the 150k lines of source in Xlib, and about 54% of the compiled size. The cms component contributes roughly 10%, and xkb contributes 15%, to the size of Xlib. The present build system allows Xlib to be built without each of these components, producing a build of Xlib that is about 75% smaller



What are the possible __shapes__ for drawing? Are the __only rectangles__?  
What is the `clip_mask` in the GC? What is the `plane_mask`?  
Can one __erase__ a drawing _without_ writting background pixels on top?  
