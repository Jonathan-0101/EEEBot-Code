// Include files for required libraries
#include <stdio.h>
#include "opencv_aee.hpp"
#include "main.hpp" // You can use this file for declaring defined values and functions
#include "pi2c.h"

Pi2c car(4); // Configure the I2C interface to the Car as a global variable

void setup(void)
{
    setupCamera(320, 240); // Enable the camera for OpenCV
}

int main(int argc, char **argv)
{
    setup(); // Call a setup function to prepare IO and devices

    cv::namedWindow("Photo"); // Create a GUI window called photo

    while (1) // Main loop to perform image processing
    {
        Mat frame;
        Mat searchArea;
        Mat image_HSV;
        Mat image_GREY;
        float gradient;

        while (frame.empty())
            frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

        rotate(frame, frame, ROTATE_180);
        // crop the image to only focus on the bottom half
        frame = frame(Rect(0, 120, 320, 120));

        cvtColor(frame, image_HSV, COLOR_BGR2HSV); // Convert the image to HSV
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(255, 255, 80), image_GREY);

        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hierarchy;
        findContours(image_GREY, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

        // draw contours on the original image
        drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);

        // draw the first contour found
        if (!contours.empty())
        {
            drawContours(frame, contours, 0, Scalar(0, 255, 0), 2);
            // fit a line to the contour
            Vec4f line_params;
            fitLine(contours[0], line_params, DIST_L2, 0, 0.01, 0.01);
            // calculate the slope and intercept of the line
            float slope = line_params[1] / line_params[0];
            float intercept = line_params[3] - slope * line_params[2];
            // draw the line on the image
            Point pt1(0, intercept);
            Point pt2(frame.cols, slope * frame.cols + intercept);
            line(frame, pt1, pt2, Scalar(255, 0, 0), 2);
            gradient = ((slope * frame.cols + intercept) + intercept) / frame.cols;
        }

        // display the slope in the image
        char gradientText[100];
        sprintf(gradientText, "Gradient: %f", gradient);
        putText(frame, gradientText, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);

        cv::imshow("Photo", frame); // Display the image in the window

        int key = cv::waitKey(1); // Wait 1ms for a keypress (required to update windows)

        key = (key == 255) ? -1 : key; // Check if the ESC key has been pressed
        if (key == 27)
            break;
    }

    closeCV(); // Disable the camera and close any windows

    return 0;
}

