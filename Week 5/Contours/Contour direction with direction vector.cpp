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

        while (frame.empty())
            frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

        rotate(frame, frame, ROTATE_180);
        frame = frame(Rect(5, 35, 315, 180));
        // crop the image to only focus on the bottom haf

        cvtColor(frame, image_HSV, COLOR_BGR2HSV); // Convert the image to HSV
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(255, 255, 80), image_GREY);

        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hierarchy;
        findContours(image_GREY, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);
        // draw contours on the original image
        drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);

        // find the largest contour
        int largest_area = 0;
        int largest_contour_index = 0;
        Rect bounding_rect;
        for (int i = 0; i < contours.size(); i++) // iterate through each contour.
        {
            double a = contourArea(contours[i], false); //  Find the area of contour
            if (a > largest_area)
            {
                largest_area = a;
                largest_contour_index = i;                 // Store the index of largest contour
                bounding_rect = boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
            }
        }

        // find the center of the biggest contour
        int x = bounding_rect.x + bounding_rect.width / 2;
        int y = bounding_rect.y + bounding_rect.height / 2;

        // calucalte the direction vector of the contour
        int x1 = bounding_rect.x;
        int y1 = bounding_rect.y;
        int x2 = bounding_rect.x + bounding_rect.width;
        int y2 = bounding_rect.y + bounding_rect.height;
        int dx = x2 - x1;
        int dy = y2 - y1;
        int direction = dy / dx;

        // draw a cicle on the bottom edge of the clostest contour
        circle(frame, Point(x, y), 5, Scalar(0, 0, 255), -1);
        // draw the direction from the circle
        line(frame, Point(x, y), Point(x - dx, y - dy), Scalar(0, 0, 255), 2);

        // add the diff to the top corner of the image
        std::string text = std::to_string(direction);
        putText(frame, text, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);

        /*
        // draw the center of the biggest contour
        circle(frame, Point(x, y), 5, Scalar(0, 0, 255), -1);
        // draw the direction onto the center of the biggest contour
        line(frame, Point(x, y), Point(x + dx, y + dy), Scalar(0, 0, 255), 2); */

        cv::imshow("Photo", frame); // Display the image in the window

        int key = cv::waitKey(1); // Wait 1ms for a keypress (required to update windows)

        key = (key == 255) ? -1 : key; // Check if the ESC key has been pressed
        if (key == 27)
            break;
    }

    closeCV(); // Disable the camera and close any windows

    return 0;
}
