#include <stdio.h>

#include "main.hpp"
#include "opencv_aee.hpp"
#include "pi2c.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

Pi2c car(4);

void setup(void) {
    setupCamera(320, 240);
}

void drive(int leftMotorSpeed, int rightMotorSpeed, int steeringAngle) {
    char dataToSend[6];
    dataToSend[0] = (leftMotorSpeed >> 8) & 0xFF;
    dataToSend[1] = leftMotorSpeed & 0xFF;
    dataToSend[2] = (rightMotorSpeed >> 8) & 0xFF;
    dataToSend[3] = rightMotorSpeed & 0xFF;
    dataToSend[4] = (steeringAngle >> 8) & 0xFF;
    dataToSend[5] = steeringAngle & 0xFF;
    car.i2cWrite(dataToSend, 6);
}

int main(int argc, char **argv) {
    setup();
    cv::namedWindow("Photo");
    char dataToSend[6];
    Pi2c arduino(4);

    while (1) {
        Mat frame;
        Mat image_HSV;
        Mat image_GREY;

        while (frame.empty()) {
            frame = captureFrame();
        }

        rotate(frame, frame, ROTATE_180);

        cvtColor(frame, image_HSV, COLOR_BGR2HSV);
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(255, 255, 80), image_GREY);

        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hierarchy;
        findContours(image_GREY, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

        drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);

        // Keep only the bottom section of the image
        int thirdHeight = image_GREY.rows / 3;
        image_GREY = image_GREY(Rect(0, image_GREY.rows * 2 / 3, image_GREY.cols, thirdHeight));

        int numberOfSections = 40;
        int pixelCount[numberOfSections];

        // Split the image into sections
        int sectionWidth = image_GREY.cols / numberOfSections;

        // Loop through each section counting the number of black pixels
        for (int i = 0; i < numberOfSections; i++) {
            int count = 0;
            for (int j = 0; j < image_GREY.rows; j++) {
                for (int k = 0; k < sectionWidth; k++) {
                    if (image_GREY.at<uchar>(j, (i * sectionWidth) + k) == 0) {
                        count++;
                    }
                }
                // store the count in an array
                pixelCount[i] = count;
            }
            // draw a rectangle at the bottom of image if the count is greater than 2/3 of the section
            if (count > (sectionWidth * image_GREY.rows * 2 / 3)) {
                rectangle(frame, Point(i * sectionWidth, image_GREY.rows), Point((i + 1) * sectionWidth, image_GREY.rows + 10), Scalar(0, 0, 255), -1);
            }
        }

        // calcualte the weighted average of the black pixels
        int total = 0;
        int sum = 0;
        int weighting = -numberOfSections;
        for (int i = 0; i < numberOfSections; i++) {
            total += pixelCount[i];
            sum += pixelCount[i] * weighting;
            weighting += 2;
            // check that weighting does not = 0
            if (weighting == 0) {
                weighting += 2;
            }
        }
        // calculate the weighted average
        float weightedAverage = sum / total;

        // calulate the steering correction
        int steeringAngle = 107 - weightedAverage * 5;

        // limit the steering angle to 35 degrees either side
        if (steeringAngle > 142) {
            steeringAngle = 142;
        } else if (steeringAngle < 72) {
            steeringAngle = 72;
        }

        // send the steering angle to the car
        drive(100, 100, steeringAngle);

        // print the weighted average
        printf("Weighted average: %f", weightedAverage);

        // add the weithted average to the image
        putText(frame, "Weighted average: " + std::to_string(weightedAverage), Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_AA);

        sleep(0.01);

        cv::imshow("Base", frame);
        cv::imshow("HSV", image_HSV);
        cv::imshow("GREY", image_GREY);

        int key = cv::waitKey(1);

        key = (key == 255) ? -1 : key;
        if (key == 27) {
            break;
        }
    }

    closeCV();

    return 0;
}
