#include <iostream>
#include <string>

#include <QtWidgets>

#include "ui_mainwindow.h"
#include "mainwindow.h"

const static std::string name_format_all("ALL IMAGE FILE (*.bmp *.png *.jpg *.jpeg *.tiff *.xpm *.xbm *.ppm)");

enum state : int{
    CALIBRATED,
    DETECTED,
    DISTORTED,
    UNDISTORTED,
    TOTAL
};

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
    ui(new Ui::MainWindow),
    file_images(std::vector<Files>(state::TOTAL)),
    flag_calibration(false),
    flag_undistortion(false),
    flag_images(state::TOTAL)
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

    // set radio button for viewer disabled
    this->findChild<QRadioButton*>("radio_viewer_input")->setEnabled(false);
    this->findChild<QRadioButton*>("radio_viewer_output")->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_viewer_list(const int index)
{
    QComboBox *combobox = this->findChild<QComboBox*>("info_viewer_image");
    combobox->clear();

    for(size_t n = 0; n < file_images[flag_images].files.size(); ++n)
    {
        combobox->addItem(file_images[flag_images].files[n].c_str());
    }
    combobox->setEditable(true);
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
    }
    update_viewer_list(flag_images);
    on_info_viewer_image_activated(0);
}

void MainWindow::on_select_calibration_images_clicked()
{
    flag_images = state::CALIBRATED;
    on_select_images(file_images[state::CALIBRATED]);
    this->findChild<QRadioButton*>("radio_viewer_input")->setEnabled(true);
    this->findChild<QRadioButton*>("radio_viewer_input")->setChecked(true);
}

void MainWindow::on_select_undistortion_images_clicked()
{
    flag_images = state::DISTORTED;
    on_select_images(file_images[state::DISTORTED]);
    this->findChild<QRadioButton*>("radio_viewer_output")->setEnabled(false);
    this->findChild<QRadioButton*>("radio_viewer_input")->setChecked(true);
}

void MainWindow::GenerateResultName(
    const Files file_src,
    const std::string str_prefix,
    const std::string str_suffix,
    Files& file_dst
)
{
    // retrieve all text information
    file_dst.clear();
    file_dst.dir = file_src.dir;
    std::string str_base, str_extension;
    for(size_t n = 0; n < file_src.files.size(); ++n)
    {
        decomposeQstringBaseExtension(file_src.files[n], str_base, str_extension);
        std::string str_result = str_prefix + str_base + str_suffix + "." + str_extension;
        file_dst.append(str_result);
    }
}

bool MainWindow::on_trigger(
    const std::string error_message,
    const int flag_src,
    const int flag_dst,
    const std::string str_prefix,
    const std::string str_suffix
)
{
    // check whether images are selected or not
    if(!file_images[flag_src].valid())
    {
        QMessageBox msgError;
        msgError.setText(error_message.c_str());
        msgError.exec();
        return false;
    }

    // set result name
    GenerateResultName(file_images[flag_src], str_prefix, str_suffix, file_images[flag_dst]);

    // update flag and the state of radio buttons
    flag_images = flag_dst;
    this->findChild<QRadioButton*>("radio_viewer_output")->setEnabled(true);
    this->findChild<QRadioButton*>("radio_viewer_output")->setChecked(true);

    return true;
}

void MainWindow::on_trigger_calibration_clicked()
{
    if(on_trigger("Please select calibration images", state::CALIBRATED, state::DETECTED, "", engine.str_suffix_corners))
    {
        engine.calibration(
            file_images[state::CALIBRATED].dir,
            file_images[state::CALIBRATED].files,
            this->findChild<QSpinBox*>("info_calibration_rows")->text().toInt(),
            this->findChild<QSpinBox*>("info_calibration_columns")->text().toInt(),
            this->findChild<QDoubleSpinBox*>("info_calibration_size")->text().toFloat(),
            this->findChild<QComboBox*>("info_calibration_type")->currentIndex()-1
        );

        on_info_viewer_image_activated(this->findChild<QComboBox*>("info_viewer_image")->currentIndex());
        flag_calibration = true;
    }
}

void MainWindow::on_trigger_undistortion_clicked()
{
    std::string str_prefix = getString("text_undistortion_prefix");
    std::string str_suffix = getString("text_undistortion_suffix");
    if(on_trigger("Please select calibration images", state::DISTORTED, state::UNDISTORTED, str_prefix, str_suffix))
    {
        engine.undistortion(
            file_images[state::DISTORTED].dir,
            file_images[state::DISTORTED].files,
            file_images[state::UNDISTORTED].files
        );

        on_info_viewer_image_activated(this->findChild<QComboBox*>("info_viewer_image")->currentIndex());
        flag_undistortion = true;
    }
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
    QLabel *imageLabel = this->findChild<QLabel*>("label_viewer_view");

    QImage image((file_images[flag_images].dir + file_images[flag_images].files[index]).c_str());
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

void MainWindow::on_radio_viewer_input_clicked()
{
    if(flag_images == state::DETECTED)
    {
        flag_images = state::CALIBRATED;
    }
    else if(flag_images == state::UNDISTORTED)
    {
        flag_images = state::DISTORTED;
    }
    on_info_viewer_image_activated(this->findChild<QComboBox*>("info_viewer_image")->currentIndex());
}

void MainWindow::on_radio_viewer_output_clicked()
{
    if(flag_images == state::CALIBRATED)
    {
        flag_images = state::DETECTED;
    }
    else if(flag_images == state::DISTORTED)
    {
        flag_images = state::UNDISTORTED;
    }
    on_info_viewer_image_activated(this->findChild<QComboBox*>("info_viewer_image")->currentIndex());
}
