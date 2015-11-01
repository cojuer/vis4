#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QKeyEvent>

#include "tool.h"
#include "canvas.h"
#include "trace_model.h"
#include "main_window.h"

namespace vis4 {

Tool::Tool(QWidget* parentPtr, Canvas* canvasPtr) :
    QWidget(parentPtr),
    canvasPtr(canvasPtr),
    actionPtr(nullptr)
{
    //? this can fail if canVasPtr->parent() will not be main window
    mainWindowPtr = dynamic_cast<MainWindow*>(canvasPtr->parent());
}

boost::shared_ptr<Trace_model>& Tool::model() const
{
    return canvasPtr->model();
}

QAction* Tool::getAction()
{
    if (actionPtr == nullptr)
    {
        actionPtr = createAction();
    }
    return actionPtr;
}

void Tool::prepareSettings(QSettings& settings) const
{
    assert(mainWindowPtr != 0);
    mainWindowPtr->prepare_settings(settings, model());
}

bool Tool::event(QEvent* eventPtr)
{
    if (eventPtr->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(eventPtr);
        switch(keyEvent->key())
        {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
                eventPtr->accept();
        }
    }

    return QWidget::event(eventPtr);
}

}
