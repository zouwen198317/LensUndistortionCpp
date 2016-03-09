#include <iostream>
#include <string>

#include <QtWidgets>

#include "ui_mainwindow.h"
#include "mainwindow.h"

const static std::string name_format_all("ALL IMAGE FILE (*.bmp *.png *.jpg *.jpeg *.tiff *.xpm *.xbm *.ppm)");

void decomposeQstringBaseExtension(
    const QString str,
    std::string& str_basename,
    std::string& str_extension
)
{
    QFileInfo file_info(str);
    str_basename = file_info.baseName().toStdString();
    str_extension = file_info.suffix().toStdString();
}

void decomposeQstringBaseExtension(
    const std::string str,
    std::string& str_basename,
    std::string& str_extension
)
{
    QString qstr(str.c_str());
    decomposeQstringBaseExtension(qstr, str_basename, str_extension);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set a list of image formats
    list_image_formats += tr("ALL IMAGE FILE (*.bmp *.png *.jpg *.jpeg *.tiff *.xpm *.xbm *.ppm)");
    list_image_formats += tr("Windows Bitmap (*.bmp)");
    list_image_formats += tr("PNG (*.png)");
    list_image_formats += tr("JPEG (*.jpg *.jpeg)");
    list_image_formats += tr("TIFF (*.tiff)");
    list_image_formats += tr("XPM (*.xpm)");
    list_image_formats += tr("XBM (*.xbm)");
    list_image_formats += tr("PPM (*.ppm)");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_select_images(Files& list_file)
{
    // set file dialog
    QFileDialog dialog(this);
    dialog.setDirectory(QDir::currentPath()); // open the current directory
    dialog.setFileMode(QFileDialog::ExistingFiles); // enable to select multiple files
    dialog.setNameFilters(list_image_formats); // set the filter for file formats
    dialog.selectNameFilter(list_image_formats[0]); // set default file format

    // retrieve the filenames

    QComboBox *combobox = this->findChild<QComboBox*>("info_viewer_image");
    combobox->clear();
    list_file.clear();
    QStringList files;
    if(dialog.exec())
    {
        files = dialog.selectedFiles();
    }
    foreach(QString file, files)
    {
        QFileInfo file_info(file);
        list_file.setDir(file_info.dir().absolutePath().toStdString() + "/");
        list_file.append(file_info.fileName().toStdString());
        combobox->addItem(file_info.fileName());
    }
    combobox->setEditable(true);
}

void MainWindow::on_select_calibration_images_clicked()
{
    on_select_images(file_calibration);
    flag_images = 0;
    on_info_viewer_image_activated(0);
}

void MainWindow::on_select_undistortion_images_clicked()
{
    on_select_images(file_distorted);
    flag_images = 1;
    on_info_viewer_image_activated(0);
}

void MainWindow::on_trigger_calibration_clicked()
{
    if(!file_calibration.valid())
    {
        QMessageBox msgError;
        msgError.setText("Please select calibration images");
        msgError.exec();
    }

    // run calibration
    engine.calibration(
        file_calibration.dir,
        file_calibration.files,
        this->findChild<QSpinBox*>("info_calibration_rows")->text().toInt(),
        this->findChild<QSpinBox*>("info_calibration_columns")->text().toInt(),
        this->findChild<QDoubleSpinBox*>("info_calibration_size")->text().toFloat()
    );
}


void MainWindow::on_trigger_undistortion_clicked()
{
    if(!file_distorted.valid())
    {
        QMessageBox msgError;
        msgError.setText("Please select distorted images");
        msgError.exec();
    }

    // retrieve all text information
    file_undistorted.clear();
    file_undistorted.dir = file_distorted.dir;
    std::string str_prefix = getString("text_undistortion_prefix");
    std::string str_suffix = getString("text_undistortion_suffix");
    std::string str_base, str_extension;
    for(size_t n = 0; n < file_distorted.files.size(); ++n)
    {
        decomposeQstringBaseExtension(file_distorted.files[n], str_base, str_extension);
        std::string str_result = str_prefix + str_base + str_suffix + "." + str_extension;
        file_undistorted.append(str_result);
    }

    // run undistortion
    engine.undistortion(
        file_distorted.dir,
        file_distorted.files,
        file_undistorted.files
    );

    flag_images = 2;
    on_info_viewer_image_activated(0);
}

void MainWindow::on_text_undistortion_prefix_textEdited(void)
{
    updateResultName();
}

void MainWindow::on_text_undistortion_suffix_textEdited(void)
{
    updateResultName();
}

void MainWindow::on_text_undistortion_original_textEdited(void)
{
    updateResultName();
}

std::string MainWindow::getString(const std::string name_lineedit)
{
    QLineEdit *lineedit = this->findChild<QLineEdit*>(name_lineedit.c_str());
    return lineedit->text().toStdString();
}

void MainWindow::updateResultName(void)
{
    // retrieve all text information
    std::string str_prefix = getString("text_undistortion_prefix");
    std::string str_suffix = getString("text_undistortion_suffix");
    std::string str_original = getString("text_undistortion_original");
    std::string str_base, str_extension;
    decomposeQstringBaseExtension(str_original, str_base, str_extension);
    std::string str_result = str_prefix + str_base + str_suffix + "." + str_extension;

    // set the result name
    QLineEdit *line_result = this->findChild<QLineEdit*>("text_undistortion_result");
    line_result->setText(str_result.c_str());

    // adjust LineEdit's size
    QFontMetrics fm = line_result->fontMetrics();
    int width = fm.boundingRect(line_result->text()).width()+10;
    line_result->resize(width, line_result->height());
}

void MainWindow::on_info_viewer_image_activated(int index)
{
    std::string file_image;
    switch(flag_images)
    {
    case 0:
        file_image = file_calibration.dir + file_calibration.files[index];
        break;
    case 1:
        file_image = file_distorted.dir + file_distorted.files[index];
        break;
    case 2:
        file_image = file_undistorted.dir + file_undistorted.files[index];
        break;
    default:
        file_image = "";
    }
    QLabel *imageLabel = this->findChild<QLabel*>("label_viewer_view");

    QImage image(file_image.c_str());
    if(image.isNull())
    {
        imageLabel->setText("No available image");
    }
    else
    {
        imageLabel->setPixmap(QPixmap::fromImage(image));
        imageLabel->setScaledContents(true);
    }
}
