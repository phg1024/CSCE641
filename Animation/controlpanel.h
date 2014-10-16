#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <vector>
#include <string>
using std::vector;
using std::string;

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = 0);

signals:
    void sig_frameIndexChanged(int);
    void sig_replaySpeedChanged(double);
    void sig_boneNameChanged(QString);
    void sig_bonePosRatioChanged(double);

public slots:
    void setMaxFrame(int);
    void setBoneNameBox(const vector<string>& names);

protected:
    void createComponents();
    void connectComponents();
    void layoutComponents();

private:
    QSpinBox *_frameIndexBox;
    QDoubleSpinBox *_redisplaySpeedBox;
    QComboBox *_boneNameBox;
    QDoubleSpinBox *_bonePosRatioBox;
};

#endif // CONTROLPANEL_H
