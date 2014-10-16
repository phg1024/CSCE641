#include "siftgui.h"
#include "imageviewer.h"

#include <QString>
#include <QFileDialog>

SiftGui::SiftGui(QWidget* parent):
    QWidget(parent)
{
    initializeComponents();
    layoutComponents();
    connectComponents();
    resize(1024, 768);
}

SiftGui::~SiftGui()
{
}

void SiftGui::connectComponents()
{
    // file io
    connect(_loadButton, SIGNAL(clicked()), this, SLOT(loadImage()));

    // operations
    connect(_execButton, SIGNAL(clicked()), this, SLOT(processImage()));
    connect(_matchButton, SIGNAL(clicked()), this, SLOT(matchImage()));

    // internal connection
    connect(&matcher, SIGNAL(sig_imageMatched(QImage)), this, SLOT(showMatchResult(QImage)));
    connect(&matcher, SIGNAL(sig_requestSIFT(string,string)), this, SLOT(handleSIFTRequest(string,string)));

    connect(_scalesBox, SIGNAL(valueChanged(QString)), this, SLOT(handleParameterSet(QString)));
    connect(_initSigmaBox, SIGNAL(valueChanged(QString)), this, SLOT(handleParameterSet(QString)));
    connect(_edgeResponseBox, SIGNAL(valueChanged(QString)), this, SLOT(handleParameterSet(QString)));
    connect(_magnitudeThredBox, SIGNAL(valueChanged(QString)), this, SLOT(handleParameterSet(QString)));

    connect(_verboseCheck, SIGNAL(stateChanged(int)), this, SLOT(handleParameterCheck(int)));
    connect(_gsCheck, SIGNAL(stateChanged(int)), this, SLOT(handleParameterCheck(int)));
    connect(_dogCheck, SIGNAL(stateChanged(int)), this, SLOT(handleParameterCheck(int)));
    connect(_exCheck, SIGNAL(stateChanged(int)), this, SLOT(handleParameterCheck(int)));
}

void SiftGui::initializeComponents()
{
    _imageContainer = new QMdiArea(this);
    createControlPanel();
}

void SiftGui::layoutComponents()
{
    QHBoxLayout* _mainLayout = new QHBoxLayout(this);
    _mainLayout->addWidget(_controlPanel);
    _mainLayout->addWidget(_imageContainer);
}

void SiftGui::createControlPanel()
{
    _controlPanel = new QWidget(this);
    _loadButton = new QPushButton("Load Image", this);
    _execButton = new QPushButton("Run SIFT", this);
    _matchButton = new QPushButton("Match Images", this);
    _scalesBox = new QSpinBox(this);
    _initSigmaBox = new QDoubleSpinBox(this);
    _magnitudeThredBox = new QDoubleSpinBox(this);
    _edgeResponseBox = new QDoubleSpinBox(this);
    _verboseCheck = new QCheckBox("Verbose Mode", this);
    _gsCheck = new QCheckBox("Ouput GSPYMD", this);
    _dogCheck = new QCheckBox("Output DOGPYMD", this);
    _exCheck = new QCheckBox("Output Extrema", this);
    QVBoxLayout* _controlPanelLayout = new QVBoxLayout(_controlPanel);
    _controlPanelLayout->addWidget(_loadButton);
    _controlPanelLayout->addWidget(_execButton);
    _controlPanelLayout->addWidget(_matchButton);
    _controlPanelLayout->addWidget(new QLabel("Scales per octave", this));
    _controlPanelLayout->addWidget(_scalesBox);
    _scalesBox->setValue(3);
    _scalesBox->setMinimum(1);
    _controlPanelLayout->addWidget(new QLabel("Initial Sigma", this));    
    _controlPanelLayout->addWidget(_initSigmaBox);
    _initSigmaBox->setValue(1.6);
    _initSigmaBox->setMinimum(1.0);
    _controlPanelLayout->addWidget(new QLabel("Grad. Mag. Threshd.", this));    
    _controlPanelLayout->addWidget(_magnitudeThredBox);
    _magnitudeThredBox->setValue(0.05);
    _magnitudeThredBox->setValue(0.01);
    _controlPanelLayout->addWidget(new QLabel("Edge Threshd.", this));    
    _controlPanelLayout->addWidget(_edgeResponseBox);
    _edgeResponseBox->setValue(10.0);
    _edgeResponseBox->setValue(1e-3);

    _verboseCheck->setChecked(false);
    _gsCheck->setChecked(false);
    _dogCheck->setChecked(false);
    _exCheck->setChecked(false);
    _controlPanelLayout->addWidget(_verboseCheck);
    _controlPanelLayout->addWidget(_gsCheck);
    _controlPanelLayout->addWidget(_dogCheck);
    _controlPanelLayout->addWidget(_exCheck);

    _controlPanelLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void SiftGui::loadImage()
{
    QStringList filename = QFileDialog::getOpenFileNames(this, "Please select an input image file.", ".", "*.jpg *.png *.bmp *.tiff *.pgm");

    foreach(QString s, filename)
    {
        // skip images already loaded
        if(_infilelist.contains(s))
            continue;

        QImage srcImg(s);
        ImageViewer* srcViewer = new ImageViewer(_imageContainer);
        srcViewer->setImage(srcImg);
        srcViewer->setWindowTitle(s);
        srcViewer->setSignature(s);
        connect(srcViewer, SIGNAL(sig_viewerClosed(QString)), this, SLOT(handleViewerClosed(QString)));
        _imageContainer->addSubWindow(srcViewer);
        srcViewer->show();

        _infilelist.push_back(s);
    }
}

void SiftGui::processImage()
{
    QList<QString>::iterator it = _infilelist.begin();
    while( it!= _infilelist.end() )
    {
        QString filename = (*it);
        QImage img = op.process(filename.toStdString());
        ImageViewer* viewer = new ImageViewer(_imageContainer);
        viewer->setWindowTitle(filename);
        viewer->setImage(img);
        _imageContainer->addSubWindow(viewer);
        viewer->show();
        ++ it;
    }
}

void SiftGui::matchImage()
{
    matcher.setVisible(true);
}

void SiftGui::closeEvent(QCloseEvent *e)
{
    matcher.close();
}

void SiftGui::showMatchResult(QImage img)
{
    ImageViewer* viewer = new ImageViewer(_imageContainer);
    viewer->setImage(img);
    _imageContainer->addSubWindow(viewer);
    viewer->show();
}

void SiftGui::handleSIFTRequest(std::string img1, std::string img2)
{
    if(!_infilelist.contains(QString::fromStdString(img1)))
        _infilelist.push_back(QString::fromStdString(img1));
    if(!_infilelist.contains(QString::fromStdString(img2)))
        _infilelist.push_back(QString::fromStdString(img2));

    processImage();

    _infilelist.removeOne(QString::fromStdString(img1));
    _infilelist.removeOne(QString::fromStdString(img2));

    string key1 = getKeyFileName(img1);
    string key2 = getKeyFileName(img2);

    string path = img1.size() > img2.size() ? img1 : img2;
    path = path.substr(0, path.find_last_of("."));
    matcher.matchImage(img1, img2, key1, key2, path + "_output.png");
}

std::string SiftGui::getKeyFileName(const std::string & img)
{
    stringstream ss;
    ss << img.substr(0, img.find_last_of(".")) << ".key";
    string s;
    ss >> s;
    cout << s;
    return s;
}

void SiftGui::handleParameterSet(QString val)
{
    QSpinBox* box = dynamic_cast<QSpinBox*>(sender());
    if(box!=NULL)
    {
        if(box == _scalesBox)
            op.setParameter(SiftOperator::SCALES, val.toInt());
        return;
    }

    QDoubleSpinBox* box_ = dynamic_cast<QDoubleSpinBox*>(sender());
    if(box_!=NULL)
    {
        if(box_ == _magnitudeThredBox)
            op.setParameter(SiftOperator::MAGNITUDE_THRESHOLD, val.toDouble());
        if(box_ == _edgeResponseBox)
            op.setParameter(SiftOperator::EDGE_THRESHOLD, val.toDouble());
        if(box_ == _initSigmaBox)
            op.setParameter(SiftOperator::INITIAL_SIGMA, val.toDouble());
        return;
    }
}

void SiftGui::handleParameterCheck(int val)
{
    bool flag = (val == Qt::Checked);
    QCheckBox* box = dynamic_cast<QCheckBox*>(sender());
    if(box!=NULL)
    {
        if(box == _verboseCheck)
        {
            op.setMode(flag?'v':'V');
            if(flag)
            {
                _gsCheck->setChecked(true);
                _gsCheck->setEnabled(false);

                _dogCheck->setChecked(true);
                _dogCheck->setEnabled(false);

                _exCheck->setChecked(true);
                _exCheck->setEnabled(false);
            }
            else
            {
                _gsCheck->setChecked(false);
                _gsCheck->setEnabled(true);

                _dogCheck->setChecked(false);
                _dogCheck->setEnabled(true);

                _exCheck->setChecked(false);
                _exCheck->setEnabled(true);
            }
        }
        if(box == _gsCheck)
            op.setMode(flag?'g':'G');
        if(box == _dogCheck)
            op.setMode(flag?'d':'D');
        if(box == _exCheck)
            op.setMode(flag?'e':'E');
    }
    return;
}

void SiftGui::handleViewerClosed(QString sig)
{
    _infilelist.removeOne(sig);
}
