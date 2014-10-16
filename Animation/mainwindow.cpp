#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _model(0),
    _cpanel(0),
    _viewer(0)
{
    ui->setupUi(this);

    setWindowTitle("Human Body Animation");

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
    QGLFormat fmt = qglformat_3d;
    fmt.setSampleBuffers(true);
    fmt.setSamples(4);
    _viewer = new ModelViewer(this, fmt);
    _cpanel = new ControlPanel(this);

    createToolBox();
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
    dock->setWidget(_viewer);
    setCentralWidget(dock);

    dock = new QDockWidget("Control Panel", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock->setWidget(_cpanel);
    _cpanel->setMaximumWidth(256);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::connectComponents()
{
    connect(ui->actionLoad_Model, SIGNAL(triggered()), this, SLOT(slot_loadModel()));
    connect(ui->actionLoad_Animation_Sequence, SIGNAL(triggered()), this, SLOT(slot_loadAnimationSequence()));

    connect(_cpanel, SIGNAL(sig_frameIndexChanged(int)), _viewer, SLOT(setCurrentFrame(int)));
    connect(_cpanel, SIGNAL(sig_replaySpeedChanged(double)), _viewer, SLOT(setAutoplaySpeed(double)));
    connect(_cpanel, SIGNAL(sig_boneNameChanged(QString)), _viewer, SLOT(setTraceBone(QString)));
    connect(_cpanel, SIGNAL(sig_bonePosRatioChanged(double)), _viewer, SLOT(setTraceBonePosRatio(double)));
}

void MainWindow::createToolBox()
{
    // interaction state button
    QAction* _interactionStateAction = ui->mainToolBar->addAction(QIcon(":/icons/interaction"), "Interaction");
    _interactionStateAction->setCheckable(true);
    _interactionStateAction->setChecked(true);
    connect(_interactionStateAction, SIGNAL(triggered()), _viewer, SLOT(switchInteractionState()));

    // replay button
    QAction* _replayAction = ui->mainToolBar->addAction(QIcon(":/icons/replay"), "Replay");
    _replayAction->setCheckable(true);
    _replayAction->setChecked(false);
    connect(_replayAction, SIGNAL(triggered()), _viewer, SLOT(slot_switchReplay()));

    // toggle trace button
    QAction* _traceAction = ui->mainToolBar->addAction(QIcon(":/icons/trace"), "Trace");
    _traceAction->setCheckable(true);
    _traceAction->setChecked(false);
    connect(_traceAction, SIGNAL(triggered()), _viewer, SLOT(toggleShowTrace()));
}

void MainWindow::slot_loadModel()
{
    QString filename = QFileDialog::getOpenFileName(this, "Please select a model file.", ".", "*.asf");
    if( !filename.isEmpty() )
    {
        processModelFile(filename);
    }
    else
    {
        ui->statusBar->showMessage("Invalid model file name!", 1000);
    }
}

void MainWindow::slot_loadAnimationSequence()
{
    QString filename = QFileDialog::getOpenFileName(this, "Please select an animation sequence file.", ".", "*.amc");
    if( !filename.isEmpty() )
    {
        processAMCFile(filename);
    }
    else
    {
        ui->statusBar->showMessage("Invalid animation sequence file name!", 1000);
    }
}

void MainWindow::processModelFile(const QString &filename)
{
    if(filename.endsWith(".asf", Qt::CaseInsensitive))
    {
        _viewer->releaseModel();

        if( _model )
        {
            delete _model;
            _model = 0;
        }

        _model = new HumanBodyModel(filename.toStdString());

        if( _model != 0 )
        {
            _viewer->bindModel(_model);
            _cpanel->setBoneNameBox(_model->getBoneNames());
        }
        else
        {
            ui->statusBar->showMessage("Invalid model file!", 1000);
        }
    }
}

void MainWindow::processAMCFile(const QString &filename)
{
    if(filename.endsWith(".amc", Qt::CaseInsensitive))
    {
        if( _model != 0 )
        {
            if(_model->addAnimationSequence(filename.toStdString()))
            {
                _viewer->setCurrentFrame(0);
                _viewer->clearTrace();
                _cpanel->setMaxFrame( _model->frameNumber() - 1 );
            }
        }
        else
        {
            ui->statusBar->showMessage("No valid model exists file!", 1000);
        }
    }
}

