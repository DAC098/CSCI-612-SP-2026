INCLUDE_DIRS = -I/usr/include/opencv4
LIB_DIRS = 
CC=g++

CDEFS=
CFLAGS=--std=c++17 -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS=-lpthread -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lrt

PRODUCT=custom_capture objectDetection sharpen_grid

HFILES= 
CFILES= sharpen_grid.c custom_capture.cpp objectDetection.cpp

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.NEW *~ *sharp*.ppm
	-rm -f ${PRODUCT} ${DERIVED} ${GARBAGE}

sharpen_grid: sharpen_grid.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ sharpen_grid.o $(LIBS)

custom_capture: custom_capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv4` $(LIBS)

objectDetection: objectDetection.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv4` $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o: $(SRCS)
	$(CC) $(CFLAGS) -c $<
