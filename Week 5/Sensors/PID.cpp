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

// Function to send motor speeds and steering angle to the car
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
    float P;
    float I;
    float D;
    float u;
    float Kp = 50;
    float Ki = 0;
    float Kd = 0;
    float K = 0.0125;
    int setPoint = 107;
    int steeringAngle;
    float lastError;

    while (1) {
        Mat frame;
        Mat image_HSV;
        Mat image_GREY;

        while (frame.empty()) {
            frame = captureFrame();
        }

        // Rotate the image 180 degrees
        rotate(frame, frame, ROTATE_180);

        // Convert the image to HSV and then to greyscale
        cvtColor(frame, image_HSV, COLOR_BGR2HSV);
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(255, 255, 55), image_GREY);

        // Find the contours of the image
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
            // Loop for each row in the section
            for (int j = 0; j < image_GREY.rows * 2 / 3; j++) {
                // Loop for each pixel in the column
                for (int k = 0; k < sectionWidth; k++) {
                    if (image_GREY.at<uchar>(image_GREY.rows + j, (i * sectionWidth) + k) == 0) {
                        count++;
                    }
                }
                // Store the count in an array
                pixelCount[i] = count;
            }
            // Draw a rectangle at the bottom of image if the count is greater than 2/3 of the section
            if (count > (sectionWidth * image_GREY.rows * 2 / 3)) {
                rectangle(frame, Point(i * sectionWidth, image_GREY.rows), Point((i + 1) * sectionWidth, image_GREY.rows + 10), Scalar(0, 0, 255), -1);
            }
        }

        // Calcualte the weighted average of the black pixels
        int total = 0;
        int sum = 0;
        int weighting = -numberOfSections / 2;
        for (int i = 0; i < numberOfSections; i++) {
            total += pixelCount[i];
            sum += pixelCount[i] * weighting;
            weighting += 0.5;
            // Check that weighting does not = 0
            if (weighting == 0) {
                weighting += 1;
            }
        }
        // Calculate the weighted average
        float weightedAverage = sum / total;

        float error = weightedAverage;

        // Calculating the PID variables
        P = error;
        I += error;
        D = error - lastError;
        float lastError = error;

        // PID equation
        u = (Kp * P) + (Ki * I) + (Kd * D);

        // Using value of u to set motor speeds
        int leftMotorSpeed = 110 + K * u;
        int rightMotorSpeed = 110 - K * u;
        steeringAngle = setPoint - u;

        // Constrain steering angle to 35 degrees either side of centre
        if (steeringAngle > 142) {
            steeringAngle = 142;
        } else if (steeringAngle < 72) {
            steeringAngle = 72;
        }

        // Send the steering angle to the car
        drive(leftMotorSpeed, rightMotorSpeed, steeringAngle);

        // Add the steering angle to the image
        putText(frame, "Steering angle: " + std::to_string(steeringAngle), Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_AA);

        sleep(0.01);

        cv::imshow("Base", frame);
        cv::imshow("HSV", image_HSV);
        cv::imshow("GREY", image_GREY);

        int key = cv::waitKey(1);

        key = (key == 255) ? -1 : key;
        if (key == 27) {
            drive(0, 0, 107);
            break;
        }
    }

    closeCV();

    return 0;
}
