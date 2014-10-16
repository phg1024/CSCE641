#include "controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent)
{
    setMinimumWidth(160);
    createComponents();
    layoutComponents();
    connectComponents();
}

void ControlPanel::createComponents()
{
    _frameIndexBox = new QSpinBox(this);
    _frameIndexBox->setValue(0);

    _redisplaySpeedBox = new QDoubleSpinBox(this);
    _redisplaySpeedBox->setValue(1.0);
    _redisplaySpeedBox->setDecimals(4);
    _redisplaySpeedBox->setMinimum(0.001);
    _redisplaySpeedBox->setMaximum(32);
    _redisplaySpeedBox->setSingleStep(0.2);

    _boneNameBox = new QComboBox(this);

    _bonePosRatioBox = new QDoubleSpinBox(this);
    _bonePosRatioBox->setValue(1.0);
    _bonePosRatioBox->setMaximum(1.0);
    _bonePosRatioBox->setMinimum(0.0);
}

void ControlPanel::connectComponents()
{
    connect(_frameIndexBox, SIGNAL(valueChanged(int)), this, SIGNAL(sig_frameIndexChanged(int)));
    connect(_redisplaySpeedBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_replaySpeedChanged(double)));

    connect(_boneNameBox, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(sig_boneNameChanged(QString)));
    connect(_bonePosRatioBox, SIGNAL(valueChanged(double)), this, SIGNAL(sig_bonePosRatioChanged(double)));
}

void ControlPanel::layoutComponents()
{
    QVBoxLayout* _layout = new QVBoxLayout(this);
    _layout->addWidget(new QLabel("Frame Index", this));
    _layout->addWidget(_frameIndexBox);
    _layout->addWidget(new QLabel("Replay Speed", this));
    _layout->addWidget(_redisplaySpeedBox);

    _layout->addWidget(new QLabel("Bone for trace", this));
    _layout->addWidget(_boneNameBox);
    _layout->addWidget(new QLabel("Bone Length Ratio", this));
    _layout->addWidget(_bonePosRatioBox);

    _layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(_layout);
}

void ControlPanel::setMaxFrame(int val)
{
    if( _frameIndexBox )
        _frameIndexBox->setMaximum(val);
}

void ControlPanel::setBoneNameBox(const vector<string> &names)
{
    if( _boneNameBox )
    {
        _boneNameBox->clear();

        for(size_t i=0;i<names.size();i++)
            _boneNameBox->addItem(names[i].c_str());
    }
}
