#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;
using namespace cv;

void detectKeypoints(Mat& objectImage, Mat& sceneImage, int minHessian, vector<KeyPoint>& objectKeyPoints, vector<KeyPoint>& sceneKeyPoints) {
    SurfFeatureDetector detector(minHessian);
    
    detector.detect(objectImage, objectKeyPoints);
    detector.detect(sceneImage, sceneKeyPoints); }

void calcDescriptors(Mat& objectImage, Mat& sceneImage, Mat& objectDesc, Mat& sceneDesc, vector<KeyPoint>& objectKeyPoints, vector<KeyPoint>& sceneKeyPoints) {
    SurfFeatureDetector extractor;
    
    extractor.compute(objectImage, objectKeyPoints, objectDesc);
    extractor.compute(sceneImage, sceneKeyPoints, sceneDesc); }

void matchDescriptors(Mat& objectDesc, Mat& sceneDesc, vector<DMatch>& matches) {
    FlannBasedMatcher matcher;
    
    matcher.match(objectDesc, sceneDesc, matches); }

void calcMinDistance(double& minDist, Mat& objectDesc, vector<DMatch> matches) {
    for (int i = 0; i < objectDesc.rows; ++i) {
        double dist = matches[i].distance;
        
        if (dist < minDist) minDist = dist; } }

/** @function main */
int main(int argc, char** argv) {
    Mat img_object = imread("object.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_scene_rgb, img_scene;
    VideoCapture cap(0);
    cap >> img_scene_rgb;
    int sceneHeight = img_scene_rgb.rows;
    int sceneWidth  = img_scene_rgb.cols;
    
    while (true) {
        // Capture and convert video stream to grayscale:
        cap >> img_scene_rgb;
        
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

        // Match descriptors:
        vector<DMatch> matches;
        
        matchDescriptors(descriptors_object, descriptors_scene, matches);

        // Calculate minimum and maximum distances between keypoints:
        double min_dist = 100;
        
        calcMinDistance(min_dist, descriptors_object, matches);

        // Draw only good matches (whose distance is less than 3 * min_dist):
        vector<DMatch> good_matches;
        Mat img_matches;

        for (int i = 0; i < descriptors_object.rows; ++i) {
            if (matches[i].distance < 3*min_dist) {
                good_matches.push_back(matches[i]); } }
        
        drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
                    good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                    vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        // Localize object:
        vector<Point2f> obj;
        vector<Point2f> scene;

        for (int i = 0; i < good_matches.size(); ++i) {
            obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
            scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt ); }

        Mat H = findHomography(obj, scene, CV_RANSAC);

        // Get the corners from the detected object in the scene:
        
        /*  NOTE: All following corners are indexed like so:
   
            [0]-----[1]
             |       |
            [3]-----[2]  */
        
        vector<Point2f> obj_corners(4);
        
        obj_corners[0] = cvPoint(0,0);
        obj_corners[1] = cvPoint(img_object.cols, 0);
        obj_corners[2] = cvPoint(img_object.cols, img_object.rows);
        obj_corners[3] = cvPoint(0, img_object.rows);
        
        vector<Point2f> scene_corners(4);

        // Transform corners based on perspective:
        perspectiveTransform(obj_corners, scene_corners, H);

        // Draw lines between the corners:
        line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
        
        // Draw crosshair in center of drone's view:
        circle(img_matches, Point(sceneWidth / 2 + img_object.cols, sceneHeight / 2), 10, Scalar(0, 255, 0));
        
        // Create scalar to adjust center mark to cross hair:
        float scale = 1 / ((obj_corners[1].x - obj_corners[0].x) / (scene_corners[1].x - scene_corners[0].x));
        
        // Mark center of recognized image only if it's actually detected:
        if (scene_corners[1].x - scene_corners[0].x > 10) {
            // Mark center:
            circle(img_matches, scene_corners[0] + Point2f(img_object.cols, 0) + Point2f(img_object.cols / 2 + 10, img_object.rows / 2 + 20), scale * 10, Scalar(0, 255, 0));
        
            // Draw line from center mark to crosshair:
            line(img_matches, Point(sceneWidth / 2 + img_object.cols, sceneHeight / 2), scene_corners[0] + Point2f(img_object.cols, 0) + Point2f(img_object.cols / 2 + 10, img_object.rows / 2 + 20), Scalar(0, 255, 0));}

        // Show detected matches
        imshow("Good Matches & Object detection", img_matches);

        waitKey(200); }
    
    return 0; }