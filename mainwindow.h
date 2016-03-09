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

private:

    Ui::MainWindow *ui;

    void on_select_images(Files& list_file);
    std::string getString(const std::string name_lineedit);
    void update_viewer_list(const int index);

    QStringList list_image_formats;

    Files file_calibration;
    Files file_distorted;
    Files file_undistorted;
    int flag_images;

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
