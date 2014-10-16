#include "controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent)
{
    createComponents();
    layoutComponents();
    connectComponents();
}

void ControlPanel::createComponents()
{
    _reconstructorChooser = new QComboBox(this);
    _reconstructorChooser->addItem("Visual Hull");
    _reconstructorChooser->addItem("Octree-based VH");
    _reconstructorChooser->addItem("Voxel Coloring");    

    _reconstructorWidget = new QStackedWidget(this);

    makeVisualHullWidget();
    makeOctreeBasedVHWidget();
    makeVoxelColoringWidget();
}

void ControlPanel::layoutComponents()
{
    QVBoxLayout* _layout = new QVBoxLayout(this);
    _layout->addWidget(new QLabel("Reconstruction Engine"));
    _layout->addWidget(_reconstructorChooser);
    _layout->addWidget(_reconstructorWidget);
    _layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    setLayout(_layout);
}

void ControlPanel::connectComponents()
{
    connect(_reconstructorChooser, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_switchWidget(QString)));
    connect(_reconstructorChooser, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(sig_engineChanged(QString)));

    connect(_vhVoxelSizeBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_voxelSizeChanged(double)));
    connect(_insideThresholdBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_insideThresholdChanged(double)));

    connect(_splitThresholdBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_splitThresholdChanged(double)));
    connect(_cutoffVoxelSizeBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_cutoffVoxelSizeChanged(double)));

    connect(_backgroundThresholdBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_backgroundThresholdChanged(double)));
    connect(_consistencyThresholdBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_consistencyThresholdChanged(double)));
    connect(_voxelSizeBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_voxelSizeChanged(double)));
}

void ControlPanel::makeVisualHullWidget()
{
    _visualHullWidget = new QWidget;

    _vhVoxelSizeBox = new QDoubleSpinBox(this);
    _vhVoxelSizeBox->setMinimum(0.25);
    _vhVoxelSizeBox->setMaximum(64.0);
    _vhVoxelSizeBox->setValue(1.0);

    _insideThresholdBox = new QDoubleSpinBox(_visualHullWidget);
    _insideThresholdBox->setMinimum(0.01);
    _insideThresholdBox->setMaximum(1.0);
    _insideThresholdBox->setValue(0.5);

    QVBoxLayout *_vhwLayout = new QVBoxLayout(_visualHullWidget);
    _vhwLayout->addWidget(new QLabel("Voxel Size"));
    _vhwLayout->addWidget(_vhVoxelSizeBox);

    _vhwLayout->addWidget(new QLabel("Inside Threshold"));
    _vhwLayout->addWidget(_insideThresholdBox);

    _vhwLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    _visualHullWidget->setLayout(_vhwLayout);

    _reconstructorWidget->addWidget(_visualHullWidget);    
}


void ControlPanel::makeOctreeBasedVHWidget()
{
    _octreeBasedVHWidget = new QWidget;

    _splitThresholdBox = new QDoubleSpinBox(_octreeBasedVHWidget);
    _splitThresholdBox->setMinimum(0.01);
    _splitThresholdBox->setMaximum(1.0);
    _splitThresholdBox->setValue(0.5);
    _splitThresholdBox->setSingleStep(0.1);

    _cutoffVoxelSizeBox = new QDoubleSpinBox(_octreeBasedVHWidget);
    _cutoffVoxelSizeBox->setMinimum(0.25);
    _cutoffVoxelSizeBox->setMaximum(32.0);
    _cutoffVoxelSizeBox->setValue(1.0);

    QVBoxLayout *_vhwLayout = new QVBoxLayout(_octreeBasedVHWidget);
    _vhwLayout->addWidget(new QLabel("Cutoff Voxel Size"));
    _vhwLayout->addWidget(_cutoffVoxelSizeBox);
    _vhwLayout->addWidget(new QLabel("Min. Split Threshold"));
    _vhwLayout->addWidget(_splitThresholdBox);

    _vhwLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    _octreeBasedVHWidget->setLayout(_vhwLayout);

    _reconstructorWidget->addWidget(_octreeBasedVHWidget);
}


void ControlPanel::makeVoxelColoringWidget()
{
    _voxelColoringWidget = new QWidget;

    _voxelSizeBox = new QDoubleSpinBox(this);
    _voxelSizeBox->setMinimum(0.25);
    _voxelSizeBox->setMaximum(64.0);
    _voxelSizeBox->setValue(1.0);

    _consistencyThresholdBox = new QDoubleSpinBox(_voxelColoringWidget);
    _consistencyThresholdBox->setMinimum(0);    
    _consistencyThresholdBox->setMaximum(255);
    _consistencyThresholdBox->setValue(25);

    _backgroundThresholdBox = new QDoubleSpinBox(_voxelColoringWidget);
    _backgroundThresholdBox->setMinimum(0);
    _backgroundThresholdBox->setMaximum(255);
    _backgroundThresholdBox->setValue(100);

    QVBoxLayout *_vcwLayout = new QVBoxLayout(_voxelColoringWidget);
    _vcwLayout->addWidget(new QLabel("Voxel Size"));
    _vcwLayout->addWidget(_voxelSizeBox);
    _vcwLayout->addWidget(new QLabel("Consistency Threshd."));
    _vcwLayout->addWidget(_consistencyThresholdBox);
    _vcwLayout->addWidget(new QLabel("Background Threshd."));
    _vcwLayout->addWidget(_backgroundThresholdBox);
    _vcwLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    _reconstructorWidget->addWidget(_voxelColoringWidget);
}

void ControlPanel::slot_switchWidget(QString name)
{
    if(name == "Visual Hull")
    {
        _reconstructorWidget->setCurrentWidget(_visualHullWidget);
    }

    if(name == "Voxel Coloring")
    {
        _reconstructorWidget->setCurrentWidget(_voxelColoringWidget);
    }

    if(name == "Octree-based VH")
    {
        _reconstructorWidget->setCurrentWidget(_octreeBasedVHWidget);
    }
}
