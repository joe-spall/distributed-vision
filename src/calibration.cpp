// Copyright (C) 2019 Eugene a.k.a. Realizator, stereopi.com, virt2real team
// Ported from Python to C++ by Konstantin Ozernov on 10/10/2019. 
// 
// Modifications for distributed-vision project by Joseph Spall IV
//

#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// Camera resolution
int photo_width = 1280;
int photo_height = 480;

// Image resolution for processing
int img_width = 320;
int img_height = 240;

// Global settings
std::string folder_name = "/home/pi/distributed-vision/calibration_data/";

void calibrate_one_camera(std::vector<std::vector<cv::Vec3f> > objpoints, std::vector<std::vector<cv::Vec2f> > imgpoints, std::string front_or_back)
{
    int N_OK = (int)objpoints.size();
    cv::Size DIM(img_width, img_height);

    cv::Mat K;
    cv::Mat D;

    cv::Vec3f pt(0, 0, 0);
    cv::Mat rvecs = cv::Mat::zeros(N_OK, 1, CV_32FC3);
    cv::Mat tvecs = cv::Mat::zeros(N_OK, 1, CV_32FC3);

    cv::TermCriteria calib_criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1e-6);
    int calibration_flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC |  cv::fisheye::CALIB_FIX_SKEW;
    double rms = cv::fisheye::calibrate(objpoints, imgpoints, DIM, K, D, rvecs, tvecs, calibration_flags, calib_criteria);
    cv::Mat map1, map2;
    cv::fisheye::initUndistortRectifyMap(K, D, pt, K, DIM, CV_16SC2, map1, map2);

    // Now we'll write our results to the file for the future use
    cv::FileStorage fs(folder_name + "calibration_camera_" + std::to_string(img_height) + "_" + front_or_back + ".yml", cv::FileStorage::WRITE);
    if (fs.isOpened())
    {
        fs << "map1" << map1 << "map2" << map2 << "objpoints" << objpoints << "imgpoints" <<
              imgpoints << "camera_matrix" << K << "distortion_coeff" << D;
    }


}

int main()
{
    // Global variables preset
    int total_photos = 50;


    cv::Size image_size(img_width,img_height);

    // Chessboard parameters
    int rows = 6;
    int columns = 9;
    int square_size = 2.5;

    // Visualization options
    bool drawCorners = false;
    bool showSingleCamUndistortionResults = true;
    bool showStereoRectificationResults = true;
    bool writeUdistortedImages = true;
    std::string imageToDisp = folder_name + "scenes/scene_1280x480_1.png";

    //Calibration settings
    cv::Size CHECKERBOARD(6,9);

    cv::Mat gray_small_front, gray_small_back;

    cv::TermCriteria subpix_criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1);

    cv::Vec3f pt(0, 0, 0);
    std::vector<cv::Vec3f> objp;
    for (int i = 0; i < CHECKERBOARD.height; i++)
    {
        for (int j = 0; j < CHECKERBOARD.width; j++)
        {
            objp.push_back(cv::Vec3f(j, i, 0));
        }
    }
    fprintf(stderr, "size: %d\n", objp.size());
    std::vector<std::vector<cv::Vec3f> > objpointsFront;
    std::vector<std::vector<cv::Vec2f> > imgpointsFront;
    std::vector<std::vector<cv::Vec3f> > objpointsBack;
    std::vector<std::vector<cv::Vec2f> > imgpointsBack;

    if (drawCorners)
        fprintf(stderr, "You can press 'Q' to quit this script.\n");

    for (int photo_counter = 1; photo_counter <= total_photos; photo_counter++)
    {
        fprintf (stderr, "Import pair No %d\n", photo_counter);
        std::string frontName = folder_name + "pairs/front_";
        frontName += std::to_string(photo_counter) + ".png";
        std::string backName = folder_name + "pairs/right_";
        backName += std::to_string(photo_counter) + ".png";

        cv::Mat imgF = cv::imread(frontName);
        cv::Mat imgB = cv::imread(backName);
        if (imgF.empty() || imgB.empty())
        {
            fprintf(stderr, "There are no images in pair No %d\n", photo_counter);
	    continue;
        }

        // If stereopair is complete - go to processing
        cv::Mat grayF;
        cv::cvtColor(imgF, grayF, cv::COLOR_BGR2GRAY);
        cv::resize (grayF, gray_small_front, cv::Size(img_width,img_height), cv::INTER_AREA);
        cv::Mat grayB;
        cv::cvtColor(imgB, grayB, cv::COLOR_BGR2GRAY);
        cv::resize(grayB, gray_small_back, cv::Size(img_width, img_height), cv::INTER_AREA);


        // Find the chessboard corners
        std::vector<cv::Vec2f> cornersF;
        bool retF = cv::findChessboardCorners(grayF, CHECKERBOARD, cornersF, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE);
        std::vector<cv::Vec2f> cornersB;
        bool retB = cv::findChessboardCorners(grayB, CHECKERBOARD, cornersB, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE);

        // Draw images with corners found
        if (drawCorners)
        {
            cv::drawChessboardCorners(imgF, CHECKERBOARD, cornersF, retF);
            cv::imshow("Corners BACK", imgF);
            cv::drawChessboardCorners(imgB, CHECKERBOARD, cornersB, retB);
            cv::imshow("Corners LEFT", imgB);
            char key = cv::waitKey();
            if (key == 'q' || key == 'Q')
                exit(1);
        }

        //       Here is our scaling trick! Hi res for calibration, low res for real work!
        //       Scale corners X and Y to our working resolution
        if ((retF && retB) && (img_height <= photo_height))
        {
            float scale_ratio = (float)img_height/photo_height;
            for (int i = 0; i < cornersF.size(); i++)
                cornersF[i] *= scale_ratio;
            for (int i = 0; i < cornersB.size(); i++)
                cornersB[i] *= scale_ratio;
        }
        else if (img_height > photo_height)
        {
            fprintf(stderr, "Image resolution is higher than photo resolution, upscale needed. Please check your photo and image parameters!\n");
            exit (0);
        }

        // Refine corners and add to array for processing
        if (retF && retB)
        {
            objpointsFront.push_back(objp);
            cv::cornerSubPix(gray_small_front,cornersF,cv::Size(3,3), cv::Size(-1,-1),subpix_criteria);
            imgpointsFront.push_back(cornersF);
            objpointsBack.push_back(objp);
            cv::cornerSubPix(gray_small_back,cornersB, cv::Size(3,3), cv::Size(-1,-1), subpix_criteria);
            imgpointsBack.push_back(cornersB);
        }
        else
        {
            fprintf(stderr, "Pair No %d ignored, as no chessboard found\n", photo_counter);
            continue;
        }
        //fprintf(stderr, "End cycle\n");
    }

    // Let's calibrate each camera, and than calibrate them together
    fprintf (stderr, "Front camera calibration...\n");
    calibrate_one_camera(objpointsFront, imgpointsFront, "front");
    fprintf(stderr, "Back camera calibration...\n");
    calibrate_one_camera(objpointsBack, imgpointsBack, "back");
    fprintf(stderr, "Calibration complete!\n");

    // The following code just shows you calibration results
    if (showSingleCamUndistortionResults)
    {
        int width = 320;
        int height = 240;

        cv::Mat map1, map2;

        fprintf(stderr, "Undistorting picture with width = %d, height = %d\n", width, height);
        cv::FileStorage fsFront(folder_name + "calibration_camera_" + std::to_string(img_height) + "_front" + ".yml", cv::FileStorage::READ);
        if (fsFront.isOpened())
        {
            fsLeft["map1"] >> map1;
            fsLeft["map2"] >> map2;
            fsFront.release();
        }
        else
        {
            fprintf(stderr, "Front camera calibration data not found in cache.\n");
            return false;
        }

        cv::Mat undistorted_front;
        cv::remap(gray_small_front, undistorted_front, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

        cv::FileStorage fsBack(folder_name + "calibration_camera_" + std::to_string(img_height) + "_back" + ".yml", cv::FileStorage::READ);
        if (fsBack.isOpened())
        {
            fsBack["map1"] >> map1;
            fsBack["map2"] >> map2;
            fsBack.release();
        }
        else
        {
            fprintf(stderr, "Back camera calibration data not found in cache.\n");
            return false;
        }

        cv::Mat undistorted_back;
        cv::remap(gray_small_back, undistorted_back, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

        cv::imshow("Undistorted FRONT", undistorted_front);
        cv::imshow("Undistorted BACK", undistorted_back);
        cv::waitKey(0);
        if (writeUdistortedImages)
        {
            cv::imwrite(folder_name + "undistorted_front.jpg", undistorted_front);
            cv::imwrite(folder_name + "undistorted_back.jpg", undistorted_back);
        }

    }

    return 0;
}
