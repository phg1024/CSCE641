#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QMdiSubWindow>
#include <QImage>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include <QWheelEvent>
#include <QMouseEvent>

class ImageViewer : public QMdiSubWindow
{
    Q_OBJECT
public:
    ImageViewer(QWidget* parent = 0);
    ~ImageViewer();

    QSize sizeHint() const;

    void setImage(const QImage&);
    void setSignature(const QString& sig){_signature = sig;}

signals:
    void sig_viewerClosed(QString);

private slots:
    void slot_saveImage();

protected:
    void keyPressEvent(QKeyEvent *);

    void closeEvent(QCloseEvent *)
    {
        emit sig_viewerClosed(_signature);
    }

private:
    QImage _img;
    QString _signature;
    QWidget* _container;
    QGraphicsScene* _scene;
    QGraphicsView* _view;
    QPushButton* _saveButton;

};

#endif // IMAGEVIEWER_H
