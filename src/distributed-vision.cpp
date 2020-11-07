// Copyright (C) 2019 Eugene a.k.a. Realizator, stereopi.com, virt2real team
// Ported from Python to C++ by Konstantin Ozernov on 10/10/2019.
//
// This file is part of StereoPi С++ tutorial scripts, and has been
// ported from Pyton version (https://github.com/realizator/stereopi-fisheye-robot)
//
// StereoPi tutorial is free software: you can redistribute it 
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of the 
// License, or (at your option) any later version.
//
// StereoPi tutorial is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with StereoPi tutorial.  
// If not, see <http://www.gnu.org/licenses/>.
//

#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

extern "C" {
#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "tagCircle21h7.h"
#include "tagCircle49h12.h"
#include "tagCustom48h12.h"
#include "tagStandard41h12.h"
#include "tagStandard52h13.h"
#include "common/getopt.h"
}



#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int threads = 1;
std::string family = "tag36h11";
double decimate = 2.0;
double blur = 0.0;
bool refine_edges = true;



float actualFPS = 0.0;

// Global settings
std::string folder_name = "/home/pi/stereopi-cpp-tutorial/";
std::string calibration_data_folder = folder_name + "calibration_data/"; 

long long getTimestamp() {
    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const std::chrono::microseconds epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return  epoch.count();
}


int main()
{
    
    
    int imgHeight = 240;
    int imgWidth = 640;

    FILE *fp;
    if ((fp = fopen("/dev/stdin", "rb")) == NULL)
    {
        fprintf(stderr, "Cannot open input file!\n");
        return 1;
    }

    int bufLen = imgWidth * imgHeight;
    char *buf = (char *)malloc(bufLen);
    int count = 0;
    
    // Loads callibration
    cv::FileStorage fsStereo(calibration_data_folder + "stereo_camera_calibration" + std::to_string(imgHeight) + ".yml", cv::FileStorage::READ);
    if (!fsStereo.isOpened())
    {
        fprintf(stderr, "Camera calibration data not found in cache\n");
        exit(1);
    }
    cv::Mat leftMapX, leftMapY, rightMapX, rightMapY;
    fsStereo["leftMapX"] >> leftMapX;
    fsStereo["leftMapY"] >> leftMapY;
    fsStereo["rightMapX"] >> rightMapX;
    fsStereo["rightMapY"] >> rightMapY;

    cv::namedWindow("Left");
    cv::moveWindow("Left", 450, 100);
    cv::namedWindow("Right");
    cv::moveWindow("Right", 850, 100);



    float  time1 = 0, time2 = 0, time3 = 0, time4 = 0, time5 = 0, time6 = 0, time7 = 0, time8 = 0, time9 = 0, time10 = 0, time11 = 0;
    int frameNumber = 0;
    
    apriltag_family_t *tf = NULL;

    
    
    

    while (true)
    {
        fseek(fp, -bufLen, SEEK_END);
        count = fread(buf, sizeof(*buf), bufLen, fp);
    	if (count == 0)
    	    break;
        long long starttime = getTimestamp();
   
        cv::Mat frame(imgHeight, imgWidth, CV_8UC1, buf);

        long long timeReadFrame = getTimestamp();
        time1 += timeReadFrame - starttime;
	
        cv::Mat left = cv::Mat(frame, cv::Rect(0, 0, imgWidth / 2, imgHeight));
        cv::Mat right = cv::Mat(frame, cv::Rect(imgWidth / 2, 0, imgWidth / 2, imgHeight));

        long long timeSplitLeftRight = getTimestamp();
        time2 += timeSplitLeftRight - timeReadFrame;

	
        // Rectifying left and right images
        cv::remap(left, left, leftMapX, leftMapY, cv::INTER_LINEAR);
        cv::remap(right, right, rightMapX, rightMapY, cv::INTER_LINEAR);

        long long timeRectify = getTimestamp();
        time3 += timeRectify - timeSplitLeftRight;
       
        cv::imshow("Left", left);
        cv::imshow("Right", right);

        long long timeShow = getTimestamp();
        time4 += timeShow - timeRectify;

    

	
        long long timeStrip = getTimestamp();
        time5 += timeStrip - timeShow;
	

        long long timeDepth = getTimestamp();
        time6 += timeDepth - timeStrip;

        time7 += timeDepth - starttime;
	
        frameNumber++;



    }

    time1 = time1 / (frameNumber * 1000);
    time2 = time2 / (frameNumber * 1000);
    time3 = time3 / (frameNumber * 1000);
    time4 = time4 / (frameNumber * 1000);
    time5 = time5 / (frameNumber * 1000);
    time6 = time6 / (frameNumber * 1000);
    time7 = time7 / (frameNumber * 1000);
    time8 = time8 / (frameNumber * 1000);
    time9 = time9 / (frameNumber * 1000);
    time10 = time10 / (frameNumber * 1000);
    time11 = time11 / (frameNumber * 1000);
    actualFPS = 1000.0/time7;

    fprintf(stderr, "Avg time (milliseconds):\nread frame: %f\nsplit to left and right: %f\nrectify: %f\nshow left and right: %f\nget strips: %f\nget depth map: %f\ncompute map: %f\nconvert to uchar: %f\ncolorize: %f\nshow map: %f\ntotal: %f\nactual FPS: %f\n", time1, time2, time3, time4, time5, time6, time8, time9, time10, time11, time7, actualFPS);
    
    return 0;
}
