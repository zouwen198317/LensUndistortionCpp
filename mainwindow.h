#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>

#include "LensUndistortion.h"

namespace Ui {
class MainWindow;
}

struct Files
{
    Files(){}
    ~Files(){}

    void clear(void){dir = ""; files.clear();}
    void setDir(const std::string _dir){dir = _dir;}
    void append(const std::string file){files.push_back(file);}
    bool valid(void) const {return files.size() != 0? true : false;}

    std::string dir;
    std::vector<std::string> files;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateResultName(void);

private slots:
    void on_select_calibration_images_clicked();
    void on_select_undistortion_images_clicked();

    void on_trigger_calibration_clicked();
    void on_trigger_undistortion_clicked();

    void on_text_undistortion_prefix_textEdited(void);
    void on_text_undistortion_suffix_textEdited(void);
    void on_text_undistortion_original_textEdited(void);

    void on_info_viewer_image_activated(int index);

    void on_radio_viewer_input_clicked();
    void on_radio_viewer_output_clicked();

private:

    Ui::MainWindow *ui;

    void on_select_images(Files& list_file);
    std::string getString(const std::string name_lineedit);
    void update_viewer_list(const int index);
    bool on_trigger(
        const std::string error_message,
        const int flag_src,
        const int flag_dst,
        const std::string str_prefix,
        const std::string str_suffix
    );

    void GenerateResultName(
        const Files file_src,
        const std::string str_prefix,
        const std::string str_suffix,
        Files& file_dst
    );
    QStringList list_image_formats;

    Files file_calibration;
    Files file_detection;
    Files file_distorted;
    Files file_undistorted;
    std::vector<Files> file_images;
    int flag_images;
    bool flag_calibration;
    bool flag_undistortion;

    LensUndistortion engine;
};

void decomposeQstringBaseExtension(
    const QString str,
    std::string& str_basename,
    std::string& str_extension
);

void decomposeQstringBaseExtension(
    const std::string str,
    std::string& str_basename,
    std::string& str_extension
);

#endif // MAINWINDOW_H
