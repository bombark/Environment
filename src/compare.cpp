#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <tiobj.hpp>
#include <tisys.hpp>
#include "database.h"

using namespace std;
using namespace  cv;







int main(int argc, char** argv){
	if ( argc < 2 ){
		cerr << "Syntax: " << argv[0] << " Url:Floor String:detect=['SIFT','SURF','REC']\n";
		return 1;
	}

	Mat framecolor, frame, output, vision, border;
	VideoCapture capture;
	/*if ( strcmp(argv[1], "/dev/video0") == 0 ){
		capture.open(0);
	} else {
		capture.open(argv[1]);
	}*/

	//if ( argc > 2 )
	//	capture.open( argv[2] );
	//else
		capture.open( 1 );

	Extractor   extractor(argv[2]);
	Database    image;
	DatabasePkg global;

	global.loadMap( argv[1], argv[2] );



	capture >> framecolor;
	//VideoWriter writer("resultado.avi", CV_FOURCC('I','Y','U','V'), 24, framecolor.size(), true);



	int frameid = 0;
	vector<Match> matches;
	vector<int>   count;
	while(true){
		
		capture >> framecolor;
		if (framecolor.empty())
			break;
		cvtColor(framecolor, frame, CV_BGR2GRAY);
		GaussianBlur(frame, frame, Size(5,5), 1.5, 1.5);

		extractor.detect(image, frame);
		global.compare(matches, count, image);

		char aux1[256];
		for (int i=0; i<matches.size(); i++){
			if ( !matches[i].isMatch ){
				circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,0,255), 2);
			} else {
				string text = "";
				//text += to_string(matches[i].dbId);
				text = global.getRoom(matches[i]);
				putText(framecolor, text, extractor.keypoints[i].pt, 0, 0.7, Scalar(10,255,0));
				circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,255,0), 2);
			}
		}


		for (int i=0; i<count.size(); i++){
			cout << global.labels[i] << " : " << count[i] << endl;
		}

		imshow("opa", framecolor);
		if (waitKey(30) == 'q')
			break;
	}



}
