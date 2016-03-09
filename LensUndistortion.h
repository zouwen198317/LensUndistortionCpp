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

///
/// \brief split splits a std::string object \c str into a std::vector of std::string object with a delimiter \c delim.
/// \param str The input string file containing some delimiters
/// \param delim The delimiter used for string split.
/// \return The vector of splited string.
///
std::vector<std::string> split(const std::string &str, char delim);

///
/// \brief The UndistortionData struct stores information, which is loaded from an XML file, required for lens undistortion.
/// The grid information means that we have grid_size.width x grid_size.height corners, each of which is a intersection of rectangle with grid_length size.
///
struct UndistortionData{
    //! Default constructor
    UndistortionData(){}
    //! Destructor
    ~UndistortionData(){}

    bool load_grid(const cv::FileNode& fn);

    bool load_calibration_data(const cv::FileNode& fn);
    bool load_distorted_data(const cv::FileNode& fn);

    bool load_undistorted_data(const cv::FileNode& fn);

    cv::Size grid_size;    //!< The number of corners on the grid.
    float grid_length;    //!< The length of a grid.
    std::string dir_calib;  //!< name of the directory where calibration images are stored.
    std::string dir_distorted;  //!< name of the directory where distorted images are stored.
    std::string dir_undistorted;  //!< name of the directory where undistorted images will be stored.
    std::vector<std::string> file_calib;    //!< a set of calibration images' filename.
    std::vector<std::string> file_distorted;    //!< a set of distorted images' filename.
    std::vector<std::string> file_undistorted;    //!< a set of undistorted images' filename.
    std::string file_undistorted_prefix;    //!< prefix of undistorted images.
    std::string file_undistorted_postfix;   //!< postfix of undistorted images.
};

///
/// \brief The calibration_data struct stores calibration result, 3x3 calibration matrix and a lens distortion coefficients as a vector.
///
struct calibration_data{
    calibration_data():
        calibration_matrix(cv::Mat::eye(3, 3, CV_64F)),
        distortion_coefficients(cv::Mat::zeros(8, 1, CV_64F))
    {}
    ~calibration_data(){}

    cv::Mat calibration_matrix; //!< 3x3 calibration matrix, intrinsic parameters in other word.
    cv::Mat distortion_coefficients;    //!< lens distortion coefficients as a vector.
};

///
/// \brief load_undistortion_data loads information required for lens undistortion from an XML file.
/// \param file_config The name of the XML file.
/// \param data_undistortion The loaded configuration.
/// \return true if the configuration load is succeedded and false otherwise.
///
bool load_undistortion_data(
    const std::string file_config,
    UndistortionData& data_undistortion
);

//std::vector<cv::Point3f> compute_chessboard_corners(
//    cv::Size grid_size,
//    float grid_length
//);

//bool calibrate_camera(
//    const std::string& dir_images,
//    const std::vector<std::string>& file_images,
//    const cv::Size& grid_size,
//    const float grid_length,
//    calibration_data& parameters
//);

///
/// \brief undistort_images removes lens distortion effects from a set of distorted images.
/// \param parameters   Calibration parameters, calibration matrix and lens distortion coefficients.
/// \param data_undistortion    All data related to lens undistortion.
///
void undistort_images(const calibration_data& parameters, const UndistortionData& data_undistortion);

// int main(int argc, char* argv[])
// {
//     // Step 0: set the name of configuration files
//     std::vector<std::string> file_config(argc-1);
//     if(argc > 1)
//     {
//         for(int n = 1; n < argc; ++n)
//         {
//             file_config[n-1] = argv[n];
//         }
//     }
//     else
//     {
//         file_config[0] = "../single.xml";
//     }

//     // run lens undistortion for each configuration
//     for(int n = 0; n < file_config.size(); ++n)
//     {
//         std::cout << "Lens undistortion with " << file_config[n] << std::endl;
//         calibration_data parameters;

//         // Step 1: load configuration
//         std::cout << "Step 1/3: loading configuration...";
//         UndistortionData data_undistortion;
//         if(!load_undistortion_data(file_config[n], data_undistortion))
//         {
//             goto ERROR_LOAD;
//         }
//         std::cout << "    done" << std::endl;

//         // Step 2: calibrate the camera
//         std::cout << "Step 2/3: calibrating camera..." << std::endl;
//         if(!calibrate_camera(data_undistortion.dir_calib, data_undistortion.file_calib, data_undistortion.grid_size, data_undistortion.grid_length, parameters))
//         {
//             goto ERROR_CALIB;
//         }
//         std::cout << "Calibration done" << std::endl;

//         // Step 3: undistort images
//         std::cout << "Step 3/3: undistorting images...";
//         undistort_images(parameters, data_undistortion);
//         std::cout << " done" << std::endl;
//         goto ERROR_NONE;

//         ERROR_LOAD:
//             std::cout << "failed to load the configuration file " <<  file_config[n] << std::endl;
//         ERROR_CALIB:
//             std::cout << "failed to calibrate the camera" << std::endl;
//         ERROR_NONE:
//             std::cout << "successed " << file_config[n] << std::endl;
//     }
//     return 0;
// }

#endif // __LENS_UNDISTORTION_H__
