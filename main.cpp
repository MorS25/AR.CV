#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;
using namespace std;

void detectKeypoints(Mat& objectImage, Mat& sceneImage, int minHessian, vector<KeyPoint>& objectKeyPoints, vector<KeyPoint>& sceneKeyPoints) {
    SurfFeatureDetector detector(minHessian);
    
    detector.detect(objectImage, objectKeyPoints);
    detector.detect(sceneImage, sceneKeyPoints);
}

void calcDescriptors(Mat& objectImage, Mat& sceneImage, Mat& objectDesc, Mat& sceneDesc, vector<KeyPoint>& objectKeyPoints, vector<KeyPoint>& sceneKeyPoints) {
    SurfFeatureDetector extractor;
    
    extractor.compute(objectImage, objectKeyPoints, objectDesc);
    extractor.compute(sceneImage, sceneKeyPoints, sceneDesc);
}

/** @function main */
int main(int argc, char** argv) {
    Mat img_object = imread("object.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_scene_rgb;
    Mat img_scene;
    VideoCapture cap(0);
    cap >> img_scene_rgb;
    int sceneHeight = img_scene_rgb.rows;
    int sceneWidth  = img_scene_rgb.cols;
    
    while (true) {
        cap >> img_scene_rgb;
        
        // Convert video stream to grayscale.
        img_scene = img_scene_rgb.clone();
        cvtColor(img_scene_rgb, img_scene, CV_RGB2GRAY);
        
        if (!img_object.data || !img_scene.data) {
            cout << " --(!) Error reading images " << endl;
            return -1; }

        // Detect key points:
        vector<KeyPoint> keypoints_object, keypoints_scene;
        
        detectKeypoints(img_object, img_scene, 250, keypoints_object, keypoints_scene);

        // Calculate descriptors:
        Mat descriptors_object, descriptors_scene;

        calcDescriptors(img_object, img_scene, descriptors_object, descriptors_scene, keypoints_object, keypoints_scene);

        //-- Step 3: Matching descriptor vectors using FLANN matcher
        FlannBasedMatcher matcher;
        vector<DMatch> matches;
        matcher.match(descriptors_object, descriptors_scene, matches);

        double max_dist = 0;
        double min_dist = 100;

        //-- Quick calculation of max and min distances between keypoints
        for (int i = 0; i < descriptors_object.rows; ++i) {
            double dist = matches[i].distance;
            if (dist < min_dist) min_dist = dist;
            if (dist > max_dist) max_dist = dist; }

        printf("-- Max dist : %f \n", max_dist);
        printf("-- Min dist : %f \n", min_dist);

        //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
        vector<DMatch> good_matches;

        for (int i = 0; i < descriptors_object.rows; ++i) {
            if (matches[i].distance < 3*min_dist) {
                good_matches.push_back(matches[i]); } }

        Mat img_matches;
        drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
                    good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                    vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        //-- Localize the object
        vector<Point2f> obj;
        vector<Point2f> scene;

        for (int i = 0; i < good_matches.size(); ++i) {
            //-- Get the keypoints from the good matches
            obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
            scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt ); }

        Mat H = findHomography(obj, scene, CV_RANSAC);

        //-- Get the corners from the image_1 ( the object to be "detected" )
        
        /*  
         
            All following corners are indexed like so:
   
            [0]-----[1]
             |       |
            [3]-----[2]
         
        */
        
        vector<Point2f> obj_corners(4);
        
        obj_corners[0] = cvPoint(0,0);
        obj_corners[1] = cvPoint(img_object.cols, 0);
        obj_corners[2] = cvPoint(img_object.cols, img_object.rows);
        obj_corners[3] = cvPoint(0, img_object.rows);
        
        vector<Point2f> scene_corners(4);

        perspectiveTransform(obj_corners, scene_corners, H);

        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
        line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        
        //-- Draw crosshair in center of drone's view.
        circle(img_matches, Point(sceneWidth / 2 + img_object.cols, sceneHeight / 2), 10, Scalar(0, 255, 0));
        
        // Create scalar to adjust center mark to cross hair.
        float scale = 1 / ((obj_corners[1].x - obj_corners[0].x) / (scene_corners[1].x - scene_corners[0].x));
        
        //-- Mark center of recognized image only if it's actually detected:
        if (scene_corners[1].x - scene_corners[0].x > 10) {
            // Mark center:
            circle(img_matches, scene_corners[0] + Point2f(img_object.cols, 0) + Point2f(img_object.cols / 2 + 10, img_object.rows / 2 + 20), scale * 10, Scalar(0, 255, 0));
        
            // Draw line from center mark to crosshair:
            line(img_matches, Point(sceneWidth / 2 + img_object.cols, sceneHeight / 2), scene_corners[0] + Point2f(img_object.cols, 0) + Point2f(img_object.cols / 2 + 10, img_object.rows / 2 + 20), Scalar(0, 255, 0));}

        //-- Show detected matches
        imshow("Good Matches & Object detection", img_matches);

        waitKey(200); }
    
    return 0; }