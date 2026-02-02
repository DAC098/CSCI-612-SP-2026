/*
 *
 * Example by Sam Siewert 
 * modified by David Cathers
 */
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <iomanip>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/viz/types.hpp>

using namespace std::literals;

struct TimerDelta {
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    TimerDelta(
        std::chrono::high_resolution_clock::time_point start,
        std::chrono::high_resolution_clock::time_point end
    ) :
        start(start), end(end)
    {}

    std::chrono::milliseconds as_millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            this->end - this->start
        );
    }

    std::chrono::duration<double, std::ratio<1>> as_secs_f() {
        return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(
            this->end - this->start
        );
    }

    double seconds_f() {
        return this->as_secs_f().count();
    }
};

struct Timer {
    std::chrono::high_resolution_clock::time_point start;

    Timer() :
        start(std::chrono::high_resolution_clock::now())
    {}

    TimerDelta elapsed() {
        auto now = std::chrono::high_resolution_clock::now();

        TimerDelta delta(this->start, now);

        this->start = now;

        return delta;
    }
};

void draw_crosshair(cv::Mat& frame, std::int32_t length, cv::Scalar color) {
    cv::Size size = frame.size();
    auto min_size = std::min(size.height, size.width);

    auto min_length = std::min(min_size, length);
    auto width_center = size.width / 2;
    auto height_center = size.height / 2;

    auto vert_start = height_center - min_length / 2;
    auto vert_end = height_center + min_length / 2;
    auto horz_start = width_center - min_length / 2;
    auto horz_end = width_center + min_length / 2;

    // vertical line
    cv::line(
        frame,
        cv::Point(width_center, vert_start),
        cv::Point(width_center, vert_end),
        color
    );
    // horizontal line
    cv::line(
        frame,
        cv::Point(horz_start, height_center),
        cv::Point(horz_end, height_center),
        color
    );
}

void draw_text(
    cv::Mat& frame,
    const std::string& text,
    cv::Point origin,
    std::int32_t font_face,
    double font_scale,
    cv::Scalar color,
    std::int32_t thickness = 1,
    std::int32_t line_type = cv::LINE_8,
    bool bottom_left_origin = false
) {
    // pulled from the opencv docs for getTextSize
    std::int32_t baseline = 0;

    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

    baseline += thickness;

    cv::rectangle(
        frame,
        origin + cv::Point(0, thickness),
        origin + cv::Point(text_size.width, -text_size.height),
        cv::Scalar(0, 0, 0),
        // should fill the rectangle
        -1
    );

    cv::putText(
        frame,
        text,
        origin,
        font_face,
        font_scale,
        color,
        thickness,
        line_type
    );
}

void draw_sys_now(
    cv::Mat& frame,
    cv::Point origin
) {
    std::stringstream ss;
    auto t = std::time(nullptr);
    auto now = *std::localtime(&t);

    ss << std::put_time(&now, "%c %Z");

    draw_text(
        frame,
        ss.str(),
        origin,
        cv::FONT_HERSHEY_COMPLEX_SMALL,
        0.8,
        cv::Scalar::all(255)
    );
}

int main(int argc, char** argv) {
    cv::CommandLineParser parser(
        argc,
        argv,
        "{help h||}"
        "{camera|0|Camera device number.}"
        "{low-res||changes the resolution down to 320x240.}"
        "{med-res||changes the resolution down to 640x480.}"
        "{show-ts||displays the current timstamp for the system.}"
        "{show-ch||displays the crosshair in the center of the frame.}"
    );
    parser.printMessage();

    std::int32_t camera_device = parser.get<std::int32_t>("camera");

    cv::VideoCapture vcap;

    // open the video stream and make sure it's opened
    // "0" is the default video device which is normally the built-in webcam
    if (!vcap.open(camera_device)) {
        std::cout << "failed to open specified camera device\n";

        exit(EXIT_FAILURE);
    }

    if (parser.has("med-res")) {
        vcap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        vcap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    } else if (parser.has("low-res")) {
        vcap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
        vcap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    }

    bool display_ch = parser.has("show-ch");
    bool display_ts = parser.has("show-ts");

    cv::Mat frame;

    // the previous n seconds
    std::uint64_t total_frames = 0;
    Timer timer;

    while (1) {
        if (!vcap.read(frame)) {
            std::cout << "No frame" << std::endl;

            cv::waitKey();
        }

        auto frame_dim = frame.size();

        if (display_ch) {
            draw_crosshair(frame, 24, cv::viz::Color::yellow());
        }

        if (display_ts) {
            draw_sys_now(frame, cv::Point(30, frame_dim.height - 30));
        }

        auto duration = timer.elapsed();

        total_frames += 1;

        {
            std::stringstream frame_data;

            frame_data << "frames: " << total_frames
                << " " << std::trunc(1.0 / duration.seconds_f()) 
                << " " <<  duration.seconds_f() << "s";

            draw_text(
                frame,
                frame_data.str(),
                cv::Point(20, 20),
                cv::FONT_HERSHEY_COMPLEX_SMALL,
                0.8,
                cv::Scalar::all(255)
            );
        }

        cv::imshow("custom-capture", frame);

        // this paces the frame processing rate
        char c = cv::waitKey(33);

        if (c == 'q') {
            break;
        }
    }
};
