#ifndef _HAS_AUTO_PTR_ETC
#define _HAS_AUTO_PTR_ETC 1
#endif

#include "KiraFan.h"
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "../../src/Path.h"

using namespace cv;
using namespace std;

void overlayImage(const Mat &background, const Mat &foreground, Mat &output, Point2i location)
{
	background.copyTo(output);


	// start at the row indicated by location, or at row 0 if location.y is negative.
	for (int y = max(location.y, 0); y < background.rows; ++y)
	{
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if (fY >= foreground.rows)
			break;

		// start at the column indicated by location,

		// or at column 0 if location.x is negative.
		for (int x = max(location.x, 0); x < background.cols; ++x)
		{
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if (fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity =
				((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

				/ 255.;

			// and now combine the background and foreground pixel, using the opacity,

			// but only if opacity > 0.
			for (int c = 0; opacity > 0 && c < output.channels(); ++c)
			{
				unsigned char foregroundPx =
					foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx =
					background.data[y * background.step + x * background.channels() + c];
				output.data[y*output.step + output.channels()*x + c] =
					backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}

int DetectChannels(Mat &src, string Path)
{
	if (src.channels() == 4)
	{
		return -1;
	}
	else if (src.channels() == 1)
	{
		cv::cvtColor(src, src, cv::COLOR_GRAY2RGB);
	}

	Mat alpha = Mat(src.rows, src.cols, CV_8UC1);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			alpha.at<uchar>(i, j) = 255;
		}
	}


	vector<Mat> srcChannels;
	vector<Mat> dstChannels;
	//分离通道
	cv::split(src, srcChannels);

	dstChannels.push_back(srcChannels[0]);
	dstChannels.push_back(srcChannels[1]);
	dstChannels.push_back(srcChannels[2]);
	//添加透明度通道
	dstChannels.push_back(alpha);
	//合并通道

	src = Mat(src.rows, src.cols, CV_8UC4);

	merge(dstChannels, src);

	imwrite(Path, src);

	return 0;
}

int GetKiraFanImage(int *Result, bool Get_5,int Count, int RankCount, int64_t fromQQ, const char *GamePath)
{
	string iPath = "";
	Mat Back = imread(iPath.assign(GetPath(Count * 10 + Get_5, 3, GamePath)),-1);
	DetectChannels(Back, iPath);
	string Output = GetPath(fromQQ, 0, GamePath);
	Mat Image;
	Mat Rank;
	ifstream Position(__UTA(CardPositionPath(GamePath)));
	if (!Position)
	{
		Position.close();
		return 0;
	}
	int noTempC = 0, TempC = 0, imgW = 0, imgH = 0, imgPX = 0, imgPY = 0;
	Position >> noTempC;
	Position >> noTempC;
	Position >> TempC;
	Position >> imgW;
	Position >> imgH;
	int *Temp = NULL;
	if ((Count > 1)&&TempC)
		Temp = new int[RankCount*10000]{ 0 };
	for (int i = 0; i < Count; i++)
	{
		Position >> imgPX;
		Position >> imgPY;
		Image = imread(iPath.assign(GetPath(Result[i], 1, GamePath)),-1);
		DetectChannels(Image,iPath);
		resize(Image, Image, Size(imgW, imgH));
		overlayImage(Back, Image, Back, Point(imgPX, imgPY));
		for (int j = 1; j <= noTempC; j++)
		{
			Image = imread(iPath.assign(GetPath(Result[i], 2, GamePath, "_" + to_string(j))), -1);
			DetectChannels(Image, iPath);
			resize(Image, Image, Size(imgW, imgH));
			overlayImage(Back, Image, Back, Point(imgPX, imgPY));
		}
		for (int j = 1; j <= TempC; j++)
		{
			if (Temp[Result[i]])
			{
				Image = imread(iPath.assign(GetPath(Result[i], 2, GamePath, "_T" + to_string(j))), -1);
				DetectChannels(Image, iPath);
				resize(Image, Image, Size(imgW, imgH));
				overlayImage(Back, Image, Back, Point(imgPX, imgPY));
			}
		}
		if (TempC)
			Temp[Result[i]] += 1;
	}
	Position.close();
	if (!Temp)
		delete[] Temp;
	imwrite(Output, Back);
	return 1;
}

//mode 0-get output image path;mode 1-get result image path;mode 2-get rank image path;mode 3-get back image path;
string GetPath(int64_t Result, int mode, const char *GamePath, string etc)	
{
	string Num = to_string(Result);
	if (!mode)
	{
		string Path = ImageDataPath.append(JPath["GachaImage"]).append(GamePath).append("\\");
		DetectFolder(Path);
		return Path.append(Num).append(JPath["Gachas"]["outpicexp"]);
	}
	else
	{
		string Path = CardImagePath.append(GamePath).append("\\");
		if (mode == 1)
			return Path.append(Num).append(JPath["Gachas"]["outpicexp"]);
		else if (mode == 3)
			return Path.append(JPath["Gachas"]["backpicpre"]).append(Num).append(JPath["Gachas"]["outpicexp"]);
		else
		{
			char Rank[16] = { 0 };
			fstream Card(__UTA(Path + Num + etc), ios::in);
			Card.getline(Rank, sizeof(Rank) - 1, '\n');
			Card.close();
			return Path.append(Rank).append(JPath["Gachas"]["outpicexp"]);
		}
	}
}