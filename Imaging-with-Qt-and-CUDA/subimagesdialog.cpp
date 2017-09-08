#include "subimagesdialog.h"
#include "ui_subimagesdialog.h"

SubImagesDialog::SubImagesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubImagesDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Soustraction d'images");
}

SubImagesDialog::~SubImagesDialog()
{
    delete ui;
}

void SubImagesDialog::on_buttonOpenLeftImage_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Ouvrir Image"),"",tr("Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));
    if(filename.isEmpty()) return;

    m_leftImage = filename;

    ui->labelLeft->setText(filename.section('/',-1));

    QPixmap pixmap(filename);
    if(pixmap.width()>IMG_SCALE) pixmap = pixmap.scaledToWidth(IMG_SCALE);
    if(pixmap.height()>IMG_SCALE) pixmap = pixmap.scaledToHeight(IMG_SCALE);
    ui->leftImage->setPixmap(pixmap);
}

void SubImagesDialog::on_buttonOpenRightImage_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Ouvrir Image"),"",tr("Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));
    if(filename.isEmpty()) return;

    m_rightImage = filename;

    ui->labelRight->setText(filename.section('/',-1));

    QPixmap pixmap(filename);
    if(pixmap.width()>IMG_SCALE) pixmap = pixmap.scaledToWidth(IMG_SCALE);
    if(pixmap.height()>IMG_SCALE) pixmap = pixmap.scaledToHeight(IMG_SCALE);
    ui->rightImage->setPixmap(pixmap);
}
QString SubImagesDialog::rightImage() const
{
    return m_rightImage;
}

QString SubImagesDialog::leftImage() const
{
    return m_leftImage;
}
