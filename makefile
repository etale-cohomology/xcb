CC  		= gcc-6
# FLAGS		= -std=c11 -D_GNU_SOURCE
FAST 		= -Ofast -mavx2 -funroll-loops -mfpmath=both -march=native -m64

XCB 				= -lxcb
XCB_SHM			= -lxcb-shm
XCB_IMAGE		= -lxcb-image
XCB_KEYSYMS	= -lxcb-keysyms
X11  				= -lX11
X11EXT 			= -lXext
X11_XCB 		= -lX11-xcb
GL 					= -lGL
GLU 				= -lGLU

out = draw bit line gl loop ev ev1 font g img input meta shm slide test

all: $(out)

draw: draw.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

line: line.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

bit: bit.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(XCB_IMAGE)

gl: gl.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(GL) $(X11) $(X11_XCB)

loop: loop.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(XCB_KEYSYMS)

ev: ev.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

ev1: ev1.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

font: font.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

g: g.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

img: img.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(XCB_IMAGE)

input: input.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(XCB_KEYSYMS)

meta: meta.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

shm: shm.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(XCB_IMAGE) $(XCB_SHM)

slide: slide.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB)

test: test.c
	$(CC) $(FLAGS) $^ -o $@ $(XCB) $(X11)

clean:
	rm $(out)
