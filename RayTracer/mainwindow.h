#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>

namespace Ui {
    class MainWindow;
}

class ModelViewer;
class ControlPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slot_setProgress(double);

protected:
    void createComponents();
    void connectComponents();
    void layoutComponents();

    void makeUIComponents();
    void makeUIConnections();

private slots:
    void slot_loadScene();
    void slot_render();

private:
    void processSceneFile(const QString&);

private:
    Ui::MainWindow *ui;

private:
    ModelViewer *_viewer;
    ControlPanel *_cpanel;

    QProgressBar *_pBar;

    QAction *_loadSceneAction;
    QAction *_renderAction;
};

#endif // MAINWINDOW_H
