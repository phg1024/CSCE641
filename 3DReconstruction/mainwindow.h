#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global_definitions.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QImage>
#include <QFile>

#include "helpdialog.h"
#include "modelviewer.h"
#include "imageviewer.h"
#include "controlpanel.h"
#include "reconstructionengine.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSize sizeHint() const {return QSize(1024, 768);}

public slots:
    void slot_setEngine(QString);
    void slot_setBackgroundThreshold(double);
    void slot_setConsistencyThreshold(double);
    void slot_setVoxelSize(double);
    void slot_setTraversalDirection(int);
    void slot_setInsideThreshold(double);
    void slot_setSplittingThreshold(double);
    void slot_setCutoffVoxelSize(double);

    void slot_setProgress(double);

protected:
    void createComponents();
    void connectComponents();
    void layoutComponents();

private slots:
    void slot_viewImage();
    void slot_loadDescriptionFile();
    void slot_loadModelFile();

private:
    void processDescriptionFile(const QString&);
    QString makeModelFilename(const QString&);
    QString makeModelDefFilename(const QString&);
    void processModelFile(const QString&);

private:
    Ui::MainWindow *ui;
    HelpDialog* _helpDlg;
    ControlPanel* _controlPanel;
    ModelViewer* _mvPanel;    

    AbstractModel* _model;

    QProgressBar *_progressBar;
    QLabel *_initializationLabel;
    QLabel *_progressLabel;
private:
    ReconstructionEngine::EngineType _engineType;
    double _background_threshold, _consistency_threshold;
    double _voxel_size;
    bool _forceRegenerateModel;
    int _traversalDirection;
    double _insideThreshold;
    double _split_threshold;
    double _cutoff_voxel_size;
};

#endif // MAINWINDOW_H
