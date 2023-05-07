// Include files for required libraries
#include <stdio.h>

#include "main.hpp"
#include "opencv_aee.hpp"
#include "pi2c.h"

// Initialize a Pi2c object on I2C bus 4
Pi2c car(4);

int max_similarity = 0;
int max_index = 0;
int checkForSymbols = 1;
int symbolDetected = 0;
int lineToFollow = 0;

// Function to send motor speeds and steering angle to the car
void drive(int leftMotor, int rightMotor, int steerAngle) {
    // Print "You have reached the drive function"
    printf("You have reached the drive function");
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
    // Print "You have reached the check data function"
    printf("You have reached the check data function");
    // Constrain the steering angle to +- 40 degrees from the set point
    *steeringAngle = max(setPoint - 50, min(setPoint + 50, *steeringAngle));

    // Constrain both motor speeds to -255-255
    *leftMotorSpeed = max(-255, min(255, *leftMotorSpeed));
    *rightMotorSpeed = max(-255, min(255, *rightMotorSpeed));
}

int symbolDetection(Mat frame) {
    Mat frame_symbol = frame;
    // Convert the image to hsv
    Mat image_hsv;
    cvtColor(frame_symbol, image_hsv, COLOR_BGR2HSV);
    // Convert the image to black and white using the inRange function for pink pixels
    Mat symbol_check;
    inRange(image_hsv, Scalar(120, 40, 70), Scalar(180, 188, 165), symbol_check);

    // Clear up the image
    dilate(symbol_check, symbol_check, getStructuringElement(MORPH_ELLIPSE, Size(7.5, 7.5)));
    erode(symbol_check, symbol_check, getStructuringElement(MORPH_ELLIPSE, Size(2.5, 2.5)));

    // Find the contours of the image saving them as img_symbol_contours
    std::vector<std::vector<Point>> img_symbol_contours;
    findContours(symbol_check, img_symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Draw the contours
    drawContours(frame_symbol, img_symbol_contours, -1, Scalar(0, 0, 255), 2);

    // Transform the perspective of the image
    std::vector<Point2f> corners;
    for (int i = 0; i < img_symbol_contours.size(); i++) {
        if (img_symbol_contours[i].size() > 4) {
            approxPolyDP(img_symbol_contours[i], corners, arcLength(img_symbol_contours[i], true) * 0.02, true);
            if (corners.size() == 4) {
                // Draw the contours
                drawContours(frame_symbol, img_symbol_contours, i, Scalar(0, 255, 0), 2);
                // Draw the corners
                for (int j = 0; j < corners.size(); j++) {
                    circle(frame_symbol, corners[j], 5, Scalar(0, 0, 255), 2);
                }
            }
        }
    }

    // Check if there are 4 corners
    if (corners.size() == 4) {
        // Change the perspective of symbol_check so that the 4 corners found are the new corners
        Mat perspective_transform = getPerspectiveTransform(corners, std::vector<Point2f>{Point2f(0, 0), Point2f(0, 100), Point2f(100, 100), Point2f(100, 0)});
        warpPerspective(symbol_check, symbol_check, perspective_transform, Size(100, 100));
        // Ccale up the image to 350x350
        resize(symbol_check, symbol_check, Size(350, 350));
        // Find the contours of the image saving them as image_symbol_contours
        std::vector<std::vector<Point>> image_symbol_contours;
        findContours(symbol_check, image_symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        // Draw the contours
        drawContours(symbol_check, image_symbol_contours, -1, Scalar(255, 255, 255), 2);

        // Make a 13 item array of Mat objects to store the symbols
        Mat symbols[13];
        symbols[0] = imread("Circle.png");
        symbols[1] = imread("Star 1.png");
        // Symbols 1-4 are the same image but roated 90 degrees
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
            // Convert the symbols to hsv and black and white
            cvtColor(symbols[i], symbols[i], COLOR_BGR2HSV);
            inRange(symbols[i], Scalar(150, 50, 50), Scalar(180, 255, 255), symbols[i]);

            // Find the contours of the symbols
            std::vector<std::vector<Point>> symbol_contours;
            findContours(symbols[i], symbol_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            // Draw the contours
            drawContours(symbols[i], symbol_contours, -1, Scalar(255, 255, 255), 2);

            // Use compare() to compare symbols[i] and symbol_check
            Mat result;
            compare(symbol_check, symbols[i], result, CMP_EQ);
            // Store the number of pixels that are the same in a list
            int pixel_count = countNonZero(result);
            // Convert the pixel count to a percentage
            pixel_count = (pixel_count / 350.0) * 100;
            similarity[i] = pixel_count;
        }
        // Find which symbol has the most similar pixels
        int max_similarity = 0;
        int max_index = 0;
        for (int i = 0; i < 13; i++) {
            if (similarity[i] > max_similarity) {
                max_similarity = similarity[i];
                max_index = i;
            }
        }
        // Check if the percentage is above 75%
        if (max_similarity > 75) {
            // Display the name of the symbol that is most similar on the frame
            putText(frame_symbol, symbol_names[max_index], Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            // Show all the images
            imshow("frame_symbol", frame_symbol);
            imshow("Black and White", symbol_check);
            if (symbol_names[max_index] == "Circle") {
                return 1;
            } else if (symbol_names[max_index] == "Star") {
                return 2;
            } else if (symbol_names[max_index] == "Triangle") {
                return 3;
            } else if (symbol_names[max_index] == "Umbrella") {
                return 4;
            } else {
                return 0;
            }
        } else {
            // Display "Unknown" on the frame
            putText(frame_symbol, "Unknown", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            // Show all the images
            imshow("frame_symbol", frame_symbol);
            imshow("Black and White", symbol_check);
            return 0;
        }
    } else {
        // Display "Unknown" on the frame
        putText(frame_symbol, "Unknown", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        // Show all the images
        imshow("frame_symbol", frame_symbol);
        imshow("Black and White", symbol_check);
        return 0;
    }
    return 0;
}

void setup(void) {
    // Enable the camera for OpenCV
    setupCamera(320, 240);
}

int main(int argc, char **argv) {
    // Call a setup function to prepare IO and devices
    setup();

    // Create a GUI window called photo
    cv::namedWindow("Photo");

    // Initialize the PID variables
    float P;
    float I;
    float D;
    float u;
    float Kp = 7.5;
    float Ki = 0.000005;
    float Kd = 0.25;
    float K = 0.0009375;
    int setPoint = 107;
    float speedModifier = 1;
    float lastError;

    // Initialize the motor speeds and steering angle
    int steeringAngle;
    int leftMotorSpeed;
    int rightMotorSpeed;

    // Main loop to perform image processing
    while (1) {
        Mat frame;
        Mat image_hsv;
        Mat image_threshold;

        while (frame.empty())
            // Capture a frame from the camera and store in a new matrix variable
            frame = captureFrame();

        rotate(frame, frame, ROTATE_180);

        // Convert the frame to HSV
        cvtColor(frame, image_hsv, COLOR_BGR2HSV);

        // Check if checkForSymbols == 1
        if (checkForSymbols == 1) {
            symbolDetected = symbolDetection(frame);
            if (symbolDetected != 0) {
                checkForSymbols = 0;
            }
        }

        // make a switch case for the symbolDetected variable
        switch (symbolDetected) {
            case 0:
                // Print "Black" on the console
                std::cout << "Black" << std::endl;
                inRange(image_hsv, Scalar(0, 0, 0), Scalar(180, 255, 50), image_threshold);
                break;
            case 1:
                // Print "Red" on the console
                std::cout << "Red" << std::endl;
                inRange(image_hsv, Scalar(0, 130, 90), Scalar(180, 255, 255), image_threshold);
                break;
            case 2:
                // Print "Green" on the console
                std::cout << "Green" << std::endl;
                inRange(image_hsv, Scalar(60, 65, 0), Scalar(90, 255, 255), image_threshold);
                break;
            case 3:
                // Print "Blue" on the console
                std::cout << "Blue" << std::endl;
                inRange(image_hsv, Scalar(80, 75, 0), Scalar(140, 255, 255), image_threshold);
                break;
            case 4:
                // Print "Yellow" on the console
                std::cout << "Yellow" << std::endl;
                inRange(image_hsv, Scalar(0, 105, 125), Scalar(30, 255, 255), image_threshold);
                break;
            default:
                // Print "Black" on the conso le
                std::cout << "Black" << std::endl;
                inRange(image_hsv, Scalar(0, 0, 0), Scalar(180, 255, 50), image_threshold);
                break;
        }

        // Check if image_threshold has any white pixels
        if (countNonZero(image_threshold) < 5) {
            // Print "No line detected" on the console
            std::cout << "No line detected" << std::endl;
            symbolDetected = 0;
            checkForSymbols = 1;
            inRange(image_hsv, Scalar(0, 0, 0), Scalar(180, 255, 50), image_threshold);
        }

        imshow("Black and White follow", image_threshold);

        // Crop the frame
        // Crop the image so only the bottom 2/3 remain
        image_threshold = image_threshold(Rect(0, image_threshold.rows / 3, image_threshold.cols, image_threshold.rows / 3 * 2));

        // Find the contours of the image
        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hierarchy;
        findContours(image_threshold, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

        Mat follow_frame = frame;

        // Draw the contours
        drawContours(follow_frame, contours, -1, Scalar(0, 255, 0), 2);

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
                        circle(follow_frame, mc[i], 4, Scalar(255, 0, 0), -1, 8, 0);
                    }
                    // check that there is at least one contour
                    if (contours.size() > 0) {
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
                            int xDistance = mc[largestContour].x - (follow_frame.cols / 2);
                            int yDistance = mc[largestContour].y - (follow_frame.rows / 2);

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

                            if (error > 10) {
                                speedModifier = 0.75;
                            } else {
                                speedModifier = 1;
                            }

                            // Print out the values of the PID variables and u
                            printf("P: %f, I: %f, D: %f, U: %f \n", P, I, D, u);

                            // Using value of u to set motor speeds
                            leftMotorSpeed = 80 + K * u * speedModifier;
                            rightMotorSpeed = 80 - K * u * speedModifier;
                            steeringAngle = setPoint + u;

                            // Print the values to the console
                            printf("Pre U: %f, Steering Angle: %d, Left Motor Speed: %d, Right Motor Speed: %d \n", u, steeringAngle, leftMotorSpeed, rightMotorSpeed);

                            // Check the data to make sure it is within the correct range
                            checkData(setPoint, &leftMotorSpeed, &rightMotorSpeed, &steeringAngle);

                            // Check if steering angle is at the maximum value
                            if (steeringAngle <= 60) {
                                drive(-100, -100, 154);
                                sleep(0.25);
                                drive(0,0,steeringAngle);
                            } else if (steeringAngle >= 154) {
                                drive(-100, -100, 60);
                                sleep(0.25);
                                drive(0,0,steeringAngle);
                            }

                            printf("Post U: %f, Steering Angle: %d, Left Motor Speed: %d, Right Motor Speed: %d \n", u, steeringAngle, leftMotorSpeed, rightMotorSpeed);

                            // Send the data to the nano
                            drive(leftMotorSpeed, rightMotorSpeed, steeringAngle);

                            // Display the error and steering angle on the image
                            std::string errorString = "Error: " + std::to_string(error);
                            std::string steeringAngleString = "Steering Angle: " + std::to_string(steeringAngle);
                            putText(follow_frame, errorString, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, 8, false);
                            putText(follow_frame, steeringAngleString, Point(10, 40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, 8, false);

                            // Show the image
                            imshow("Follow View", follow_frame);
                        }
                    }
                }
            }
        }

        // Display the image in the window
        cv::imshow("Photo", frame);

        // Wait 1ms for a keypress (required to update windows)
        int key = cv::waitKey(1);

        // Check if the ESC key has been pressed
        key = (key == 255) ? -1 : key;
        if (key == 27) {
            drive(0, 0, 107);
            break;
        }
    }

    // Disable the camera and close any windows
    closeCV();

    return 0;
}
