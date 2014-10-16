#include "imageviewer.h"

#include <iostream>
using namespace std;

ImageViewer::ImageViewer(QWidget* parent):
    QMdiSubWindow(parent)
{
    _scene = new QGraphicsScene(this);
    _view = new QGraphicsView(_scene, this);    
    _view->setRenderHint(QPainter::Antialiasing);
    _view->setRenderHint(QPainter::SmoothPixmapTransform);
    _saveButton = new QPushButton("Save", this);
    _container = new QWidget(this);

    connect(_saveButton, SIGNAL(clicked()), this, SLOT(slot_saveImage()));

    QVBoxLayout* _layout = new QVBoxLayout();
    _layout->addWidget(_view);
    QHBoxLayout* _saveButtonLayout = new QHBoxLayout();
    _saveButtonLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
    _saveButtonLayout->addWidget(_saveButton);
    _saveButtonLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
    _layout->addLayout(_saveButtonLayout);
    _container->setLayout(_layout);

    setAttribute(Qt::WA_DeleteOnClose);
    setWidget(_container);
}

ImageViewer::~ImageViewer()
{

}

void ImageViewer::slot_saveImage()
{
    QString filename = QFileDialog::getOpenFileName(this, "Please input a file name:",
                                                    ".", "*.png\n*.jpg\n*.bmp");

    if( !filename.isEmpty() )
    {
        _img.save(filename);
    }
}

QSize ImageViewer::sizeHint() const
{
    return QSize(700, 540);
}

void ImageViewer::setImage(const QImage& img)
{    
    _img = img;
    _scene->clear();
    _scene->addPixmap(QPixmap::fromImage(img));
    _view->update();

    const int edgeSize = 60;
    resize(img.width() + edgeSize, img.height() + edgeSize);
}

void ImageViewer::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Plus:
    case Qt::Key_Equal:
    {
        _view->scale(1.05, 1.05);
        _view->update();
        break;
    }
    case Qt::Key_Minus:
    case Qt::Key_hyphen:
    {
        _view->scale(0.95, 0.95);
        _view->update();
        break;
    }

    }
}

