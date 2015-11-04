// ����i�v��.cpp : �w�q�D���x���ε{�����i�J�I�C (0618)  �����~��  ���[�i�P�_�O��~�٬O�G�D��
  //                                                               ���O�l�v�l�����D�٬O�S�ѨM

#include <stdio.h>
#include <math.h>
#include "stdafx.h"

#include <opencv2\opencv.hpp>
#include <opencv2\gpu\gpu.hpp>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h> 
#include <algorithm>    // std::max


using namespace std;
using namespace cv;

typedef unsigned char uchar ;

# define PI 3.1415926
# define NUM 30  // vanishing point queue ���j�p
# define VIDEO_WIDTH 640  // �w�q�v���j�p
# define VIDEO_HEIGHT 480
# define TRACKING 0  // �ݬO�_�n�l��  1��true 
# define WRITEVIDEO 1
# define WRITEFRAME 0
int frame_num ;

int vanishing_point_queue_x[NUM]; // �s��x �y��
int vanishing_point_queue_y[NUM]; // �s��y �y��
int sizeof_vanishing_point_queue = 0, index_in_queue = 0;
Point VP ; // �ӵe����vanish point ��m
Point left_lane_top_point,left_lane_bot_point,right_lane_top_point,right_lane_bot_point;
// �̫�M�w�����k���D ��global�覡�s

//CvCapture *CAPTURE;
VideoCapture CAPTURE ;
Rect FRAME_ROI_RECT;

void Find_HoughLines_and_VanishingPoint( cv::Mat image, cv::Mat frame,
	           Point &pre_RT, Point &pre_RB, Point &pre_LT, Point &pre_LB) ;// �䪽�u�B��Xvanishimg point
double Slope( int x1, int y1, int x2 , int y2 );  // �p��ײv
void laneMarkingDetector( cv::Mat &srcGRAY , cv::Mat &dstGRAY , int tau ) ; // �j�v��filter
Point Find_Intersection_Point( Point LT, Point LB, Point RT, Point RB ) ; // �Q�Ψ⪽�u ��vanishing point
void AddInQueue(int data, int data1) ; // �[�J�F��
void quick_sort(int queue[],int low,int high) ; // QUICKSORT
Point Find_Vanishing_Point( Point ans ) ;  //�s��queue ��vanishing point�������
double Distance_of_two_points( Point pt1, Point pt2 ) ; // �D���I�������Z��
void Extand_line( Point pt1,Point pt2,Point &ans_pt1, Point &ans_pt2 ) ; //�������D�u

bool LoadTrainingDetect( std::vector<float>&desc, char* filename, int DIM ) ;
void RunDetectionLoop(cv::Mat frame , Rect FRAME_ROI_RECT, std::vector<cv::Rect> &found, int type ) ;
void SelectingAndDrowingObject( cv::Mat frame, std::vector<cv::Rect> found ) ;
void Label_Tracking( cv::Mat temp_frame, cv::Mat frame,std::vector<cv::Rect> found ) ;   // ���l�ܪ����l���s��
double CalculateDistance( CvPoint a , CvPoint b ) ;
bool If_tracking_rect_already_exist( CvPoint detect_pt, CvSize detect_size,cv::Mat frame ) ;
bool IsOverlap( cv::Rect rect1, cv::Rect rect2 ) ;
double Template_matching_rate( CvPoint a, CvSize a_size, CvPoint b, CvSize b_size, cv::Mat frame ) ; 
                   // �p����template ���ۦ��{��
bool If_Point_Outofframe( CvPoint pt, CvSize size ) ;
void Calculate_car_color( CvPoint position, CvSize size, cv::Mat frame, int which_Car ) ;
// �p�⨮���C��  �H�ΫD���C�� �������
double Point_Compare_Vehicle_Color( CvPoint pt, CvPoint midfound, int which_car ) ;
double detect_compare_tracking(CvPoint midfound, int which_car) ;
void Cvt1dto2d( IplImage *src, int **r, int **g, int **b ) ;// �o��function�ΨӦs�v�����Ҧ�pixel
void Cvt2dto1d( int **r, int **g, int **b, IplImage *dst ) ;//�o��function�Ψӧ�T�ӳq�쪺��assign�^�@�i�v��
double Calculating_overlap_rate(cv::Rect A, cv::Rect B) ; // �p���ӯx�Ϊ��л\�v
void Delete_tracking_overlap_tracking() ; // �R�����l�ܮح��ƪ�
bool Environment_is_Outdoor(); // �P�_�{�b�O�b��~�٬O�b�G�D��

cv::Scalar BLUE = cv::Scalar(255,0,0) ;
cv::Scalar RED = cv::Scalar(0,0,255) ;
cv::Scalar GREEN = cv::Scalar(0,255,0) ;
cv::Scalar YELLOW = cv::Scalar(0,255,255) ;
cv::Scalar PURPLE = cv::Scalar(255,0,255) ;
cv::Scalar ORANGE = cv::Scalar(0,128,255) ;
cv::Scalar WHITE = cv::Scalar(255,255,255) ;
cv::Scalar DARK_RED = cv::Scalar(50,50,180) ;
cv::Scalar DARK_GREEN = cv::Scalar(50,180,50) ;
cv::Scalar DARK_BLUE = cv::Scalar(180,50,50) ;


int BIN = 9;
int DIM = 324;   
cv::Size SAMPLE_SIZE(32, 32);
cv::gpu::HOGDescriptor *GPU_MODE;
cv::gpu::HOGDescriptor *GPU_MODE_TUNNEL;
cv::HOGDescriptor *HOG_PTR;
bool Write_Retraining_Image = false ; // �O�_�n��Xretraining image
bool Write_Tracking_Result_image = false ;
int outputSample_Index = 0;  // ��Xretraining image �ɪ��s��
int outputTracking_Index =0 ;
bool Is_TUNNEL = false ;
//char FILE_ADDR[300] = "C:\\Users\\Joy\\Desktop\\TME�v��\\tme08.avi" ;
char FILE_ADDR[300] = "C:\\Users\\Joy\\Desktop\\dataset\\test data\\�@��\\FILE0008.mov" ;
char Output[300]="C:\\Users\\Joy\\Desktop\\RESULT\\�@��\\FILE2417\\DETECT\\";
char Output_frame[300]="C:\\Users\\Joy\\Desktop\\RESULT\\�@��\\FILE2417\\";
char TRAIN_DATA_ADDR[300] = "C:\\Users\\Joy\\Desktop\\�n���˥�\\�@��ALL\\SVMD.txt"; // ��~��SV
char TRAIN_DATA_ADDR_TUNNEL[300] = "C:\\Users\\Joy\\Desktop\\�n���˥�\\�G�DALL\\SVMD.txt"; // �G�D��SV
char Tracking_ADDR[300] = "C:\\Users\\Joy\\Desktop\\RESULT\\�@��\\FILE2417\\TRACKING\\";
char Tracking_ADDR_TUNNEL[300] = "C:\\Users\\Joy\\Desktop\\RESULT\\�@��\\FILE2417\\TRACKING_TUNNEL\\";
char Output_TUNNEL[300] = "C:\\Users\\Joy\\Desktop\\RESULT\\�@��\\FILE2417\\DETECT_TUNNEL\\";


class LineFinder{  
private:  
        // ���u�������I�ѼƦV�q
        std::vector<cv::Vec4i> lines;  
        //�B�� 
        double deltaRho;  
        double deltaTheta;  
        // �P�_�O���u���̤p�벼��  
        int minVote;  
        // �P�_�O���u���̤p����  
        double minLength;  
        // �P�@�����u�W�I�Z���������̤p�e�ԫ� 
        double maxGap;  
public:  
        //��l��  
        LineFinder() : deltaRho(1), deltaTheta(PI/180),  
        minVote(10), minLength(0.), maxGap(0.) {}  
        // �]�m�B�� 
        void setAccResolution(double dRho, double dTheta) {  
                deltaRho= dRho;  
                deltaTheta= dTheta;  
        }  
        // �]�m�̤p�벼��
        void setMinVote(int minv) {  
                minVote= minv;  
        }  
        // �]�m�̤p���שM�̤p�벼�� 
        void setLineLengthAndGap(double length, double gap) {  
                minLength= length;  
                maxGap= gap;  
        }  
  
        // �M��u�q  
        std::vector<cv::Vec4i> findLines(cv::Mat& binary) {  
                lines.clear();  
                cv::HoughLinesP(binary,lines, deltaRho, deltaTheta, minVote,minLength, maxGap);  
                return lines;  
        }  
  
        // �e���D�u�q 
        void drawDetectedLines(cv::Mat &image) {           
          std::vector<cv::Vec4i>::const_iterator it2=lines.begin();  
          Point closest_left_top,closest_left_bot,closest_right_top,closest_right_bot; // �����VP�̪񪺽u
          double min_left = 1000, min_right = 1000;
          bool find_right = false, find_left = false ;

          if ( lines.size() != 0 ) {
            while (it2!=lines.end()) {  
              cv::Point pt1((*it2)[0],(*it2)[1]+150);  
              cv::Point pt2((*it2)[2],(*it2)[3]+150); 

				    if( abs(Slope( pt1.x, pt1.y, pt2.x ,pt2.y ) ) < 0.5) // ������ȫ᪺�ײv �Ӥ��� ���|�O���D
				    	;
				    else if (Slope( pt1.x, pt1.y,pt2.x ,pt2.y ) > 0 ) {   // �k���D
				    	if ( pt1.y >240 && pt2.y > 240 && pt1.x > VP.x && pt2.x > VP.x ) {
                if ( Distance_of_two_points(  pt1, VP ) < min_right ) {
                  closest_right_top.x = pt1.x;
                  closest_right_top.y = pt1.y;
                  closest_right_bot.x = pt2.x;
                  closest_right_bot.y = pt2.y;
                  min_right = Distance_of_two_points(  pt1, VP ) ;
                  find_right = true ;
                }//if
              } // if
				    } // else if
				    else if (Slope( pt1.x, pt1.y, pt2.x ,pt2.y ) < 0 ) {  // �����D
				    	if (pt1.y >240 && pt2.y > 240 && pt1.x < VP.x && pt2.x < VP.x ) {
                if ( Distance_of_two_points(  pt1, VP ) < min_left ) {
                  closest_left_top.x = pt1.x;
                  closest_left_top.y = pt1.y;
                  closest_left_bot.x = pt2.x;
                  closest_left_bot.y = pt2.y;
                  min_left = Distance_of_two_points( pt1, VP ) ;
                  find_left = true ;
                 }//if
               } // if
			    	 } // else if

                ++it2;
            }  // while

            if ( find_left == true && find_right == true ) {  // �Y�O���k�⨮�D�������
              Extand_line( closest_left_top,closest_left_bot,left_lane_top_point,left_lane_bot_point ) ;
              Extand_line( closest_right_top,closest_right_bot,right_lane_top_point,right_lane_bot_point ) ;
              cv::line( image, left_lane_top_point, left_lane_bot_point, BLUE,3,8,0);
              cv::line( image, right_lane_top_point, right_lane_bot_point, RED,3,8,0);
            } // if
            else {
              cv::line( image, left_lane_top_point, left_lane_bot_point, BLUE,3,8,0);
              cv::line( image, right_lane_top_point, right_lane_bot_point, RED,3,8,0);
            } // end else
          } // if
          else {
            cv::line( image, left_lane_top_point, left_lane_bot_point, BLUE,3,8,0);
            cv::line( image, right_lane_top_point, right_lane_bot_point, RED,3,8,0);
          } // else
        } // drawDetectedLines
}; // LineFinder()


//1.condensation setup  
const int stateNum=4;  
const int measureNum=2;  
const int sampleNum=600;

const double template_matching_threshold = 0.5;  // �l�ܮػP�����ؤ����m���֭�
const double spatial_distance_threshold = 50 ;
CvMat* lowerBound;  
CvMat* upperBound;  

struct Particle{
 bool Create_Condens ;   // �O�_���Ыسo��particle filter
 bool Condens_work ;  // �o��particle ���S���b�ϥ�
 CvSize Condens_size ;  // �o��particle �ثe�l�ܪ���  size ���ӬO�h��
 bool detecting_tracking_overlapping ;  // �O���O���򰻴��ح��|  �]�N�O�P�_�l�ܮڰ����O�_�O�P�@�x��
 CvPoint temp_predict_pt ;  // �Ȧs�e�@�Ӯɨ誺�w�����G
 CvPoint detect_result_pt ;  // �O�� �����ت��j�p
 CvPoint pre_tracking_position ; // �e�@��l�ܪ����G��m
 int counter_start ; // �p��h�[�S���ư����خ��л\������ܼ�
 int counter_end ;  // �p��h�[�S���ư����خ��л\������ܼ�
 cv::Scalar car_color ;
 cv::Scalar not_car_color;
 double R_car_color[256] ;    // �s�o��particle �O���l��Histogram 
 double G_car_color[256] ;
 double B_car_color[256] ;
 double R_not_car_color[256] ;  // �s�o��particle ���O���l��Histogram
 double G_not_car_color[256] ;
 double B_not_car_color[256] ;
 double B_confidence[256] ;  // �ΨӦs�D���������l����mhistogram �n�M�wconfidence��histogram
 double G_confidence[256] ;
 double R_confidence[256] ;
 bool If_Draw_Result ;
} ;
//�ŧi���Ӱl�ܾ�
Particle particle_1, particle_2, particle_3, particle_4, particle_5, particle_6 ;
// �Ыؤ��Ӱl�ܾ�

CvConDensation* condens_1 = cvCreateConDensation(stateNum,measureNum,sampleNum);
CvConDensation* condens_2 = cvCreateConDensation(stateNum,measureNum,sampleNum);
CvConDensation* condens_3 = cvCreateConDensation(stateNum,measureNum,sampleNum);
CvConDensation* condens_4 = cvCreateConDensation(stateNum,measureNum,sampleNum);
CvConDensation* condens_5 = cvCreateConDensation(stateNum,measureNum,sampleNum);
CvConDensation* condens_6 = cvCreateConDensation(stateNum,measureNum,sampleNum);


int **BLUE_ARRAY,**RED_ARRAY,**GREEN_ARRAY ;  // �s�v����pixel��

int control = 0 ;

int _tmain(int argc, _TCHAR* argv[])
{
	frame_num = 0 ;
  /* particle filter �]�w*/	
    lowerBound = cvCreateMat(stateNum, 1, CV_32F);  
    upperBound = cvCreateMat(stateNum, 1, CV_32F);  
    cvmSet(lowerBound,0,0,0.0 );   
    cvmSet(upperBound,0,0,VIDEO_WIDTH );  
    cvmSet(lowerBound,1,0,0.0 );   
    cvmSet(upperBound,1,0,VIDEO_HEIGHT );  
    cvmSet(lowerBound,2,0,0.0);   
    cvmSet(upperBound,2,0,0.0);  
    cvmSet(lowerBound,3,0,0.0 );   
    cvmSet(upperBound,3,0,0.0 );  
    float A[stateNum][stateNum] ={  
        1,0,1,0,  
        0,1,0,1,  
        0,0,1,0,  
        0,0,0,1  
    };  
    memcpy(condens_1->DynamMatr,A,sizeof(A));  
    memcpy(condens_2->DynamMatr,A,sizeof(A));  
    memcpy(condens_3->DynamMatr,A,sizeof(A));  
    memcpy(condens_4->DynamMatr,A,sizeof(A));  
    memcpy(condens_5->DynamMatr,A,sizeof(A));  
    memcpy(condens_6->DynamMatr,A,sizeof(A));  
    cvConDensInitSampleSet(condens_1, lowerBound, upperBound);   
    cvConDensInitSampleSet(condens_2, lowerBound, upperBound);   
    cvConDensInitSampleSet(condens_3, lowerBound, upperBound);   
    cvConDensInitSampleSet(condens_4, lowerBound, upperBound);   
    cvConDensInitSampleSet(condens_5, lowerBound, upperBound);   
    cvConDensInitSampleSet(condens_6, lowerBound, upperBound);  
  
    CvRNG rng_state = cvRNG(0xffffffff);  
    for(int i=0; i < sampleNum; i++){  
        condens_1->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_1->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
        condens_2->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_2->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
        condens_3->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_3->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
        condens_4->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_4->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
        condens_5->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_5->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
        condens_6->flSamples[i][0] = float(cvRandInt( &rng_state ) % VIDEO_WIDTH); //width  
        condens_6->flSamples[i][1] = float(cvRandInt( &rng_state ) % VIDEO_HEIGHT);//height  
    }  

	particle_1.Condens_work = false ;
	particle_2.Condens_work = false ;
	particle_3.Condens_work = false ;
	particle_4.Condens_work = false ;
	particle_5.Condens_work = false ;
	particle_6.Condens_work = false ;

	particle_1.detecting_tracking_overlapping = false ;
	particle_2.detecting_tracking_overlapping = false ;
	particle_3.detecting_tracking_overlapping = false ;
	particle_4.detecting_tracking_overlapping = false ;
	particle_5.detecting_tracking_overlapping = false ;
	particle_6.detecting_tracking_overlapping = false ;

	particle_1.Create_Condens = false ;
	particle_2.Create_Condens = false ;
	particle_3.Create_Condens = false ;
	particle_4.Create_Condens = false ;
	particle_5.Create_Condens = false ;
	particle_6.Create_Condens = false ;

	particle_1.If_Draw_Result = false ;
	particle_2.If_Draw_Result = false ;
	particle_3.If_Draw_Result = false ;
	particle_4.If_Draw_Result = false ;
	particle_5.If_Draw_Result = false ;
	particle_6.If_Draw_Result = false ;

  /* particle filter �]�w*/	



  cv::Mat frame;
  cv::Mat ROI_img;
  cv::Mat black_white_img ;
  cv::Mat temp_frame ;

  // �]�wROI�y��
  FRAME_ROI_RECT = Rect(0,150,640,330) ; // ���W��x  ���W�� y  ROI �e  ROI ��
  // �]�wROI�y��

  CAPTURE=VideoCapture(FILE_ADDR);  // �q���w�ɮק�v�� (*) avi�榡
  
  /* �O�_�n�g��*/

# if WRITEVIDEO
  int AviForamt = CV_FOURCC('D', 'I', 'V', 'X')  ;   
  double FPS = CAPTURE.get( CV_CAP_PROP_FPS ) ;
  int AviColor = 1;

  VideoWriter writer("C:\\Users\\Joy\\Desktop\\result.avi", CV_FOURCC('M', 'J', 'P', 'G'), FPS, cvSize(VIDEO_WIDTH,VIDEO_HEIGHT)); 

#endif

  /*SVM HOG descriptor*/
  GPU_MODE = new cv::gpu::HOGDescriptor( SAMPLE_SIZE, cv::Size(16,16), cv::Size(8,8), cv::Size(8,8), BIN,
	                                          cv::gpu::HOGDescriptor::DEFAULT_WIN_SIGMA, 0.2, true,
	                                          cv::gpu::HOGDescriptor::DEFAULT_NLEVELS );
  GPU_MODE_TUNNEL = new cv::gpu::HOGDescriptor( SAMPLE_SIZE, cv::Size(16,16), cv::Size(8,8), cv::Size(8,8), BIN,
	                                          cv::gpu::HOGDescriptor::DEFAULT_WIN_SIGMA, 0.2, true,
	                                          cv::gpu::HOGDescriptor::DEFAULT_NLEVELS );											  
  /*HOG_PTR = new cv::HOGDescriptor( SAMPLE_SIZE, cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 
                                     BIN, 1, -1, cv::HOGDescriptor::L2Hys, 0.2, false, cv::HOGDescriptor::DEFAULT_NLEVELS );
  */
  std::vector<float> training_descriptors ;
  std::vector<float> training_descriptors_TUNNEL ;
  if ( !LoadTrainingDetect( training_descriptors, TRAIN_DATA_ADDR, DIM ) 
	  || !LoadTrainingDetect( training_descriptors_TUNNEL,TRAIN_DATA_ADDR_TUNNEL, DIM) ) {
    std::cout << "****Error:SVMDescriptors���J����" << std::endl;
    system( "pause" );
    return 0;
  } // if
  else {
    GPU_MODE->setSVMDetector( training_descriptors );
	GPU_MODE_TUNNEL->setSVMDetector( training_descriptors_TUNNEL );
    //HOG_PTR.setSVMDetector( training_descriptors );
	std::cout << "SVMDescriptors���J���\" << std::endl;
    std::cout << "����(�trho)�G" << training_descriptors.size() << std::endl;
	system("pause");
  } // else

  /*SVM HOG descriptor*/

  std::vector<cv::Rect> found ; // ��쪺���l��m  �@��خت����X


  if ( !CAPTURE.isOpened() ) {
    std::cout << "****Error:�v��Ū������" << std::endl;
    system( "pause" );
    return 0;
  } // if

  std::cout << "�v����fps�O:" << CAPTURE.get( CV_CAP_PROP_FPS) <<endl;
  std::cout <<"�o���v���`�@��" << CAPTURE.get( CV_CAP_PROP_FRAME_COUNT) << "�iframe" << endl;
  system("pause");

  Point pre_RT, pre_RB, pre_LT,pre_LB ; // �����e�@������ƪ�vanishing line ����S�����n��vanishing line
  // �u�q��l�Ƭ��e���S�W�쥪�U �H�Υ��U��k�W ������u	
  pre_RT.x = 0;
  pre_RT.y = 0;
  pre_RB.x = frame.cols;
  pre_RB.y = frame.rows;
  pre_LT.x = frame.cols;
  pre_LT.y = 0;
  pre_LB.x = 0;
  pre_LB.y = frame.rows;
  // �����n�M�wvanishing point������u�q


  // *************************************************
  CAPTURE.grab() ;
  CAPTURE.retrieve( frame );
  cv::resize(frame,frame,cv::Size(640,480));
  IplImage * pixel_img = cvCreateImage(cvSize(640,480),8,3);
  pixel_img = &IplImage(frame) ;
 
  /*�s�v����pixel�Ȩ�BGR��*/
  
  int i ;  
  int *pData ;  // �simage��pixel�� �ΤG���������

  BLUE_ARRAY = new int*[pixel_img->height];
  RED_ARRAY = new int*[pixel_img->height];
  GREEN_ARRAY = new int*[pixel_img->height];
  for ( i = 0 ; i < pixel_img->height ; i++ ) {
	  BLUE_ARRAY[i] = new int[pixel_img->width];
	  RED_ARRAY[i] = new int[pixel_img->width];
	  GREEN_ARRAY[i] = new int[pixel_img->width];
  } // for

   // ************************************************* 


  // QueryFrame = GrabFrame + RetrieveFrame !!!!!!!!!!!!!!!
  int detect_per_how_many_frame = 0;
  while ( CAPTURE.grab() ) {
    std::cout <<"�o�O��" << CAPTURE.get( CV_CAP_PROP_POS_FRAMES) << "�iframe" << endl;
	//if ( CAPTURE.get( CV_CAP_PROP_POS_FRAMES) == 9 ) system("pause");

    CAPTURE.retrieve( frame );

	temp_frame = frame.clone() ;  // �ƻs�@���v��
    cv::resize(frame,frame,cv::Size(640,480));
    cv::resize(temp_frame,temp_frame,cv::Size(640,480));

	pixel_img = &IplImage(frame) ;


	Cvt1dto2d(pixel_img,RED_ARRAY,GREEN_ARRAY,BLUE_ARRAY); // ��v�����Ȥ��O���BGR�� Array ��

    double tt1 = (double)cvGetTickCount();	
	/*
	IplImage * R_img = cvCreateImage(cvSize(640,480),8,1);
	
	int k = 0 ;
	for ( int i =0 ;i<pixel_img->height;i++ ) {
		for( int j=0;j<pixel_img->width ;j++) {	
			double RR = (double)(uchar)RED_ARRAY[i][j] ;	
			double BB = (double)(uchar)BLUE_ARRAY[i][j] ;	
			double GG = (double)(uchar)GREEN_ARRAY[i][j] ;
			RR = max(RR-BB-GG-fabs(BB-GG),0.0);
		    double t = pow(RR/255.0,0.2)*1020.0 ;
			R_img->imageData[k] = (uchar)min(t,255.0) ;
			k++ ;
		} // for
	} // for
	
	cvNamedWindow("pixel_img pic",1);
	cvShowImage( "pixel_img pic", pixel_img);	
	cvNamedWindow("Red pic",1);
	cvShowImage( "Red pic", R_img);
	cvWaitKey(27) ;

	cvReleaseImage( &R_img );	
	*/
    tt1 = (double)cvGetTickCount() - tt1;	
    //printf( "Extraction time1 = %gms\n", tt1/(cvGetTickFrequency()*1000.));

	
    ROI_img = cv::Mat( frame, FRAME_ROI_RECT );  // ���oROI
    cv::cvtColor(ROI_img, ROI_img, CV_BGR2GRAY); // ��Ƕ���
	black_white_img = ROI_img.clone();  // �ƻs�@���v��

	//cv::Canny (ROI_img,ROI_img,125,350);  
    laneMarkingDetector( ROI_img , black_white_img , 45 ) ;   // �j�v����Ƕ���k �����o�i
    cv::threshold(black_white_img, black_white_img, 100, 255, CV_THRESH_OTSU);  


	

	// �ন�G�ȤƼv�� �N�G�׭Ȧb100�H�W���]��255  
	if (0) {
		/*�����T��*/
		if ( detect_per_how_many_frame == 1 ) { //  �C�X�iframe�����@��???
		  //if ( control < 650 ) {
			  if ( Environment_is_Outdoor() ) { // ���P�_�O��~���٬O�G�D�������� 
				cv::putText( frame, "OUTDOOR", cv::Point(10,30), CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				RunDetectionLoop( temp_frame ,FRAME_ROI_RECT, found,1 ) ; // ���������l�i�H�o�쨮�l���Ъ����X(found)
				Is_TUNNEL = false ;
			  } // if
			  else {
				cv::putText( frame, "TUNNEL", cv::Point(10,30), CV_FONT_HERSHEY_COMPLEX , 0.7, YELLOW, 2, 8, false );
				RunDetectionLoop( temp_frame ,FRAME_ROI_RECT, found,2 ) ; // ���������l�i�H�o�쨮�l���Ъ����X(found)
				Is_TUNNEL = true ;
			  } // else
		//  } // if

	      //SelectingAndDrowingObject( cv::Mat(frame,FRAME_ROI_RECT), found );  // �e�X�خ�


#if TRACKING
		  /* ���������G�@�Ӥ@�ӥh��l�ܪ��e����*/

		//if ( control < 650 ) {
		Label_Tracking( temp_frame,frame,found ) ; // �����n�l�ܪ����l���L�l�ܾ�
	    //control++ ;
	   // } // if
		//else if ( control == 650 ) {
		//   control++ ;
		//	  system("pause");
		//} // else if
	   // else{ ;}


  double slope_left = Slope( left_lane_top_point.x, left_lane_top_point.y, left_lane_bot_point.x, left_lane_bot_point.y );
  double offset_left = left_lane_bot_point.y - (slope_left*left_lane_bot_point.x) ;
  double slope_right = Slope(right_lane_top_point.x,right_lane_top_point.y,right_lane_bot_point.x,right_lane_bot_point.y );
  double offset_right = right_lane_bot_point.y - (slope_right*right_lane_bot_point.x) ;




		if ( particle_1.Condens_work == true ) {
			CvPoint predict_pt1=cvPoint((int)condens_1->State[0],(int)condens_1->State[1]);

			particle_1.temp_predict_pt.x = predict_pt1.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_1.temp_predict_pt.y = predict_pt1.y;
			particle_1.pre_tracking_position.x = predict_pt1.x;
			particle_1.pre_tracking_position.y = predict_pt1.y;

			int radius_draw = particle_1.Condens_size.width/2;  // �e�l�ܶꪺ�b�|

			for (int i=0;i<condens_1->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_1->flSamples[i][0],condens_1->flSamples[i][1]);
				//cv::circle(frame,a,1,YELLOW,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_1.Condens_size.width, h = particle_1.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_1->flSamples[i][0]-(w/2) > 0 && condens_1->flSamples[i][1]-(h/2) > 0 && 
					 condens_1->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_1->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_1->flSamples[i][0],condens_1->flSamples[i][1]);
					if ( particle_1.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt1.x = particle_1.detect_result_pt.x ;
						predict_pt1.y = particle_1.detect_result_pt.y ;
						condens_1->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt1,1) ;				
						particle_1.If_Draw_Result = false ;
						} // if
					else if (particle_1.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_1.Create_Condens = false ;		
						particle_1.If_Draw_Result = false ;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_1->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt1,1) ;	
						particle_1.If_Draw_Result = true ;								
						} // else
				} // if
			}  // for
			particle_1.detecting_tracking_overlapping = false ;
			//4.update condensation  

			cvConDensUpdateByTime(condens_1); 
			//cv::circle(frame,predict_pt1,5,particle_1.not_car_color,3);
			if ( particle_1.If_Draw_Result == true ) {

				if ( predict_pt1.y-(slope_left*predict_pt1.x)-offset_left < 0 ) {
				  cv::putText( frame, "1", predict_pt1, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt1,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt1.y-(slope_right*predict_pt1.x)-offset_right < 0) {
				  cv::putText( frame, "1", predict_pt1, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt1,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "1", predict_pt1, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt1,radius_draw,ORANGE,3);//predicted point with green  
				} // else
				/*
			  cv::putText( frame, "1", predict_pt1, CV_FONT_HERSHEY_COMPLEX , 0.7, DARK_RED, 2, 8, false );
			  cv::circle(frame,predict_pt1,radius_draw,DARK_RED,3);//predicted point with green  
			  */

				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt1.x-radius_draw > 0 && predict_pt1.y-radius_draw > 0
					&& predict_pt1.x+radius_draw < 640 && predict_pt1.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt1.x-radius_draw,predict_pt1.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

							cv::imwrite( str, retraining);
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if
			} // if
		} // if
		if ( particle_2.Condens_work == true ) {
			CvPoint predict_pt2=cvPoint((int)condens_2->State[0],(int)condens_2->State[1]);

			particle_2.temp_predict_pt.x = predict_pt2.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_2.temp_predict_pt.y = predict_pt2.y;
			particle_2.pre_tracking_position.x = predict_pt2.x;
			particle_2.pre_tracking_position.y = predict_pt2.y;

			int radius_draw = particle_2.Condens_size.width/2;  // �e�l�ܶꪺ�b�|


			for (int i=0;i<condens_2->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_2->flSamples[i][0],condens_2->flSamples[i][1]);
				//cv::circle(frame,a,1,RED,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_2.Condens_size.width, h = particle_2.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_2->flSamples[i][0]-(w/2) > 0 && condens_2->flSamples[i][1]-(h/2) > 0 && 
					 condens_2->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_2->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_2->flSamples[i][0],condens_2->flSamples[i][1]);
					if ( particle_2.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt2.x = particle_2.detect_result_pt.x ;
						predict_pt2.y = particle_2.detect_result_pt.y ;
						condens_2->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt2,2) ;
						particle_2.If_Draw_Result = false ;
					} // if
					else if (particle_2.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_2.Create_Condens = false ;
						particle_2.If_Draw_Result = false ;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_2->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt2,2) ;
						particle_2.If_Draw_Result = true ;
						} // else
				} // if
			}  // for

			particle_2.detecting_tracking_overlapping = false ;
			//4.update condensation  

			cvConDensUpdateByTime(condens_2); 
			//cv::circle(frame,predict_pt6,5,particle_6.not_car_color,3);
		if ( particle_2.If_Draw_Result == true ) {

				if ( predict_pt2.y-(slope_left*predict_pt2.x)-offset_left < 0 ) {
				  cv::putText( frame, "2", predict_pt2, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt2,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt2.y-(slope_right*predict_pt2.x)-offset_right < 0) {
				  cv::putText( frame, "2", predict_pt2, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt2,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "2", predict_pt2, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt2,radius_draw,ORANGE,3);//predicted point with green  
				} // else


		    /*cv::putText( frame, "2", predict_pt2, CV_FONT_HERSHEY_COMPLEX , 0.7, DARK_GREEN, 2, 8, false );
			cv::circle(frame,predict_pt2,radius_draw,DARK_GREEN,3);//predicted point with green  
			*/
				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt2.x-radius_draw > 0 && predict_pt2.y-radius_draw > 0
					&& predict_pt2.x+radius_draw < 640 && predict_pt2.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt2.x-radius_draw,predict_pt2.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

	
							cv::imwrite( str, retraining );
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if

		} // if


		} // if
		if ( particle_3.Condens_work == true ) {
			CvPoint predict_pt3=cvPoint((int)condens_3->State[0],(int)condens_3->State[1]);

			particle_3.temp_predict_pt.x = predict_pt3.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_3.temp_predict_pt.y = predict_pt3.y;
			particle_3.pre_tracking_position.x = predict_pt3.x;
			particle_3.pre_tracking_position.y = predict_pt3.y;

			int radius_draw = particle_3.Condens_size.width/2;  // �e�l�ܶꪺ�b�|


			for (int i=0;i<condens_3->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_3->flSamples[i][0],condens_3->flSamples[i][1]);
				//cv::circle(frame,a,1,RED,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_3.Condens_size.width, h = particle_3.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_3->flSamples[i][0]-(w/2) > 0 && condens_3->flSamples[i][1]-(h/2) > 0 && 
					 condens_3->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_3->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_3->flSamples[i][0],condens_3->flSamples[i][1]);
					if ( particle_3.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt3.x = particle_3.detect_result_pt.x ;
						predict_pt3.y = particle_3.detect_result_pt.y ;
						condens_3->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt3,3) ;
						particle_3.If_Draw_Result = false ;
						} // if
					else if (particle_3.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_3.Create_Condens = false ;
						particle_3.If_Draw_Result = false ;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_3->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt3,3) ;
						particle_3.If_Draw_Result = true ;
						} // else
				} // if
			}  // for
			particle_3.detecting_tracking_overlapping = false ;

			//4.update condensation  

			cvConDensUpdateByTime(condens_3); 
			//cv::circle(frame,predict_pt3,5,particle_3.not_car_color,3);
			if (particle_3.If_Draw_Result == true) {

				if ( predict_pt3.y-(slope_left*predict_pt3.x)-offset_left < 0 ) {
				  cv::putText( frame, "3", predict_pt3, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt3,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt3.y-(slope_right*predict_pt3.x)-offset_right < 0) {
				  cv::putText( frame, "3", predict_pt3, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt3,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "3", predict_pt3, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt3,radius_draw,ORANGE,3);//predicted point with green  
				} // else

				/*cv::putText( frame, "3", predict_pt3, CV_FONT_HERSHEY_COMPLEX , 0.7, DARK_BLUE, 2, 8, false );
				cv::circle(frame,predict_pt3,radius_draw,DARK_BLUE,3);//predicted point with green*/
			
				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt3.x-radius_draw > 0 && predict_pt3.y-radius_draw > 0
					&& predict_pt3.x+radius_draw < 640 && predict_pt3.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt3.x-radius_draw,predict_pt3.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

	
							cv::imwrite( str, retraining );
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if			
			
			
			} // if



		} // if
		if ( particle_4.Condens_work == true ) {
			CvPoint predict_pt4=cvPoint((int)condens_4->State[0],(int)condens_4->State[1]);

			particle_4.temp_predict_pt.x = predict_pt4.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_4.temp_predict_pt.y = predict_pt4.y;
			particle_4.pre_tracking_position.x = predict_pt4.x;
			particle_4.pre_tracking_position.y = predict_pt4.y;


			int radius_draw = particle_4.Condens_size.width/2;  // �e�l�ܶꪺ�b�|


			for (int i=0;i<condens_4->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_4->flSamples[i][0],condens_4->flSamples[i][1]);
				//cv::circle(frame,a,1,GREEN,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_4.Condens_size.width, h = particle_4.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_4->flSamples[i][0]-(w/2) > 0 && condens_4->flSamples[i][1]-(h/2) > 0 && 
					 condens_4->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_4->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_4->flSamples[i][0],condens_4->flSamples[i][1]);
					if ( particle_4.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt4.x = particle_4.detect_result_pt.x ;
						predict_pt4.y = particle_4.detect_result_pt.y ;
						condens_4->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt4,4) ;
						particle_4.If_Draw_Result = false ;
					} // if
					else if (particle_4.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_4.Create_Condens = false ;
						particle_4.If_Draw_Result = false ;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_4->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt4,4) ;
						particle_4.If_Draw_Result = true ;
						} // else
				} // if
			}  // for
			
			particle_4.detecting_tracking_overlapping = false ;
			//4.update condensation  

			cvConDensUpdateByTime(condens_4); 
			//cv::circle(frame,predict_pt4,5,particle_4.not_car_color,3);
			if ( particle_4.If_Draw_Result == true  ) {
				if ( predict_pt4.y-(slope_left*predict_pt4.x)-offset_left < 0 ) {
				  cv::putText( frame, "4", predict_pt4, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt4,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt4.y-(slope_right*predict_pt4.x)-offset_right < 0) {
				  cv::putText( frame, "4", predict_pt4, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt4,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "4", predict_pt4, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt4,radius_draw,ORANGE,3);//predicted point with green  
				} // else


				/*cv::putText( frame, "4", predict_pt4, CV_FONT_HERSHEY_COMPLEX , 0.7, YELLOW, 2, 8, false );
				cv::circle(frame,predict_pt4,radius_draw,YELLOW,3);//predicted point with green
			*/
				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt4.x-radius_draw > 0 && predict_pt4.y-radius_draw > 0
					&& predict_pt4.x+radius_draw < 640 && predict_pt4.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt4.x-radius_draw,predict_pt4.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

	
							cv::imwrite( str, retraining );
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if			
						
			
			
			} // if


		} // if
		if ( particle_5.Condens_work == true ) {
			CvPoint predict_pt5=cvPoint((int)condens_5->State[0],(int)condens_5->State[1]);

			particle_5.temp_predict_pt.x = predict_pt5.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_5.temp_predict_pt.y = predict_pt5.y;
			particle_5.pre_tracking_position.x = predict_pt5.x;
			particle_5.pre_tracking_position.y = predict_pt5.y;

			int radius_draw = particle_5.Condens_size.width/2;  // �e�l�ܶꪺ�b�|


			for (int i=0;i<condens_5->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_5->flSamples[i][0],condens_5->flSamples[i][1]);
				//cv::circle(frame,a,1,WHITE,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_5.Condens_size.width, h = particle_5.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_5->flSamples[i][0]-(w/2) > 0 && condens_5->flSamples[i][1]-(h/2) > 0 && 
					 condens_5->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_5->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_5->flSamples[i][0],condens_5->flSamples[i][1]);
					if ( particle_5.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt5.x = particle_5.detect_result_pt.x ;
						predict_pt5.y = particle_5.detect_result_pt.y ;
						condens_5->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt5,5) ;
						particle_5.If_Draw_Result = false;
						} // if
					else if (particle_5.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_5.Create_Condens = false ;
						particle_5.If_Draw_Result = false;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_5->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt5,5) ;
						particle_5.If_Draw_Result = true;
						} // else
				} // if
			}  // for

		    particle_5.detecting_tracking_overlapping = false ;
     
			//4.update condensation  

			cvConDensUpdateByTime(condens_5); 
			//cv::circle(frame,predict_pt5,5,particle_5.not_car_color,3);
			if ( particle_5.If_Draw_Result == true ) { 

				if ( predict_pt5.y-(slope_left*predict_pt5.x)-offset_left < 0 ) {
				  cv::putText( frame, "5", predict_pt5, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt5,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt5.y-(slope_right*predict_pt5.x)-offset_right < 0) {
				  cv::putText( frame, "5", predict_pt5, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt5,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "5", predict_pt5, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt5,radius_draw,ORANGE,3);//predicted point with green  
				} // else



				/*
				cv::putText( frame, "5", predict_pt5, CV_FONT_HERSHEY_COMPLEX , 0.7, PURPLE, 2, 8, false );
				cv::circle(frame,predict_pt5,radius_draw,PURPLE,3);//predicted point with green 
		*/
				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt5.x-radius_draw > 0 && predict_pt5.y-radius_draw > 0
					&& predict_pt5.x+radius_draw < 640 && predict_pt5.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt5.x-radius_draw,predict_pt5.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

	
							cv::imwrite( str, retraining );
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if			
			
			
			} // if


		} // if
		if ( particle_6.Condens_work == true ) {
			CvPoint predict_pt6=cvPoint((int)condens_6->State[0],(int)condens_6->State[1]);

			particle_6.temp_predict_pt.x = predict_pt6.x; // �Ȧs  ��������O�_���������خظ�L����
			particle_6.temp_predict_pt.y = predict_pt6.y;
			particle_6.pre_tracking_position.x = predict_pt6.x;
			particle_6.pre_tracking_position.y = predict_pt6.y;

			int radius_draw = particle_6.Condens_size.width/2;  // �e�l�ܶꪺ�b�|


			for (int i=0;i<condens_6->SamplesNum;i++) {  
				// �eparticle
				
				CvPoint a = cvPoint(condens_6->flSamples[i][0],condens_6->flSamples[i][1]);
				//cv::circle(frame,a,1,DARK_BLUE,1);
				
				// �l�ܮت����j�p�O�ھڰ����خئӨӪ�

				int w = particle_6.Condens_size.width, h = particle_6.Condens_size.height ;
				// �إX�C��particle �n�h�򰻴��خؤ��hist����
				
				if ( condens_6->flSamples[i][0]-(w/2) > 0 && condens_6->flSamples[i][1]-(h/2) > 0 && 
					 condens_6->flSamples[i][0]+(h/2) < VIDEO_WIDTH && condens_6->flSamples[i][1]+(h/2)<VIDEO_HEIGHT) {
				
					CvPoint temp_sample=cvPoint(condens_6->flSamples[i][0],condens_6->flSamples[i][1]);
					if ( particle_6.detecting_tracking_overlapping == true ) {  
					//   �p�G���ڰ������خضZ���ܱ���   �N�ΰ������خت������I	
						predict_pt6.x = particle_6.detect_result_pt.x ;
						predict_pt6.y = particle_6.detect_result_pt.y ;
						condens_6->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt6,6) ;
						particle_6.If_Draw_Result = false ;
						} // if
					else if (particle_6.Create_Condens == true) {  // �p�G�O��Ыئn���s���l�ܮ� �N�ΰ������G			
						particle_6.Create_Condens = false ;
						particle_6.If_Draw_Result = false ;
					} // else if
					else {
						// �p�G�O�S���W�z�ⶵ  �N�ιw�������G��@������I
						condens_6->flConfidence[i] = Point_Compare_Vehicle_Color(temp_sample,predict_pt6,6) ;
						particle_6.If_Draw_Result = true ;
					} // else
				} // if
			}  // for

			particle_6.detecting_tracking_overlapping = false ;
			//4.update condensation  

			cvConDensUpdateByTime(condens_6); 
			//cv::circle(frame,predict_pt6,5,particle_6.not_car_color,3);
			if ( particle_6.If_Draw_Result == true ) {

				if ( predict_pt6.y-(slope_left*predict_pt6.x)-offset_left < 0 ) {
				  cv::putText( frame, "6", predict_pt6, CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
				  cv::circle(frame,predict_pt6,radius_draw,BLUE,3);//predicted point with green  
				} // if
				else if(predict_pt6.y-(slope_right*predict_pt6.x)-offset_right < 0) {
				  cv::putText( frame, "6", predict_pt6, CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
				  cv::circle(frame,predict_pt6,radius_draw,RED,3);//predicted point with green  
				} // else if
				else {
				  cv::putText( frame, "6", predict_pt6, CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
				  cv::circle(frame,predict_pt6,radius_draw,ORANGE,3);//predicted point with green  
				} // else



				/*
				cv::putText( frame, "6", predict_pt6, CV_FONT_HERSHEY_COMPLEX , 0.7, WHITE, 2, 8, false );
				cv::circle(frame,predict_pt6,radius_draw,WHITE,3);//predicted point with green  
			*/
				cv::Mat sample ;
				frame.copyTo(sample) ;
	      
				if ( predict_pt6.x-radius_draw > 0 && predict_pt6.y-radius_draw > 0
					&& predict_pt6.x+radius_draw < 640 && predict_pt6.y+radius_draw < 480) {
				  cv::Rect r = Rect( predict_pt6.x-radius_draw,predict_pt6.y-radius_draw,2*radius_draw,2*radius_draw);	  
				  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

				  bool OutOfRange = true ;
				  if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
				    OutOfRange = false ;

				/*�g�Xtracking ���G���Ϥ�*/
				  
				  if ( Write_Tracking_Result_image == true ) {
						if( OutOfRange == false ) {
							cv::Mat retraining = cv::Mat( sample, r ) ;
							cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
							char str[200];
							if ( Is_TUNNEL == false ) 
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR,outputTracking_Index );
							else if ( Is_TUNNEL == true )
							  sprintf( str, "%s_%d.JPEG", Tracking_ADDR_TUNNEL,outputTracking_Index );

	
							cv::imwrite( str, retraining );
							outputTracking_Index++;
							retraining.release();
						} // if
				  } // if
				  
				sample.release();
			  } // if				
			
			
			
			} // if


		} // if

#endif

          SelectingAndDrowingObject( cv::Mat(frame,FRAME_ROI_RECT), found );  // �e�X�����خ�

		  detect_per_how_many_frame = 0 ;
		} // if
		detect_per_how_many_frame++;

	} // if

	if (1) {  // �n���n�}�Ҩ��D����??????
		/* ���D */   
		LineFinder finder ;
		finder.setMinVote (50);  
		finder.setLineLengthAndGap (50,30);  
	
		Find_HoughLines_and_VanishingPoint( black_white_img, frame, pre_RT, pre_RB, pre_LT, pre_LB);
	
		finder.findLines (black_white_img);    
		finder.drawDetectedLines (frame);  
		/* ���D */  
	} // if


# if WRITEVIDEO

	writer << frame ; // �g�X�v��
	/*IplImage * result = &IplImage( frame );
	cvWriteFrame(writer,result);*/
#endif
	/*
	cv::line( frame, Point(0,340), Point(640,340), WHITE ,3,8,0);

	cv::line( frame, Point(304,154),Point(336,154), WHITE,3,8,0);
	cv::line( frame, Point(336,154),Point(336,186), WHITE,3,8,0);
	cv::line( frame, Point(304,186),Point(336,186), WHITE,3,8,0);
	cv::line( frame, Point(304,154),Point(304,186), WHITE,3,8,0);
	*/
# if WRITEFRAME
	char str[500];
	sprintf( str, "%s_%d.jpeg", Output_frame,frame_num );
	cv::imwrite( str, frame );
	frame_num++ ;
# endif

	imshow("�o�i�ᵲ�G��", black_white_img);
	imshow("���G��", frame);
    imshow("�G�ȤƼv��",ROI_img) ;

    if( waitKey (30) >= 0) break;

	black_white_img.release();
	frame.release();
	ROI_img.release();
	temp_frame.release();
  } // while

  std::cout << "DONE!!!";
  system("pause");

  

  frame.release();


  free(BLUE_ARRAY);
  free(GREEN_ARRAY);
  free(RED_ARRAY);

  return 0;
} // main()
//*******************************************************************
bool LoadTrainingDetect( std::vector<float>&desc, char* filename, int DIM ) {
    FILE*file = fopen( filename, "r" );
    if ( !file )
        return false;
    float input = 0.0;
    for( int i = 0 ; i < DIM+1; i++ ) {
        desc.push_back( 0.0 ) ;
        fscanf( file, "%f", &desc[i] );
    } // for
    return true;
} // LoadTrainingDetec()
//*******************************************************************
void AddInQueue(int data, int data1) { 

	// �p�G�񺡤F �q�Ĥ@�Ӷ}�l�̦��洫
	if ( index_in_queue >= NUM ) index_in_queue = 0; // ���^�Y

	vanishing_point_queue_x[index_in_queue] = data ;
	vanishing_point_queue_y[index_in_queue] = data1 ;
	index_in_queue++;

	if ( sizeof_vanishing_point_queue < NUM ) sizeof_vanishing_point_queue++;

} // AddInQueue()
//*******************************************************************
void Find_HoughLines_and_VanishingPoint( cv::Mat image, cv::Mat frame,
	                                     Point &pre_RT, Point &pre_RB, Point &pre_LT, Point &pre_LB) {

    Point Vanish_Right_Line_Top, Vanish_Right_Line_Bot , Vanish_Left_Line_Top , Vanish_Left_Line_Bot ;
    std::vector<cv::Vec2f> lines; 
    //�N���ܴ��A��o�@�դήy�аѼ�(RHO,theta) �C�@������@�����u�A�O�s��lines 
    //�a3.4�ӰѼƪ�ܦb(rho,theta) �y�Шt�����a�y�Ъ��̤p���A�Y�B��
    HoughLines(image, lines,1,CV_PI/180,60);
	  bool find_left = false, find_right = false ;
     for( size_t i =0 ; i < lines.size(); i++ ) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta) ;
        double x0 = a*rho, y0 = b*rho;

        Point pt1(cvRound(x0+1000*(-b)), cvRound(y0+1000*(a))+150);
        Point pt2(cvRound(x0-1000*(-b)), cvRound(y0-1000*(a))+150);

        cv::clipLine(frame.size(),pt1,pt2) ;

		//line(frame,pt1,pt2,YELLOW,1,8);
		    if ( find_right == true && find_left == true ) 
              break ;
		    else if ( abs(Slope( pt1.x, pt1.y , pt2.x , pt2.y )) < 0.5 )
			    ;
		    else if ( Slope( pt1.x, pt1.y, pt2.x , pt2.y ) > 0.5 && Slope( pt1.x, pt1.y, pt2.x , pt2.y )< 2) {
			    find_right = true ;
			    Vanish_Right_Line_Top.x = pt1.x;   // �M�wvanishing point���k�u
			    Vanish_Right_Line_Top.y = pt1.y;
			    Vanish_Right_Line_Bot.x = pt2.x;
			    Vanish_Right_Line_Bot.y = pt2.y;
				//line(frame,pt1,pt2,YELLOW,1,8);
		    } // else if
		    else if ( Slope( pt1.x, pt1.y , pt2.x , pt2.y ) < -0.5 && Slope( pt1.x, pt1.y , pt2.x , pt2.y )>-2) {
			    find_left = true ;
			    Vanish_Left_Line_Top.x = pt1.x;  // �M�wvanishing point �����u
			    Vanish_Left_Line_Top.y = pt1.y;
			    Vanish_Left_Line_Bot.x = pt2.x;
			    Vanish_Left_Line_Bot.y = pt2.y;
				//line(frame,pt1,pt2,YELLOW,1,8);
		    } // else if

        
    } // for

	if ( find_right == true && find_left == true ) { // �Y�O�����vanishing line �O���U�ӳo�@��
		pre_RT.x = Vanish_Right_Line_Top.x;
		pre_RT.y = Vanish_Right_Line_Top.y;
		pre_RB.x = Vanish_Right_Line_Bot.x;
		pre_RB.y = Vanish_Right_Line_Bot.y;
		pre_LT.x = Vanish_Left_Line_Top.x;
		pre_LT.y = Vanish_Left_Line_Top.y;
		pre_LB.x = Vanish_Left_Line_Bot.x;
		pre_LB.y = Vanish_Left_Line_Bot.y;		
	} // if
	else {                  // �Y�O�S�������󤺪�vanishing line �Τ��e������
		Vanish_Right_Line_Top.x = pre_RT.x;
		Vanish_Right_Line_Top.y = pre_RT.y;
		Vanish_Right_Line_Bot.x = pre_RB.x;
		Vanish_Right_Line_Bot.y = pre_RB.y;
		Vanish_Left_Line_Top.x = pre_LT.x;
		Vanish_Left_Line_Top.y = pre_LT.y;
		Vanish_Left_Line_Bot.x = pre_LB.x;
		Vanish_Left_Line_Bot.y = pre_LB.y;
	} // else 

	// �o��}�l�� vanishinf point
	Point Intersection = Find_Intersection_Point( Vanish_Left_Line_Top,Vanish_Left_Line_Bot,Vanish_Right_Line_Top,Vanish_Right_Line_Bot );
     // �������I

    VP = Find_Vanishing_Point(Intersection) ; // ��n�D��vanishing point 
  
	//cv::line( frame, Vanish_Left_Line_Top, Vanish_Left_Line_Bot, PURPLE,2,8,0);
	//cv::line( frame, Vanish_Right_Line_Top, Vanish_Right_Line_Bot, PURPLE,2,8,0);
  
	Point a,b,c,d,e,f,g,h ;
	a.x = VP.x-10 ;
	a.y = VP.y;
	b.x = VP.x+10 ;
	b.y = VP.y;
	c.x = VP.x;
	c.y = VP.y-10;
	d.x = VP.x;
	d.y = VP.y+10;

	e.x = VP.x-10 ;
	e.y = VP.y;
	f.x = VP.x+10 ;
	f.y = VP.y;
	g.x = VP.x;
	g.y = VP.y-10;
	h.x = VP.x;
	h.y = VP.y+10;
  
  
	cv::line( frame, a, b, GREEN ,3,8,0);
	cv::line( frame, c, d, GREEN ,3,8,0);
	cv::line( frame, e, f, GREEN ,3,8,0);
	cv::line( frame, g, h, GREEN ,3,8,0);
  

}// Find_HoughLines_and_VanishingPoint()
//*******************************************************************
Point Find_Intersection_Point( Point LT, Point LB, Point RT, Point RB ) {  // y = mx+b

	Point ans ; // �n�D�����I  vanishing point
	double Slope_Left = Slope( LT.x, LT.y, LB.x, LB.y );  // �D�ײv
	double Slope_Right = Slope( RT.x, RT.y, RB.x, RB.y );

	double offset_Left = LB.y - (Slope_Left*LB.x);  // b = y-mx
	double offset_Right = RB.y - (Slope_Right*RB.x);

	ans.x =(offset_Right-offset_Left)/(Slope_Left-Slope_Right);
	                        // m1*x + b1 = m2*x + b2    =>  x = (b2 - b1)/(m1 - m2)
	ans.y = (Slope_Right*ans.x) + offset_Right ;

	return ans ;
} // Find_Intersection_Point()
//*******************************************************************
void Extand_line( Point pt1,Point pt2,Point &ans_pt1, Point &ans_pt2 ) {
  int x ;
  double slope = Slope( pt1.x, pt1.y, pt2.x, pt2.y );
  double offset = pt2.y - (slope*pt2.x) ;
  /*�V�W������VP*/
  ans_pt1.x = (VP.y-offset)/slope ;
  ans_pt1.y = VP.y;
  /*�V�W������VP*/
  /*�V�U������e���j�p*/
  if ( slope < 0 ) {  // left
    x = (480-offset)/slope;
    if ( x >=0 && x <= 640 ) {
      ans_pt2.x = x;
      ans_pt2.y = 480 ;
    } // if
    else {
      ans_pt2.x = 0 ;
      ans_pt2.y = (slope*0)+offset;
    } // else
  } // if
  else if ( slope > 0 ) {  // right
    x = (480-offset)/slope;
    if ( x >=0 && x <= 640 ) {
      ans_pt2.x = x;
      ans_pt2.y = 480 ;
    } // if
    else {
      ans_pt2.x = 640 ;
      ans_pt2.y = (slope*640)+offset;
    } // else
  } // else if
  /*�V�U������e���j�p*/

} // Extand_line()
//*******************************************************************
double Distance_of_two_points( Point pt1, Point pt2 ) {
  double a = pt1.x-pt2.x;
  double b = pt1.y-pt2.y;
  double c = pow(a,2) + pow(b,2);
  c = pow(c,0.5) ;
  return c ;
} // double Distance_of_two_points()
//*******************************************************************
double Slope( int x1, int y1, int x2 , int y2 ) {
  return (double)(y2-y1)/(double)(x2-x1) ; 
} // Slope
//*******************************************************************
void laneMarkingDetector( cv::Mat &srcGRAY , cv::Mat &dstGRAY , int tau ) {
  dstGRAY.setTo(0);

  int aux = 0;
  for ( int j=0; j < srcGRAY.rows ; ++j ) {
    unsigned char *ptRowSrc = srcGRAY.ptr<uchar>(j);
	  unsigned char *ptRowDst = dstGRAY.ptr<uchar>(j);

	  for ( int i=tau ; i < srcGRAY.cols-tau ; ++i ) {
        if( ptRowSrc[i] != 0 ) {
          aux = 2*ptRowSrc[i];
          aux += -ptRowSrc[i-tau];
          aux += -ptRowSrc[i+tau];
          aux += -abs((int)(ptRowSrc[i-tau] - ptRowSrc[i+tau])) ;

          aux = (aux<0) ? (0):(aux);
          aux = (aux>255) ? (255):aux;

          ptRowDst[i] = (unsigned char)aux ;
        } // if
	  } // for
  } // for

} // laneMarkingDetector()
//*******************************************************************
void quick_sort(int *queue,int low,int high) {  
   int pivot_point,pivot_item,i,j,temp;  
   // ���Х�ɵ����Ƨ�  
  // cout << queue[high] ;
   if(high<=low){return ;}  
  
   // �����ϯí�  
    //  cout << queue[low] ;
   pivot_item = queue[low];  
   j=low;  
     
   // �M���ϯäp����  
   for(i=low+1; i<=high; i++) {  
       // ���L����Τj�󪺼�  
       if(queue[i]>=pivot_item){continue;}  
  
       j++;  
       // �洫 array[i] , array[j]  
       temp = queue[i];  
       queue[i] = queue[j];  
       queue[j] = temp;  
   }  
  
   // �N�ϯæ�}���줤��  
   pivot_point=j;  
   // �洫 array[low] , array[pivot_point]  
   temp = queue[low];  
   queue[low] = queue[pivot_point];  
   queue[pivot_point] = temp;  
   // ���j�B�z�����Ϭq  
   quick_sort(queue,low,pivot_point-1);  
   // ���j�B�z�k���Ϭq  
   quick_sort(queue,pivot_point+1,high);  
  
} // quick_sort()

//*******************************************************************
Point Find_Vanishing_Point( Point ans ) {  
	 // �n�D�����I  vanishing point
  if (ans.y >= 200 && ans.y <= 280 ) // �Y�O���I����m ���b���d�򤺴N���n�a�iqueue��
	  AddInQueue( ans.x, ans.y ); // �N�o���o�쪺vanishing point �s�i�h 

  int *array_for_x = new int[NUM]; // �ʺA�ŧi �Ȧs�Ϊ�queue
  int *array_for_y = new int[NUM];

  for( int i =0; i < NUM ; i++ ) {   // ���ƼȦs��ʺA�ŧi��array����quick sort  �쥻��queue��Ƥ���
    array_for_x[i] = vanishing_point_queue_x[i];
    array_for_y[i] = vanishing_point_queue_y[i];
  } // end for

  quick_sort( array_for_x,0,sizeof_vanishing_point_queue-1); // ��x��m���Ƨǫ�������
  quick_sort( array_for_y,0,sizeof_vanishing_point_queue-1); // ��y��m���Ƨǫ�������

  ans.x = array_for_x[sizeof_vanishing_point_queue/2];
  ans.y = array_for_y[sizeof_vanishing_point_queue/2];

  delete []array_for_x;
  delete []array_for_y;
  
  return ans ;
} // Find_Vanishing_Point()
//*******************************************************************
bool Environment_is_Outdoor(){  // �Y��e�����ҬO��~�^��true   �G�D���ܴN�Ofalse
	int *B = new int[3200];  // �e�����W�b�� ���������@������m
	int *G = new int[3200];
	int *R = new int[3200];

	int index = 0 ;
	
	for ( int i = 0 ; i < 100 ; i++ ) {
		for ( int j = 304 ; j < 336 ; j++ ) {
			B[index] = (int)(uchar)BLUE_ARRAY[i][j];
			G[index] = (int)(uchar)GREEN_ARRAY[i][j];
			R[index] = (int)(uchar)RED_ARRAY[i][j];
			index++;
		} // for
	} // for

	quick_sort( B,0,index-1);
	quick_sort( G,0,index-1);
	quick_sort( R,0,index-1);


	if ( (B[index/2]+G[index/2]+R[index/2])/3 > 100 ) {  //  �Y�O��~�����p  ����ƪ��G�פj��@�w����
		delete []B;
		delete []G;
		delete []R;
		return true ;
	} // if
	else {
		delete []B;
		delete []G;
		delete []R;
		return false ;
	} // else

} // Environment_is_Outdoor()
//*******************************************************************
void RunDetectionLoop(cv::Mat frame , Rect roi, std::vector<cv::Rect> &found, int type) {
	Mat test = cv::Mat(frame,roi);  //�ϥαm�Ϫ�ROI�h������
	cvtColor(test,test,CV_BGR2BGRA); // �ন4�q�D���v��  �]��GPU�B��u�䴩�|�q�D��

	if ( type == 1 )  // �Y�O��~�����p
	  GPU_MODE->detectMultiScale( cv::gpu::GpuMat::GpuMat( test ), found, 0, cv::Size( 8, 8 ), cv::Size( 0, 0 ), 1.05, 2 );
	  //HOG_PTR.detectMultiScale( image, found, 0, cv::Size( 8, 8 ), cv::Size( 0, 0 ), 1.05, 2 );
	else if ( type == 2 )  // �Y�O�G�D�����p
	   //GPU_MODE->detectMultiScale( cv::gpu::GpuMat::GpuMat( test ), found, 0, cv::Size( 8, 8 ), cv::Size( 0, 0 ), 1.05, 2 );
	   GPU_MODE_TUNNEL->detectMultiScale( cv::gpu::GpuMat::GpuMat( test ), found, 0, cv::Size( 8, 8 ), cv::Size( 0, 0 ), 1.05, 2 );
	else
		system("pause");

	for ( int i = 0 ; i < found.size() ; i++ ) {  // �����ب��l�ɤj�]�p�����p
		for ( int j = 0 ; j < found.size() ; j++ ) {
			if (j==i) ;   // ����ۤv���  
			else {  // ���L���خؤ��  �p�G���j�]�p�����p  �����p�خ�
				cv::Rect r = found[j];	  
	            int h1_j = r.tl().y, w1_j = r.tl().x, h2_j = r.br().y, w2_j = r.br().x;
				cv::Rect r1 = found[i];	 
	            int h1_i = r1.tl().y, w1_i = r1.tl().x, h2_i = r1.br().y, w2_i = r1.br().x;
				if ( h1_j > h1_i && h2_j < h2_i && w1_j > w1_i && w2_j < w2_i ) { // �Yj�bi�̭�  ����i�خ�
					found.erase (found.begin()+i) ;   // ��~�����خخ���
					i = 0 ;
					break ;
				} // if
			} // else
		} // for
	} // for  

	
} // RunDetectionLoop()
//*******************************************************************
void SelectingAndDrowingObject( cv::Mat frame, std::vector<cv::Rect> found ) {
	
  double slope_left = Slope( left_lane_top_point.x, left_lane_top_point.y, left_lane_bot_point.x, left_lane_bot_point.y );
  double offset_left = left_lane_bot_point.y - (slope_left*left_lane_bot_point.x) ;
  double slope_right = Slope(right_lane_top_point.x,right_lane_top_point.y,right_lane_bot_point.x,right_lane_bot_point.y );
  double offset_right = right_lane_bot_point.y - (slope_right*right_lane_bot_point.x) ;

	cv::Mat sample ;
	frame.copyTo(sample) ;
	

	for( int i = 0; i < found.size(); i++ ) {
	  cv::Rect r = found[i];	  
	  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

      bool OutOfRange = true ;
      if ( h1 <= VIDEO_HEIGHT && w1 <= VIDEO_WIDTH &&  h2 <= VIDEO_HEIGHT && w2 <= VIDEO_WIDTH ) 
        OutOfRange = false ;

	  h1 = ( h1 < VIDEO_HEIGHT ) ? h1:VIDEO_HEIGHT-1;
      h2 = ( h2 < VIDEO_HEIGHT ) ? h2:VIDEO_HEIGHT-1;
      w1 = ( w1 < VIDEO_WIDTH ) ? w1:VIDEO_WIDTH-1;
      w2 = ( w2 < VIDEO_WIDTH ) ? w2:VIDEO_WIDTH-1;

	  int mid_height = h2 + 150;  // ���خة��u�������I
      int mid_width = (w1+w2)/2 ;
	  
	  /*�g�Xretraining�Ϊ��Ϥ�*/
	  
	  if ( Write_Retraining_Image == true ) {
		  if( OutOfRange == false ) {
			cv::Mat retraining = cv::Mat( sample, r ) ;
			cv::resize( retraining, retraining, SAMPLE_SIZE, cv::INTER_LINEAR);
			char str[200];
			if ( Is_TUNNEL == false ) 
				sprintf( str, "%s_%d.JPEG", Output,outputSample_Index );
			else if ( Is_TUNNEL == true )
				sprintf( str, "%s_%d.JPEG", Output_TUNNEL,outputSample_Index );

	
			cv::imwrite( str, retraining );
			outputSample_Index++;
		  } // if
	  } // if
	  
	  /*�g�Xretraining�Ϊ��Ϥ�*/	
	// if (control <650) {
    if ( mid_height-(slope_left*mid_width)-offset_left < 0 ) {
      cv::rectangle( frame, r.tl(), r.br(), BLUE, 3 );  // �إ���T��
      //cv::putText( frame, "L", cv::Point( r.tl().x, r.tl().y-10 ), CV_FONT_HERSHEY_COMPLEX , 0.7, BLUE, 2, 8, false );
    } // if
    else if(mid_height-(slope_right*mid_width)-offset_right < 0) {
      cv::rectangle( frame, r.tl(), r.br(), RED, 3 );  // �إk��T��
      //cv::putText( frame, "R", cv::Point( r.br().x, r.tl().y-10 ), CV_FONT_HERSHEY_COMPLEX , 0.7, RED, 2, 8, false );
    } // else if
    else {
      cv::rectangle( frame, r.tl(), r.br(), ORANGE, 3 );  // �بT��
      //cv::putText( frame, "M", cv::Point( mid_width, r.tl().y-10 ), CV_FONT_HERSHEY_COMPLEX , 0.7, ORANGE, 2, 8, false );
    } // else
	} // for
	//} // if
    sample.release();
} // SelectingAndDrowingObject()
//*******************************************************************
void Label_Tracking( cv::Mat temp_frame, cv::Mat frame,std::vector<cv::Rect> found ) {  // ���l�ܪ��������s��

	/*�����R�� �ݦ��S���l�ܾ��w�g�l��N����ɤF*/

	if ( If_Point_Outofframe(particle_1.temp_predict_pt, particle_1.Condens_size ) == true 
		|| (particle_1.counter_end-particle_1.counter_start) > 120 ) // �Y�O���q�ɶ��S���Q�������л\��  �N����
		particle_1.Condens_work = false ;
	if ( If_Point_Outofframe(particle_2.temp_predict_pt, particle_2.Condens_size ) == true 
		|| (particle_2.counter_end-particle_2.counter_start) > 120 ) // �Y�O���q�ɶ��S���Q�������л\��  �N����) 
		particle_2.Condens_work = false ;
	if ( If_Point_Outofframe(particle_3.temp_predict_pt, particle_3.Condens_size ) == true 
		|| (particle_3.counter_end-particle_3.counter_start) > 120) 
		particle_3.Condens_work = false ;
	if ( If_Point_Outofframe(particle_4.temp_predict_pt, particle_4.Condens_size ) == true 
		|| (particle_4.counter_end-particle_4.counter_start) > 120) 
		particle_4.Condens_work = false ;
	if ( If_Point_Outofframe(particle_5.temp_predict_pt, particle_5.Condens_size ) == true 
		|| (particle_5.counter_end-particle_5.counter_start) > 120) 
		particle_5.Condens_work = false ;
    if ( If_Point_Outofframe(particle_6.temp_predict_pt, particle_6.Condens_size ) == true 
		|| (particle_6.counter_end-particle_6.counter_start) > 120) 
		particle_6.Condens_work = false ;
	
	/*�����R�� �ݦ��S���l�ܾ��w�g�l��N����ɤF*/

	/*�o��}�l�O�n�⦳���ۭ��|�쪺�l�ܮص��o�����@��   �n�o�����l�ܮت�size����p������
	  �]���q�`�O����j���l�ܮ����ڭ̦ۤv�������  �l�ܮؤ���p����ܬO�Q�B���쪺*/



	
	Delete_tracking_overlap_tracking(); // �R���h�l���l�ܮ�

	bool Condens_1_has_been_matched = false ;  // �b�@�iframe�̭�  �@�Ӱl�ܾ�  �u�|match��@�Ӱ�����
	bool Condens_2_has_been_matched = false ;
	bool Condens_3_has_been_matched = false ;
	bool Condens_4_has_been_matched = false ;
	bool Condens_5_has_been_matched = false ;
	bool Condens_6_has_been_matched = false ;


	for( int i = 0 ; i < found.size() ; i++ ) {  // *************************
	  bool create_new_tracking = true ; // �O�_�n�Ыطs���l�ܮ�(particle filter)
	  bool has_been_matched = false ;  // �T�{�o�Ӱ����ئ��S����l�ܮ�match�L�F
      cv::Rect r = found[i];	  
	  int h1 = r.tl().y, w1 = r.tl().x, h2 = r.br().y, w2 = r.br().x;

	  CvPoint mid_found = cvPoint((w1+w2)/2,(h1+h2)/2+150);  //���l�����خت������I�����I

	  if ( particle_1.Condens_work == true && has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_1.Condens_size ) == false 
			  && If_Point_Outofframe(particle_1.temp_predict_pt, particle_1.Condens_size ) == false
			  && CalculateDistance( mid_found, particle_1.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found,1 ) > template_matching_threshold
			  && Condens_1_has_been_matched == false ) {

			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_1, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_1.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_1.detect_result_pt.x = mid_found.x ;
			  particle_1.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_1.detect_result_pt,particle_1.Condens_size,frame,1);
			  particle_1.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_1_has_been_matched = true ;
		  } // if
	  } // if
	  if ( particle_2.Condens_work == true &&  has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_2.Condens_size ) == false 
			  && If_Point_Outofframe(particle_2.temp_predict_pt, particle_2.Condens_size ) == false 
			  && CalculateDistance( mid_found, particle_2.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found,2) > template_matching_threshold
			  && Condens_2_has_been_matched == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_2, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_2.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_2.detect_result_pt.x = mid_found.x ;
			  particle_2.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_2.detect_result_pt,particle_2.Condens_size,frame,2);
			  particle_2.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_2_has_been_matched = true ;
		  } // if
	  } //if 
	  if ( particle_3.Condens_work == true &&  has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_3.Condens_size ) == false 
			  && If_Point_Outofframe(particle_3.temp_predict_pt, particle_3.Condens_size ) == false 
			  && CalculateDistance( mid_found, particle_3.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found,3) > template_matching_threshold
			  && Condens_3_has_been_matched == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_3, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_3.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_3.detect_result_pt.x = mid_found.x ;
			  particle_3.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_3.detect_result_pt,particle_3.Condens_size,frame,3);
			  particle_3.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_3_has_been_matched = true ;
		  } // if
	  } //if 
	  if ( particle_4.Condens_work == true && has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_4.Condens_size ) == false 
			  && If_Point_Outofframe(particle_4.temp_predict_pt, particle_4.Condens_size ) == false 
			  && CalculateDistance( mid_found, particle_4.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found, 4 ) > template_matching_threshold
			  && Condens_4_has_been_matched == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_4, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_4.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_4.detect_result_pt.x = mid_found.x ;
			  particle_4.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_4.detect_result_pt,particle_4.Condens_size,frame,4);
			  particle_4.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_4_has_been_matched = true ;
		  } // if
	  } //if 
	  if ( particle_5.Condens_work == true &&  has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_5.Condens_size ) == false 
			  && If_Point_Outofframe(particle_5.temp_predict_pt, particle_5.Condens_size ) == false
			  && CalculateDistance( mid_found, particle_5.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found, 5 ) > template_matching_threshold
			  && Condens_5_has_been_matched == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_5, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_5.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_5.detect_result_pt.x = mid_found.x ;
		      particle_5.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_5.detect_result_pt,particle_5.Condens_size,frame,5);
			  particle_5.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_5_has_been_matched = true ;
		  } // if
	  } //if
	  if ( particle_6.Condens_work == true &&  has_been_matched == false ) {
		  if ( If_Point_Outofframe(mid_found, particle_6.Condens_size ) == false 
			  && If_Point_Outofframe(particle_6.temp_predict_pt, particle_6.Condens_size ) == false
			  && CalculateDistance( mid_found, particle_6.pre_tracking_position ) < spatial_distance_threshold
			  && detect_compare_tracking( mid_found, 6 ) > template_matching_threshold
			  && Condens_6_has_been_matched == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_6, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_6.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_6.detect_result_pt.x = mid_found.x ;
			  particle_6.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_6.detect_result_pt,particle_6.Condens_size,frame,6);
			  particle_6.detecting_tracking_overlapping = true ;
			  create_new_tracking = false ;
			  has_been_matched = true ;
			  Condens_6_has_been_matched = true ;
		  } // if
	  } //if

	  // ******************************************************************************
	  // ******************************************************************************
	  // ******************************************************************************
	  // �o��}�l�O�Ыطs��tracking

	  if ( create_new_tracking == true 
		  && If_tracking_rect_already_exist(mid_found,cvSize(w2-w1,h2-h1),frame) == false ) { 
		                     // �p�G�ݭn�إ߷s���l�ܾ�  �ݭ��@�Ӱl�ܾ��S�A��
		  if ( particle_1.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_1, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_1.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_1.detect_result_pt.x = mid_found.x ;
			  particle_1.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_1.detect_result_pt,particle_1.Condens_size,frame,1);
			  particle_1.Condens_work = true ;
			  particle_1.Create_Condens = true ;
			  create_new_tracking = false ;

		  } // if
		  else if( particle_2.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_2, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_2.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_2.detect_result_pt.x = mid_found.x ;
			  particle_2.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_2.detect_result_pt,particle_2.Condens_size,frame,2);
			  particle_2.Condens_work = true ;
			  particle_2.Create_Condens = true ;
			  create_new_tracking = false ;


		  } // else if
		  else if( particle_3.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_3, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_3.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_3.detect_result_pt.x = mid_found.x ;
			  particle_3.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_3.detect_result_pt,particle_3.Condens_size,frame,3);
			  particle_3.Condens_work = true ;
			  particle_3.Create_Condens = true ;
			  create_new_tracking = false ;


		  } // else if
		  else if( particle_4.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_4, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_4.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_4.detect_result_pt.x = mid_found.x ;
			  particle_4.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_4.detect_result_pt,particle_4.Condens_size,frame,4);
			  particle_4.Condens_work = true ;
			  particle_4.Create_Condens = true ;
			  create_new_tracking = false ;
		  } // else if
		  else if( particle_5.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_5, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_5.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_5.detect_result_pt.x = mid_found.x ;
			  particle_5.detect_result_pt.y = mid_found.y ;
              Calculate_car_color( particle_5.detect_result_pt,particle_5.Condens_size,frame,5);
			  particle_5.Condens_work = true ;
			  particle_5.Create_Condens = true ;
			  create_new_tracking = false ;

		  } // else if
		  else if( particle_6.Condens_work == false ) {
			  /*��s�x�I��m*/
			  cvmSet(lowerBound,0,0,w1 );   
		      cvmSet(upperBound,0,0,w2 );  
			  cvmSet(lowerBound,1,0,h1+150 );   
			  cvmSet(upperBound,1,0,h2+150 );  
			  cvmSet(lowerBound,2,0,0.0 );   
			  cvmSet(upperBound,2,0,0.0 );  
			  cvmSet(lowerBound,3,0,0.0 );   
			  cvmSet(upperBound,3,0,0.0 ); 
			  cvConDensInitSampleSet(condens_6, lowerBound, upperBound);  
			  /*��s�x�I��m*/

			  particle_6.Condens_size = cvSize( (w2-w1),(h2-h1) ); // �o�찻���خت��j�p
			  particle_6.detect_result_pt.x = mid_found.x ;
			  particle_6.detect_result_pt.y = mid_found.y ;
			  Calculate_car_color( particle_6.detect_result_pt,particle_6.Condens_size,frame,6);
			  particle_6.Condens_work = true ;
			  particle_6.Create_Condens = true ;
			  create_new_tracking = false ;

		  } // else if
	  } // if
	} // for

	/* �o��n�ӧP�_���O���O���l�ܮؤӤ[�S���Q�����ص��л\�F  �o��n�ӭp��*/

	if ( particle_1.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_1.detecting_tracking_overlapping == true || particle_1.Create_Condens == true )
			particle_1.counter_start = particle_1.counter_end = 0 ;
		else particle_1.counter_end++ ;
	} // if
	if ( particle_2.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_2.detecting_tracking_overlapping == true || particle_2.Create_Condens == true )
			particle_2.counter_start = particle_2.counter_end = 0 ;
		else particle_2.counter_end++ ;
	} // if
	if ( particle_3.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_3.detecting_tracking_overlapping == true || particle_3.Create_Condens == true )
			particle_3.counter_start = particle_3.counter_end = 0 ;
		else particle_3.counter_end++ ;
	} // if
	if ( particle_4.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_4.detecting_tracking_overlapping == true || particle_4.Create_Condens == true )
			particle_4.counter_start = particle_4.counter_end = 0 ;
		else particle_4.counter_end++ ;
	} // if
	if ( particle_5.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_5.detecting_tracking_overlapping == true || particle_5.Create_Condens == true )
			particle_5.counter_start = particle_5.counter_end = 0 ;
		else particle_5.counter_end++ ;
	} // if
	if ( particle_6.Condens_work == true ) {  //  �Y�O���Q�л\�� �άO��Ш��� ��start��end���O0   �_�hend +1
		if ( particle_6.detecting_tracking_overlapping == true || particle_6.Create_Condens == true )
			particle_6.counter_start = particle_6.counter_end = 0 ;
		else particle_6.counter_end++ ;
	} // if

} // Label_Tracking()
//*******************************************************************
double CalculateDistance( CvPoint a , CvPoint b ) {
	return pow(pow((double)(a.x-b.x), 2.0) + pow((double)(a.y-b.y), 2.0),0.5);
} // CalculateDistance()
//*******************************************************************
void Delete_tracking_overlap_tracking() {  
	// �P�_�O�_���l�ܮة������ۭ��|��   �n�R���p�����@��
	// �P�_�O�_���l�ܮة������ۭ��|��   �n�R���p�����@��
	// �Ϋܲª���k�g  �`�@�����Ӱl�ܾ�  ���h���  = =
	double overlapRate = 0.6 ;  // �л\�v

	if ( particle_1.Condens_work == true && particle_2.Condens_work == true ) {
		int tl_x_A = particle_1.temp_predict_pt.x-(particle_1.Condens_size.width/2);
		int tl_y_A = particle_1.temp_predict_pt.y-(particle_1.Condens_size.height/2);
		int tl_x_B = particle_2.temp_predict_pt.x-(particle_2.Condens_size.width/2);
		int tl_y_B = particle_2.temp_predict_pt.y-(particle_2.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_1.Condens_size.width,particle_1.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_2.Condens_size.width,particle_2.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			if ( A.width < B.width ) particle_1.Condens_work = false ;  // �R��size����p��
			else particle_2.Condens_work = false ;
		} // if
	} // if
	if ( particle_1.Condens_work == true && particle_3.Condens_work == true ) {
		int tl_x_A = particle_1.temp_predict_pt.x-(particle_1.Condens_size.width/2);
		int tl_y_A = particle_1.temp_predict_pt.y-(particle_1.Condens_size.height/2);
		int tl_x_B = particle_3.temp_predict_pt.x-(particle_3.Condens_size.width/2);
		int tl_y_B = particle_3.temp_predict_pt.y-(particle_3.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_1.Condens_size.width,particle_1.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_3.Condens_size.width,particle_3.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "2:  in" << endl ;
			if ( A.width < B.width ) particle_1.Condens_work = false ;  // �R��size����p��
			else particle_3.Condens_work = false ;
		} // if
	} // if
	if ( particle_1.Condens_work == true && particle_4.Condens_work == true ) {
		int tl_x_A = particle_1.temp_predict_pt.x-(particle_1.Condens_size.width/2);
		int tl_y_A = particle_1.temp_predict_pt.y-(particle_1.Condens_size.height/2);
		int tl_x_B = particle_4.temp_predict_pt.x-(particle_4.Condens_size.width/2);
		int tl_y_B = particle_4.temp_predict_pt.y-(particle_4.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_1.Condens_size.width,particle_1.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_4.Condens_size.width,particle_4.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "3:  in" << endl ;
			if ( A.width < B.width ) particle_1.Condens_work = false ;  // �R��size����p��
			else particle_4.Condens_work = false ;
		} // if
	} // if
	if ( particle_1.Condens_work == true && particle_5.Condens_work == true ) {
		int tl_x_A = particle_1.temp_predict_pt.x-(particle_1.Condens_size.width/2);
		int tl_y_A = particle_1.temp_predict_pt.y-(particle_1.Condens_size.height/2);
		int tl_x_B = particle_5.temp_predict_pt.x-(particle_5.Condens_size.width/2);
		int tl_y_B = particle_5.temp_predict_pt.y-(particle_5.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_1.Condens_size.width,particle_1.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_5.Condens_size.width,particle_5.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "4:  in" << endl ;
			if ( A.width < B.width ) particle_1.Condens_work = false ;  // �R��size����p��
			else particle_5.Condens_work = false ;
		} // if
	} // if
	if ( particle_1.Condens_work == true && particle_6.Condens_work == true ) {
		int tl_x_A = particle_1.temp_predict_pt.x-(particle_1.Condens_size.width/2);
		int tl_y_A = particle_1.temp_predict_pt.y-(particle_1.Condens_size.height/2);
		int tl_x_B = particle_6.temp_predict_pt.x-(particle_6.Condens_size.width/2);
		int tl_y_B = particle_6.temp_predict_pt.y-(particle_6.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_1.Condens_size.width,particle_1.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_6.Condens_size.width,particle_6.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "5:  in" << endl ;
			if ( A.width < B.width ) particle_1.Condens_work = false ;  // �R��size����p��
			else particle_6.Condens_work = false ;
		} // if
	} // if
	if ( particle_2.Condens_work == true && particle_3.Condens_work == true ) {
		int tl_x_A = particle_2.temp_predict_pt.x-(particle_2.Condens_size.width/2);
		int tl_y_A = particle_2.temp_predict_pt.y-(particle_2.Condens_size.height/2);
		int tl_x_B = particle_3.temp_predict_pt.x-(particle_3.Condens_size.width/2);
		int tl_y_B = particle_3.temp_predict_pt.y-(particle_3.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_2.Condens_size.width,particle_2.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_3.Condens_size.width,particle_3.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "6:  in" << endl ;
			if ( A.width < B.width ) particle_2.Condens_work = false ;  // �R��size����p��
			else particle_3.Condens_work = false ;
		} // if
	} // if
	if ( particle_2.Condens_work == true && particle_4.Condens_work == true ) {
		int tl_x_A = particle_2.temp_predict_pt.x-(particle_2.Condens_size.width/2);
		int tl_y_A = particle_2.temp_predict_pt.y-(particle_2.Condens_size.height/2);
		int tl_x_B = particle_4.temp_predict_pt.x-(particle_4.Condens_size.width/2);
		int tl_y_B = particle_4.temp_predict_pt.y-(particle_4.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_2.Condens_size.width,particle_2.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_4.Condens_size.width,particle_4.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "7:  in" << endl ;
			if ( A.width < B.width ) particle_2.Condens_work = false ;  // �R��size����p��
			else particle_4.Condens_work = false ;
		} // if
	} // if
	if ( particle_2.Condens_work == true && particle_5.Condens_work == true ) {
		int tl_x_A = particle_2.temp_predict_pt.x-(particle_2.Condens_size.width/2);
		int tl_y_A = particle_2.temp_predict_pt.y-(particle_2.Condens_size.height/2);
		int tl_x_B = particle_5.temp_predict_pt.x-(particle_5.Condens_size.width/2);
		int tl_y_B = particle_5.temp_predict_pt.y-(particle_5.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_2.Condens_size.width,particle_2.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_5.Condens_size.width,particle_5.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "8:  in" << Calculating_overlap_rate( A,B ) <<  endl ;
			//cout << particle_2.Condens_size.width << "  " << particle_5.Condens_size.width << endl ;
			if ( A.width < B.width ) particle_2.Condens_work = false ;  // �R��size����p��
			else particle_5.Condens_work = false ;
		} // if
	} // if
	if ( particle_2.Condens_work == true && particle_6.Condens_work == true ) {
		int tl_x_A = particle_2.temp_predict_pt.x-(particle_2.Condens_size.width/2);
		int tl_y_A = particle_2.temp_predict_pt.y-(particle_2.Condens_size.height/2);
		int tl_x_B = particle_6.temp_predict_pt.x-(particle_6.Condens_size.width/2);
		int tl_y_B = particle_6.temp_predict_pt.y-(particle_6.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_2.Condens_size.width,particle_2.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_6.Condens_size.width,particle_6.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "9:  in" << endl ;
			if ( A.width < B.width ) particle_2.Condens_work = false ;  // �R��size����p��
			else particle_6.Condens_work = false ;
		} // if
	} // if
	if ( particle_3.Condens_work == true && particle_4.Condens_work == true ) {
		int tl_x_A = particle_3.temp_predict_pt.x-(particle_3.Condens_size.width/2);
		int tl_y_A = particle_3.temp_predict_pt.y-(particle_3.Condens_size.height/2);
		int tl_x_B = particle_4.temp_predict_pt.x-(particle_4.Condens_size.width/2);
		int tl_y_B = particle_4.temp_predict_pt.y-(particle_4.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_3.Condens_size.width,particle_3.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_4.Condens_size.width,particle_4.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "10:  in" << endl ;
			if ( A.width < B.width ) particle_3.Condens_work = false ;  // �R��size����p��
			else particle_4.Condens_work = false ;
		} // if
	} // if
	if ( particle_3.Condens_work == true && particle_5.Condens_work == true ) {
		int tl_x_A = particle_3.temp_predict_pt.x-(particle_3.Condens_size.width/2);
		int tl_y_A = particle_3.temp_predict_pt.y-(particle_3.Condens_size.height/2);
		int tl_x_B = particle_5.temp_predict_pt.x-(particle_5.Condens_size.width/2);
		int tl_y_B = particle_5.temp_predict_pt.y-(particle_5.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_3.Condens_size.width,particle_3.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_5.Condens_size.width,particle_5.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "11:  in" << endl ;
			if ( A.width < B.width ) particle_3.Condens_work = false ;  // �R��size����p��
			else particle_5.Condens_work = false ;
		} // if
	} // if
	if ( particle_3.Condens_work == true && particle_6.Condens_work == true ) {
		int tl_x_A = particle_3.temp_predict_pt.x-(particle_3.Condens_size.width/2);
		int tl_y_A = particle_3.temp_predict_pt.y-(particle_3.Condens_size.height/2);
		int tl_x_B = particle_6.temp_predict_pt.x-(particle_6.Condens_size.width/2);
		int tl_y_B = particle_6.temp_predict_pt.y-(particle_6.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_3.Condens_size.width,particle_3.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_6.Condens_size.width,particle_6.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "12:  in" << endl ;
			if ( A.width < B.width ) particle_3.Condens_work = false ;  // �R��size����p��
			else particle_6.Condens_work = false ;
		} // if
	} // if
	if ( particle_4.Condens_work == true && particle_5.Condens_work == true ) {
		int tl_x_A = particle_4.temp_predict_pt.x-(particle_4.Condens_size.width/2);
		int tl_y_A = particle_4.temp_predict_pt.y-(particle_4.Condens_size.height/2);
		int tl_x_B = particle_5.temp_predict_pt.x-(particle_5.Condens_size.width/2);
		int tl_y_B = particle_5.temp_predict_pt.y-(particle_5.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_4.Condens_size.width,particle_4.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_5.Condens_size.width,particle_5.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "13:  in" << endl ;
			if ( A.width < B.width ) particle_4.Condens_work = false ;  // �R��size����p��
			else particle_5.Condens_work = false ;
		} // if
	} // if
	if ( particle_4.Condens_work == true && particle_6.Condens_work == true ) {
		int tl_x_A = particle_4.temp_predict_pt.x-(particle_4.Condens_size.width/2);
		int tl_y_A = particle_4.temp_predict_pt.y-(particle_4.Condens_size.height/2);
		int tl_x_B = particle_6.temp_predict_pt.x-(particle_6.Condens_size.width/2);
		int tl_y_B = particle_6.temp_predict_pt.y-(particle_6.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_4.Condens_size.width,particle_4.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_6.Condens_size.width,particle_6.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "14:  in" << endl ;
			if ( A.width < B.width ) particle_4.Condens_work = false ;  // �R��size����p��
			else particle_6.Condens_work = false ;
		} // if
	} // if
	if ( particle_5.Condens_work == true && particle_6.Condens_work == true ) {
		int tl_x_A = particle_5.temp_predict_pt.x-(particle_5.Condens_size.width/2);
		int tl_y_A = particle_5.temp_predict_pt.y-(particle_5.Condens_size.height/2);
		int tl_x_B = particle_6.temp_predict_pt.x-(particle_6.Condens_size.width/2);
		int tl_y_B = particle_6.temp_predict_pt.y-(particle_6.Condens_size.height/2);
		cv::Rect A = Rect( tl_x_A, tl_y_A,particle_5.Condens_size.width,particle_5.Condens_size.height);
		cv::Rect B = Rect( tl_x_B, tl_y_B,particle_6.Condens_size.width,particle_6.Condens_size.height);
		if ( Calculating_overlap_rate( A,B ) > overlapRate ) { // �л\�v�j�󤭦� �N�n�屼�䤤�@��
			//cout << "15:  in" << endl ;
			if ( A.width < B.width ) particle_5.Condens_work = false ;  // �R��size����p��
			else particle_6.Condens_work = false ;
		} // if
	} // if

} // If_tracking_rect_exist()
//*******************************************************************
double Calculating_overlap_rate(cv::Rect A, cv::Rect B) {
	// ��X��ӯx�Υ涰���ϰ�  �íp��X�л\�v�O�h�֦^��
    int tl_x = ( A.tl().x > B.tl().x ) ? A.tl().x:B.tl().x;
    int tl_y = ( A.tl().y > B.tl().y ) ? A.tl().y:B.tl().y;
    int br_x = ( A.br().x < B.br().x ) ? A.br().x:B.br().x;
    int br_y = ( A.br().y < B.br().y ) ? A.br().y:B.br().y;
	//cout << tl_x << "  " << tl_y << "  " << br_x << "  " << br_y << endl;
    int area_join = ( br_x - tl_x ) * ( br_y - tl_y );
	//cout << area_join << endl ;
    int area_rect1 = ( A.br().x - A.tl().x ) * ( A.br().y - A.tl().y );
    int area_rect2 = ( B.br().x - B.tl().x ) * ( B.br().y - B.tl().y );
	//cout << ( (double)area_join / ( (double)area_rect1 + (double)area_rect2 - (double)area_join ) ) << endl ;
    return ( (double)area_join / ( (double)area_rect1 + (double)area_rect2 - (double)area_join ) );
} // Calculating_overlap_rate()
//*******************************************************************
bool If_tracking_rect_already_exist( CvPoint detect_pt, CvSize detect_size, cv::Mat frame) {  
	// �P�_�ǳƭn�Ыذl�ܮت��ɭ�  �O���O�w�g���l�ܮئb�o�F
	if ( particle_1.Condens_work == true ) {
		int tl_x = particle_1.temp_predict_pt.x - particle_1.Condens_size.width/2 ;
		int tl_y = particle_1.temp_predict_pt.y - particle_1.Condens_size.height/2 ;
		
		// �l�ܮت��x��/2 ����]�O�]���Ȧ����l�Ӫ�  ���٬O�n�h�l��  �ҥH�P�_���n�p�@�I
		cv::Rect tracking = Rect( tl_x,tl_y,particle_1.Condens_size.width/2,particle_1.Condens_size.height/2 );
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;

		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );

		if ( IsOverlap( tracking, detecting ) == true  ) return true ;
	} // if
	if (  particle_2.Condens_work == true  ) {
		int tl_x = particle_2.temp_predict_pt.x - particle_2.Condens_size.width/2 ;
		int tl_y = particle_2.temp_predict_pt.y - particle_2.Condens_size.height/2 ;
		cv::Rect tracking = Rect( tl_x,tl_y,particle_2.Condens_size.width/2,particle_2.Condens_size.height/2 );

		//cv::rectangle( frame, tracking, cv::Scalar(128,128,128), 3 );	
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;
		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );
		//system( "pause" );
		//cv::rectangle( frame, detecting, cv::Scalar(0,0,0), 3 );
		if ( IsOverlap( tracking, detecting ) == true  ) return true ;
	} // if
	if (  particle_3.Condens_work == true  ) {
		int tl_x = particle_3.temp_predict_pt.x - particle_3.Condens_size.width/2 ;
		int tl_y = particle_3.temp_predict_pt.y - particle_3.Condens_size.height/2 ;
		cv::Rect tracking = Rect( tl_x,tl_y,particle_3.Condens_size.width/2,particle_3.Condens_size.height/2 );
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;
		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );

		if ( IsOverlap( tracking, detecting ) == true  ) return true ;
	} // if
	if (  particle_4.Condens_work == true  ) {
		int tl_x = particle_4.temp_predict_pt.x - particle_4.Condens_size.width/2 ;
		int tl_y = particle_4.temp_predict_pt.y - particle_4.Condens_size.height/2 ;
		cv::Rect tracking = Rect( tl_x,tl_y,particle_4.Condens_size.width/2,particle_4.Condens_size.height/2 );
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;
		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );

		if ( IsOverlap( tracking, detecting ) == true  ) return true ;
	} // if
	if (  particle_5.Condens_work == true  ) {
		int tl_x = particle_5.temp_predict_pt.x - particle_5.Condens_size.width/2 ;
		int tl_y = particle_5.temp_predict_pt.y - particle_5.Condens_size.height/2 ;
		cv::Rect tracking = Rect( tl_x,tl_y,particle_5.Condens_size.width/2,particle_5.Condens_size.height/2 );
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;
		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );

		if ( IsOverlap( tracking, detecting ) == true ) return true ;
	} // if
	if (  particle_6.Condens_work == true  ) {
		int tl_x = particle_6.temp_predict_pt.x - particle_6.Condens_size.width/2 ;
		int tl_y = particle_6.temp_predict_pt.y - particle_6.Condens_size.height/2 ;
		cv::Rect tracking = Rect( tl_x,tl_y,particle_6.Condens_size.width/2,particle_6.Condens_size.height/2 );
		tl_x = detect_pt.x - detect_size.width/2 ;
		tl_y = detect_pt.y - detect_size.height/2 ;
		cv::Rect detecting = Rect( tl_x,tl_y,detect_size.width,detect_size.height );

		if ( IsOverlap( tracking, detecting ) == true ) return true ;
	} // if

	return false ;
} // If_tracking_rect_exist()
//*******************************************************************
bool IsOverlap( cv::Rect rect1, cv::Rect rect2 ) {
	// �P�_��ӯx�άO�_�����|
	if ( ( rect1.br().x <= rect2.tl().x || rect2.br().x <= rect1.tl().x ) ||
         ( rect1.br().y <= rect2.tl().y || rect2.br().y <= rect1.tl().y ) )
        return false;
    else
        return true;
} // IsOverlap()
//*******************************************************************
bool If_Point_Outofframe( CvPoint pt, CvSize size ) {
  //if ( pt.x == 491 )  system("pause");
	double boundary_left = 40.0, boundary_right = 600, boundary_top = 150.0, boundary_bot = 450.0 ;
    // �]�wparticle tracking �����
	if ( (double)pt.x - (double)( size.width/2 ) > boundary_left 
		&& (double)pt.x + (double)( size.width/2 ) < boundary_right 
		&& (double)pt.y - (double)(size.height/2) > boundary_top 
		&& (double)pt.y + (double)( size.height/2 ) < boundary_bot ) {
		return false ;
	} // if
	else{
		return true ;
	} // else
} // If_Point_Outofframe()
//*******************************************************************
void Calculate_car_color( CvPoint position, CvSize size, cv::Mat frame, int which_Car) {
	// �o��function�n��X���l���C��  �H�Ψ��إ~��@��"�D��"���C��
	// �u���blabel tracking�̭����Ψ�

	for ( int a =0 ; a < 256 ; a++ ) {
		particle_1.B_car_color[a] = particle_1.G_car_color[a] = particle_1.R_car_color[a] = 0 ;   
		particle_2.B_car_color[a] = particle_2.G_car_color[a] = particle_2.R_car_color[a] = 0 ;   
		particle_3.B_car_color[a] = particle_3.G_car_color[a] = particle_3.R_car_color[a] = 0 ;   
		particle_4.B_car_color[a] = particle_4.G_car_color[a] = particle_4.R_car_color[a] = 0 ;   
		particle_5.B_car_color[a] = particle_5.G_car_color[a] = particle_5.R_car_color[a] = 0 ;   
		particle_6.B_car_color[a] = particle_6.G_car_color[a] = particle_6.R_car_color[a] = 0 ;      
	} // for


	int extend_x = cvRound(pow(2,0.5)*size.width) ; // �V�~�����D�����Z��  �ڸ�2 * w 
	int extend_y = cvRound(pow(2,0.5)*size.height) ; // �V�~�����D��������  �ڸ�2 * h 
	
	// ��ɤ���W�L�V�~�������Z��

	if ( position.x-(extend_x/2)>0 && position.y-(extend_y/2)>150
		&& position.x+(extend_x/2)<640 && position.y+(extend_y/2)<440) {
		// �b�خإ~���B�~����  ��X�D�����C�⪺�P���C��
			CvRect ROI_RECT = cvRect(position.x-(extend_x/2),position.y-(extend_y/2),extend_x,extend_y);
			IplImage * image = &IplImage( frame ) ;
			cvSetImageROI(image,ROI_RECT);
			IplImage *img = cvCreateImage( cvSize(ROI_RECT.width,ROI_RECT.height), 8, 3 );
			cvCopy(image,img); 
			cvResetImageROI(image);

			double area = size.width*size.height ; //���l�خؤ���pixel�`��
			double outside_area = (extend_x*extend_y)-area ;  // �~��pixel �`��

			CvPoint innerPt_tl = cvPoint((extend_x/2)-(size.width/2),(extend_y/2)-(size.height/2));  // ���l�خؤ�80%�����ӯx�Υ��W�I ��k�U�I
			CvPoint innerPt_br = cvPoint((extend_x/2)+(size.width/2),(extend_y/2)+(size.height/2));

			if ( which_Car == 1 ) {
				// �o��ΨӦs�n���l�M�D��������pixel
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_1.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_1.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_1.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) { 
							// �Y�O�D���������C��
						particle_1.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_1.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_1.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for

			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0

			      particle_1.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_1.G_car_color[a]/=area ;
				  particle_1.R_car_color[a]/=area ;
				  
				  particle_1.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_1.G_not_car_color[a]/=outside_area ;
				  particle_1.R_not_car_color[a]/=outside_area ;

				  if ( particle_1.B_car_color[a] - particle_1.B_not_car_color[a] > 0 ) {
					particle_1.B_confidence[a] = particle_1.B_car_color[a] - particle_1.B_not_car_color[a] ;
				    if ( particle_1.B_confidence[a] > max_B ) 
						max_B = particle_1.B_confidence[a] ;
				  } // if
				  else
					particle_1.B_confidence[a] = 0 ;
				  
				  if ( particle_1.G_car_color[a] - particle_1.G_not_car_color[a] > 0 ) {
					particle_1.G_confidence[a] = particle_1.G_car_color[a] - particle_1.G_not_car_color[a] ;
				    if ( particle_1.G_confidence[a] > max_G )
						max_G = particle_1.G_confidence[a] ;
				  } // if
				  else
					particle_1.G_confidence[a] = 0 ;

				  if ( particle_1.R_car_color[a] - particle_1.R_not_car_color[a] > 0 ) {
					particle_1.R_confidence[a] = particle_1.R_car_color[a] - particle_1.R_not_car_color[a] ;
				    if ( particle_1.R_confidence[a] > max_R )
						max_R = particle_1.R_confidence[a] ;
				  } // if
				  else
					particle_1.R_confidence[a] = 0 ;

			  } // for
			  
 			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;

			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbin�]�����v����  
				  particle_1.B_confidence[a]*=scale_B;
				  particle_1.G_confidence[a]*=scale_G;
				  particle_1.R_confidence[a]*=scale_R;
			  } // for

			} // if
			else if (which_Car == 2) {
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_2.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_2.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_2.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
						/*img->imageData[i*img->widthStep+j] = 0 ;
						img->imageData[i*img->widthStep+j+1] = 255 ;
						img->imageData[i*img->widthStep+j+2] = 255 ;*/
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) {  // �Y�O�D���������C��
						particle_2.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_2.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_2.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
						/*img->imageData[i*img->widthStep+j] = 0 ;
						img->imageData[i*img->widthStep+j+1] = 255 ;
						img->imageData[i*img->widthStep+j+2] = 255 ;*/
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for	

			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0
				  particle_2.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_2.G_car_color[a]/=area ;
				  particle_2.R_car_color[a]/=area ;
				  particle_2.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_2.G_not_car_color[a]/=outside_area ;
				  particle_2.R_not_car_color[a]/=outside_area ;

				  if ( particle_2.B_car_color[a] - particle_2.B_not_car_color[a] >= 0 ) {
					particle_2.B_confidence[a] = particle_2.B_car_color[a] - particle_2.B_not_car_color[a] ;
				    if ( particle_2.B_confidence[a] > max_B )
						max_B = particle_2.B_confidence[a] ;
				  } // if
				  else
					particle_2.B_confidence[a] = 0 ;

				  if ( particle_2.G_car_color[a] - particle_2.G_not_car_color[a] >= 0 ) {
					particle_2.G_confidence[a] = particle_2.G_car_color[a] - particle_2.G_not_car_color[a] ;
				    if ( particle_2.G_car_color[a] > max_G )
						max_G = particle_2.G_car_color[a] ;
				  } // if
				  else
					particle_2.G_confidence[a] = 0 ;

				  if ( particle_2.R_car_color[a] - particle_2.R_not_car_color[a] >= 0 ) {
					particle_2.R_confidence[a] = particle_2.R_car_color[a] - particle_2.R_not_car_color[a] ;
				    if ( particle_2.R_confidence[a] > max_R )
						max_R = particle_2.R_confidence[a] ;
				  } // if
				  else
					particle_2.R_confidence[a] = 0 ;

			  } // for

			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;
			  
			  double scale_BT = 300.0/max_B;
			  double scale_GT = 300.0/max_G;
			  double scale_RT = 300.0/max_R;
			  
			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbin�]�����v����  
				  //particle_6.B_confidence[a] = particle_2.B_confidence[a] ;
				 // particle_6.G_confidence[a] = particle_2.G_confidence[a] ;
				 // particle_6.R_confidence[a] = particle_2.R_confidence[a] ;
				  particle_2.B_confidence[a]*=scale_B;
				  particle_2.G_confidence[a]*=scale_G;
				  particle_2.R_confidence[a]*=scale_R;
				 // particle_6.B_confidence[a]*=scale_BT;
				 // particle_6.G_confidence[a]*=scale_GT;
				 // particle_6.R_confidence[a]*=scale_RT;
			  } // for

			  
			  /*
			  
				IplImage *HistogramImageB;
				HistogramImageB = cvCreateImage(cvSize(256,300),8,3);
				HistogramImageB->origin=1;
				IplImage *HistogramImageG;
				HistogramImageG = cvCreateImage(cvSize(256,300),8,3);
				HistogramImageG->origin=1;	
				IplImage *HistogramImageR;
				HistogramImageR = cvCreateImage(cvSize(256,300),8,3);
				HistogramImageR->origin=1;


			  for(int i=0;i<256;i++) {
				cvLine(HistogramImageB,cvPoint(i,0),cvPoint(i,particle_6.B_confidence[i]),CV_RGB(0,0,255));
				cvLine(HistogramImageG,cvPoint(i,0),cvPoint(i,particle_6.G_confidence[i]),CV_RGB(0,255,0));
				cvLine(HistogramImageR,cvPoint(i,0),cvPoint(i,particle_6.R_confidence[i]),CV_RGB(255,0,0));
			  }

			  cvNamedWindow("B",1);
              cvShowImage("B",HistogramImageB);
			  cvNamedWindow("G",1);
              cvShowImage("G",HistogramImageG);
			  cvNamedWindow("R",1);
              cvShowImage("R",HistogramImageR);

              //if( waitKey (30) >= 0) ;
			 
			
			 cvReleaseImage(&HistogramImageB);			
			 cvReleaseImage(&HistogramImageG);			
			 cvReleaseImage(&HistogramImageR);


			  
			  cvNamedWindow("test",1);
              cvShowImage("test",img);
			  cvReleaseImage(&img);




			  cout << "IN!!!" << endl;
			  
			  */
			} // else if
			else if (which_Car == 3) {
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_3.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_3.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_3.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) {  // �Y�O�D���������C��
						particle_3.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_3.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_3.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for	
	
			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0
				  particle_3.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_3.G_car_color[a]/=area ;
				  particle_3.R_car_color[a]/=area ;
				  particle_3.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_3.G_not_car_color[a]/=outside_area ;
				  particle_3.R_not_car_color[a]/=outside_area ;

				  if ( particle_3.B_car_color[a] - particle_3.B_not_car_color[a] >= 0 ) {
					particle_3.B_confidence[a] = particle_3.B_car_color[a] - particle_3.B_not_car_color[a] ;
				    if ( particle_3.B_confidence[a] > max_B )
						max_B = particle_3.B_confidence[a] ;
				  } // if
				  else
					particle_3.B_confidence[a] = 0 ;

				  if ( particle_3.G_car_color[a] - particle_3.G_not_car_color[a] >= 0 ) {
					particle_3.G_confidence[a] = particle_3.G_car_color[a] - particle_3.G_not_car_color[a] ;
				    if ( particle_3.G_confidence[a] > max_G )
						max_G = particle_3.G_confidence[a] ;
				  } // if
				  else
					particle_3.G_confidence[a] = 0 ;

				  if ( particle_3.R_car_color[a] - particle_3.R_not_car_color[a] >= 0 ) {
					particle_3.R_confidence[a] = particle_3.R_car_color[a] - particle_3.R_not_car_color[a] ;
				    if ( particle_3.R_confidence[a] > max_R )
						max_R = particle_3.R_confidence[a] ;
				  } // if
				  else
					particle_3.R_confidence[a] = 0 ;

			  } // for

			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;

			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbun�]�����v����  
				  particle_3.B_confidence[a]*=scale_B;
				  particle_3.G_confidence[a]*=scale_G;
				  particle_3.R_confidence[a]*=scale_R;
			  } // for

			} // else if
			else if (which_Car ==4) {
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_4.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_4.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_4.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) {  // �Y�O�D���������C��
						particle_4.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_4.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_4.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for	

			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0
				  particle_4.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_4.G_car_color[a]/=area ;
				  particle_4.R_car_color[a]/=area ;
				  particle_4.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_4.G_not_car_color[a]/=outside_area ;
				  particle_4.R_not_car_color[a]/=outside_area ;

				  if ( particle_4.B_car_color[a] - particle_4.B_not_car_color[a] >= 0 ) {
					particle_4.B_confidence[a] = particle_4.B_car_color[a] - particle_4.B_not_car_color[a] ;
				    if ( particle_4.B_confidence[a] > max_B )
						max_B = particle_4.B_confidence[a] ;
				  } // if
				  else
					particle_4.B_confidence[a] = 0 ;

				  if ( particle_4.G_car_color[a] - particle_4.G_not_car_color[a] >= 0 ) {
					particle_4.G_confidence[a] = particle_4.G_car_color[a] - particle_4.G_not_car_color[a] ;
				    if ( particle_4.G_confidence[a] > max_G )
						max_G = particle_4.G_confidence[a] ;
				  } // if
				  else
					particle_4.G_confidence[a] = 0 ;

				  if ( particle_4.R_car_color[a] - particle_4.R_not_car_color[a] >= 0 ) {
					particle_4.R_confidence[a] = particle_4.R_car_color[a] - particle_4.R_not_car_color[a] ;
				    if ( particle_4.R_confidence[a] > max_R )
						max_R = particle_4.R_confidence[a] ;
				  } // if
				  else
					particle_4.R_confidence[a] = 0 ;

			  } // for


			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;

			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbun�]�����v����  
				  particle_4.B_confidence[a]*=scale_B;
				  particle_4.G_confidence[a]*=scale_G;
				  particle_4.R_confidence[a]*=scale_R;
			  } // for


			} // else if
			else if (which_Car == 5) {
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_5.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_5.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_5.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) {  // �Y�O�D���������C��
						particle_5.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_5.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_5.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for	
	
			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0
				  particle_5.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_5.G_car_color[a]/=area ;
				  particle_5.R_car_color[a]/=area ;
				  particle_5.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_5.G_not_car_color[a]/=outside_area ;
				  particle_5.R_not_car_color[a]/=outside_area ;

				  if ( particle_5.B_car_color[a] - particle_5.B_not_car_color[a] >= 0 ) {
					particle_5.B_confidence[a] = particle_5.B_car_color[a] - particle_5.B_not_car_color[a] ;
				    if ( particle_5.B_confidence[a] > max_B )
						max_B = particle_5.B_confidence[a] ;
				  } // if
				  else
					particle_5.B_confidence[a] = 0 ;

				  if ( particle_5.G_car_color[a] - particle_5.G_not_car_color[a] >= 0 ) {
					particle_5.G_confidence[a] = particle_5.G_car_color[a] - particle_5.G_not_car_color[a] ;
				    if ( particle_5.G_confidence[a] > max_G )
						max_G = particle_5.G_confidence[a] ;
				  } // if
				  else
					particle_5.G_confidence[a] = 0 ;

				  if ( particle_5.R_car_color[a] - particle_5.R_not_car_color[a] >= 0 ) {
					particle_5.R_confidence[a] = particle_5.R_car_color[a] - particle_5.R_not_car_color[a] ;
				    if ( particle_5.R_confidence[a] > max_R )
						max_R = particle_5.R_confidence[a] ;
				  } // if
				  else
					particle_5.R_confidence[a] = 0 ;

			  } // for

			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;

			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbin�]�����v����  
				  particle_5.B_confidence[a]*=scale_B;
				  particle_5.G_confidence[a]*=scale_G;
				  particle_5.R_confidence[a]*=scale_R;
			  } // for

			} // else if
			else if (which_Car == 6) {
			  for ( int i =0 ;i<img->height;i++ ) {
				for( int j=0;j<img->widthStep;j=j+3) {		
					if ( (j/3) > innerPt_tl.x && (j/3) < innerPt_br.x
						&& i > innerPt_tl.y && i < innerPt_br.y ) {  // �Y�O�����C��
						particle_6.B_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_6.G_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_6.R_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // if
					else if ( (j/3) > (extend_y/2)+(size.height/2) || i >(extend_x/2)+(size.width/2)
						|| i < (extend_x/2)-(size.width/2) || (j/3) < (extend_y/2)-(size.height/2) ) {  // �Y�O�D���������C��
						particle_6.B_not_car_color[(uchar)img->imageData[i*img->widthStep+j]]++ ;
						particle_6.G_not_car_color[(uchar)img->imageData[i*img->widthStep+j+1]]++ ;
						particle_6.R_not_car_color[(uchar)img->imageData[i*img->widthStep+j+2]]++ ;
					} // else if
					else {;}  // ���b���������z�L
				} // for
			  } // for	
		
			  double max_B=0.0,max_G=0.0,max_R=0.0 ; // �n��j���Ƚվ㦨1  �ҥH�n�O���U�̤j��bin�ȬO�h��

			  for ( int a = 0 ; a < 256 ; a++ ) {  // �O���l���������O���l��  �M�w�Xconfidence
				  // �۴��p�G�p��0  �]��0
				  particle_6.B_car_color[a]/=area ; // �@���W��  �����O���l�ϰ쪺�`�� �ܦ�0~1
				  particle_6.G_car_color[a]/=area ;
				  particle_6.R_car_color[a]/=area ;
				  particle_6.B_not_car_color[a]/=outside_area ; // �@���W��  ���������������`�� �ܦ�0~1
				  particle_6.G_not_car_color[a]/=outside_area ;
				  particle_6.R_not_car_color[a]/=outside_area ;

				  if ( particle_6.B_car_color[a] - particle_6.B_not_car_color[a] >= 0 ) {
					particle_6.B_confidence[a] = particle_6.B_car_color[a] - particle_6.B_not_car_color[a] ;
				    if ( particle_6.B_confidence[a] > max_B ) 
						max_B = particle_6.B_confidence[a] ; // �O���U�̤j��bin�� �n�⥦�ܦ�1
				  } // if
				  else
					particle_6.B_confidence[a] = 0 ;

				  if ( particle_6.G_car_color[a] - particle_6.G_not_car_color[a] >= 0 ) {
					particle_6.G_confidence[a] = particle_6.G_car_color[a] - particle_6.G_not_car_color[a] ;
					if ( particle_6.G_confidence[a] > max_G ) 
						max_G = particle_6.G_confidence[a] ;
				  } // if
				  else
					particle_6.G_confidence[a] = 0 ;

				  if ( particle_6.R_car_color[a] - particle_6.R_not_car_color[a] >= 0 ) {
					particle_6.R_confidence[a] = particle_6.R_car_color[a] - particle_6.R_not_car_color[a] ;
				    if ( particle_6.R_confidence[a] > max_R )
						max_R = particle_6.R_confidence[a] ;
				  } // if
				  else
					particle_6.R_confidence[a] = 0 ;

			  } // for

			  double scale_B = 1.0/max_B;  // ��X��̤j��bin ���W�h�ܦ�1�����v
			  double scale_G = 1.0/max_G;
			  double scale_R = 1.0/max_R;

			  for ( int a = 0 ; a < 256 ; a++ ) {  // ��confidence�̤jbin�ȳ]��1 ��Lbun�]�����v����  
				  particle_6.B_confidence[a]*=scale_B;
				  particle_6.G_confidence[a]*=scale_G;
				  particle_6.R_confidence[a]*=scale_R;
			  } // for

			} // else if

		} // if

} // Calculate_car_color() 
//*******************************************************************
double Point_Compare_Vehicle_Color( CvPoint pt, CvPoint midfound, int which_car ) {
	// �o��function�Ψ�����particles confidence  
    // Pt �O particle ����m   midfound �O�T������m 

	int i = pt.y, j=pt.x ;

	double temp = (double)(pt.x-midfound.x)*(pt.x-midfound.x) + (double)(pt.y-midfound.y)*(pt.y-midfound.y) ;

	if ( which_car == 1 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_1.Condens_size.width*particle_1.Condens_size.width/2)) ;
		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_1.B_confidence[B]+particle_1.G_confidence[G]+particle_1.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // if
	else if ( which_car == 2 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_2.Condens_size.width*particle_2.Condens_size.width/2)) ;

		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_2.B_confidence[B]+particle_2.G_confidence[G]+particle_2.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // else if
	else if ( which_car == 3 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_3.Condens_size.width*particle_3.Condens_size.width/2)) ;

		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_3.B_confidence[B]+particle_3.G_confidence[G]+particle_3.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // else if
	else if ( which_car == 4 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_4.Condens_size.width*particle_4.Condens_size.width/2)) ;

		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_4.B_confidence[B]+particle_4.G_confidence[G]+particle_4.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // else if
	else if ( which_car == 5 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_5.Condens_size.width*particle_5.Condens_size.width/2)) ;

		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_5.B_confidence[B]+particle_5.G_confidence[G]+particle_5.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // else if
	else if ( which_car == 6 ) {
	    // spatial Gaussian ��weight	    
		double weight = (double)exp(-temp/(particle_6.Condens_size.width*particle_6.Condens_size.width/2)) ;

		int B = BLUE_ARRAY[i][j] ;
		int G = GREEN_ARRAY[i][j] ;
		int R = RED_ARRAY[i][j] ;
		// �^�� RGB �T�Ӭۥ[/3��confidence  
		double color = (particle_6.B_confidence[B]+particle_6.G_confidence[G]+particle_6.R_confidence[R])/3 ;
	    return pow( 100, color*weight );
	} // else if
	  
} // Point_Compare_Vehicle_Color()
//*******************************************************************
double detect_compare_tracking(CvPoint midfound, int which_car){
	// �o��function�ΨӤ�������ػP�l�ܮب�Ӯخت���m�ۦ���  �H�F��matching���ĪG	
	// �ofunction�u���b Label tracking �Q�I�s��
	// �^�Ǥ@�Ө�̤�������m�ۦ���  �P�_�O���O�P�@�x��(�����حn�hmatch tracking)

	// �ǳƭn�b�o��[�JGAUSSIAN �i�h���!!!!
 
	// �ϥΰ����Z����@matching���̾�
	int i = midfound.y, j=midfound.x ;
	
	if ( which_car == 1 ) {
		double temp = (double)(midfound.x-particle_1.pre_tracking_position.x)*(midfound.x-particle_1.pre_tracking_position.x) + (double)(midfound.y-particle_1.pre_tracking_position.y)*(midfound.y-particle_1.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_1.Condens_size.width*particle_1.Condens_size.width/2)) ;
		return weight ;
	} // if
	else if ( which_car == 2 ) {
		double temp = (double)(midfound.x-particle_2.pre_tracking_position.x)*(midfound.x-particle_2.pre_tracking_position.x) + (double)(midfound.y-particle_2.pre_tracking_position.y)*(midfound.y-particle_2.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_2.Condens_size.width*particle_2.Condens_size.width/2)) ;
		return weight ;
	} // else if
	else if ( which_car == 3 ) {
		double temp = (double)(midfound.x-particle_3.pre_tracking_position.x)*(midfound.x-particle_3.pre_tracking_position.x) + (double)(midfound.y-particle_3.pre_tracking_position.y)*(midfound.y-particle_3.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_3.Condens_size.width*particle_3.Condens_size.width/2)) ;
		return weight ;
	} // else if
	else if ( which_car == 4 ) {
		double temp = (double)(midfound.x-particle_4.pre_tracking_position.x)*(midfound.x-particle_4.pre_tracking_position.x) + (double)(midfound.y-particle_4.pre_tracking_position.y)*(midfound.y-particle_4.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_4.Condens_size.width*particle_4.Condens_size.width/2)) ;
		return weight ;
	} // else if
	else if ( which_car == 5 ) {
		double temp = (double)(midfound.x-particle_5.pre_tracking_position.x)*(midfound.x-particle_5.pre_tracking_position.x) + (double)(midfound.y-particle_5.pre_tracking_position.y)*(midfound.y-particle_5.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_5.Condens_size.width*particle_5.Condens_size.width/2)) ;
		return weight ;
	} // else if
	else if ( which_car == 6 ) {
		double temp = (double)(midfound.x-particle_6.pre_tracking_position.x)*(midfound.x-particle_6.pre_tracking_position.x) + (double)(midfound.y-particle_6.pre_tracking_position.y)*(midfound.y-particle_6.pre_tracking_position.y) ;
		double weight = (double)exp(-temp/(particle_6.Condens_size.width*particle_6.Condens_size.width/2)) ;
		return weight ;
	} // else if

} // detect_compare_tracking()
//*******************************************************************
void Cvt1dto2d( IplImage *src, int **r, int **g, int **b ) {
	// �o�ӯx�}�Ψӧ�@�i�v�����Ҧ�pixel �s��RGB �T�ӭӧO���G���}�C
	
  int k = 0;
  if ( src -> nChannels == 1 )
  {
    for ( int i = 0 ; i < src -> height ; i++ )
    {
      if ( ( k + 3 ) % src -> widthStep == 0 ) k += 3;
      for ( int j = 0 ; j < src -> width ; j++, k += 3 )
      {
        b[i][j] = (uchar)src->imageData[k];
        g[i][j] = (uchar)src->imageData[k];
        r[i][j] = (uchar)src->imageData[k];
      } // for
    } // for
  } // if
  else if ( src -> nChannels == 3 )
  {
    for ( int i = 0 ; i < src -> height ; i++ )
    {
      if ( ( k + 3 ) % src -> widthStep == 0 ) k += 3;
      for ( int j = 0 ; j < src -> width ; j++, k += 3 )
      {

        b[i][j] = (uchar)src->imageData[k];
        g[i][j] = (uchar)src->imageData[k+1];
        r[i][j] = (uchar)src->imageData[k+2];
      } // for
    } // for
  } // else if
  else cout << "Number of channel error!!" << endl;

} // Cvt1dto2d()
//*******************************************************************
void Cvt2dto1d( int **r, int **g, int **b, IplImage *dst ){
	// �o��function�Ψӧ�T�ӳq�쪺��assign�^�@�i�v��

  int k = 0;
  if ( dst -> nChannels == 3 )
  {
    for ( int i = 0 ; i < dst -> height ; i++ )
    {
      if ( ( k + 3 ) % dst -> widthStep == 0 ) k += 3;
      for ( int j = 0 ; j < dst -> width ; j++, k += 3 )
      {
        dst->imageData[k] = b[i][j];
        dst->imageData[k+1] = g[i][j];
        dst->imageData[k+2] = r[i][j];
      } // for
    } // for
  } // if
  else cout << "Number of channel error!!" << endl;

} // Cvt2dto1d()
//*******************************************************************