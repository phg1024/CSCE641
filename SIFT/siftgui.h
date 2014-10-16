#ifndef SIFTGUI_H
#define SIFTGUI_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QMdiArea>
#include <QList>
#include <QString>
#include <QCheckBox>

#include "sift.h"
#include "imagematcher.h"

class ImageViewer;

class SiftGui : public QWidget
{
    Q_OBJECT
public:
    SiftGui(QWidget* parent = 0);
    ~SiftGui();

private:
    void initializeComponents();
    void layoutComponents();
    void connectComponents();

private:
    void createControlPanel();
    void closeEvent(QCloseEvent *e);

private slots:
    void processImage();
    void matchImage();
    void loadImage();

    void handleSIFTRequest(string, string);
    void showMatchResult(QImage);
    void handleParameterSet(QString);
    void handleParameterCheck(int);
    void handleViewerClosed(QString);

private:
    string getKeyFileName(const string&);

private:
    QList<QString> _infilelist;

private:
    QMdiArea* _imageContainer;
    ImageViewer* _dummyImage;


    // control panel related
private:
    QWidget* _controlPanel;

    QPushButton* _loadButton;
    QPushButton* _execButton;
    QPushButton* _matchButton;
    QSpinBox* _scalesBox;
    QDoubleSpinBox* _initSigmaBox;
    QDoubleSpinBox* _magnitudeThredBox;
    QDoubleSpinBox* _edgeResponseBox;

    QCheckBox* _verboseCheck;
    QCheckBox* _gsCheck;
    QCheckBox* _dogCheck;
    QCheckBox* _exCheck;

    // sift engine related
private:
    SiftOperator op;
    ImageMatcher matcher;
};

#endif // SIFTGUI_H
