#include <QFileDialog>

#include <iostream>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "scene.h"
#include "modelviewer.h"
#include "controlpanel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("RayTracer");

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
    makeUIComponents();

    _viewer = new ModelViewer(this);
    //_cpanel = new ControlPanel(this);

    _pBar = new QProgressBar(ui->statusBar);
    _pBar->setVisible(false);
}

void MainWindow::makeUIComponents()
{
    _loadSceneAction = ui->mainToolBar->addAction(QIcon(":/icons/loadscene"), "Load Scene", this, SLOT(slot_loadScene()));
    _renderAction = ui->mainToolBar->addAction(QIcon(":/icons/render"), "Render", this, SLOT(slot_render()));
}

void MainWindow::connectComponents()
{
    makeUIConnections();

    connect(_viewer, SIGNAL(sig_progress(double)), this, SLOT(slot_setProgress(double)));
}

void MainWindow::makeUIConnections()
{
    connect(ui->actionLoad_Scene, SIGNAL(triggered()), this, SLOT(slot_loadScene()));
    connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::layoutComponents()
{
    QDockWidget* dock = new QDockWidget("Scene View", this);
    dock->setAllowedAreas(
                Qt::LeftDockWidgetArea |
                Qt::RightDockWidgetArea |
                Qt::BottomDockWidgetArea |
                Qt::TopDockWidgetArea
                );
    dock->setWidget(_viewer);
    setCentralWidget(dock);

//    dock = new QDockWidget("Control Panel", this);
//    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
//    dock->setWidget(_cpanel);
//    _cpanel->setMinimumWidth(192);
//    _cpanel->setMaximumWidth(256);
//    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::slot_loadScene()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Please select a scene file ...",
                ".",
                "*.scn");

    processSceneFile(filename);
}

void MainWindow::processSceneFile(const QString &filename)
{
    if( filename.isEmpty() )
        return;

    if( filename.endsWith(".scn", Qt::CaseInsensitive) )
    {
        try{
            Scene* scene = new Scene(filename.toStdString());

            // set the scene to the ray tracer
            _viewer->bindScene(scene);
        }
        catch(const string& errstr)
        {
            cerr << errstr << endl;
        }
    }
}

void MainWindow::slot_setProgress(double p)
{
    ui->statusBar->addWidget(_pBar);

    int progress = p * 100.0;

    if( progress >= 99 )
    {
        _pBar->setVisible(false);
        ui->statusBar->removeWidget(_pBar);
    }
    else
    {
        _pBar->setValue(progress);
        _pBar->setVisible(true);
    }
}

void MainWindow::slot_render()
{
    _viewer->slot_render();
}
