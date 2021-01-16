#pragma once

#ifdef COMPILE_WITH_EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
//#include <Utilities/Image.h>

namespace SCFUSION {
    template<typename T, int x>
    Eigen::Map<Eigen::Matrix<T, x, 1>> getEigenVector(T *data){
        return Eigen::Map<Eigen::Matrix<T, x, 1>>(data);
    }

    // T: data type. can be float, x: the dimension.
    template<typename T, int x>
    Eigen::Map<Eigen::Matrix<T, x, x, Eigen::RowMajor>> getEigenRowMajor(T *data){
        return Eigen::Map<Eigen::Matrix<T, x, x, Eigen::RowMajor>>(data);
    }

    template<typename T, int x>
    Eigen::Map<const Eigen::Matrix<T, x, x, Eigen::RowMajor>> getEigenRowMajor(const T *data){
        return Eigen::Map<const Eigen::Matrix<T, x, x, Eigen::RowMajor>>(data);
    }
    
//    template<int x>
//    Eigen::Map<Eigen::Matrix<float, x, x, Eigen::RowMajor>> getEigenRowMajor(float *data){
//        return Eigen::Map<Eigen::Matrix<float, x, x, Eigen::RowMajor>>(data);
//    }
    
    inline Eigen::Map<Eigen::Matrix4f> getEigen4f(float *data){
        return Eigen::Map<Eigen::Matrix4f>(data);
    }
    inline Eigen::Map<Eigen::Matrix3f> getEigen3f(float *data){
        return Eigen::Map<Eigen::Matrix3f>(data);
    }
    
    inline Eigen::AngleAxisf getAxisAngle(const Eigen::Matrix3f &rotmatrix){
        Eigen::AngleAxisf newAngleAxis(rotmatrix);
        return newAngleAxis;
    }
}
#endif