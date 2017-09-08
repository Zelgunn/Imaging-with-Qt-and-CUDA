#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt + Cuda");
    setWindowState(Qt::WindowMaximized);

    ui->graphicsView->setScene(new QGraphicsScene);
    m_image = new QGraphicsPixmapItem;
    ui->graphicsView->scene()->addItem(m_image);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QGraphicsPixmapItem *MainWindow::image() const
{
    return m_image;
}

void MainWindow::setImage(const QImage &image)
{
    m_image->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::setImage(const QString &filename)
{
    setImage(QImage(filename));
}

void MainWindow::on_actionSoustraire_images_triggered()
{
    SubImagesDialog simgDialog(this);
    int ret = simgDialog.exec();
    if(ret==0)
        return;

    QImage *leftImage = new QImage(simgDialog.leftImage());
    QImage *rightImage = new QImage(simgDialog.rightImage());

    QImage *image = ImageSiftF::subImages(leftImage, rightImage);
    setImage(*image);

    delete leftImage;
    delete rightImage;
    delete image;
}

void MainWindow::on_actionFlou_gaussien_triggered()
{
    QImage *image = new QImage(m_image->pixmap().toImage());
    image = ImageSiftF::blurImage(image);
    setImage(*image);

    delete image;
}

void MainWindow::on_actionOuvrir_image_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Ouvrir Image"),"",tr("Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));
    if(filename.isEmpty()) return;

    QImage image(filename);
    setImage(image);
}

void MainWindow::on_actionSauvegarder_image_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Ouvrir Image"),"",tr("Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));
    if(filename.isEmpty()) return;

    QImage image(m_image->pixmap().toImage());
    image.save(filename);
}

void MainWindow::on_actionDoG_triggered()
{
    QImage *image = new QImage(m_image->pixmap().toImage());

    image = ImageSiftF::imageDoG(image);
    setImage(*image);

    delete image;
}

void MainWindow::on_actionPyramide_de_diff_rences_de_gaussiennes_triggered()
{
    QImage image(m_image->pixmap().toImage());
    ImageSiftF imageSift(image);

    QList<QList<QImage *> > pyramid = imageSift.computeDoGPyramid();
    QList<QList<QImage *> > p2 = imageSift.LoGPyramid();
    QList<QImage *> octav;
    QList<QImage *> o2;

    for(int i=0; i<pyramid.size(); i++)
    {
        octav = pyramid.at(i);
        o2 = p2.at(i);
        for(int j=0; j<octav.size(); j++)
        {
            octav.at(j)->save(QString("DoG_").append(QString::number(i*10+j).append(".png")));
            o2.at(j)->save(QString("LoG_").append(QString::number(i*10+j).append(".png")));
        }
    }
}

void MainWindow::on_actionKeyPoints_triggered()
{
    QImage image(m_image->pixmap().toImage());
    ImageSiftF sift(image);
    sift.computeKeyPoints();

    QList<SiftDescriptor> descriptors = sift.descriptors();
    SiftDescriptor descriptor;
    int scale, octav;
    QPoint tmpPoint;
    QPainter painter(&image);

    QColor color;
    for(int i = 0; i<descriptors.size(); i++)
    {
        descriptor = descriptors.at(i);
        scale = descriptor.scale();
        octav = descriptor.octav();
        tmpPoint = QPoint(descriptor.x(), descriptor.y());
        color.setRed(255 * (scale%2));
        color.setGreen(255 * (((scale + octav)/2)%2));
        color.setBlue(255 * (((scale * octav)/4)%2));

        painter.setPen(QPen(color));
        painter.drawEllipse(tmpPoint, 5, 5);

//        QLineF line;

//        for(int orientation = 0; orientation < 36; orientation ++)
//        {
//            if(descriptor.histogrammeAt(orientation))
//            {
//                line.setP1(QPointF(descriptor.x(), descriptor.y()));
//                line.setAngle((orientation*2)/9 * 45);
//                line.setLength(10 * descriptor.histogrammeAt(orientation));
//                painter.drawLine(line);
//            }
//        }
    }
    setImage(image);
}
