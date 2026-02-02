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

struct time_record {
    std::uint32_t frames;
    std::chrono::nanoseconds delta;

    time_record() :
        frames(0), delta(std::chrono::nanoseconds::zero())
    {}

    time_record(const time_record& other) :
        frames(other.frames), delta(other.delta)
    {}

    time_record(std::uint32_t f, std::chrono::nanoseconds d) :
        frames(f), delta(d)
    {}

    void update(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& end,
        const std::chrono::time_point<std::chrono::high_resolution_clock>& start
    ) {
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        );

        this->frames += 1;
        this->delta += diff;
    }

    double get_fps() {
        return ((double)this->frames / (double)this->delta.count()) * 1000000000.0;
    }

    void reset() {
        this->frames = 0;
        this->delta = std::chrono::nanoseconds::zero();
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

int main (int argc, char** argv) {
    cv::VideoCapture vcap;

    // open the video stream and make sure it's opened
    // "0" is the default video device which is normally the built-in webcam
    if (!vcap.open(0)) {
        std::cout << "Error opening video stream or file" << std::endl;

        exit(EXIT_FAILURE);
    } else {
        std::cout << "Opened default camera interface" << std::endl;
    }

    vcap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    vcap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat mat_frame;

    // the previous 60 seconds
    auto max_size = 60;
    auto index = 0;

    time_record current;

    std::vector<time_record> time_points;
    time_points.reserve(max_size);

    auto last_updated = std::chrono::high_resolution_clock::now();

    while (1) {
        auto start = std::chrono::high_resolution_clock::now();

        if (!vcap.read(mat_frame)) {
            std::cout << "No frame" << std::endl;

            cv::waitKey();
        }

        auto frame_dim = mat_frame.size();

        draw_crosshair(mat_frame, 24, cv::viz::Color::yellow());
        draw_sys_now(mat_frame, cv::Point(30, frame_dim.height - 30));

        auto end = std::chrono::high_resolution_clock::now();

        current.update(end, start);

        std::stringstream fps;

        fps << "fps: " << std::trunc(current.get_fps());

        if (end - last_updated > 1s) {
            if (time_points.size() == max_size) {
                time_points[index] = current;

                index = (index + 1) % max_size;
            } else {
                time_points.push_back(time_record(current));
            }

            current.reset();
        }

        double total = 0.0;

        for (auto& calc : time_points) {
            total += calc.get_fps();
        }

        double avg = total / (double)time_points.size();

        fps << " " << std::trunc(avg);

        draw_text(
            mat_frame,
            fps.str(),
            cv::Point(30, 30),
            cv::FONT_HERSHEY_COMPLEX_SMALL,
            0.8,
            cv::Scalar::all(255)
        );

        cv::imshow("custom-capture", mat_frame);

        // this paces the frame processing rate
        char c = cv::waitKey(33);

        if (c == 'q') {
            break;
        }
    }
};
