#pragma once
#ifdef COMPILE_WITH_OPENCV
#include "Image.h"
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace ORUtils {
    class Image2CVShow{
    public:
         static void show(ORUtils::Image<float> *img, cv::Mat **mat) {
            (*mat)->create(img->noDims.y, img->noDims.x, CV_32FC1);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    (*mat)->at<float>(i, j) = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j];
                }
            }
            double min;
            double max;
            cv::minMaxIdx(*(*mat), &min, &max);
            cv::convertScaleAbs(*(*mat), *(*mat), 255 / max);
        }

        static void show(ORUtils::Image<float> *img, const std::string &name = "image") {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_32FC1);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    mat.at<float>(i, j) = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j];
                }
            }
            double min;
            double max;
            cv::minMaxIdx(mat, &min, &max);
            cv::convertScaleAbs(mat, mat, 255 / max);
            imshow(name, mat);
            cv::waitKey(0);
        }

        static void show(ORUtils::Image<unsigned short> *img, const std::string &name = "image") {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_32FC1);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    mat.at<float>(i, j) = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j];
                }
            }
            double min;
            double max;
            cv::minMaxIdx(mat, &min, &max);
            cv::convertScaleAbs(mat, mat, 255 / max);
            imshow(name, mat);
            cv::waitKey(0);
        }

        static void show(ORUtils::Image<Vector3<float>> *img, const std::string &name = "image") {
            cv::Mat channels[3];
            for (size_t c = 0; c < 3; ++c) {
                cv::Mat &mat = channels[c];
                mat.create(img->noDims.y, img->noDims.x, CV_32FC1);
                for (int i = 0; i < img->noDims.y; ++i) {
                    for (int j = 0; j < img->noDims.x; ++j) {
                        mat.at<float>(i, j) = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j][c];
                    }
                }
                double min;
                double max;
                cv::minMaxIdx(mat, &min, &max);
                cv::convertScaleAbs(mat, mat, 255 / max);
            }
            cv::Mat output(img->noDims.y, img->noDims.x, CV_32FC3);
            cv::merge(channels, 3, output);
            imshow(name, output);
            cv::waitKey(0);
        }

        static void show(ORUtils::Image<Vector4<float>> *img, const std::string &name = "image") {
            cv::Mat channels[3];
            for (size_t c = 0; c < 3; ++c) {
                cv::Mat &mat = channels[c];
                mat.create(img->noDims.y, img->noDims.x, CV_32FC1);
                for (int i = 0; i < img->noDims.y; ++i) {
                    for (int j = 0; j < img->noDims.x; ++j) {
                        mat.at<float>(i, j) = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j][c];
                    }
                }
                double min;
                double max;
                cv::minMaxIdx(mat, &min, &max);
                cv::convertScaleAbs(mat, mat, 255 / max);
            }
            cv::Mat output(img->noDims.y, img->noDims.x, CV_32FC3);
            cv::merge(channels, 3, output);
            imshow(name, output);
            cv::waitKey(0);
        }

        static void show(ORUtils::Image<Vector4<unsigned char>> *img, const std::string &name = "image") {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_8UC4);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    for (size_t c = 0; c < 4; ++c)
                        mat.at<cv::Vec<uchar, 4>>(i, j)[c] = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j][c];
                }
            }
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2RGBA);
//            double min;
//            double max;
//            cv::minMaxIdx(mat, &min, &max);
//            cv::convertScaleAbs(mat, mat, 255 / max);
            imshow(name, mat);
            cv::waitKey(0);
        }
        static void show(const ORUtils::Image<Vector4<unsigned char >> *img, const std::string &name = "image") {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_8UC4);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    for (size_t c = 0; c < 4; ++c)
                        mat.at<cv::Vec<uchar, 4>>(i, j)[c] = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j][c];
                }
            }
            double min;
            double max;
            cv::minMaxIdx(mat, &min, &max);
            cv::convertScaleAbs(mat, mat, 255 / max);
            imshow(name, mat);
            cv::waitKey(0);
        }

        static void show(ORUtils::Image<unsigned char> *img, const std::string &name = "image") {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_8UC4);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    for (size_t c = 0; c < 3; ++c)
                        mat.at<cv::Vec<uchar, 4>>(i, j)[c] = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j];
                    mat.at<cv::Vec<uchar, 4>>(i, j)[3]=255;
                }
            }
            double min;
            double max;
            cv::minMaxIdx(mat, &min, &max);
            cv::convertScaleAbs(mat, mat, 255 / max);
            imshow(name, mat);
            cv::waitKey(0);
        }
    };


    class Image2CvMat {
    public:
        static cv::Mat get(ORUtils::Image<Vector4<unsigned char>> *img) {
            cv::Mat mat(img->noDims.y, img->noDims.x, CV_8UC4);
            for (int i = 0; i < img->noDims.y; ++i) {
                for (int j = 0; j < img->noDims.x; ++j) {
                    for (size_t c = 0; c < 4; ++c)
                        mat.at<cv::Vec<uchar, 4>>(i, j)[c] = img->GetDataConst(MEMORYDEVICE_CPU)[i * img->noDims.x + j][c];
                }
            }
//        double min;
//        double max;
//        cv::minMaxIdx(mat, &min, &max);
//        cv::convertScaleAbs(mat, mat, 255 / max);
            return mat;
        }

    };



}
#endif