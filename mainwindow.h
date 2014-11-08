#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "qcustomplot.h"
#include <QtSerialPort>



QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE


class SettingsDialog;
class ViewLog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    void setupSimpleDemo(QCustomPlot *customPlot);
    void addplot(const QVector<double>& xVoltaje,const QVector<double>& yAmplitude);
    void getDatafromSerial();

    void DecodeData();


private slots:
    void openSerialPort();
    void closeSerialPort();

    void writeData(const QByteArray &data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);

    void startProceso();

    void cleartext();
    void MostrarData();
    void printVoltajes(const QVector<double>& voltaje);
    //configuration options


    void titleDoubleClick(QMouseEvent *event, QCPPlotTitle *title);
    void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void removeSelectedGraph();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable);

private:

    void initActionsConnections();

    QVector<double> Voltaje;
    QVector<double> Time;

    Ui::MainWindow *ui;

    SettingsDialog *settings;
    ViewLog *logfile;
    QSerialPort *serialPort;
    QString portName;

    QString Stringdata ;
};

#endif // MAINWINDOW_H
