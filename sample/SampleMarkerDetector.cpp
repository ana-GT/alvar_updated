#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include "CvTestbed.h"
#include "MarkerDetector.h"
#include "GlutViewer.h"
#include "Shared.h"
using namespace alvar;
using namespace std;
using namespace cv;

bool init=true;
const int marker_size=2.8;
Camera cam;
Drawable d[32];
std::stringstream calibrationFilename;

IplImage mImage;

void videocallback(IplImage *image)
{
    static IplImage *rgba;
    bool flip_image = (image->origin?true:false);
    if (flip_image) {
        cvFlip(image);
        image->origin = !image->origin;
    }

    if (init) {
        init = false;
        cout<<"Loading calibration: "<<calibrationFilename.str();
        if (false) {
            cout<<" [Ok]"<<endl;
        } else {
            cam.SetRes(image->width, image->height);
            cout<<" [Fail]"<<endl;
        }
        double p[16];
        cam.GetOpenglProjectionMatrix(p,image->width,image->height);
        GlutViewer::SetGlProjectionMatrix(p);
        for (int i=0; i<32; i++) {
            d[i].SetScale(marker_size);
        }
        rgba = CvTestbed::Instance().CreateImageWithProto("RGBA", image, 0, 4);
    }
    static MarkerDetector<MarkerData> marker_detector;
    marker_detector.SetMarkerSize(marker_size); // for marker ids larger than 255, set the content resolution accordingly
    //static MarkerDetector<MarkerArtoolkit> marker_detector;
    //marker_detector.SetMarkerSize(2.8, 3, 1.5);

    // Here we try to use RGBA just to make sure that also it works...
    //cvCvtColor(image, rgba, CV_RGB2RGBA);
    marker_detector.Detect(image, &cam, true, true);
    GlutViewer::DrawableClear();
    for (size_t i=0; i<marker_detector.markers->size(); i++) {
        if (i >= 32) break;
        
        Pose p = (*(marker_detector.markers))[i].pose;
        p.GetMatrixGL(d[i].gl_mat);

        int id = (*(marker_detector.markers))[i].GetId();
        double r = 1.0 - double(id+1)/32.0;
        double g = 1.0 - double(id*3%32+1)/32.0;
        double b = 1.0 - double(id*7%32+1)/32.0;
        d[i].SetColor(r, g, b);

        GlutViewer::DrawableAdd(&(d[i]));
        std::cout << p.translation[0] << " " << p.translation[2] << std::endl;
    } 

    if (flip_image) {
        cvFlip(image);
        image->origin = !image->origin;
    }
}


//hide the local functions in an anon namespace
namespace {
    void help(char** av) {
        cout << "\nThis program justs gets you started reading images from video\n"
            "Usage:\n./" << av[0] << " <video device number>\n"
            << "q,Q,esc -- quit\n"
            << "space   -- save frame\n\n"
            << "\tThis is a starter sample, to get you up and going in a copy pasta fashion\n"
            << "\tThe program captures frames from a camera connected to your computer.\n"
            << "\tTo find the video device number, try ls /dev/video* \n"
            << "\tYou may also pass a video file, like my_vide.avi instead of a device number"
            << endl;
    }

    int process(VideoCapture& capture) {
        int n = 0;
        char filename[200];
        string window_name = "video | q or esc to quit";
        cout << "press space to save a picture. q or esc to quit" << endl;
        namedWindow(window_name, CV_WINDOW_KEEPRATIO); //resizable window;
        Mat frame;
        for (;;) {
            capture >> frame;
            if (frame.empty())
                break;
	    mImage = frame;
	    videocallback(&mImage);
            imshow(window_name, frame);
            char key = (char)waitKey(5); //delay N millis, usually long enough to display and capture input
            switch (key) {
        case 'q':
        case 'Q':
        case 27: //escape key
            return 0;
        case ' ': //Save an image
            sprintf(filename,"filename%.3d.jpg",n++);
            imwrite(filename,frame);
            cout << "Saved " << filename << endl;
            break;
        default:
            break;
            }
        }
        return 0;
    }

}

int main(int ac, char** av) {

    if (ac != 2) {
        help(av);
        return 1;
    }
    std::cout << "----------------------------------------" << std::endl;
    std::string arg = av[1];
    VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file
    if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
        capture.open(atoi(arg.c_str()));
    if (!capture.isOpened()) {
        cerr << "Failed to open a video device or video file!\n" << endl;
        help(av);
        return 1;
    }
    
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1600);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 896);
    
    //capture.set(CV_CAP_PROP_FRAME_WIDTH, 2304);
    //capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1536);
    return process(capture);
}