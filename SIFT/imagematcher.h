#ifndef IMAGEMATCHER_H
#define IMAGEMATCHER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cfloat>

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QImage>
#include <QPushButton>
#include <QPainter>
#include <QString>
#include <QApplication>

using namespace std;

namespace Ui {
class ImageMatcher;
}

class ImageMatcher : public QDialog
{
    Q_OBJECT
public:
    ImageMatcher(QWidget* parent = 0);

public slots:
    void matchImage(const string &imgfile1, const string &imgfile2, const string &keyfile1, const string &keyfile2, const string &outfilename);

signals:
    void sig_imageMatched(QImage);
    void sig_requestSIFT(string, string);

protected:
    void connectComponents();

private slots:
    void handleSetPath();
    void handleExec();

    void setImage1(const QString& str);
    void setImage2(const QString& str);
    void setKey1(const QString& str);
    void setKey2(const QString& str);
    void startMatch()
    {
        matchImage(imgfile[0], imgfile[1], keyfile[0], keyfile[1], "output.png");
    }

private:
    string imgfile[2], keyfile[2];

    struct Feature
    {
        double x, y, scale, orientation;
        double signature[128];
    };

    Ui::ImageMatcher *ui;

    void printArray(const double*, int);
    bool testMatch(const Feature &f1, const Feature &f2);
    double calEuclideanDistance(const Feature &f1, const Feature &f2);
    QImage combineImages(const string &imgfile1, const string &imgfile2, const list<pair<int, int> > matchPairs, Feature *f1, Feature *f2);
};

#endif // IMAGEMATCHER_H
