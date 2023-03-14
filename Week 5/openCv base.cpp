// Include files for required libraries
#include <stdio.h>
#include "opencv_aee.hpp"
#include "main.hpp"

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

        while (frame.empty())
            frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

        rotate(frame, frame, ROTATE_180);

        cv::imshow("Photo", frame); // Display the image in the window

        int key = cv::waitKey(1); // Wait 1ms for a keypress (required to update windows)

        key = (key == 255) ? -1 : key; // Check if the ESC key has been pressed
        if (key == 27)
            break;
    }

    closeCV(); // Disable the camera and close any windows

    return 0;
}
