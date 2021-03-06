#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidgetAction>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWhatsThis>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMenu>
#include <QDebug>

#include "main_window.h"
#include "trace_model.h"
#include "trace_painter.h"
#include "canvas.h"
#include "tools/tool.h"

namespace vis4 {

MainWindow::MainWindow() : 
    QMainWindow(),
    currentTool(nullptr),
    modeActions(new QActionGroup(this))
{}

void MainWindow::initialize(TraceModelPtr& model)
{
    model = restoreModel(model);//? trying to take settings from QSettings, but this is sooo bad
    initCanvas(model);
    // Restore window geometry
    QRect r = restoreGeometry();
    if (r.isValid())
    {
        setGeometry(r);
    }
    else
    {
        r = QApplication::desktop()->screenGeometry();
        resize(r.width(), r.height());
    }

    initToolbar();
    initSidebar();

    xinitialize(sidebarContents, canvas);

    toolbar->addActions(modeActions->actions());

    Q_ASSERT(browser);
    browser->addToolbarActions(toolbar);

    QAction* resetView = new QAction(this);
    resetView->setShortcut(Qt::Key_Escape);
    resetView->setShortcutContext(Qt::WindowShortcut);
    addAction(resetView);

    connect(resetView, SIGNAL(triggered(bool)), this,
                       SLOT(resetView()));

    toolbar->addSeparator();

    // Restore time unit and format
    QSettings settings;
    QString time_unit = settings.value("time_unit", Time::unit_name(0)).toString();
    for(int i = 0; i < Time::units().size(); ++i)
    {
        if (Time::unit_name(i) == time_unit)
        {
            Time::setUnit(i);
            break;
        }
    }
    if (settings.value("time_format", "separated") == "separated")
    {
        Time::setFormat(Time::Advanced);
    }

    // Adding freestanding tools
    foreach (QAction * action, freestandingTools)
    {
        toolbar->addAction(action);
    }

    actPrint = new QAction(tr("Print"), this);
    actPrint->setIcon(QIcon(":/printer.png"));
    actPrint->setShortcut(Qt::Key_P);
    actPrint->setToolTip(tr("Print"));
    actPrint->setWhatsThis(
        tr("<b>Print</b>"
           "<p>Prints the time diagram or any portion of it."));
    toolbar->addAction(actPrint);
    connect(actPrint, SIGNAL(triggered()), this, SLOT(actionPrint()));

    toolbar->addAction(QWhatsThis::createAction(this));


    addShortcuts(toolbar);

    // Restore toolbar's settings
    QString iconSize = settings.value("toolbar_icon_size", "big").toString();
    if (iconSize == "small")
        toolbar->setIconSize(QSize(16, 16));
    if (iconSize == "big")
        toolbar->setIconSize(QSize(24, 24));

    QString textPosition = settings.value("toolbar_text_position", "text beside icon").toString();
    if (textPosition == "icon only")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    if (textPosition == "text only")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    if (textPosition == "text beside icon")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    if (textPosition == "text under icon")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }

    // Restore active tool
    prepareSettings(settings, model);

    QString current_tool = settings.value("current_tool").toString();
    bool tool_set = false;

    if (!current_tool.isEmpty())
    {
        if (current_tool == "none")
        {
            sidebar->setVisible(false);
        }
        else if (Tool* tool = findChild<Tool*>(current_tool))
        {
            tool->getAction()->setChecked(true);
            activateTool(tool);
            tool_set = true;
        }
    }

    if (!tool_set)
    {
        activateTool(browser);
    }

    connect(canvas, SIGNAL(mouseEvent(QEvent*, Canvas::clickTarget, int, StateModel*, const Time&, bool)), this,
                    SLOT(mouseEvent(QEvent*, Canvas::clickTarget, int, StateModel*, const Time&, bool)));

    connect(canvas, SIGNAL(mouseMoveEvent(QMouseEvent*, Canvas::clickTarget, int, const Time&)), this,
                    SLOT(mouseMoveEvent(QMouseEvent*, Canvas::clickTarget, int, const Time&)));

    connect(canvas, SIGNAL(modelChanged(TraceModelPtr &)), this,
                    SLOT(modelChanged(TraceModelPtr &)));

    // Now we can draw the trace
    canvas->setVisible(true);
}

void MainWindow::initCanvas(TraceModelPtr & model)
{
    canvas = new Canvas(this);

    if (char *e = getenv("VIS3_PORTABLE_DRAWING"))
    {
        char *v = strstr(e, "=");
        if (!v || strcmp(v, "1") == 0)
        {
            canvas->setPortableDrawing(true);
        }
    }//? is this working?

    // Prevent trace drawing until window geometry restore.
    canvas->setVisible(false);

    canvas->setModel(model);
    setCentralWidget(canvas);
}

void MainWindow::installTool(Tool* tool)
{
    QAction* action = tool->getAction();

    action->setCheckable(true);

    tools_list.push_back(tool);

    sidebarContents->addWidget(tool);
    modeActions->addAction(action);

    connect(action, SIGNAL(triggered(bool)), this,
                    SLOT(actionTriggered()));

    connect(tool, SIGNAL(activateMe()), this,
                  SLOT(activateTool()));

    connect(tool, SIGNAL(showEvent(EventModel*)), this,
                  SLOT(showEvent(EventModel*)));

    connect(tool, SIGNAL(showState(StateModel*)), this,
                  SLOT(showState(StateModel*)));

    connect(tool, SIGNAL(browse()), this,
                  SLOT(browse()));

    action->setWhatsThis(tool->whatsThis());
}

void MainWindow::installBrowser(Browser *browser)
{
    installTool(browser);
    this->browser = browser;
    browse_action = browser->getAction();
    browse_action->setToolTip(browse_action->toolTip() + " " +
        tr("(Shortcut: <b>%1</b>)").arg("Esc"));
}

void MainWindow::installFreestandingTool(QAction *action)
{
    freestandingTools.push_back(action);
}

void MainWindow::activateTool(Tool* tool)
{
    bool firstTime = (currentTool == nullptr);

    if (tool == nullptr)
    {
        tool = static_cast<Tool*>(sender());
    }

    if (currentTool)
    {
        currentTool->deactivate();
    }

    sidebarContents->setCurrentWidget(tool);
    sidebar->setWindowTitle(tool->windowTitle());

    currentTool = tool;
    tool->activate();

    if (sidebar->isVisible())
    {
        tool->getAction()->setChecked(true);
    }

    // Save current tool in settings
    QSettings settings;
    prepareSettings(settings, model());

    QString toolName = (sidebar->isVisible() || firstTime) ? tool->objectName() : "none";
    settings.setValue("current_tool", toolName);
}

void MainWindow::addShortcuts(QWidget *widget)
{
    foreach(QAction *action, widget->actions())
    {
        QString tooltip = action->toolTip();
        if (!tooltip.isEmpty() && !action->shortcut().isEmpty())
        {
                action->setToolTip(tooltip + " " +
                                   tr("(Shortcut: <b>%1</b>)")
                                   .arg(action->shortcut().toString()));
        }
    }
}

//? it is better to use local trace settings for some number of last traces
/** use settings directory of settings */
void MainWindow::prepareSettings(QSettings& settings,
                                  const TraceModelPtr & model) const
{
    // No per-trace settings, everything is global.
    settings.beginGroup("settings");
}

void MainWindow::saveGeometry(const QRect& rect) const
{
    QSettings settings;
    prepareSettings(settings, model());
    settings.setValue("window_geometry", rect);
}

/** just returns saved geometry, doesn't really restore it */
QRect MainWindow::restoreGeometry() const
{
    QSettings settings;
    prepareSettings(settings, model());
    return settings.value("window_geometry").toRect();
}

void MainWindow::sidebarShowHide()
{
    sidebar->setVisible(sidebar->isHidden());
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != sidebar)
    {
        return QMainWindow::eventFilter(watched, event);
    }

    if (!currentTool)
    {
        return false;
    }

    if (event->type() == QEvent::Close || event->type() == QEvent::Hide)
    {
        currentTool->getAction()->setChecked(false);
    }

    if (event->type() == QEvent::Show)
    {
        currentTool->getAction()->setChecked(true);
    }

    return false;
}

void MainWindow::moveEvent(QMoveEvent*)
{
    saveGeometry(geometry());
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    saveGeometry(geometry());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Pass close event to canvas in order to stop background drawing
    canvas->closeEvent(event);
}

void MainWindow::actionTriggered()
{
    QAction* action = static_cast<QAction*>(sender());
    Tool* tool = qobject_cast<Tool*>(action->parent());

    sidebar->show();
    activateTool(tool);
}

void MainWindow::modelChanged(TraceModelPtr& model)
{
    saveModel(model);
    // Disable/enable printing button
    actPrint->setEnabled(model->getVisibleComponents().size() != 0);
}

void MainWindow::showEvent(EventModel* event)
{
    browser->doShowEvent(event);
    activateTool(browser);
}

void MainWindow::showState(StateModel* state)
{
    browser->doShowState(state);
    activateTool(browser);
}


void MainWindow::browse()
{
    activateTool(browser);
}

void MainWindow::actionPrint()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog printDialog(&printer, this->parentWidget());

    /** Set default parameters */
    printDialog.addEnabledOption(QAbstractPrintDialog::PrintPageRange);
    printDialog.addEnabledOption(QAbstractPrintDialog::PrintSelection);
    printDialog.setPrintRange(QAbstractPrintDialog::Selection);

    // Convert max_time from Time to int
    TraceModelPtr model = canvas->getModel();
    timespec maxTime = model->root()->getMaxTime().getData();
    long long maxTimeLL = maxTime.tv_sec * MAX_NSEC + maxTime.tv_nsec;
    printDialog.setMinMax(0, static_cast<int>(maxTimeLL));

    // Show dialog
    if (printDialog.exec() == QDialog::Accepted)
    {
        Time min_time, max_time;
        Time timePerPage = model->getMaxTime() - model->getMinTime();

        if (printDialog.printRange() == QAbstractPrintDialog::Selection)
        {
            min_time = model->getMinTime();
            max_time = model->getMaxTime();
        }
        else if (printDialog.printRange() == QAbstractPrintDialog::AllPages)
        {
            min_time = model->root()->getMinTime();
            max_time = model->root()->getMaxTime();
        }
        else if (printDialog.printRange() == QAbstractPrintDialog::PageRange)
        {
            min_time = Time(printDialog.fromPage());
            max_time = Time(printDialog.toPage());
        }

        model = model->setRange(min_time, max_time);

        TracePainter tp;
        tp.setModel(model);
        tp.setPaintDevice(&printer);

        QApplication::setOverrideCursor(Qt::WaitCursor);
        tp.drawTrace(timePerPage, true /* start in background */);
        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::saveModel(TraceModelPtr& model)
{
    QSettings settings;
    prepareSettings(settings, model);
    settings.setValue("trace_state", model->save());
}

TraceModelPtr & MainWindow::restoreModel(TraceModelPtr & model)
{
    QSettings settings;
    prepareSettings(settings, model);
    QString model_desc = settings.value("trace_state").toString();
    if (!model_desc.isEmpty())
    {
        model->restore(model_desc);
    }
    return model;
}

TraceModelPtr MainWindow::model() const
{
    return canvas->getModel();
}


void MainWindow::mouseEvent(QEvent* event,
                    Canvas::clickTarget target,
                    int component,
                    StateModel* state,
                    const Time& time,
                    bool events_near)
{
    bool done = currentTool->mouseEvent(event, target, component, state, time, events_near);
    if (!done)
    {
        for (int i = 0; i < tools_list.size(); ++i)
        {
            Tool* tool = tools_list[i];
            if (tool != currentTool)
            {
                tool->mouseEvent(event, target, component, state, time, events_near);
            }
        }
    }
}

 void MainWindow::mouseMoveEvent(QMouseEvent* event, Canvas::clickTarget target, int component, const Time& time)
{
    bool done = currentTool->mouseMoveEvent(event, target, component, time);
    if (!done)
    {
        for (int i = 0; i < tools_list.size(); ++i)
        {
            Tool* tool = tools_list[i];
            if (tool != currentTool)
            {
                tool->mouseMoveEvent(event, target, component, time);
            }
        }
    }
}

/** Configure toolbar right-click menu. */
void MainWindow::toolbarContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(menu, SIGNAL(triggered(QAction*)), this,
                  SLOT( toolbarSettingsChanged(QAction*)));

    /** Adding icon size menu. */
    QMenu* iconsMenu = menu->addMenu(tr("Icons size"));

    QAction* action;
    action = iconsMenu->addAction(tr("Small icons"));
    action->setObjectName("act_smallIcons");
    action->setCheckable(true);
    action->setChecked(toolbar->iconSize().width() == 16);

    action = iconsMenu->addAction(tr("Big icons"));
    action->setObjectName("act_bigIcons");
    action->setCheckable(true);
    action->setChecked(toolbar->iconSize().width() == 24);

    /** Adding text position menu. */
    QMenu* textposMenu = menu->addMenu(tr("Text position"));

    action = textposMenu->addAction(tr("Icon only"));
    action->setObjectName("act_iconOnly");
    action->setCheckable(true);
    action->setChecked(toolbar->toolButtonStyle() == Qt::ToolButtonIconOnly);

    action = textposMenu->addAction(tr("Text only"));
    action->setObjectName("act_textOnly");
    action->setCheckable(true);
    action->setChecked(toolbar->toolButtonStyle() == Qt::ToolButtonTextOnly);

    action = textposMenu->addAction(tr("Text beside icon"));
    action->setObjectName("act_textBeside");
    action->setCheckable(true);
    action->setChecked(toolbar->toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

    action = textposMenu->addAction(tr("Text under icon"));
    action->setObjectName("act_textUnder");
    action->setCheckable(true);
    action->setChecked(toolbar->toolButtonStyle() == Qt::ToolButtonTextUnderIcon);

    menu->popup(toolbar->mapToGlobal(pos));
}

void MainWindow::toolbarSettingsChanged(QAction* action)
{
    QSettings settings;

    if (action->objectName() == "act_smallIcons")
    {
        toolbar->setIconSize(QSize(16, 16));
        settings.setValue("toolbar_icon_size", "small");
    }
    else if (action->objectName() == "act_bigIcons")
    {
        toolbar->setIconSize(QSize(24, 24));
        settings.setValue("toolbar_icon_size", "big");
    }
    else if (action->objectName() == "act_iconOnly")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        settings.setValue("toolbar_text_position", "icon only");
    }
    else if (action->objectName() == "act_textOnly")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
        settings.setValue("toolbar_text_position", "text only");
    }
    else if (action->objectName() == "act_textBeside")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        settings.setValue("toolbar_text_position", "text beside icon");
    }
    else if (action->objectName() == "act_textUnder")
    {
        toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        settings.setValue("toolbar_text_position", "text under icon");
    }
}

void MainWindow::resetView()
{
    for (int i = 0; i < tools_list.size(); ++i)
    {
        tools_list[i]->reset();
    }
    activateTool(browser);
    canvas->setFocus(Qt::OtherFocusReason);
}

void MainWindow::initToolbar()
{
    toolbar = new QToolBar(tr("Toolbar"), this);
    addToolBar(toolbar);

    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->toggleViewAction()->setEnabled(false);
    toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolbar, SIGNAL(customContextMenuRequested(const QPoint&)), this,
                     SLOT(toolbarContextMenu(const QPoint&)));

}

void MainWindow::initSidebar()
{
    sidebar = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, sidebar);
    sidebar->setWindowTitle(tr("Browse trace"));
    sidebar->setFeatures(QDockWidget::DockWidgetClosable);
    sidebar->installEventFilter(this);

    sidebarContents = new QStackedWidget(sidebar);
    sidebar->setWidget(sidebarContents);

    QAction* sidebarTrigger = new QAction(this);
    sidebarTrigger->setShortcut(Qt::Key_F9);
    sidebarTrigger->setShortcutContext(Qt::WindowShortcut);
    addAction(sidebarTrigger);

    connect(sidebarTrigger, SIGNAL(triggered(bool)), this,
                            SLOT(sidebarShowHide()));
}

void MainWindow::xinitialize(QWidget* toolContainer, Canvas* canvas)
{
    Browser* browser = createBrowser(toolContainer, canvas);
    installBrowser(browser);

    installTool(createGoto(toolContainer, canvas));//работает только показ всей трассы
    installTool(createMeasure(toolContainer, canvas));//работает полностью

    installTool(createFilter(toolContainer, canvas));//частично работает

    //Tool* find = createFind(toolContainer, canvas);//не работает
    //installTool(find);
    //connect(find, SIGNAL(extraHelp(const QString&)), browser,
    //              SLOT(extraHelp(const QString&)));
}

}
