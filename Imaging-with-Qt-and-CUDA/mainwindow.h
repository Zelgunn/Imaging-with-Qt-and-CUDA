#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QPainter>

#include "subimagesdialog.h"
#include "imagesiftf.h"

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QGraphicsPixmapItem *image() const;
    void setImage(const QImage &image);
    void setImage(const QString &filename);

private slots:
    void on_actionSoustraire_images_triggered();
    void on_actionFlou_gaussien_triggered();
    void on_actionOuvrir_image_triggered();
    void on_actionSauvegarder_image_triggered();
    void on_actionDoG_triggered();
    void on_actionPyramide_de_diff_rences_de_gaussiennes_triggered();
    void on_actionKeyPoints_triggered();

private:
    Ui::MainWindow *ui;
    QGraphicsPixmapItem *m_image;
};

#endif // MAINWINDOW_H
