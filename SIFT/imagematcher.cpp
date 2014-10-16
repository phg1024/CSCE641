#include "imagematcher.h"
#include "ui_imagematcher.h"

ImageMatcher::ImageMatcher(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ImageMatcher)
{
    ui->setupUi(this);
    connectComponents();
}

void ImageMatcher::connectComponents()
{
    connect(ui->imgButton1, SIGNAL(clicked()), this, SLOT(handleSetPath()));
    connect(ui->imgButton2, SIGNAL(clicked()), this, SLOT(handleSetPath()));
    connect(ui->keyButton1, SIGNAL(clicked()), this, SLOT(handleSetPath()));
    connect(ui->keyButton2, SIGNAL(clicked()), this, SLOT(handleSetPath()));

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(handleExec()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(handleExec()));
}

void ImageMatcher::setImage1(const QString& str)
{
    imgfile[0] = str.toStdString();
    ui->imgPath1->setText(str);
}

void ImageMatcher::setImage2(const QString& str)
{
    imgfile[1] = str.toStdString();
    ui->imgPath2->setText(str);
}

void ImageMatcher::setKey1(const QString& str)
{
    keyfile[0] = str.toStdString();
    ui->keyPath1->setText(str);
}

void ImageMatcher::setKey2(const QString& str)
{
    keyfile[1] = str.toStdString();
    ui->keyPath2->setText(str);
}


void ImageMatcher::handleExec()
{
    QPushButton* button = dynamic_cast<QPushButton*>(sender());

    if(button == ui->okButton)
    {
        if(keyfile[0].size() <= 0
        || keyfile[1].size() <=0 )
        {
            emit sig_requestSIFT(imgfile[0], imgfile[1]);
        }
        else
        {
            startMatch();
        }
        close();
    }
    else
        close();
}

void ImageMatcher::handleSetPath()
{
    QPushButton* button = dynamic_cast<QPushButton*>(sender());
    QString filename = QFileDialog::getOpenFileName(this);
    if(!filename.isEmpty())
    {
        if(button == ui->imgButton1)
            setImage1(filename);
        if(button == ui->imgButton2)
            setImage2(filename);
        if(button == ui->keyButton1)
            setKey1(filename);
        if(button == ui->keyButton2)
            setKey2(filename);
    }
}

void ImageMatcher::printArray(const double* a, int s)
{
    printf("\n");
    for(int i=0;i<s;i++)
        printf("%.4f ", a[i]);
    printf("\n");
}

bool ImageMatcher::testMatch(const Feature& f1, const Feature& f2)
{
    const double FEAT_VEC_DIFF_THRESHOLD = 0.2;
    bool flag = true;
    double vec_diff_sum = 0;
    for(int i=0;i<128;i++)
        vec_diff_sum += abs(f1.signature[i] - f2.signature[i]);
    flag &= (vec_diff_sum <= FEAT_VEC_DIFF_THRESHOLD);
    if(flag) cout << "diff = " << vec_diff_sum << endl;
    return flag;
}

double ImageMatcher::calEuclideanDistance(const Feature& f1, const Feature& f2)
{
    double vec_diff_sum = 0;
    for(int i=0;i<128;i++)
        vec_diff_sum += pow((f1.signature[i] - f2.signature[i]), 2);

    return sqrt(vec_diff_sum);
}

QImage ImageMatcher::combineImages(const string& imgfile1, const string& imgfile2,
                     const list<pair<int, int> > matchPairs,
                     Feature* f1, Feature* f2)
{
    QImage img1(imgfile1.c_str()), img2(imgfile2.c_str());
    int width = img1.width() + img2.width();
    int height = img1.height() > img2.height() ? img1.height() : img2.height();

    QImage img(width, height, QImage::Format_ARGB32);

    // copy the pixels to the img
    for(int y=0;y<img1.height();y++)
    {
        for(int x=0;x<img1.width();x++)
            img.setPixel(x, y, img1.pixel(x, y));
    }
    int xShift = img1.width();
    for(int y=0;y<img2.height();y++)
    {
        for(int x=0;x<img2.width();x++)
            img.setPixel(x + xShift, y, img2.pixel(x, y));
    }

    // paint the feature points
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    list<pair<int, int> >::const_iterator it = matchPairs.begin();
    double colorRatio = 0.0;
    int size = matchPairs.size();
    const int shadowShift = 1;
    int idx = 0;
    while( it != matchPairs.end() )
    {
        colorRatio = (double)idx / (double)size;
        idx++;

        QColor c = QColor::fromHsvF(colorRatio, 0.75, 1.0, 0.75);
        int idx1 = (*it).first, idx2 = (*it).second;
        Feature &f_img1 = f1[idx1], &f_img2 = f2[idx2];
        p.setPen(c);
        p.drawLine(f_img1.x, f_img1.y, f_img2.x + xShift, f_img2.y);
        p.setPen(Qt::red);
        p.drawPoint(f_img1.x, f_img1.y);
        p.drawPoint(f_img2.x + xShift, f_img2.y);

        it++;
    }

    it = matchPairs.begin();
    idx = 0;
    while( it!=matchPairs.end() )
    {
        string tag;
        stringstream sstag;
        sstag << idx++;
        sstag >> tag;
        QString s(tag.c_str());

        int idx1 = (*it).first, idx2 = (*it).second;
        Feature &f_img1 = f1[idx1], &f_img2 = f2[idx2];

        p.setFont(QFont("serif", 10));
        p.setPen(Qt::white);
        p.drawText(f_img2.x + xShift + shadowShift, f_img2.y + shadowShift, s);
        p.drawText(f_img1.x + shadowShift, f_img1.y + shadowShift,  s);
        p.setPen(Qt::darkGray);
        p.drawText(f_img2.x + xShift, f_img2.y,  s);
        p.drawText(f_img1.x, f_img1.y,  s);
        ++it;
    }

    return img;
}

void ImageMatcher::matchImage(const string &imgfile1, const string &imgfile2, const string &keyfile1, const string &keyfile2, const string &outfilename)
{
    ifstream infile1, infile2;
    infile1.open(keyfile1.c_str(), ios::in);
    infile2.open(keyfile2.c_str(), ios::in);

    int size1, size2;
    int vecSize;
    Feature *f1, *f2;
    infile1 >> size1 >> vecSize;
    infile2 >> size2 >> vecSize;
    cout << size1 << " features in file " << keyfile1 << endl;
    cout << size2 << " features in file " << keyfile2 << endl;
    f1 = new Feature[size1];
    f2 = new Feature[size2];

    int idx = 0;
    while(!infile1.eof() && (idx < size1))
    {
        infile1 >> f1[idx].x
                >> f1[idx].y
                >> f1[idx].scale
                >> f1[idx].orientation;

        for(int i=0;i<vecSize;i++)
            infile1 >> f1[idx].signature[i];
        idx++;
    }

    idx = 0;
    while(!infile2.eof() && (idx < size2))
    {
        infile2 >> f2[idx].x
                >> f2[idx].y
                >> f2[idx].scale
                >> f2[idx].orientation;

        for(int i=0;i<vecSize;i++)
            infile2 >> f2[idx].signature[i];
        idx++;
    }

    cout << "matching keys ... " << endl;

    // brute force match
    int matchCount = 0;
    list<pair<int, int> > matchPairs;
    for(int i=0;i<size1;i++)
    {
        pair<int, double> matchCase[2];
        matchCase[0].second = matchCase[1].second = DBL_MAX;

        for(int j=0;j<size2;j++)
        {
            double dist = calEuclideanDistance(f1[i], f2[j]);
            if( dist < matchCase[0].second )
            {
                matchCase[0].first = j;
                matchCase[0].second = dist;
            }

            if(matchCase[0].second < matchCase[1].second)
            {
                // swap it
                int tmpIdx = matchCase[1].first;
                double tmpDist = matchCase[1].second;
                matchCase[1] = matchCase[0];
                matchCase[0].first = tmpIdx;
                matchCase[0].second = tmpDist;
            }
        }

        const double DIST_RATIO = 0.6;
        if( matchCase[0].first != -1
                && matchCase[1].first != -1 )
        {
            if( matchCase[1].second < DIST_RATIO * matchCase[0].second )
            {
                cout << "match @ " << i << ", " << matchCase[1].first << endl;
                matchPairs.push_back(pair<int,int>(i, matchCase[1].first));
                matchCount ++ ;
            }
        }
    }

    cout << matchCount << " matches found!" << endl;

    QImage combined = combineImages(imgfile1, imgfile2, matchPairs, f1, f2);
    combined.save(outfilename.c_str());

    emit sig_imageMatched(combined);

    delete[] f1;
    delete[] f2;
}
