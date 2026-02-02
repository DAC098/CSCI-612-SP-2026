#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// See notes on images in Week-3-1 for tips on thread gridding and note that in this example, tiles
// are used for a 4:3 aspect ratio image resolution.
//
// This could be simplified into horizontal slices (rows) of the 1D (or 2D version) of the image array.
//

#define IMG_WIDTH (1280)
#define IMG_HEIGHT (960)

//#define IMG_HEIGHT (300)
//#define IMG_WIDTH (400)


// Scheme to index by simple row threads, ignoring number of columns
//#define NUM_ROW_THREADS (1)
//#define NUM_COL_THREADS (1)

//#define NUM_ROW_THREADS (2)
//#define NUM_COL_THREADS (1)

//#define NUM_ROW_THREADS (3)
//#define NUM_COL_THREADS (1)

//#define NUM_ROW_THREADS (4)
//#define NUM_COL_THREADS (1)


// Row and column threads to process in tiles for 4:3 aspect ratio
#define NUM_ROW_THREADS (3)
#define NUM_COL_THREADS (4)

//#define NUM_ROW_THREADS (6)
//#define NUM_COL_THREADS (8)

//#define NUM_ROW_THREADS (9)
//#define NUM_COL_THREADS (12)

//#define NUM_ROW_THREADS (12)
//#define NUM_COL_THREADS (16)

//#define NUM_ROW_THREADS (15)
//#define NUM_COL_THREADS (20)

//#define NUM_ROW_THREADS (18)
//#define NUM_COL_THREADS (24)

//#define NUM_ROW_THREADS (21)
//#define NUM_COL_THREADS (28)

//#define NUM_ROW_THREADS (24)
//#define NUM_COL_THREADS (32)


#define IMG_H_SLICE (IMG_HEIGHT / NUM_ROW_THREADS)
#define IMG_W_SLICE (IMG_WIDTH / NUM_COL_THREADS)

#define HEADER_SIZE (59)

#define SHARPEN_GRID_ITERATIONS (900)  // Number of times threads are created to process one image

#define FAST_IO

typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef double FLOAT;

typedef struct _threadArgs {
    int start_height;
    int start_width;
    int height;
    int width;
} threadArgsType;

pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;

struct sched_param fifo_param;

// PPM Edge Enhancement Code in row x column format
UINT8 header[HEADER_SIZE];
UINT8 R[IMG_HEIGHT][IMG_WIDTH];
UINT8 G[IMG_HEIGHT][IMG_WIDTH];
UINT8 B[IMG_HEIGHT][IMG_WIDTH];
UINT8 convR[IMG_HEIGHT][IMG_WIDTH];
UINT8 convG[IMG_HEIGHT][IMG_WIDTH];
UINT8 convB[IMG_HEIGHT][IMG_WIDTH];

// PPM format array with RGB channels all packed together
UINT8 RGB[IMG_HEIGHT * IMG_WIDTH * 3];

#define K 4.0
#define F 8.0
//#define F 80.0

FLOAT PSF[9] = { -K / F, -K / F, -K / F, -K / F, K + 1.0, -K / F, -K / F, -K / F, -K / F };

void *sharpen_thread(void *threadptr) {
    threadArgsType thargs = *((threadArgsType *)threadptr);
    FLOAT temp = 0;

    for (int i = thargs.start_height; i < thargs.height; i++) {
        for (int j = thargs.start_width; j < thargs.width; j++) {
            temp = 0;
            temp += (PSF[0] * (FLOAT)R[(i-1)][j-1]);
            temp += (PSF[1] * (FLOAT)R[(i-1)][j]);
            temp += (PSF[2] * (FLOAT)R[(i-1)][j+1]);
            temp += (PSF[3] * (FLOAT)R[(i)][j-1]);
            temp += (PSF[4] * (FLOAT)R[(i)][j]);
            temp += (PSF[5] * (FLOAT)R[(i)][j+1]);
            temp += (PSF[6] * (FLOAT)R[(i+1)][j-1]);
            temp += (PSF[7] * (FLOAT)R[(i+1)][j]);
            temp += (PSF[8] * (FLOAT)R[(i+1)][j+1]);

            if (temp < 0.0) {
                temp = 0.0;
            }

            if (temp > 255.0) {
                temp = 255.0;
            }

            convR[i][j] = (UINT8)temp;

            temp = 0;
            temp += (PSF[0] * (FLOAT)G[(i-1)][j-1]);
            temp += (PSF[1] * (FLOAT)G[(i-1)][j]);
            temp += (PSF[2] * (FLOAT)G[(i-1)][j+1]);
            temp += (PSF[3] * (FLOAT)G[(i)][j-1]);
            temp += (PSF[4] * (FLOAT)G[(i)][j]);
            temp += (PSF[5] * (FLOAT)G[(i)][j+1]);
            temp += (PSF[6] * (FLOAT)G[(i+1)][j-1]);
            temp += (PSF[7] * (FLOAT)G[(i+1)][j]);
            temp += (PSF[8] * (FLOAT)G[(i+1)][j+1]);

            if (temp < 0.0) {
                temp = 0.0;
            }

            if (temp > 255.0) {
                temp = 255.0;
            }

            convG[i][j] = (UINT8)temp;

            temp = 0;
            temp += (PSF[0] * (FLOAT)B[(i-1)][j-1]);
            temp += (PSF[1] * (FLOAT)B[(i-1)][j]);
            temp += (PSF[2] * (FLOAT)B[(i-1)][j+1]);
            temp += (PSF[3] * (FLOAT)B[(i)][j-1]);
            temp += (PSF[4] * (FLOAT)B[(i)][j]);
            temp += (PSF[5] * (FLOAT)B[(i)][j+1]);
            temp += (PSF[6] * (FLOAT)B[(i+1)][j-1]);
            temp += (PSF[7] * (FLOAT)B[(i+1)][j]);
            temp += (PSF[8] * (FLOAT)B[(i+1)][j+1]);

            if (temp < 0.0) {
                temp = 0.0;
            }

            if (temp > 255.0) {
                temp = 255.0;
            }

            convB[i][j] = (UINT8)temp;
        }
    }

    pthread_exit((void **)0);
}

FLOAT get_now() {
    static struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    return (FLOAT)now.tv_sec + (FLOAT)now.tv_nsec / 1000000000.0;
}

int main(int argc, char *argv[]) {
    int fdin, fdout, i, j, pixel;
    int bytesLeft = 0;
    int rc;
    struct timespec now, start;
    int thread_count = 4;
    int iterations = 90;

    FLOAT fstart = get_now();

    if (argc < 3) {
       printf("Usage: sharpen input_file.ppm output_file.ppm\n");

       exit(EXIT_FAILURE);
    }

    if ((fdin = open(argv[1], O_RDONLY, 0644)) < 0) {
        printf("Error opening %s\n", argv[1]);
    }

    if ((fdout = open(argv[2], (O_RDWR | O_CREAT), 0666)) < 0) {
        printf("Error opening %s\n", argv[2]);
    }

    if (argc >= 4) {
        sscanf(argv[3], "%d", &thread_count);
    }

    if (argc >= 5) {
        sscanf(argv[4], "%d", &iterations);
    }

    pthread_t* threads = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

    if (threads == NULL) {
        fprintf(
            stderr,
            "failed to allocate memory for thread ids. %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    printf("threads %p\n", threads);

    threadArgsType* threadarg = (threadArgsType*)malloc(thread_count * sizeof(threadArgsType));

    if (threadarg == NULL) {
        free(threads);

        fprintf(
            stderr,
            "failed to aloocate memory for thread args. %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    bytesLeft = HEADER_SIZE - 1;

    int bytesRead = 0;
    int readcnt = 0;

    do {
        bytesRead = read(fdin, (void *)header, bytesLeft);
        bytesLeft -= bytesRead;
    } while (bytesLeft > 0);

    for (int index = 0, counted = 0; index < HEADER_SIZE; index += 1) {
        if (counted == 0) {
            printf("%.8x", index);
        }

        if (header[index] == '\n') {
            printf(" \\n");
        } else if (header[index] == '\r') {
            printf(" \\r");
        } else if (header[index] >= ' ' || header[index] <= '~') {
            printf("  %c", header[index]);
        } else if (header[index] < 0xf0) {
            printf(" 0%x", header[index]);
        } else {
            printf(" %x", header[index]);
        }

        counted += 1;

        if (counted == 16) {
            printf("\n");
            counted = 0;
        }
    }

    printf("\n");

    header[HEADER_SIZE - 1] = '\0';

    bytesLeft = IMG_HEIGHT * IMG_WIDTH * 3;

    printf(
        "START: read 0, bytesRead = 0, bytesLeft = %d\n",
        bytesLeft
    );

    // Read in RGB data in large chunks, requesting all and reading residual
    do {
        bytesRead = read(fdin, (void *)&RGB[bytesRead], bytesLeft);
        bytesLeft -= bytesRead;
        readcnt++;

        printf(
            "read %d, bytesRead = %d, bytesLeft = %d\n",
            readcnt,
            bytesRead,
            bytesLeft
        );
    } while ((bytesLeft > 0) && (readcnt < 3));

    printf(
        "END: read %d, bytesRead = %d, bytesLeft = %d\n",
        readcnt,
        bytesRead,
        bytesLeft
    );

    // create in memory copy from input by channel
    for (i = 0, pixel = 0; i < IMG_HEIGHT; i += 1) {
        for (j = 0; j < IMG_WIDTH; j += 1) {
            R[i][j] = RGB[pixel + 0];
            convR[i][j] = R[i][j];

            G[i][j] = RGB[pixel + 1];
            convG[i][j] = G[i][j];

            B[i][j] = RGB[pixel + 2];
            convB[i][j] = B[i][j];

            pixel += 3;
        }
    }

    close(fdin);

    printf("source file %s read\n", argv[1]);
    printf("\nstart test at %lf\n", get_now() - fstart);

    fstart = get_now();

    int height_divisor = IMG_HEIGHT / thread_count;

    for (int runs = 0; runs < iterations; runs += 1) {
        for (int thread_idx = 0; thread_idx < thread_count; thread_idx += 1) {
            if (thread_idx == 0) {
                // we need to skip the first row of the image and since thread
                // 0 will be the only one that works on that image we will
                // only need to calculate on thread 0
                threadarg[thread_idx].start_height = (thread_idx * height_divisor) + 1;
            } else {
                threadarg[thread_idx].start_height = thread_idx * height_divisor;
            }

            if (thread_idx == (thread_count - 1)) {
                threadarg[thread_idx].height = IMG_HEIGHT - 1;
            } else {
                threadarg[thread_idx].height = (thread_idx + 1) * height_divisor;
            }

            threadarg[thread_idx].start_width = 1;
            threadarg[thread_idx].width = IMG_WIDTH - 1;

            rc = pthread_create(
                &threads[thread_idx],
                NULL,
                sharpen_thread,
                (void *)&threadarg[thread_idx]
            );

            if (rc < 0) {
                perror("pthread_create");

                free(threads);
                free(threadarg);

                exit(EXIT_FAILURE);
            }
        }

        for (int thread_idx = 0; thread_idx < thread_count; thread_idx += 1) {
            if (pthread_join(threads[thread_idx], NULL) < 0) {
                perror("pthread_join");

                free(threads);
                free(threadarg);

                exit(EXIT_FAILURE);
            }
        }
    }

    FLOAT fnow = get_now();

    printf(
        "stop test at %lf for %d frames, fps (frames per second) = %lf, pps (pixels per second) = %lf\n\n",
        fnow - fstart,
        iterations,
        iterations/ (fnow - fstart),
        ((double)iterations * (double)IMG_HEIGHT * (double)IMG_WIDTH) / ((double)(fnow - fstart))
    );

    printf("Starting output file %s write\n", argv[2]);
    rc = write(fdout, (void *)header, HEADER_SIZE - 1);

    printf("\nstart test input at %lf\n", get_now() - fstart);

    // create in memory copy from input by channel
    for (i = 0, pixel = 0; i < IMG_HEIGHT; i += 1) {
        for (j = 0; j < IMG_WIDTH; j += 1) {
            RGB[pixel + 0] = convR[i][j];
            RGB[pixel + 1] = convG[i][j];
            RGB[pixel + 2] = convB[i][j];

            pixel += 3;
        }
    }

    bytesLeft = IMG_HEIGHT * IMG_WIDTH * 3;

    printf(
        "START: write 0, bytesWritten = 0, bytesLeft = %d\n",
        bytesLeft
    );

    int bytesWritten = 0;
    int writecnt = 0;

    // Write RGB data in large chunks, requesting all at once and writing residual
    do {
        bytesWritten = write(fdout, (void *)&RGB[bytesWritten], bytesLeft);
        bytesLeft -= bytesWritten;
        writecnt++;

        printf(
            "write %d, bytesWritten=%d, bytesLeft=%d\n",
            writecnt,
            bytesWritten,
            bytesLeft
        );
    } while ((bytesLeft > 0) && (writecnt < 3));

    printf(
        "END: write %d, bytesWritten = %d, bytesLeft = %d\n",
        writecnt,
        bytesWritten,
        bytesLeft
    );

    printf("\ncompleted test input at %lf\n", get_now() - fstart);
    printf("Output file %s written\n", argv[2]);

    free(threads);
    free(threadarg);

    close(fdout);
}
