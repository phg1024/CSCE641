#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "reconstructionengine.h"

#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _helpDlg(new HelpDialog),
    _model(0),
    _engineType(ReconstructionEngine::VisualHull),
    _background_threshold(100.0),
    _consistency_threshold(25.0),
    _voxel_size(1.0),
    _forceRegenerateModel(false),
    _insideThreshold(0.5),
    _split_threshold(0.5),
    _cutoff_voxel_size(1.0)
{
    ui->setupUi(this);
    setWindowTitle("3D Model Reconstructor");

    _helpDlg->hide();

    createComponents();
    layoutComponents();
    connectComponents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createComponents()
{
    // model viewer
    _mvPanel = new ModelViewer(this);

    // control panel
    _controlPanel = new ControlPanel(this);

    _initializationLabel = new QLabel("Initializing reconstruction ...", ui->mainToolBar);
    ui->statusBar->addWidget(_initializationLabel);
    _initializationLabel->hide();

    _progressLabel = new QLabel("Reconstrunction progress: ", ui->mainToolBar);
    ui->statusBar->addWidget(_progressLabel);
    _progressLabel->hide();

    _progressBar = new QProgressBar(ui->mainToolBar);
    ui->statusBar->addWidget(_progressBar);
    _progressBar->hide();

}

void MainWindow::connectComponents()
{
    connect(ui->actionUsage, SIGNAL(triggered()), _helpDlg, SLOT(show()));

    connect(ui->actionView_Image, SIGNAL(triggered()), this, SLOT(slot_viewImage()));
    connect(ui->actionLoad_Description_File, SIGNAL(triggered()), this, SLOT(slot_loadDescriptionFile()));
    connect(ui->actionLoad_Model, SIGNAL(triggered()), this, SLOT(slot_loadModelFile()));

    connect(_controlPanel, SIGNAL(sig_insideThresholdChanged(double)), this, SLOT(slot_setInsideThreshold(double)));

    connect(_controlPanel, SIGNAL(sig_splitThresholdChanged(double)), this, SLOT(slot_setSplittingThreshold(double)));
    connect(_controlPanel, SIGNAL(sig_cutoffVoxelSizeChanged(double)), this, SLOT(slot_setCutoffVoxelSize(double)));

    connect(_controlPanel, SIGNAL(sig_engineChanged(QString)), this, SLOT(slot_setEngine(QString)));
    connect(_controlPanel, SIGNAL(sig_backgroundThresholdChanged(double)), this, SLOT(slot_setBackgroundThreshold(double)));
    connect(_controlPanel, SIGNAL(sig_consistencyThresholdChanged(double)), this, SLOT(slot_setConsistencyThreshold(double)));
    connect(_controlPanel, SIGNAL(sig_voxelSizeChanged(double)), this, SLOT(slot_setVoxelSize(double)));
}

void MainWindow::layoutComponents()
{
    QDockWidget* dock = new QDockWidget("Model View", this);
    dock->setAllowedAreas(
                Qt::LeftDockWidgetArea |
                Qt::RightDockWidgetArea |
                Qt::BottomDockWidgetArea |
                Qt::TopDockWidgetArea
                );
    dock->setWidget(_mvPanel);
    _mvPanel->setMaximumWidth(1000);
    setCentralWidget(dock);

    dock = new QDockWidget("Control Panel", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock->setWidget(_controlPanel);
    _controlPanel->setMaximumWidth(256);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::slot_viewImage()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,
                                                          "Please select image files to view ...",
                                                          ".",
                                                          "*.jpg *.png *.bmp");
    foreach(QString s, filenames)
    {
        ImageViewer* v = new ImageViewer();
        v->setWindowTitle(s);
        v->setImage(QImage(s));
        v->show();
    }
}

void MainWindow::slot_loadDescriptionFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Please select a description file ...",
                                                    ".",
                                                    "*.des");
    processDescriptionFile(filename);
}

void MainWindow::processDescriptionFile(const QString& filename)
{
    if(filename.isEmpty())
        return;

    if(!filename.endsWith(".des"))
    {
        ui->statusBar->showMessage("Not a valid description file!", 1000);
        return;
    }

    // test if the model is already created
    QString modelFilename = makeModelFilename(filename);
    QString modelDefFilename = makeModelDefFilename(filename);
    QFile modelFile(modelFilename);
    if((!modelFile.exists()) || _forceRegenerateModel)
    {
        _initializationLabel->show();
        ui->statusBar->update();
        qApp->processEvents();
        _forceRegenerateModel = false;
        QTime start = QTime::currentTime();
        // reconstruct the model, and output to a model file
        ReconstructionEngine e(_engineType);
        connect(&e, SIGNAL(sig_progress(double)), this, SLOT(slot_setProgress(double)));
        e.setBackgroundThreshold(_background_threshold);
        e.setConsistencyThreshold(_consistency_threshold);
        e.setVoxelSize(_voxel_size);
        e.setTraversalDirection(_traversalDirection);
        e.setInsideThreshold(_insideThreshold);
        e.setSplitThreshold(_split_threshold);
        e.setCutoffVoxelSize(_cutoff_voxel_size);
        e.reconstructModel(filename.toStdString(), modelFilename.toStdString());
        cout << "Reconstruction time is " << start.elapsed() / 1000.0 << " seconds." << endl;
        disconnect(&e, 0, 0, 0);
        _progressBar->setValue(100);
        _progressLabel->hide();
        _progressBar->hide();
        ui->statusBar->showMessage("Reconstruction finished.", 1000);
    }

    // notify the renderer to load the model
    if(_model != 0)
    {
        delete _model;
        _model = 0;
    }

    AbstractModel::ModelType mType = AbstractModel::determineModelType(modelDefFilename);
    switch(mType)
    {
    case AbstractModel::VOXELARRAY:
    {
        _model = dynamic_cast<AbstractModel*>(new VoxelArrayModel(modelDefFilename));
        break;
    }
    default:
    {
        _model = 0;
        break;
    }
    }

    _mvPanel->setModel(_model);
}

void MainWindow::slot_loadModelFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Please select a model file ...",
                                                    ".",
                                                    "*.def");
    processModelFile(filename);
}

void MainWindow::processModelFile(const QString& filename)
{
    if(filename.isEmpty())
        return;

    if(!filename.endsWith(".vxl"))
    {
        ui->statusBar->showMessage("Not a valid model file!", 1000);
        return;
    }

    // notify the renderer to load the model
    if(_model != 0)
        delete _model;

    AbstractModel::ModelType mType = AbstractModel::determineModelType(filename);
    switch(mType)
    {
    case AbstractModel::VOXELARRAY:
    {
        _model = dynamic_cast<AbstractModel*>(new VoxelArrayModel(filename));
        break;
    }
    default:
    {
        _model = 0;
        break;
    }
    }
    _mvPanel->setModel(_model);
}

QString MainWindow::makeModelFilename(const QString &filename)
{
    QString n;
    QFileInfo info(filename);
    n = info.path() + "/" + info.baseName() + ".vxl";
    return n;
}

QString MainWindow::makeModelDefFilename(const QString &filename)
{
    QString n;
    QFileInfo info(filename);
    n = info.path() + "/" + info.baseName() + ".vxl";
    return n;
}

void MainWindow::slot_setEngine(QString t)
{
    cout << "current reconstruction engine is " << qPrintable(t) << endl;
    _engineType = ReconstructionEngine::interpretEngineType(t);
    _forceRegenerateModel = true;
}

void MainWindow::slot_setBackgroundThreshold(double t)
{
    _background_threshold = t;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setConsistencyThreshold(double t)
{
    _consistency_threshold = t;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setVoxelSize(double size)
{
    _voxel_size = size;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setTraversalDirection(int dir)
{
    _traversalDirection = dir;
}

void MainWindow::slot_setInsideThreshold(double threshd)
{
    _insideThreshold = threshd;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setSplittingThreshold(double threshd)
{
    _split_threshold = threshd;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setCutoffVoxelSize(double size)
{
    _cutoff_voxel_size = size;
    _forceRegenerateModel = true;
}

void MainWindow::slot_setProgress(double v)
{
    _initializationLabel->hide();
    _progressLabel->show();
    _progressBar->setValue(v * 100.0);
    _progressBar->show();
    ui->statusBar->update();
    if(v >= 1.0)
    {
        _progressBar->setValue(0);
        _progressBar->hide();
    }
}
