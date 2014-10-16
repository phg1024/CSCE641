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

    _container = new QWidget(this);

    QHBoxLayout* _layout = new QHBoxLayout();
    _layout->addWidget(_view);
    _container->setLayout(_layout);

    setAttribute(Qt::WA_DeleteOnClose);
    setWidget(_container);
}

ImageViewer::~ImageViewer()
{

}

QSize ImageViewer::sizeHint() const
{
    return QSize(700, 540);
}

void ImageViewer::setImage(const QImage& img)
{    
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

