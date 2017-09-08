#ifndef SUBIMAGESDIALOG_H
#define SUBIMAGESDIALOG_H

#include <QDialog>
#include <QFileDialog>

#define IMG_SCALE 300

namespace Ui {
class SubImagesDialog;
}

class SubImagesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubImagesDialog(QWidget *parent = 0);
    ~SubImagesDialog();

    QString leftImage() const;
    QString rightImage() const;

private slots:
    void on_buttonOpenLeftImage_clicked();
    void on_buttonOpenRightImage_clicked();

private:
    Ui::SubImagesDialog *ui;
    QString m_leftImage;
    QString m_rightImage;
};

#endif // SUBIMAGESDIALOG_H
