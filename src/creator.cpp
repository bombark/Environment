#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <math.h>
#include <tiobj.hpp>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <tisys.hpp>

#include "database.h"


using namespace std;
using namespace  cv;




void proc_image(Database& global, TiObj& node_img, std::string detectmethod){
	Database  image;
	Extractor extractor(detectmethod);

	Mat framecolor, frame;
	framecolor = imread( node_img.atStr("url") );

	cvtColor(framecolor, frame, CV_BGR2GRAY);
	GaussianBlur(frame, frame, Size(5,5), 1.5, 1.5);

	vector<Match> matches;
	extractor.detect(image, frame);
	global.matching(matches, image);


	// Mostra o Matching
	for (int i=0; i<matches.size(); i++){
		if ( matches[i].isMatch == true ){
			circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,255,0));
			putText(framecolor, to_string(matches[i].dbId), extractor.keypoints[i].pt, 0, 0.4, Scalar(10,255,0));
		} else {
			circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,0,255));
			global.addRow(image, i);
		}
	}

	imshow("Frame", framecolor);
	waitKey(30);
}


void proc_video(Database& global, TiObj& node_video, std::string detectmethod){
	Database  image;
	Extractor extractor(detectmethod);	

	Mat framecolor, frame;
	VideoCapture capture;
	capture.open(node_video.atStr("url"));

	while (true){
		capture >> framecolor;
		if ( framecolor.empty() )
			break;

		cvtColor(framecolor, frame, CV_BGR2GRAY);
		GaussianBlur(frame, frame, Size(5,5), 1.5, 1.5);

		vector<Match> matches;
		extractor.detect(image, frame);
		global.matching(matches, image);

		// Mostra o Matching
		for (int i=0; i<matches.size(); i++){
			if ( matches[i].isMatch == true ){
				circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,255,0));
				putText(framecolor, to_string(matches[i].dbId), extractor.keypoints[i].pt, 0, 0.4, Scalar(10,255,0));
			} else {
				circle(framecolor, extractor.keypoints[i].pt, extractor.keypoints[i].size, Scalar(10,0,255));
				global.addRow(image, i);
			}
		}

		imshow("Frame", framecolor);
		waitKey(30);
	}
}



int main(int argc, char** argv){
	if ( argc <= 2 ){
		cerr << "Syntax: " << argv[0] << " Url:Room String:detect=['SIFT','SURF','REC']\n";
		return 0;
	}

	string detectmethod = argv[2];
	Database  global;
	Filesystem fs;
	fs.listdir( path_add(argv[1], "images") );

	for (int ai=0; ai<fs.box.size(); ai++){
		TiObj& node = fs.box[ai];
		cout << node.atStr("url") << endl;
		if ( node.is("Image") )
			proc_image(global, node, detectmethod);
		else if ( node.is("Video") )
			proc_video(global, node, detectmethod);
	}

	char filename[1024];
	sprintf(filename, "%s-features.ti", detectmethod.c_str());
	global.save( path_add(argv[1], filename), fs.atStr("name") );
}


