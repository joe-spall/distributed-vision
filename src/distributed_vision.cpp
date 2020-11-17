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

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

extern "C" {
#include <apriltag.h>
#include <tag36h11.h>
#include <common/getopt.h>
#include <apriltag_pose.h>
}



#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h> 


#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */

typedef int SOCKET;
#endif




struct TagPacket{
    int id;
    double x;
    double y;
    double z;
};



#define PI 3.14159
#define PORT_NUM 30000
#define NET_BUFFER 1024
#define SA struct sockaddr 

std::string TARGET_ADDRESS = "192.168.1.209";
int lastSentId = 99;
double lastSentX = 99;
double lastSentY = 99;
double lastSentZ = 99;
int lastSentSize = 99;

int threads = 1;
std::string family = "tag36h11";
double decimate = 1;
double blur = 0.8;
bool refine_edges = true;
bool debug = false;

int imgHeight = 240; // Pixels
int imgWidth = 640;  // Pixels
int FOV = 194; // Degrees
float tagsize = 0.12; // meters
double cameraCenterX = float(imgWidth)/2.0;
double cameraCenterY = float(imgHeight)/2.0;
double focal_pixel = (imgWidth * 0.5) / tan(FOV * 0.5 * PI/180); // Pixels

float actualFPS = 0.0;



// Global settings
std::string folder_name = "/home/pi/distributed-vision/";
std::string calibration_data_folder = folder_name + "calibration_data/"; 

long long getTimestamp() {
    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const std::chrono::microseconds epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return  epoch.count();
}

int sockInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
    return 0;
#endif
}

int sockQuit(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

/* Note: For POSIX, typedef SOCKET as an int. */

int sockClose(SOCKET sock)
{

    int status = 0;

#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0)
    {
        status = closesocket(sock);
    }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0)
    {
        status = close(sock);
    }
#endif

    return status;

}

bool isVecTagPacketSame(std::vector<TagPacket> vec)
{
    bool sameSize = vec.size() == lastSentSize;
    if(!sameSize)
    {
        return false;
    }
    bool sameLastId = false;
    bool sameLastX = false;
    bool sameLastY = false;
    bool sameLastZ = false;
    if(vec.size() > 0)
    {
        sameLastId = vec[0].id == lastSentId;
        if(!sameLastId)
        {
            return false;
        }
        
        sameLastX = vec[0].x == lastSentX;
        if(!sameLastX)
        {
            return false;
        }
        
        sameLastY = vec[0].y == lastSentY;
        if(!sameLastY)
        {
            return false;
        }
        
        sameLastZ = vec[0].z == lastSentZ;
        if(!sameLastZ)
        {
            return false;
        }
    }
    return true;
}

void sendPacket(std::vector<TagPacket> vec, SOCKET sockfd)
{
    if(!isVecTagPacketSame(vec))
    {
        socklen_t toLen = sizeof(struct sockaddr_in);
        struct sockaddr_in toInfo;

        //Generate the packet string
        char buffer[NET_BUFFER] = {0};
        std::string packet = "$";
        if(vec.size()>0)
        {
            for (auto &it :vec)
            {
                packet.append("id:");
                packet.append(std::to_string(it.id));
                packet.append(",x:");
                packet.append(std::to_string(it.x));
                packet.append(",y:");
                packet.append(std::to_string(it.y));
                packet.append(",z:");
                packet.append(std::to_string(it.z));
                packet.append("|");
        
            }
            packet.pop_back(); // Remove extra | at the end
        }

        std::cout << "Sending: " << packet << std::endl;
        strcpy(buffer,packet.c_str());
        write(sockfd, buffer, sizeof(buffer));
        
        // Keep track of last item
        lastSentSize = vec.size();
        
        if(vec.size() > 0)
        {
            lastSentId = vec[0].id;
            lastSentX = vec[0].x;
            lastSentY = vec[0].y;
            lastSentZ = vec[0].z;
        }
    }


}


std::vector<TagPacket> labelAprilTags(apriltag_detector_t &td, cv::Mat &picture, bool isFront)
{
    // Make an image_u8_t header for the Mat data
        image_u8_t im = { .width = picture.cols,
            .height = picture.rows,
            .stride = picture.cols,
            .buf = picture.data
        };

        zarray_t *detections = apriltag_detector_detect(&td, &im);
        std::vector<TagPacket> tagPose;
        // Draw detection outlines
        for (int i = 0; i < zarray_size(detections); i++) {
            apriltag_detection_t *det;
            zarray_get(detections, i, &det);
            
            cv::line(picture, cv::Point(det->p[0][0], det->p[0][1]),
                     cv::Point(det->p[1][0], det->p[1][1]),
                     cv::Scalar(0, 0xff, 0), 2);
            cv::line(picture, cv::Point(det->p[0][0], det->p[0][1]),
                     cv::Point(det->p[3][0], det->p[3][1]),
                    cv::Scalar(0, 0, 0xff), 2);
            cv::line(picture, cv::Point(det->p[1][0], det->p[1][1]),
                     cv::Point(det->p[2][0], det->p[2][1]),
                     cv::Scalar(0xff, 0, 0), 2);
            cv::line(picture, cv::Point(det->p[2][0], det->p[2][1]),
                     cv::Point(det->p[3][0], det->p[3][1]),
                     cv::Scalar(0xff, 0, 0), 2);

            apriltag_detection_info_t info;
            info.det = det;
            info.tagsize = tagsize;
            info.fx = focal_pixel;
            info.fy = focal_pixel;
            info.cx = cameraCenterX;
            info.cy = cameraCenterY;

            // Then call estimate_tag_pose.
            apriltag_pose_t pose;
            double err = estimate_tag_pose(&info, &pose);
            
            double x = round( pose.t->data[0] * 1000.0 ) / 1000.0;
            double y = round( pose.t->data[1] * 1000.0 ) / 1000.0;
            double z = round( pose.t->data[2] * 1000.0 ) / 1000.0;
            
            // If from the behind camera, invert x and z to maintain a robot coordinate plane
            if(!isFront)
            {
                x = 2 - x;
                z*=-1;
            }
            
            

            std::stringstream ss;
            ss << det->id;
            std::string tagText = ss.str();
            int tagNumber = std::stoi (tagText,nullptr,0);
            
            //std::cout << "id:" << tagNumber << ",x:" << x << ",y:" << y << ",z:" << z << std::endl;

            TagPacket foundTag = {tagNumber, x, y,z};
            
            tagPose.push_back(foundTag);
            
            int fontface = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
            double fontscale = 1.0;
            int baseline;
            cv::Size textsize = cv::getTextSize(tagText, fontface, fontscale, 2,
                                            &baseline);
            cv::putText(picture, tagText, cv::Point(det->c[0]-textsize.width/2,
                                       det->c[1]+textsize.height/2),
                    fontface, fontscale, cv::Scalar(0xff, 0x99, 0), 2);
        }
        apriltag_detections_destroy(detections);
        return tagPose;
}


int main()
{
    struct sockaddr_in servaddr, cli;
    sockInit();
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(TARGET_ADDRESS.c_str()); 
    servaddr.sin_port = htons(PORT_NUM); 
    bool networkConnected = false
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        //exit(0); 
    } 
    else
    {
        printf("connected to the server..\n"); 
        networkConnected = true;
    }
   

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
    cv::Mat map1, map2;

    cv::Mat frontMapX, frontMapY, backMapX, backMapY;

    cv::FileStorage fsFront(calibration_data_folder + "calibration_camera_" + std::to_string(imgHeight) + "_front" + ".yml", cv::FileStorage::READ);
    if (fsFront.isOpened())
    {
        fsFront["map1"] >> frontMapX;
        fsFront["map2"] >> frontMapY;
        fsFront.release();
    }
    else
    {   
        std::string frontFile = calibration_data_folder + "calibration_camera_" + std::to_string(imgHeight) + "_front" + ".yml";
        fprintf(stderr, frontFile.c_str());
        fprintf(stderr, "Front camera calibration data not found in cache.\n");
        return false;
    }

    cv::FileStorage fsBack(calibration_data_folder + "calibration_camera_" + std::to_string(imgHeight) + "_back" + ".yml", cv::FileStorage::READ);
    if (fsBack.isOpened())
    {
        fsBack["map1"] >> backMapX;
        fsBack["map2"] >> backMapY;
        fsBack.release();
    }
    else
    {
        std::string backFile = calibration_data_folder + "calibration_camera_" + std::to_string(imgHeight) + "_back" + ".yml";
        fprintf(stderr, backFile.c_str());
        fprintf(stderr, "Back camera calibration data not found in cache.\n");
        return false;
    }
    

    cv::namedWindow("Front");
    cv::moveWindow("Front", 450, 100);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 850, 100);



    float  time1 = 0, time2 = 0, time3 = 0, time4 = 0, time5 = 0, time6 = 0, time7 = 0, time8 = 0, time9 = 0, time10 = 0, time11 = 0;
    int frameNumber = 0;
    
    apriltag_family_t *tf = NULL;
    tf = tag36h11_create();
    apriltag_detector_t *td = apriltag_detector_create();
    apriltag_detector_add_family(td, tf);
    
    
    td->quad_decimate = decimate;
    td->quad_sigma = blur;
    td->nthreads = threads;
    td->debug = debug;
    td->refine_edges = refine_edges;

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
	
        // Constrains region of interest
        cv::Mat backTemp = cv::Mat(frame, cv::Rect(0, 0, imgWidth / 2, imgHeight));
        cv::Mat frontTemp = cv::Mat(frame, cv::Rect(imgWidth / 2, 0, imgWidth / 2, imgHeight));
        
        // Makes deep copy of image data to a new array
        cv::Mat front = frontTemp.clone();
        cv::Mat back = backTemp.clone();

        long long timeSplitLeftRight = getTimestamp();
        time2 += timeSplitLeftRight - timeReadFrame;

        

	
        // Rectifying left and right images
        cv::remap(front, front, frontMapX, frontMapY, cv::INTER_LINEAR);
        cv::remap(back, back, backMapX, backMapY, cv::INTER_LINEAR);

        long long timeRectify = getTimestamp();
        time3 += timeRectify - timeSplitLeftRight;
       


        long long timeShow = getTimestamp();
        time4 += timeShow - timeRectify;

        std::vector<TagPacket> tags;
        std::vector<TagPacket> frontTags = labelAprilTags(*td,front,true);
        std::vector<TagPacket> backTags = labelAprilTags(*td,back,false);
        for(const auto& foundTag: frontTags) {
            tags.push_back(foundTag);
        }
        
        for(const auto& foundTag: backTags) {
            tags.push_back(foundTag);
        }
 
        if(networkConnected)
        {
            sendPacket(tags,sockfd);
        }   
        
        
        
        cv::imshow("Front", front);
        cv::imshow("Back", back);
	
	
        frameNumber++;

        char k = cv::waitKey(1);
        if (k == 'q' || k == 'Q')
        {
            break;
        }

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
