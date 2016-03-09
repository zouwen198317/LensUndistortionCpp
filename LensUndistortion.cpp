
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>

#include <LensUndistortion.h>

std::string addPrefixPostfix(
    const std::string& str_original,
    const std::string& str_prefix,
    const std::string& str_postfix
)
{
    size_t pos = str_original.rfind('.');
    if(pos == std::string::npos)
    {
        return str_prefix + str_original + str_postfix;
    }
    std::string str_stem = str_original.substr(0, pos);
    std::string str_extension = str_original.substr(pos, str_original.size()-pos);
    return str_prefix + str_stem + str_postfix + str_extension;
}

LensUndistortion::LensUndistortion():
    calibration_matrix(cv::Mat::eye(3, 3, CV_64F)),
    distortion_coefficients(cv::Mat::zeros(8, 1, CV_64F))
{
}

LensUndistortion::~LensUndistortion()
{
    calibration_matrix.release();
    distortion_coefficients.release();
}

bool LensUndistortion::calibration(
    const std::string& str_dir,
    const std::vector<std::string>& str_images,
    const int number_of_rows,
    const int number_of_columns,
    const float pattern_size
)
{
    size_t number_of_images = str_images.size();
    int number_of_patterns = number_of_rows*number_of_columns;
    cv::Size grid_size{cv::Size(number_of_rows, number_of_columns)};

    std::vector< std::vector<cv::Point3f> > points_object{1};
    std::vector< std::vector<cv::Point2f> > points_image{number_of_images};
    std::vector<bool> flag_pattern_found(number_of_images, false);

    // Step 1: corner detection
    std::cout << "    Corner detection:" << std::endl;
    cv::Size image_size;
    for(size_t n = 0; n < number_of_images; ++n)
    {
        std::cout << "    image" << n << "...";

        // load an image
        std::string file_image = str_dir + str_images[n];
        cv::Mat img = cv::imread(file_image.c_str(), 0);
        if(n == 0)
        {
            image_size = img.size();
        }

        // detect a set of corners
        std::vector<cv::Point2f> corners;
        flag_pattern_found[n] = cv::findChessboardCorners(img, grid_size, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK);
        std::cout << corners.size() << "/" << number_of_patterns << " corners detected ";
        if(flag_pattern_found[n])
        { // refine the detected corners
            std::cout << "successed, and refined";
            cv::cornerSubPix(img, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
        }
        else
        {
            cv::Mat imgDraw = cv::imread(file_image.c_str(), 1);
            cv::drawChessboardCorners(imgDraw, grid_size, corners, flag_pattern_found[n]);
            cv::imwrite((addPrefixPostfix(file_image, "", "_corners")).c_str(), imgDraw);
            std::cout << "failed";
        }
        std::cout << std::endl;

        // store the detected corners
        points_image[n] = corners;
    }

    // Step 2: remove images on which the chessboard cannot be detected.
    for(int n = number_of_images-1; n >= 0; --n)
    {
        if(!flag_pattern_found[n])
        {
            points_image.erase(points_image.begin()+n);
        }
    }
    number_of_images = points_image.size();
    if(number_of_images < 4)
    {
        return false;
    }

    // Step 3: parameter estimation
    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;
    points_object[0] = compute_chessboard_corners(grid_size, pattern_size);
    points_object.resize(points_image.size(), points_object[0]);
    double rmse = cv::calibrateCamera(points_object, points_image, image_size, calibration_matrix, distortion_coefficients, rvecs, tvecs);
    std::cout << "    RMSE = " << rmse << " with " << number_of_images << "/" << str_images.size() << "images" << std::endl << std::endl;

    return true;
}

std::vector<cv::Point3f> LensUndistortion::compute_chessboard_corners(
    cv::Size grid_size,
    float pattern_size
)
{
    std::vector<cv::Point3f> corners;

    for(int i = 0; i < grid_size.height; ++i)
    {
        for(int j = 0; j < grid_size.width; ++j)
        {
            corners.push_back(
                cv::Point3f(
                    float(j*pattern_size),
                    float(i*pattern_size),
                    0.0f
                )
            );
        }
    }

    return corners;
}

bool LensUndistortion::undistortion(
    const std::string& str_dir,
    const std::vector<std::string>& str_images_src,
    const std::vector<std::string>& str_images_dst
)
{
    size_t number_of_images = str_images_src.size();
    for(size_t n = 0; n < number_of_images; ++n)
    {
        std::cout << "    lens undistortion on " << n << "-th image..." << std::endl;

        // load a distorted image
        std::string file_distorted = str_dir + str_images_src[n];
        cv::Mat img_distorted = cv::imread(file_distorted.c_str(), -1);

        // undistort the distorted image
        cv::Mat img_undistorted{img_distorted.clone()};
        cv::undistort(img_distorted, img_undistorted, calibration_matrix, distortion_coefficients);

        // save the undistorted image
        std::string file_undistorted = str_dir + str_images_dst[n];
        cv::imwrite(file_undistorted.c_str(), img_undistorted);
    }
    return true;
}

std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> strs;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        strs.push_back(item);
    }
    return strs;
}


    bool UndistortionData::load_grid(const cv::FileNode& fn)
    {
        fn["points_per_row"] >> grid_size.width;
        fn["points_per_column"] >> grid_size.height;
        fn["grid_length"] >> grid_length;
    }

    bool UndistortionData::load_calibration_data(const cv::FileNode& fn)
    {
        fn["dir"] >> dir_calib;
        file_calib = split(std::string(fn["images"]), ' ');

        return true;
    }
    bool UndistortionData::load_distorted_data(const cv::FileNode& fn)
    {
        fn["dir"] >> dir_distorted;
        file_distorted = split(std::string(fn["images"]), ' ');

        return true;
    }

    bool UndistortionData::load_undistorted_data(const cv::FileNode& fn)
    {
        fn["dir"] >> dir_undistorted;
        fn["prefix"] >> file_undistorted_prefix;
        fn["postfix"] >> file_undistorted_postfix;
        file_undistorted.clear();
        for(size_t n = 0; n < file_distorted.size(); ++n)
        {
            file_undistorted.push_back(
                file_undistorted_prefix +
                file_distorted[n] +
                file_undistorted_postfix
            );
        }

        return true;
    }

bool load_undistortion_data(
    const std::string file_config,
    UndistortionData& data_undistortion
)
{
    // check whether the configuration file exists
    cv::FileStorage fs(file_config, cv::FileStorage::READ);
    if(!fs.isOpened())
    {
        std::cout << "Could not open the XML file " << file_config << std::endl;
        return false;
    }

    // load all information related to lens undistortion
    cv::FileNode fn = fs["undistortion_data"];
    // grid information
    if(!data_undistortion.load_grid(fn["grid"]))
    {
        std::cout << "  grid data information has some problem!" << std::endl;
        return false;
    }

    // calibration images
    if(!data_undistortion.load_calibration_data(fn["calibration_data"]))
    {
        std::cout << "  calibration data information has some problem!" << std::endl;
        return false;
    }

    // distorted images
    if(!data_undistortion.load_distorted_data(fn["distorted_data"]))
    {
        std::cout << "  distorted data information has some problem!" << std::endl;
        return false;
    }

    // undistorted images
    if(!data_undistortion.load_undistorted_data(fn["undistortion"]))
    {
        std::cout << "  undistorted data information has some problem!" << std::endl;
        return false;
    }

    // release the configuration file.
    fs.release();

    return true;
}

//std::vector<cv::Point3f> compute_chessboard_corners(
//    cv::Size grid_size,
//    float grid_length
//)
//{
//    std::vector<cv::Point3f> corners;

//    for(int i = 0; i < grid_size.height; ++i)
//    {
//        for(int j = 0; j < grid_size.width; ++j)
//        {
//            corners.push_back(
//                cv::Point3f(
//                    float(j*grid_length),
//                    float(i*grid_length),
//                    0.0f
//                )
//            );
//        }
//    }

//    return corners;
//}

//bool calibrate_camera(
//    const std::string& dir_images,
//    const std::vector<std::string>& file_images,
//    const cv::Size& grid_size,
//    const float grid_length,
//    calibration_data& parameters
//)
//{
//    size_t number_of_images = file_images.size();

//    std::vector< std::vector<cv::Point3f> > points_object{1};
//    std::vector< std::vector<cv::Point2f> > points_image{number_of_images};
//    std::vector<bool> flag_pattern_found(number_of_images, false);

//    // Step 1: corner detection
//    std::cout << "    Corner detection:" << std::endl;
//    cv::Size image_size;
//    for(size_t n = 0; n < number_of_images; ++n)
//    {
//        std::cout << "    image" << n << "...";

//        // load an image
//        std::string file_image = dir_images + "/" + file_images[n];
//        cv::Mat img = cv::imread(file_image.c_str(), 0);
//        if(n == 0)
//        {
//            image_size = img.size();
//        }

//        // detect a set of corners
//        std::vector<cv::Point2f> corners;
//        flag_pattern_found[n] = cv::findChessboardCorners(img, grid_size, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK);
//        std::cout << corners.size() << "/" << grid_size.height*grid_size.width << " corners detected ";
//        if(flag_pattern_found[n])
//        { // refine the detected corners
//            std::cout << "successed, and refined";
//            cv::cornerSubPix(img, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
//        }
//        else
//        {
//            cv::Mat imgDraw = cv::imread(file_image.c_str(), 1);
//            cv::drawChessboardCorners(imgDraw, grid_size, corners, flag_pattern_found[n]);
//            cv::imwrite((addPrefixPostfix(file_image, "", "_corners")).c_str(), imgDraw);
//            std::cout << "failed";
//        }
//        std::cout << std::endl;

//        // store the detected corners
//        points_image[n] = corners;
//    }

//    // Step 2: remove images on which the chessboard cannot be detected.
//    for(int n = number_of_images-1; n >= 0; --n)
//    {
//        if(!flag_pattern_found[n])
//        {
//            points_image.erase(points_image.begin()+n);
//        }
//    }
//    number_of_images = points_image.size();
//    if(number_of_images < 4)
//    {
//        return false;
//    }

//    // Step 3: parameter estimation
//    std::vector<cv::Mat> rvecs;
//    std::vector<cv::Mat> tvecs;
//    points_object[0] = compute_chessboard_corners(grid_size, grid_length);
//    points_object.resize(points_image.size(), points_object[0]);
//    double rmse = cv::calibrateCamera(points_object, points_image, image_size, parameters.calibration_matrix, parameters.distortion_coefficients, rvecs, tvecs);
//    std::cout << "    RMSE = " << rmse << " with " << number_of_images << "/" << file_images.size() << "images" << std::endl << std::endl;

//    return true;
//}

void undistort_images(const calibration_data& parameters, const UndistortionData& data_undistortion)
{
    std::string dir_distorted = data_undistortion.dir_distorted;
    std::string dir_undistorted = data_undistortion.dir_undistorted;
    std::string prefix = data_undistortion.file_undistorted_prefix;
    std::string postfix = data_undistortion.file_undistorted_postfix;

    size_t number_of_images = data_undistortion.file_distorted.size();
    for(size_t n = 0; n < number_of_images; ++n)
    {
        std::cout << "    lens undistortion on " << n << "-th image..." << std::endl;

        // load a distorted image
        std::string file_distorted = dir_distorted + "/" + data_undistortion.file_distorted[n];
        cv::Mat img_distorted = cv::imread(file_distorted.c_str(), -1);

        // undistort the distorted image
        cv::Mat img_undistorted{img_distorted.clone()};
        cv::undistort(img_distorted, img_undistorted, parameters.calibration_matrix, parameters.distortion_coefficients);

        // save the undistorted image
        std::string file_undistorted = dir_undistorted + "/" + addPrefixPostfix(data_undistortion.file_distorted[n], prefix, postfix);
        cv::imwrite(file_undistorted.c_str(), img_undistorted);
    }
}
