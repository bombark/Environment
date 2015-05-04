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
#include <sys/stat.h>
#include <tiobj.hpp>
#include <tisys.hpp>
#include <algorithm>


using namespace std;
using namespace  cv;




/*============================================================================================================*/



void detectBorder (Mat& src_gray, Mat& out){
	int dilation_size = 2;
	int dilation_type = MORPH_ELLIPSE;
	Mat element = getStructuringElement( 
		dilation_type,Size( 2*dilation_size + 1, 2*dilation_size+1 ),
		Point( dilation_size, dilation_size ) 
	);

	Mat tmp;
	double high_thres = cv::threshold( src_gray, tmp, 0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU );

	Mat border;
	//GaussianBlur(src_gray, src_gray, Size(5,5), 1.5, 1.5);
	Canny( src_gray, out, high_thres/4, high_thres/3, 3 );

}


void filtroDistance(Mat& border, Mat& dst){
	int margin = 1;

	dst.create(border.size(), CV_8UC1);
	border.copyTo(dst);

	for ( int ix=margin; ix<border.cols-margin; ix++){
		if ( border.at<unsigned char>(margin,ix) == 0 )
			dst.at<unsigned char>(margin,ix) = 255;
		if ( border.at<unsigned char>(border.rows-margin,ix) == 0 )
			dst.at<unsigned char>(border.rows-margin,ix) = 255;
	}
	for ( int iy=margin; iy<border.rows-margin; iy++){
		if ( border.at<unsigned char>(iy, margin) == 0 )
			dst.at<unsigned char>(iy, margin) = 255;
		if ( border.at<unsigned char>(iy, border.cols-margin) == 0 )
			dst.at<unsigned char>(iy, border.cols-margin) = 255;
	}
//imshow("border12", dst);

	int dilation_size = 3;
	int dilation_type = MORPH_ELLIPSE;
	Mat element = getStructuringElement( 
		dilation_type,Size( 2*dilation_size + 1, 2*dilation_size+1 ),
		Point( dilation_size, dilation_size ) 
	);
	dilate (dst, dst, element);
}


class Figure {
  public:
	int id;
	RotatedRect rect;
	vector<Point> points;
	double area;
	Vec3b color;
	float dp, radius;
	Point center;
	Point2f cc_center;

  public:
	int tr,tg,tb,count;
	int tx, ty;


	Figure (vector<Point> contour, float epsilon, int id=0){
		approxPolyDP( Mat(contour), this->points, epsilon, true );
		this->rect = minAreaRect( Mat(this->points) );
		minEnclosingCircle( (Mat)this->points, cc_center, radius );
		this->id = id;
		this->area = this->rect.size.height*this->rect.size.width;

		tr = tg = tb = count = 0;
		color = Vec3b(0,0,0);
		dp = 0.0;
		tx = ty = 0;
	}

	void draw(Mat& drawing){
/*		if ( points.size() <= 2){
			return;
		}*/

		if ( this->id == -1 ){
			return;
		}

//		cout << this->subs.size() << endl;

		Scalar color = Scalar( 255, 0, 0 );
		//drawContours( drawing, this->points, this->id, Scalar(55,55,55), 1, 8, vector<Vec4i>(), 0, Point() );

		/*for (int i=0; i<this->points.size(); i++){
			//circle( drawing, this->points[i], 3, color, 1, 8, 0 );
		}
		for (int i=1; i<this->points.size(); i++){
			line( drawing, this->points[i-1], this->points[i], color, 1, 8 );
		}
		line( drawing, this->points[this->points.size()-1], this->points[0], color, 1, 8 );*/
		circle( drawing, this->cc_center, this->radius, color, 1, 8, 0 );

		/*Point p;
		for (int i=1; i<this->points.size(); i++){
			p.x = (this->points[i-1].x + this->points[i].x)/2; 
			p.y = (this->points[i-1].y + this->points[i].y)/2; 
			line( drawing, p, this->center, color, 1, 8 );
		}

		p.x = (this->points[this->points.size()-1].x + this->points[0].x)/2; 
		p.y = (this->points[this->points.size()-1].y + this->points[0].y)/2; 
		line( drawing, p, this->center, color, 1, 8 );*/


		char text[32];
		sprintf(text,"%d", this->id);
		circle( drawing, this->center, 2, color, 2, 8, 0 );

		// Desenha o Retangulo
		/*Point2f rect_points[4]; 
		this->rect.points( rect_points );
		for( int j = 0; j < 4; j++ )
			line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );*/
		//putText(drawing, text, this->rect.center, 0, 0.4, Scalar(255,0,0));

		//this->percorre (drawing);
	}

	void percorre (Mat& drawing){
		Point2f d1;
		float v1_x;
		
		Point2f rect_points[4]; 
		this->rect.points( rect_points );
		d1 = rect_points[0] - rect_points[1];
		int l1_base = rect_points[0].y;
		int l1_size = fabs(d1.y); 
		v1_x = d1.x/d1.y;
		float x = rect_points[0].x;
		for (int iy=0; iy<l1_size; iy++, x-=v1_x){
			drawing.at<Vec3b>(l1_base-iy, x) = Vec3b(255,255,0);			
		}



		//line( drawing, rect_points[0], rect_points[1], Scalar(0,255,0), 1, 8 );
	}


};



bool compareFigure (Figure a, Figure b){
	return a.count > b.count;
}



int paint(Mat& labels, int color, int y, int x){
	int c = labels.at<unsigned char>(y,x);
	if ( c == 255 ){
		labels.at<unsigned char>(y,x) = color;

		int ix;
		int inix, endx;		
		for ( ix=x; ix>=0; ix-- ){
			c = labels.at<unsigned char>(y,ix);
			if ( c != 255 )
				break;
			labels.at<unsigned char>(y,ix) = color;
		}
		inix = ix;
		for ( ix=x+1; ix<labels.cols; ix++ ){
			c = labels.at<unsigned char>(y,ix);
			if ( c != 255 )
				break;
			labels.at<unsigned char>(y,ix) = color;
		}
		endx = ix;

		if ( y < labels.rows-1 ){
			int ty = y + 1;
			for ( ix=inix; ix<endx; ix++ ){
				paint(labels, color, ty, ix);
			}
		}

		if ( y > 0 ){
			int ty = y - 1;
			for ( ix=inix; ix<endx; ix++ ){
				paint(labels, color, ty, ix);
			}
		}

	} else
		return c;
}


void detectLabels (vector<Figure> figures, Size size, Mat& labels){
	labels.create(size, CV_8UC1);
	labels.setTo(255);

	for (int i=0; i<figures.size(); i++){
		Figure& figure = figures[i];
		for (int i_p=1; i_p<figure.points.size(); i_p++){
			line( labels, figure.points[i_p-1], figure.points[i_p], i, 1, 8 );
		}
		line( labels, figure.points[figure.points.size()-1], figure.points[0], i, 1, 8 );
	}

	int cont = 0;
	int color = 0;
	for (int iy=0; iy<labels.rows; iy++){
		for (int ix=0; ix<labels.cols; ix++){
			color = paint(labels, color, iy, ix);
		}
	}



}

void detectKeyPoints(Mat& image, vector<KeyPoint>& out_keypoints){
	Mat border, border2;
	
	//cvtColor(image_color, image, CV_RGB2GRAY);
	
	detectBorder(image, border);
	filtroDistance(border, border2);

	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// EXTERNAL|TREE
	findContours( border2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );
	if ( contours.size() == 0 )
		return;

	vector<Figure> out_figures;
	for( int i = 0; i < contours.size(); i++ ){
		Figure tmp( Figure(contours[i],4,i) );
		if ( tmp.points.size() > 2 ){
			int id = out_figures.size();
			tmp.id = id;
			out_figures.push_back( tmp );
		}
	}

	//Mat labels;
	//detectLabels (out_figures, image.size(), labels);

	/*for (int iy=0; iy<labels.rows; iy++){
		for (int ix=0; ix<labels.cols; ix++){	
			int id  = labels.at<unsigned char>(iy,ix);
			Figure& figure = out_figures[id];
			Vec3b v = image_color.at<Vec3b>(iy,ix);	
			figure.tr +=  v[0];
			figure.tg +=  v[1];
			figure.tb +=  v[2];
			figure.tx += ix;
			figure.ty += iy;
			figure.count += 1;
		}
	}*/

	/*for ( int i=0; i<out_figures.size(); i++){
		Figure& figure = out_figures[i];
		int total = figure.count;
		if ( total > 0 ){
			figure.color = Vec3b( figure.tr/total, figure.tg/total, figure.tb/total );
			figure.center = Point( figure.tx/total, figure.ty/total );
		}
	}*/


	/*for (int iy=0; iy<labels.rows; iy++){
		for (int ix=0; ix<labels.cols; ix++){	
			int id  = labels.at<unsigned char>(iy,ix);
			Vec3b v = image_color.at<Vec3b>(iy,ix);	
			Figure& figure = out_figures[id];

			float t;
			t  = abs(figure.color[0] - v[0]);
			t += abs(figure.color[1] - v[1]);
			t += abs(figure.color[2] - v[2]);
			figure.dp += t/3.0;   
		}
	}*/


	/*for ( int i=0; i<out_figures.size(); i++){
		Figure& figure = out_figures[i];
		int total = figure.count;
		if ( total > 0 )
			figure.dp /= total;
	}*/

	out_keypoints.clear();
	for (int i=0; i<out_figures.size(); i++){
		Figure& figure = out_figures[i];
		out_keypoints.push_back( KeyPoint( figure.cc_center.x, figure.cc_center.y, figure.radius*1.5, 0, 100, 0 ) );
	}

}














/*============================================================================================================*/




bool compareKeypoint (KeyPoint a, KeyPoint b){
	return a.response > b.response;
}


float modAngle(float angle){
	if ( angle > 360.0 )
		return angle-360.0;
	else if ( angle < 0.0 )
		return 360.0+angle;
	return angle;
}




Match::Match(bool isMatch,	DMatch& dmatch){
	this->isMatch = isMatch;
	this->queryId = dmatch.queryIdx;
	this->dbId = dmatch.trainIdx;
	this->distance = dmatch.distance;
}

Match::Match(bool isMatch, int queryId){
	this->isMatch = isMatch;
	this->queryId = queryId;
	this->dbId = 0;
}




void Database::addRow(Database& query, int queryid){
	this->desc.push_back(query.desc.row(queryid));
}



void Database::matching(vector<Match>& out, Database& query){
	out.clear();
	if ( query.size() == 0 )
		return;
	if ( this->size() == 0 ){
		for( int i=0; i<query.size(); i++ ){
			out.push_back(Match(false, i));
		}
		return;
	}

	std::vector< DMatch > matches;
	BFMatcher matcher(NORM_L2);
	matcher.match(query.desc, this->desc, matches );

	// Get the good matches
	for( int i = 0; i < matches.size(); i++ ){
		if( matches[i].distance < 225 ){
			out.push_back(Match(true, matches[i]));
		} else {
			out.push_back(Match(false, matches[i]));
		}
	}
}

void Database::save(string filename, string roomname){
	FILE* fd = fopen(filename.c_str(),"w");
	fprintf(fd,"name='%s';\n", roomname.c_str());
	fprintf(fd,"keypoint_data='%s.dat';\n", filename.c_str());
	fprintf(fd,"keypoint_size=%d;\n", SIFT_DESCSIZE);
	fclose(fd);

	string file = filename+".dat";
	fd = fopen(file.c_str(),"w");
	if ( !fd )
		return;
	fwrite(desc.data, sizeof(float), desc.cols*desc.rows, fd);
	fclose(fd);
}

void Database::load(string tiobj_url){
	TiObj obj;
	obj.loadFile(tiobj_url);
	int keypoint_size = obj.atInt("keypoint_size");
	string file = tiobj_url+".dat";


	struct stat st;
	stat(file.c_str(), &st);
	int keyamount = st.st_size/keypoint_size;

	FILE* fd = fopen(file.c_str(),"r");
	if ( !fd ){
		cerr << "Nao foi possivel abrir o arquivo\n";
		return;
	}
	this->desc.create( Size(keypoint_size, keyamount), CV_32F);

	fread(this->desc.data, sizeof(float), keypoint_size*keyamount, fd);
	fclose(fd);
}

int Database::size(){
	return this->desc.rows;
}


void DatabasePkg::loadMap(string url, string method){
	Filesystem fs;
	fs.listdir( url );
	for (int i=0; i<fs.box.size(); i++){
		TiObj& node = fs.box[i];
		if ( node.is("Room") ){
			this->addRoom( node.atStr("name"), path_add( node.atStr("url"),method+"-features.ti" ) );
		}
	}
}

void DatabasePkg::addRoom(string name, string tiobj_url){
	Database aux;
	int added = 0;
	if ( this->desc.data ){
		aux.load( tiobj_url );
		vconcat(this->desc, aux.desc, this->desc );
		added = aux.desc.rows;
	} else {
		this->load( tiobj_url );
		added = this->desc.rows;
	}
	int id = this->labels.size();
	this->labels.push_back( name );
	
	// Completa a tabela desc2lbl
	int tbl_base = this->desc2lbl.size();
	int tbl_all  = tbl_base + added;
	this->desc2lbl.resize(tbl_all);
	for ( int i=tbl_base; i<tbl_all; i++){
		this->desc2lbl[i] = id;
	}
	
	cout << "Loaded " << name << " " << added << endl;
}

void DatabasePkg::compare(vector<Match>& matches, vector<int>& count, Database& image){
	this->matching(matches, image);

	count.resize( this->labels.size() );
	for (int i=0; i<this->labels.size(); i++){
		count[i] = 0;
	}

	for (int i=0; i<matches.size(); i++){
		Match& match = matches[i];
		if ( match.isMatch ){
			count[ this->desc2lbl[ match.dbId ] ] += 1;
		}
	}
}

string DatabasePkg::getRoom(Match& match){
	if ( match.isMatch ){
		return this->labels[ this->desc2lbl[ match.dbId ] ]; 
	} else
		return "";
}


Extractor::Extractor (std::string detectmethod){
	this->detectmethod = detectmethod;
	if ( this->detectmethod != "REC" )
		this->detector = FeatureDetector::create(this->detectmethod);
}



bool buffer_compare(KeyPoint a, KeyPoint b){
	return a.response >= b.response;
}


int Extractor::detect(Database& out, Mat& image){
	this->keypoints.clear();
	vector<KeyPoint> buffer;

	if ( this->detectmethod == "REC" )
		detectKeyPoints(image, buffer);
	else
		detector->detect( image, buffer );


	/*float max_resp = 0, min_resp=0; 
	for (int i=0; i<buffer.size(); i++){
		float resp = buffer[i].response;
		if ( resp > max_resp ){
			max_resp = resp;
		} 
		if ( resp < min_resp ){
			min_resp = resp;
		}
	}
	if ( min_resp < 1.0/2048 )
		min_resp = 0;*/

	float dsize = sqrt(image.cols*image.cols + image.rows*image.rows)/2;
	std::sort (buffer.begin(), buffer.end(), buffer_compare);


	int size = min((uint) buffer.size(),(uint) 32);
	for (int i=0,c=0; c<size&&i<buffer.size(); i++){
		float fs = buffer[i].size;
		fs = fs/dsize;

		if ( fs >= 0.1  &&  fs < 0.6 ){
			this->keypoints.push_back( buffer[i] );
			c += 1;
		}
	}


	extractor.compute( image, this->keypoints, out.desc );

	return this->keypoints.size();
}

