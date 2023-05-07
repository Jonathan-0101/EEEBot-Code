#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    // get the user to input the path to the image
    string path;
    cout << "Please enter the path to the image: ";
    cin >> path;

    // read the image
    Mat image = imread(path);

    if (image.data == NULL) {
        // check there is an image
        cout << "No image found! Check path." << endl;
        return 1;  // ERROR
    } else {
        // create a window to show the image
        namedWindow("Image", CV_WINDOW_AUTOSIZE);

        // convert the image to hsv
        Mat image_hsv;
        cvtColor(image, image_hsv, CV_BGR2HSV);

        // equalise the histogram of the image
        vector<Mat> channels;
        split(image_hsv, channels);
        equalizeHist(channels[2], channels[2]);
        merge(channels, image_hsv);

        // create 4 greyscale images for each colour
        Mat image_greyscale_red1;
        Mat image_greyscale_red2;
        Mat image_greyscale_green;
        Mat image_greyscale_blue;

        // convert the image to greyscale for their respective colours
        inRange(image_hsv, Scalar(0, 90, 90), Scalar(25, 255, 255), image_greyscale_red1);
        inRange(image_hsv, Scalar(145, 85, 85), Scalar(180, 255, 255), image_greyscale_red2);
        inRange(image_hsv, Scalar(100, 150, 0), Scalar(140, 255, 255), image_greyscale_blue);
        inRange(image_hsv, Scalar(35, 50, 35), Scalar(90, 220, 220), image_greyscale_green);

        // combine the two red images due to red having two hsv ranges
        Mat image_greyscale_red = image_greyscale_red1 + image_greyscale_red2;

        // erode each grey scale image to reduce noise
        erode(image_greyscale_red, image_greyscale_red, Mat(), Point(-1, -1), 2, 1, 1);
        erode(image_greyscale_green, image_greyscale_green, Mat(), Point(-1, -1), 2, 1, 1);
        erode(image_greyscale_blue, image_greyscale_blue, Mat(), Point(-1, -1), 2, 1, 1);

        // count the numver of non zero pixels in each image
        int red_pixels = countNonZero(image_greyscale_red);
        int green_pixels = countNonZero(image_greyscale_green);
        int blue_pixels = countNonZero(image_greyscale_blue);

        // run a for loop to find which colour has the most pixels
        int max_pixels = 0;
        string colour;
        for (int i = 0; i < 3; i++) {
            if (red_pixels > max_pixels) {
                max_pixels = red_pixels;
                colour = "Red";
            } else if (green_pixels > max_pixels) {
                max_pixels = green_pixels;
                colour = "Green";
            } else if (blue_pixels > max_pixels) {
                max_pixels = blue_pixels;
                colour = "Blue";
            }
        }

        // add the text to the image
        putText(image, "Colour: " + colour, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 2);

        // show each of the  images
        imshow("Image", image);
        imshow("Red", image_greyscale_red);
        imshow("Green", image_greyscale_green);
        imshow("Blue", image_greyscale_blue);

        waitKey();  // without this image won't be shown
        return 0;   // OK
    }
}
