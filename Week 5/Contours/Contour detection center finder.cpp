#include <stdio.h>

#include "main.hpp"
#include "opencv_aee.hpp"
#include "pi2c.h"

// Initialize a Pi2c object on I2C bus 4
Pi2c car(4);

// Function to send motor speeds and steering angle to the car
void drive(int leftMotor, int rightMotor, int steerAngle) {
    char dataToSend[6];
    dataToSend[0] = (char)((leftMotor >> 8) & 0xFF);
    dataToSend[1] = (char)(leftMotor & 0xFF);
    dataToSend[2] = (char)((rightMotor >> 8) & 0xFF);
    dataToSend[3] = (char)(rightMotor & 0xFF);
    dataToSend[4] = (char)((steerAngle >> 8) & 0xFF);
    dataToSend[5] = (char)(steerAngle & 0xFF);
    // Send the data to the nano
    car.i2cWrite(dataToSend, 6);
}

void checkData(int setPoint, int *leftMotorSpeed, int *rightMotorSpeed, int *steeringAngle) {
    // Constrain the steering angle to +- 40 degrees from the set point
    *steeringAngle = max(setPoint - 50, min(setPoint + 50, *steeringAngle));

    // Constrain both motor speeds to -255-255
    *leftMotorSpeed = max(-255, min(255, *leftMotorSpeed));
    *rightMotorSpeed = max(-255, min(255, *rightMotorSpeed));
}

void setup(void) {
    // Initialize the camera
    setupCamera(320, 240);
}

int main(int argc, char **argv) {
    setup();
    cv::namedWindow("Photo");

    // Initialize the PID variables
    float P;
    float I;
    float D;
    float u;
    float Kp = 8;
    float Ki = 0.000005;
    float Kd = 0.5;
    float K = 0.009375;
    int setPoint = 107;
    float lastError;

    // Initialize the motor speeds and steering angle
    int steeringAngle;
    int leftMotorSpeed;
    int rightMotorSpeed;

    while (1) {
        Mat frame;
        Mat image_HSV;
        Mat image_GREY;

        // Capture a frame from the camera
        while (frame.empty()) {
            frame = captureFrame();
        }

        // Rotate the image 180 degrees
        rotate(frame, frame, ROTATE_180);

        // Crop the image so only the bottom 2/3 remain
        frame = frame(Rect(0, frame.rows / 3, frame.cols, frame.rows / 3 * 2));

        // Increase the contrast of the image
        frame.convertTo(frame, -1, 1.5, 0);

        // Convert the image to HSV and then to greyscale
        cvtColor(frame, image_HSV, COLOR_BGR2HSV);
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(255, 255, 75), image_GREY);

        // Find the contours of the image
        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hierarchy;
        findContours(image_GREY, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

        // Draw the contours
        drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);

        // Check that there is at least one contour
        if (contours.size() > 0) {
            // Find the moments of the contours
            std::vector<Moments> mu(contours.size());
            for (int i = 0; i < contours.size(); i++) {
                mu[i] = moments(contours[i], false);
            }
            // Check that there is at least one item in the mu vector
            if (mu.size() > 0) {
                // Find the centroid of the contours
                std::vector<Point2f> mc(contours.size());
                for (int i = 0; i < contours.size(); i++) {
                    mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
                }
                // Check that there is at least one item in the mc vector
                if (mc.size() > 0) {
                    // Draw the centroid of the contours
                    for (int i = 0; i < contours.size(); i++) {
                        circle(frame, mc[i], 4, Scalar(255, 0, 0), -1, 8, 0);
                    }
                    // Find the largest contour
                    int largestContour = 0;
                    for (int i = 0; i < contours.size(); i++) {
                        if (contourArea(contours[i]) > contourArea(contours[largestContour])) {
                            largestContour = i;
                        }
                    }
                    // Check that there is at least one item in the mc vector
                    if (mc.size() > 0) {
                        // Calulate the distance from the centre of the image to the centre of the largest contour
                        int xDistance = mc[largestContour].x - (frame.cols / 2);
                        int yDistance = mc[largestContour].y - (frame.rows / 2);

                        float error = xDistance / 16;

                        // Print the error to the console
                        printf("Error: %f \n", error);

                        // Calculating the PID variables
                        P = error;
                        I += error;
                        D = error - lastError;
                        lastError = error;

                        // PID equation
                        u = (Kp * P) + (Ki * I) + (Kd * D);

                        // Print out the values of the PID variables and u
                        printf("P: %f, I: %f, D: %f, U: %f \n", P, I, D, u);

                        // Using value of u to set motor speeds
                        leftMotorSpeed = 100 + K * u;
                        rightMotorSpeed = 100 - K * u;
                        steeringAngle = setPoint + u;

                        // Print the values to the console
                        printf("Pre U: %f, Steering Angle: %d, Left Motor Speed: %d, Right Motor Speed: %d \n", u, steeringAngle, leftMotorSpeed, rightMotorSpeed);

                        // Check the data to make sure it is within the correct range
                        checkData(setPoint, &leftMotorSpeed, &rightMotorSpeed, &steeringAngle);

                        printf("Post U: %f, Steering Angle: %d, Left Motor Speed: %d, Right Motor Speed: %d \n", u, steeringAngle, leftMotorSpeed, rightMotorSpeed);

                        // Send the data to the nano
                        drive(leftMotorSpeed, rightMotorSpeed, steeringAngle);

                        // Display the error and steering angle on the image
                        std::string errorString = "Error: " + std::to_string(error);
                        std::string steeringAngleString = "Steering Angle: " + std::to_string(steeringAngle);
                        putText(frame, errorString, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, 8, false);
                        putText(frame, steeringAngleString, Point(10, 40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, 8, false);
                    }
                }
            }
        }

        // Display the image
        cv::imshow("Photo", frame);

        // Wait for a key press
        int key = cv::waitKey(1);

        // If the key is ESC, break out of the loop
        key = (key == 255) ? -1 : key;
        if (key == 27) {
            drive(0, 0, 107);
            break;
        }
    }

    closeCV();
    return 0;
}
