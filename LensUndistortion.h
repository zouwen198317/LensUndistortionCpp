#ifndef __LENS_UNDISTORTION_H__
#define __LENS_UNDISTORTION_H__

#include <iostream>
#include <sstream>

#include <opencv2/core.hpp>
//#include <opencv2/core/utility.hpp>

std::string addPrefixPostfix(
        const std::string& str_original,
        const std::string& str_prefix,
        const std::string& str_postfix
);

class LensUndistortion{
public:
    LensUndistortion();
    ~LensUndistortion();

    ///
    /// \brief calibration run calibration given the calibration information.
    /// \param dir_images   The name of a directory where calibration images are stored.
    /// \param file_images  The name of calibration images.
    /// \param number_of_rows  The number of control points on a row of the calibration pattern.
    /// \param number_of_columns The number of control points on a column of the calibration pattern.
    /// \param pattern_size  The distance between two neighboring patterns.
    /// \return
    ///
    bool calibration(
        const std::string& str_dir,
        const std::vector<std::string>& str_images,
        const int number_of_rows,
        const int number_of_columns,
        const float pattern_size
    );

    bool undistortion(
        const std::string& str_dir,
        const std::vector<std::string>& str_images_src,
        const std::vector<std::string>& str_images_dst
    );

private:
    ///
    /// \brief compute_chessboard_corners computes the 3D position of corners on a chessboard.
    /// \param grid_size    The size of a grid. The chessboard has grid_size.width x grid_size.height corners in total.
    /// \param grid_length  The lengs of a rectangle.
    /// \return
    ///
    std::vector<cv::Point3f> compute_chessboard_corners(
        cv::Size grid_size,
        float pattern_size
    );
public:
    cv::Mat calibration_matrix; //!< 3x3 calibration matrix, intrinsic parameters in other word.
    cv::Mat distortion_coefficients;    //!< lens distortion coefficients as a vector.
};

#endif // __LENS_UNDISTORTION_H__
