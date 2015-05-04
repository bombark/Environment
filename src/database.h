#ifndef DATABASE_H
#define DATABASE_H

#include "database.h"

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


using namespace std;
using namespace  cv;







// Constantes da Deteccao
//#define DETECTOR_METHOD "REC"        //["FAST"|"STAR"|"SIFT"|"SURF"|"ORB"|"BRISK"|"MSER"|"GFTT"|"HARRIS"|"Dense"|"SimpleBlob"] 
#define DETECTOR_MAX    32

// Constantes da Extracao
#define SIFT_DESCSIZE 128            //[128|64]



float modAngle(float angle);


class Match {
  public:
	bool isMatch;
	int queryId, dbId;
	double distance;

	Match(bool isMatch,	DMatch& dmatch);

	Match(bool isMatch, int queryId);
};

class Database {
  public:
	Mat desc;

	void addRow(Database& query, int queryid);             // insere a linha queryid na tabela
	void addRow(Database& query, int queryid, int dbid);   // atualiza a linha queryid da tabela
	void matching(vector<Match>& out, Database& query);
	void save(string tiobj_url, string roomname);
	void load(string tiobj_url);
	int size();
};


class DatabasePkg : public Database {
  public:
	vector<int> desc2lbl;
	vector<string> labels;

	void loadMap(string url, string method);
	void addRoom(string name, string tiobj_url);
	void compare(vector<Match>& matches, vector<int>& count, Database& image);

	string getRoom(Match& match);
};


class Extractor {
	Ptr<FeatureDetector> detector;
	SiftDescriptorExtractor extractor;
	std::string  detectmethod;

  public:
	vector<KeyPoint> keypoints;

  public:
	Extractor (std::string detectmethod);
	int detect(Database& out, Mat& image);

};


#endif
