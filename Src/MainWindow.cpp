#include <QtWidgets>
#include <QMenu>
#include <QLabel>
#include <QSettings>

#include "MainWindow.h"
#include "Actions.h"
#include "GeometryPlot.h"
#include "engine/global.h"

namespace BooleanOffset {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _actions(new Actions(this)),
    _geomPlot(new GeometryPlot),
    _coordinateLabel(new QLabel),
    _settings(new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()))
{
    {
        QMenu * const fileMenu = menuBar()->addMenu("File");
        fileMenu->addAction(_actions->fileNew);
        fileMenu->addAction(_actions->fileOpen);
        fileMenu->addAction(_actions->fileSaveAs);
        fileMenu->addAction(_actions->fileQuit);

        QMenu * const toolMenu = menuBar()->addMenu("Tool");
        toolMenu->addAction(_actions->toolLine);
        toolMenu->addAction(_actions->toolBox);
        toolMenu->addAction(_actions->toolRoundedBox);
        toolMenu->addAction(_actions->toolCircle);
        toolMenu->addAction(_actions->toolArc);
        toolMenu->addAction(_actions->toolStick);
        toolMenu->addAction(_actions->toolDonut);

        QMenu * const operationMenu = menuBar()->addMenu("Operation");
        operationMenu->addAction(_actions->operationBoolean);
        operationMenu->addAction(_actions->operationOffsetOut);
        operationMenu->addAction(_actions->operationOffsetIn);
        operationMenu->addSeparator();
        operationMenu->addAction(_actions->operationGhostMode);
        operationMenu->addSeparator();
        operationMenu->addAction(_actions->operationReload);
    }
    {
        QToolBar * const toolbar = new QToolBar;
        toolbar->setFloatable(false);
        toolbar->setMovable(false);
        toolbar->setAllowedAreas(Qt::TopToolBarArea);
        setContextMenuPolicy(Qt::NoContextMenu);
        toolbar->setObjectName("toolbar");
        toolbar->addAction(_actions->fileNew);
        toolbar->addSeparator();
        toolbar->addAction(_actions->toolLine);
        toolbar->addAction(_actions->toolBox);
        toolbar->addAction(_actions->toolRoundedBox);
        toolbar->addAction(_actions->toolCircle);
        toolbar->addAction(_actions->toolArc);
        toolbar->addAction(_actions->toolStick);
        toolbar->addAction(_actions->toolDonut);
        toolbar->addSeparator();
        toolbar->addAction(_actions->operationBoolean);
        toolbar->addAction(_actions->operationOffsetOut);
        toolbar->addAction(_actions->operationOffsetIn);
        toolbar->addSeparator();
        toolbar->addAction(_actions->operationGhostMode);
        toolbar->addSeparator();
        toolbar->addAction(_actions->operationReload);
        toolbar->addAction(_actions->fileQuit);
        addToolBar(Qt::RightToolBarArea, toolbar);
    }
    statusBar()->addPermanentWidget(_coordinateLabel);
    setCentralWidget(_geomPlot);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~(Qt::WindowMaximizeButtonHint));

    _loadSettings();

    connect(_actions->fileNew, &QAction::triggered, _geomPlot, &GeometryPlot::clear);
    connect(_actions->fileOpen, &QAction::triggered, this, &MainWindow::slot_FileOpen);
    connect(_actions->fileSaveAs, &QAction::triggered, this, &MainWindow::slot_FileSaveAs);
    connect(_actions->fileQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(_actions->toolsGroup, &QActionGroup::triggered, this, &MainWindow::slot_ToolSelected);
    connect(_actions->operationBoolean, &QAction::triggered, _geomPlot, &GeometryPlot::BooleanButtonFunction);
    static const double OFFSET_RADIUS = 10.0;
    connect(_actions->operationOffsetOut, &QAction::triggered, _geomPlot, [this]() {
        if (_actions->operationGhostMode->isChecked())
            _geomPlot->ghostOffset(OFFSET_RADIUS);
        else
            _geomPlot->offset(OFFSET_RADIUS);
    });
    connect(_actions->operationOffsetIn, &QAction::triggered, _geomPlot, [this]() {
        if (_actions->operationGhostMode->isChecked())
            _geomPlot->ghostOffset(-OFFSET_RADIUS);
        else
            _geomPlot->offset(-OFFSET_RADIUS);
    });
    connect(_actions->operationReload, &QAction::triggered, _geomPlot, &GeometryPlot::reload);
    connect(_geomPlot, &GeometryPlot::pointHovered, this, &MainWindow::slot_CoordinateHovered);
    connect(_geomPlot, &GeometryPlot::toolChanged, this, &MainWindow::slot_ToolChanged);

    //_actions->toolSelect->setChecked(true);
    slot_CoordinateHovered(QPointF(0, 0));
}

MainWindow::~MainWindow()
{
}

void MainWindow::slot_FileOpen()
{
    const QString filePath = QFileDialog::getOpenFileName(this, "Open Test Case File", _currentDirectory, "Test Case Files (*.txt)");
    if (!filePath.isEmpty())
    {
        const QFileInfo info(filePath);
        _currentDirectory = info.absolutePath();
        _geomPlot->open(filePath);
    }
}

void MainWindow::slot_FileSaveAs()
{
    const QString filePath = QFileDialog::getSaveFileName(this, "Save Test Case File", _currentDirectory, "Test Case Files (*.txt)");
    if (!filePath.isEmpty())
    {
        const QFileInfo info(filePath);
        _currentDirectory = info.absolutePath();
        _geomPlot->save((info.suffix().compare("txt", Qt::CaseInsensitive) == 0) ? filePath : (filePath + ".txt"));
    }
}

void MainWindow::slot_ToolSelected(QAction *act)
{
    const int newTool = act ? act->data().toInt() : -1;
    _geomPlot->setTool(newTool);
}

void MainWindow::slot_ToolChanged(int newTool)
{
    const QList<QAction*> acts = _actions->toolsGroup->actions();
    for (QList<QAction*>::const_iterator it = acts.begin(); it != acts.end(); ++it)
    {
        QAction * const act = *it;
        const bool matches = (act->data().toInt() == newTool);
        act->setChecked(matches);
    }
}

void MainWindow::slot_CoordinateHovered(const QPointF &pt)
{
    _coordinateLabel->setText(QString("X: ") + QString::number(pt.x()) + ", Y: "+ QString::number(pt.y()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    _saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::_saveSettings()
{
    _settings->beginGroup("MainWindow");
    _settings->setValue("geometry", saveGeometry());
    _settings->setValue("windowState", saveState());
    _settings->setValue("working_directory", _currentDirectory);
    _settings->endGroup();
}

void MainWindow::_loadSettings()
{
    _settings->beginGroup("MainWindow");
    restoreGeometry(_settings->value("geometry").toByteArray());
    restoreState(_settings->value("windowState").toByteArray());
    _currentDirectory = _settings->value("working_directory").toString();
    _settings->endGroup();
}

} // namespace BooleanOffset
