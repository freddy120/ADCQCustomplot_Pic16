#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingsdialog.h"
#include "viewlog.h"


#include <QMessageBox>
#include <QtSerialPort>

#include <cstdlib>

#include "util.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand(QDateTime::currentDateTime().toTime_t());

    setupSimpleDemo(ui->customPlot);// Aca es!

    settings = new SettingsDialog;
    logfile =  new ViewLog;
    serialPort = new QSerialPort(0);
    portName = "";

    Stringdata = "";

    initActionsConnections();

    connect(serialPort  , SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(logfile, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));



    // GRAPH


    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect some interaction slots:
    connect(ui->customPlot, SIGNAL(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)), this, SLOT(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)));
    connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

    // setup policy and connect slot for context menu popup:
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings(); //estructura de datos


    serialPort->setPortName(p.name);
    serialPort->setBaudRate(p.baudRate);
    serialPort->setDataBits(p.dataBits);
    serialPort->setParity(p.parity);
    serialPort->setStopBits(p.stopBits);
    serialPort->setFlowControl(p.flowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
            ui->pushButton_viewlog->setEnabled(true);
            ui->pushButton_connect->setEnabled(false);
            ui->pushButton_disconnect->setEnabled(true);

            ui->pushButton_settings->setEnabled(false);

            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
            portName = p.name;
    } else {
        QMessageBox::critical(this, tr("Error"), serialPort->errorString());

        ui->statusBar->showMessage(tr("Open error"));
        portName = "";
    }
}


//! [5]
void MainWindow::closeSerialPort()
{
    serialPort->close();
    ui->pushButton_viewlog->setEnabled(false);
    ui->pushButton_connect->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);
    ui->pushButton_settings->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
    portName = "";
}
//! [5]

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serialPort->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
   QByteArray data = serialPort->readAll();


   QString aux = QString(data);
   Stringdata = Stringdata + QString(data);


    if (aux.contains("X")){

        qDebug() << "Fin";

        DecodeData(); //decode and plot.

        Stringdata = "";
        Voltaje.clear();
        Time.clear();
    }

   logfile->putData(data);

   //! mostrar data here

}
void MainWindow::DecodeData(){


   std::string Stringvolt = Stringdata.toStdString();

   std::stringstream ss;
   ss << Stringvolt;


   std::string line;
   while(!ss.eof()){

       std::getline(ss,line);


       if (line.empty())
           continue;

       else if (line[0] == '$')
           continue;


       // DATOS
       else if (line[0] == 'D'){

           std::vector<std::string> tokens = tokenize(line,' ');
           Voltaje.push_back(std::stod(tokens[1]));

       }

   }

   for (int i = 0; i < Voltaje.size(); i++) {
       Time.push_back(i);
   }

   addplot(Time,Voltaje);
   printVoltajes(Voltaje);

  //qDebug() << Voltaje.at(19);

}

void MainWindow::printVoltajes(const QVector<double>& voltaje){

    for (int i=0;i<voltaje.size();i++){

        ui->textEdit_serial->insertPlainText(QString::number(voltaje.at(i)));
        ui->textEdit_serial->insertPlainText("\r\n");

    }

}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serialPort->errorString());
        closeSerialPort();
    }

}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->pushButton_connect, SIGNAL(clicked()), this, SLOT(openSerialPort()));
    connect(ui->pushButton_disconnect, SIGNAL(clicked()), this, SLOT(closeSerialPort()));
   // connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->pushButton_settings, SIGNAL(clicked()), settings, SLOT(show()));
    connect(ui->pushButton_viewlog,SIGNAL(clicked()), logfile , SLOT(show()));


    connect(ui->pushButton_startproceso,SIGNAL(clicked()),this,SLOT(startProceso()));


    connect(ui->pushButton_cleartext,SIGNAL(clicked()),this,SLOT(cleartext()));
    connect(ui->pushButton_muestra,SIGNAL(clicked()),this,SLOT(MostrarData()));

}


void MainWindow::startProceso(){

    if(serialPort->isOpen()){

    QByteArray data("A");

    writeData(data);

    ui->textEdit_serial->clear();

    }

}

void MainWindow::setupSimpleDemo(QCustomPlot *customPlot)
{ // configure  the presentation of the voltagramas


  ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

  ui->customPlot->xAxis->setRange(0, 20);
  ui->customPlot->yAxis->setRange(0, 1024);


  ui->customPlot->axisRect()->setupFullAxesBox();


  ui->customPlot->xAxis->setLabel("kT (muestras)");
  ui->customPlot->yAxis->setLabel("Amplitud");

  ui->customPlot->legend->setVisible(true);

  QFont legendFont = font();
  legendFont.setPointSize(10);

  ui->customPlot->legend->setFont(legendFont);
  ui->customPlot->legend->setSelectedFont(legendFont);
  ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

  //addRandomGraph();// Add graph voltagrama.


  // configure right and top axis to show ticks but no labels:
  // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
  customPlot->xAxis2->setVisible(true);
  customPlot->xAxis2->setTickLabels(false);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTickLabels(false);


}


void MainWindow::titleDoubleClick(QMouseEvent* event, QCPPlotTitle* title)
{
  Q_UNUSED(event)
  // Set the plot title by double clicking on it
  bool ok;
  QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
  if (ok)
  {
    title->setText(newTitle);
    ui->customPlot->replot();
  }
}

void MainWindow::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
  // Set an axis label by double clicking on it
  if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
  {
    bool ok;
    QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
    if (ok)
    {
      axis->setLabel(newLabel);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->customPlot->graphCount(); ++i)
  {
    QCPGraph *graph = ui->customPlot->graph(i);
    QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelected(true);
    }
  }
}

void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::removeSelectedGraph()
{
  if (ui->customPlot->selectedGraphs().size() > 0)
  {
    ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
    ui->customPlot->replot();
  }
}

void MainWindow::removeAllGraphs()
{
  ui->customPlot->clearGraphs();
  ui->customPlot->replot();
}

void MainWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
    //menu->addAction("Add random graph", this, SLOT(addRandomGraph()));

    if (ui->customPlot->selectedGraphs().size() > 0)
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->customPlot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
  }

  menu->popup(ui->customPlot->mapToGlobal(pos));
}

void MainWindow::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::graphClicked(QCPAbstractPlottable *plottable)
{
  ui->statusBar->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 1000);
}


void MainWindow:: addplot(const QVector<double>& xVoltaje,const QVector<double>& yAmplitude){


    ui->customPlot->addGraph();
    QCPScatterStyle::ScatterShape shapes (QCPScatterStyle::ssCross);


    ui->customPlot->graph()->setName(QString("New graph %1").arg(ui->customPlot->graphCount()-1));
    ui->customPlot->graph()->setData(xVoltaje, yAmplitude);
    ui->customPlot->graph()->setLineStyle(QCPGraph::lsLine);

    //ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));

    //if (rand()%100 > 75)
    //  ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%9+1)));

    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(shapes, 8));

    QPen graphPen;
    graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);

    ui->customPlot->graph()->setPen(graphPen);
    ui->customPlot->graph()->rescaleAxes(true);
    ui->customPlot->replot();

}


void MainWindow::cleartext(){

    ui->textEdit_serial->clear();

}

void MainWindow::MostrarData(){
    ui->textEdit_serial->insertPlainText(Stringdata);

}
