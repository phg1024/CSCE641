#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>

#include "humanbodymodel.h"
#include "modelviewer.h"
#include "controlpanel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void createComponents();
    void layoutComponents();
    void connectComponents();

    void createToolBox();

private slots:
    void slot_loadModel();
    void slot_loadAnimationSequence();

private:
    void processModelFile(const QString& filename);
    void processAMCFile(const QString& filename);

private:
    Ui::MainWindow *ui;

private:
    HumanBodyModel* _model;

private:
    ControlPanel* _cpanel;
    ModelViewer* _viewer;

private:
    QAction* _interactionStateAction;
    QAction* _replayAction;
};

#endif // MAINWINDOW_H
