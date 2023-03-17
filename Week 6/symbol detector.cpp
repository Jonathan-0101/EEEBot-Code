// Include files for required libraries
#include <stdio.h>

#include "main.hpp"
#include "opencv_aee.hpp"

void setup(void) {
    setupCamera(320, 240);  // Enable the camera for OpenCV
}

int main(int argc, char **argv) {
    setup();  // Call a setup function to prepare IO and devices

    cv::namedWindow("Photo");  // Create a GUI window called photo

    while (1)  // Main loop to perform image processing
    {
        Mat frame;

        while (frame.empty())
            frame = captureFrame();  // Capture a frame from the camera and store in a new matrix variable

        rotate(frame, frame, ROTATE_180);

        // Convert the image to hsv
        Mat image_hsv;
        cvtColor(frame, image_hsv, COLOR_BGR2HSV);
        // convert the image to black and white using the inRange function for pink pixels
        Mat symbol_check;
        inRange(image_hsv, Scalar(150, 50, 50), Scalar(180, 255, 255), symbol_check);

        // find the contours of the image saving them as img_symbol_contours
        std::vector<std::vector<Point>> img_symbol_contours;
        findContours(symbol_check, img_symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // draw the contours
        drawContours(frame, img_symbol_contours, -1, Scalar(0, 0, 255), 2);

        // transform the perspective of the image
        std::vector<Point2f> corners;
        for (int i = 0; i < img_symbol_contours.size(); i++) {
            if (img_symbol_contours[i].size() > 4) {
                approxPolyDP(img_symbol_contours[i], corners, arcLength(img_symbol_contours[i], true) * 0.02, true);
                if (corners.size() == 4) {
                    // draw the contours
                    drawContours(frame, img_symbol_contours, i, Scalar(0, 255, 0), 2);
                    // draw the corners
                    for (int j = 0; j < corners.size(); j++) {
                        circle(frame, corners[j], 5, Scalar(0, 0, 255), 2);
                    }
                }
            }
        }

        // check nif there are 4 corners
        if (corners.size() == 4) {
            // change the perspective of symbol_check so that the 4 corners found are the new corners
            Mat perspective_transform = getPerspectiveTransform(corners, std::vector<Point2f>{Point2f(0, 0), Point2f(0, 100), Point2f(100, 100), Point2f(100, 0)});
            warpPerspective(symbol_check, symbol_check, perspective_transform, Size(100, 100));
            // scale up the image to 350x350
            resize(symbol_check, symbol_check, Size(350, 350));
            // find the contours of the image saving them as image_symbol_contours
            std::vector<std::vector<Point>> image_symbol_contours;
            findContours(symbol_check, image_symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            // draw the contours
            drawContours(symbol_check, image_symbol_contours, -1, Scalar(255, 255, 255), 2);

            // make a 13 item array of Mat objects to store the symbols
            Mat symbols[13];
            symbols[0] = imread("Circle.png");
            symbols[1] = imread("Star 1.png");
            // symbols 1-4 are the same image but roated 90 degrees
            rotate(symbols[1], symbols[2], ROTATE_90_CLOCKWISE);
            rotate(symbols[1], symbols[3], ROTATE_180);
            rotate(symbols[1], symbols[4], ROTATE_90_COUNTERCLOCKWISE);
            symbols[5] = imread("Triangle 1.png");
            rotate(symbols[5], symbols[6], ROTATE_90_CLOCKWISE);
            rotate(symbols[5], symbols[7], ROTATE_180);
            rotate(symbols[5], symbols[8], ROTATE_90_COUNTERCLOCKWISE);
            symbols[9] = imread("Umbrella 1.png");
            rotate(symbols[9], symbols[10], ROTATE_90_CLOCKWISE);
            rotate(symbols[9], symbols[11], ROTATE_180);
            rotate(symbols[9], symbols[12], ROTATE_90_COUNTERCLOCKWISE);
            String symbol_names[13] = {"Circle", "Star", "Star", "Star", "Star", "Triangle", "Triangle", "Triangle", "Triangle", "Umbrella", "Umbrella", "Umbrella", "Umbrella"};

            int similarity[13];

            for (int i = 0; i < 13; i++) {
                // convert the symbols to hsv and black and white
                cvtColor(symbols[i], symbols[i], COLOR_BGR2HSV);
                inRange(symbols[i], Scalar(150, 50, 50), Scalar(180, 255, 255), symbols[i]);

                // find the contours of the symbols
                std::vector<std::vector<Point>> symbol_contours;
                findContours(symbols[i], symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

                // draw the contours
                drawContours(symbols[i], symbol_contours, -1, Scalar(255, 255, 255), 2);

                // use compare() to compare symbols[i] and symbol_check
                Mat result;
                compare(symbol_check, symbols[i], result, CMP_EQ);
                // store the number of pixels that are the same in a list
                int pixel_count = countNonZero(result);
                // convert the pixel count to a percentage
                pixel_count = (pixel_count / 350.0) * 100;
                similarity[i] = pixel_count;
            }
            // find which symbol has the most similar pixels
            int max = 0;
            int max_index = 0;
            for (int i = 0; i < 13; i++) {
                if (similarity[i] > max) {
                    max = similarity[i];
                    max_index = i;
                }
            }
            // check if the percentage is above 75%
            if (max > 75) {
                // display the name of the symbol that is most similar on the frame
                putText(frame, symbol_names[max_index], Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            } else {
                // display "Unknown" on the frame
                putText(frame, "Unknown", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            }
        } else {
            // display "Unknown" on the frame
            putText(frame, "Unknown", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        }
        cv::imshow("Photo", frame);  // Display the image in the window
        // show all the images
        imshow("HSV", image_hsv);
        imshow("Black and White", symbol_check);

        int key = cv::waitKey(1);  // Wait 1ms for a keypress (required to update windows)

        key = (key == 255) ? -1 : key;  // Check if the ESC key has been pressed
        if (key == 27)
            break;
    }

    closeCV();  // Disable the camera and close any windows

    return 0;
}
