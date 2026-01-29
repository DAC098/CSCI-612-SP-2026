# CSCI 612 - Assignment 1

## Q1

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

1. 

