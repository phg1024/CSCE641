#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = 0);

    QSize sizeHint() const{return QSize(200, 600);}

signals:
    void sig_resolutionChanged(int);
    void sig_engineChanged(QString);
    void sig_backgroundThresholdChanged(double);
    void sig_consistencyThresholdChanged(double);
    void sig_voxelSizeChanged(double);
    void sig_traversalDirectionChanged(int);

    void sig_insideThresholdChanged(double);

    void sig_splitThresholdChanged(double);
    void sig_cutoffVoxelSizeChanged(double);

public slots:

protected:
    void createComponents();
    void layoutComponents();
    void connectComponents();

    void makeVisualHullWidget();
    void makeOctreeBasedVHWidget();
    void makeVoxelColoringWidget();

private slots:
    void slot_switchWidget(QString);

private:
    QComboBox *_reconstructorChooser;
    QStackedWidget *_reconstructorWidget;

    // visual hull related
private:
    QWidget *_visualHullWidget;
    QDoubleSpinBox *_vhVoxelSizeBox;
    QDoubleSpinBox *_insideThresholdBox;

    // octree-based visual hull related
private:
    QWidget *_octreeBasedVHWidget;
    QDoubleSpinBox *_splitThresholdBox;
    QDoubleSpinBox *_cutoffVoxelSizeBox;

    // voxel coloring related
private:
    QWidget *_voxelColoringWidget;
    QDoubleSpinBox *_voxelSizeBox;
    QDoubleSpinBox *_consistencyThresholdBox;
    QDoubleSpinBox *_backgroundThresholdBox;
};

#endif // CONTROLPANEL_H
