# CSCI 612 - Assignment 1

## Q1

The paper talks about various applications of computer vision with some of them
being currently used while others are theoritical or under active research. The
major point of having some of these applications use cloud infrastructure to
provide faster compute and updated models. some of the major taking segments
are:

1. due to some of the constraints of embedded devices, they may not be able to 
   process everything they need fast enough but would be able to send off its
   data for processing and then return a result for continued operation.
   autonomuous vehicles being an application where having the vehicle do some
   local processing but then send off data to the cloud for more detailed
   processing.
2. another application talked about in the was the use of performing "skeletal
   transformations". the idea that you could calculate the "skeleton" of an
   object and then use that to for other purposes relating to medical
   treatments and interactive media to name some.

The skeletal transformations is an area that I have been interested in
investigating after seeing some of its uses.

## Q2

### Q2-1

1. it changes the brightness of the pixels for the image to higher or lower than
   its current values.
2. a simple camera app to alter the brightness of images take or a simple image
   modification app that can change do the same.
3. - `namedWindow` initializes a new window the the specified name and any
     additional flags to set for the window. the flag specified is
     `WINDOW_AUTOSIZE`.
   - `imshow` displays the provided image matrix to the specified named window.
     since the `WINDOW_AUTOSIZE` flag is specified the window will be adjusted to
     the size of the image but still limted to the size of the display.
   - `saturate_cast` alters a single value to be cast to the specified value and
     saturate the value if the given value is larger than the max.

### Q2-2

1. captures a video stream from the first available camera that is connected to
   the system
2. you could use this as a starting point for a simple video recording software,
   video conferencing, and motion capture.
3. - `waitKey` waits for a specific keyboard input indefinitly or with a
     specified delay and returns an integer result.
   - `VideoCapture::set` allows for setting specific parameters of the capture
     object such as frame width and height.
   - `VideoCapture::read` reads in a single frame from the video capture device
     and writes the data into a `Mat` for access.

### Q2-3

1. captures a video stream from the first available camera that is connected to
   the system. it converts each frame to gray scale to find the difference
   between the current frame and the previous. it will display the current frame
   in one window, the previous frame in a second window, and the difference
   between the two in a third window.
2. you could use this for security applications to detect movement on a camera
   feed.
3. - `putText` writes a string to a given frame with various attributes for
     font, size, thickness, etc.
   - `cvtColor` alters the color space of the input frame into another and
     writes the result to the output frame.
   - `absdiff` calculates the difference between to arrays and or a scalar and
     outputs the result into a provided array. in this instance we are providing
     frames for each argument.

## Q3



needed to modify the `IMG_HEIGHT` and `IMG_WIDTH` variables to be the proper
size for the tree image since the values are hard coded in `sharpen.c`. no other
modifications needed to be made other.

I pulled code from a different implementation of `sharpen_grid` that has
additional code included. modifications to the `IMG_HEIGHT` and `IMG_WIDTH` had
to be made here as well. an additional modification needed to be made to
properly parse out the ppm header information since the header size is also hard
coded. the primary difference between for grid is that it implements a threading
model to sharpen the image faster across multiple threads vs a single thread.

Ideally using more threads to process images faster would help with realtime
systems as it would be able to get the processed image faster after
modifications being made to it. the system in use would need to have additional
cores / threads for this implementation to work vs using dedicated hardware such
as GPU's.

![`Trees.sharpen.ppm`](./Trees.sharpen.ppm)
![`Trees.sharpen_grid.ppm`](./Trees.sharpen_grid.ppm)

## Q4

getting and running the code a bit of a task. I switched to the version of
opencv that is on the Jetson and ran that code. encounered some issues with how
exactly to feed the xml files to the application but managed to get it working.
modified the code to take in a single image, perform the face detection, and
output the result of the calculation to a file. below is the result of running
the face detection with `haarcascade_eye.xml` and
`haarcascade_frontalface_default.xml`.

![`sam_detect.png`](./sam_detect.png)

further testing can be done with the other detection files that opencv provides
to see if better results can be achieved.

Note: the application still references file paths that are expected to be in the
structure of the opencv project directory so be sure to supply the file paths.

## Q5

The application is fairly straight forward in that takes a given camera input
and will provide some simple display information such as total frames rendered,
fps, and frame time. you can optionally display a crosshair in the center of the
frame and also display the current system time in the lower left hand corner of
the frame.

building the application only involves running the `Make` file located in the
folder and ensuring that opencv4 is available on the system to be linked with.
once the application has been built you can provide the following options to
customize the execution of the program:

```
Usage: custom_capture [params] 

	--camera (value:0)
		Camera device number.
	-h, --help
		displays this help message.
	--low-res
		changes the resolution down to 320x240.
	--med-res
		changes the resolution down to 640x480.
	--show-ch
		displays the crosshair in the center of the frame.
	--show-ts
		displays the current timstamp for the system.
```

Testing the application mostly involved just running the application and fixing
issues as they came up. there was no additional automated testing or unit tests.
