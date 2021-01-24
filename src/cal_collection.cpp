// Copyright (C) 2019 Eugene a.k.a. Realizator, stereopi.com, virt2real team
// Ported from Python to C++ by Konstantin Ozernov on 10/10/2019. 
// 
// Modifications for distributed-vision project by Joseph Spall IV
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>


long long getTimestamp()
{
   const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   const std::chrono::microseconds epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
   return  epoch.count();
}

int main()
{ 
        //User quit method message
    fprintf(stderr, "You can press 'Q' to quit this script.\n");

    //Photo session settings
    int total_photos = 50;//            # Number of images to take
    int countdown = 3;//                 # Interval for count-down timer, seconds
    int font = cv::FONT_HERSHEY_SIMPLEX;// # Cowntdown timer font
    std::string folder_name = "/home/pi/distributed-vision/calibration_data/";    
     
    //Camera settimgs
    int cam_width = 1280;//            # Cam sensor width settings
    int cam_height = 480;//            # Cam sensor height settings

    int half_width = 640;
    int half_height = 480;
    
    //Final image capture settings
    float scale_ratio = 1;

    // Camera resolution height must be dividable by 16, and width by 32
        
    cam_width = (int)(((cam_width+31)/32)*32);
    cam_height = (int)(((cam_height+15)/16)*16);
    fprintf(stderr, "Camera resolution: %d x %d\n", cam_width, cam_height);

    // Buffer for captured image settings
    int img_width = (int)((cam_width * scale_ratio));
    int img_height = (int)((cam_height * scale_ratio));
    fprintf(stderr, "Scaled image resolution: %d x %d\n", img_width, img_height);

    // Initialize the camera
    FILE *fp;
    if ((fp = fopen("/dev/stdin", "rb")) == NULL)
    {
        fprintf(stderr, "Cannot open input file!\n");
        return 1;
    }

    int bufLen = cam_width * cam_height;
    char *buf = (char *)malloc(bufLen);
    int count = 0;

    // Lets start taking photos!
    int counter = 0;
    long long t2 = getTimestamp();
    fprintf(stderr, "Starting photo sequence\n");
    cv::Mat frame;
    while (true)
    {
        fseek(fp, -bufLen, SEEK_END);
        count = fread(buf, sizeof(*buf), bufLen, fp);
	    if (count == 0)
	        break;
        cv::Mat frame(cam_height, cam_width, CV_8UC1, buf);
        long long t1 = getTimestamp();
        int cntdwn_timer = countdown - (int)((t1-t2) / 1000000);
	
        // If cowntdown is zero - let's record next image
        if (cntdwn_timer == -1)
        {
            counter += 1;
            std::string filename = folder_name + "scenes/scene_" + std::to_string(img_width) + "x" + std::to_string(img_height) + "_" + std::to_string(counter) + ".png";
            cv::imwrite(filename, frame);
            fprintf(stderr, "[%d of %d] %s\n", counter, total_photos, filename.c_str());

	        cv::Mat img_front = cv::Mat(frame, cv::Rect(0, 0 , half_width, half_height));
	        cv::Mat img_back = cv::Mat(frame, cv::Rect(half_width, 0, half_width, half_height));
	        std::string front_name = folder_name + "pairs/front_" + std::to_string(counter) + ".png";
	        std::string back_name = folder_name + "pairs/back_" + std::to_string(counter) + ".png";
	        cv::imwrite(front_name, img_front);
	        cv::imwrite(back_name, img_back);
	    
            t2 = getTimestamp();
            sleep(1);
            cntdwn_timer = 0;      // To avoid "-1" timer display
        }
            
        // Draw cowntdown counter, seconds
        cv::putText(frame, std::to_string(cntdwn_timer), cv::Point(50,50), font, 2.0, cv::Scalar(0,0,255), 4, 16 /*CV_AA*/);
        cv::imshow("pair", frame);
        char key = cv::waitKey(1);
        // Press 'Q' key to quit, or wait till all photos are taken
        if (key == 'q' || key == 'Q' || counter == total_photos)
            break;
    }
         
    fprintf(stderr, "Photo sequence finished\n");
    
    return 0;
}
