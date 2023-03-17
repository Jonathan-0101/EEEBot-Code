// Include files for required libraries
#include <stdio.h>

#include "main.hpp"
#include "opencv_aee.hpp"

void setup(void) {
    // Enable the camera for OpenCV
    setupCamera(320, 240);
}

int main(int argc, char **argv) {
    // Call a setup function to prepare IO and devices
    setup();

    // Create a GUI window called photo
    cv::namedWindow("Photo");

    // Main loop to perform image processing
    while (1) {
        Mat frame;

        while (frame.empty())
            // Capture a frame from the camera and store in a new matrix variable
            frame = captureFrame();

        rotate(frame, frame, ROTATE_180);

        // Display the image in the window
        cv::imshow("Photo", frame);

        // Wait 1ms for a keypress (required to update windows)
        int key = cv::waitKey(1);

        // Check if the ESC key has been pressed
        key = (key == 255) ? -1 : key;
        if (key == 27)
            break;
    }

    // Disable the camera and close any windows
    closeCV();

    return 0;
}
