#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QSettings>
#include <QtWidgets/QAction>
#include <QKeyEvent>
#include <QtWidgets/QShortcut>
#include <QKeySequence>

#include "tool.h"
#include "trace_model.h"
#include "canvas.h"
#include "timeedit.h"

namespace vis4 {

/**
 * Standard implementation of the 'goto' tool. Works only
 * with Time instances where result of raw() is convertible
 * either to int or long long, and
 * only when the actual integer value fits into int.
 */
class Goto : public Tool
{
    Q_OBJECT
public:
    Goto(QWidget* parent, Canvas* canvasPtr) :
        Tool(parent, canvasPtr)
    {
        setObjectName("goto");
        setWhatsThis(tr("<b>Goto</b>"
                     "<p>Sets the start time, or the time range that must "
                     "be shown."));

        setWindowTitle(tr("Goto"));
        setFocusPolicy(Qt::StrongFocus);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QGroupBox* group = new QGroupBox(tr("Goto"), this);
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(group);

        QVBoxLayout* gotoLayout = new QVBoxLayout(group);

        setStart_ = new QRadioButton(tr("Set start"), group);
        setStart_->setObjectName("setStart");
        gotoLayout->addWidget(setStart_);

        QHBoxLayout* setStartBox = new QHBoxLayout(0);
        gotoLayout->addLayout(setStartBox);

        setStartBox->addSpacing(12);
        setStartBox->addWidget(new QLabel(tr("Time:"), group));
        setStartBox->addWidget(setStart_time = new TimeEdit(group));

        setRange_ = new QRadioButton(tr("Set range"), group);
        setRange_->setObjectName("setRange");
        gotoLayout->addWidget(setRange_);

        QGridLayout* setRangeBox = new QGridLayout(0);
        gotoLayout->addLayout(setRangeBox);
        setRangeBox->setColumnMinimumWidth(0, 12);
        setRangeBox->addWidget(new QLabel(tr("Start:"), group), 0, 1);
        setRangeBox->addWidget(setRange_begin = new TimeEdit(group), 0, 2);
        setRangeBox->addWidget(new QLabel(tr("End:"), group), 1, 1);
        setRangeBox->addWidget(setRange_end = new TimeEdit(group), 1, 2);

        showAllTrace = new QRadioButton(tr("Show all trace"), group);
        showAllTrace->setObjectName("showAllTrace");
        gotoLayout->addWidget(showAllTrace);

        errorMessage = new QLabel(group);
        errorMessage->setWordWrap(true);
        gotoLayout->addWidget(errorMessage);

        gotoLayout->addStretch();

        QHBoxLayout* buttons = new QHBoxLayout(0);
        gotoLayout->addLayout(buttons);
        buttons->addStretch();

        okButton = new QPushButton(tr("OK"), group);
        buttons->addWidget(okButton);

        QPushButton* reset = new QPushButton(tr("Reset"), group);
        buttons->addWidget(reset);

        QShortcut* setStartShortcut = new QShortcut(
            QKeySequence(Qt::Key_5), window());
        setStartShortcut->setContext(Qt::ApplicationShortcut);

        QShortcut* setRangeShortcut = new QShortcut(
            QKeySequence(Qt::Key_Slash), window());
        setRangeShortcut->setContext(Qt::ApplicationShortcut);

        connect(okButton, SIGNAL(clicked(bool)), this,
                          SLOT(done()));

        connect(reset, SIGNAL(clicked(bool)), this,
                       SLOT(reset()));

        connect(setStart_, SIGNAL(toggled(bool)), this,
                           SLOT(checkboxToggled(bool)));

        connect(setRange_, SIGNAL(toggled(bool)), this,
                           SLOT(checkboxToggled(bool)));

        connect(showAllTrace, SIGNAL(toggled(bool)), this,
                              SLOT(checkboxToggled(bool)));

        connect(setStart_time, SIGNAL(focusIn()), this,
                               SLOT(spinButtonFocusIn()));

        connect(setStart_time, SIGNAL(timeChanged(Time)), this,
                               SLOT(validate()));

        connect(setStart_time, SIGNAL(textChanged()), this,
                               SLOT(validate()));

        connect(setRange_begin, SIGNAL(focusIn()), this,
                                SLOT(spinButtonFocusIn()));

        connect(setRange_end, SIGNAL(focusIn()), this,
                              SLOT(spinButtonFocusIn()));

        connect(setRange_begin, SIGNAL(timeChanged(Time)), this,
                                SLOT(validate()));

        connect(setRange_begin, SIGNAL(textChanged()), this,
                                SLOT(validate()));

        connect(setRange_end, SIGNAL(timeChanged(Time)), this,
                              SLOT(validate()));

        connect(setRange_end, SIGNAL(textChanged()), this,
                              SLOT(validate()));

        connect(setStartShortcut, SIGNAL(activated()), this,
                                  SLOT(setStart()));

        connect(setRangeShortcut, SIGNAL(activated()), this,
                                  SLOT(setRange()));

        connect(getCanvas(), SIGNAL(modelChanged(TraceModelPtr &)), this,
                             SLOT(modelChanged(TraceModelPtr &)));

        modelChanged(model());
        restoreState();
    }

    QAction* createAction()
    {
        selfAction = new QAction(QIcon(":/link.png"), tr("Go to"), this);
        selfAction->setShortcut(QKeySequence(Qt::Key_R));
        return selfAction;
    }

    void activate()
    {
    }

    void deactivate()
    {
    }

    void restoreState()
    {
        QSettings settings;
        prepareSettings(settings);

        QString checked = settings.value("goto_tool/checkbox").toString();
        if (checked.isEmpty())
        {
            setStart_->setChecked(true);
        }
        else
        {
            if (QRadioButton* rb = findChild<QRadioButton*>(checked))
            {
                rb->setChecked(true);
            }
            else
            {
                setStart_->setChecked(true);
            }
        }
    }

    void saveState()
    {
        QSettings settings;
        prepareSettings(settings);

        QString s = setStart_->objectName();
        if (setRange_->isChecked())
        {
            s = setRange_->objectName();
        }
        if (showAllTrace->isChecked())
        {
            s = showAllTrace->objectName();
        }
        settings.setValue("goto_tool/checkbox", s);
    }

private slots:

    void modelChanged(TraceModelPtr & model)
    {
        TraceModelPtr root = model->root();

        setStart_time->setMinimum(root->getMinTime());
        setRange_begin->setMinimum(root->getMinTime());
        setRange_end->setMinimum(root->getMinTime());

        setStart_time->setMaximum(root->getMaxTime());
        setRange_begin->setMaximum(root->getMaxTime());
        setRange_end->setMaximum(root->getMaxTime());

        setStart_time->setTime(model->getMinTime());
        setRange_begin->setTime(model->getMinTime());
        setRange_end->setTime(model->getMaxTime());
    }

    void done()
    {
        if (setStart_->isChecked())
        {
            Time min = model()->getMinTime();
            Time max = model()->getMaxTime();
            Time delta = max - min;

            Time new_min = setStart_time->time();
            Time new_max = new_min + delta;
            if (new_max > model()->root()->getMaxTime())
            {
                new_max = model()->root()->getMaxTime();
            }

            getCanvas()->setModel(model()->setRange(new_min, new_max));
        }
        else if (setRange_->isChecked())
        {
            Time new_max = setRange_end->time();
            getCanvas()->setModel(model()->setRange(setRange_begin->time(), new_max));
        }
        else if (showAllTrace->isChecked())
        {
            TraceModelPtr root = model()->root();
            getCanvas()->setModel(model()->setRange(root->getMinTime(), root->getMaxTime()));
        }
    }

    void reset()
    {
        activate();
    }


    void checkboxToggled(bool checked)
    {
        if (checked)
        {
            validate();
            saveState();
        }
    }

    void spinButtonFocusIn()
    {
        if (sender() == setStart_time)
        {
            setStart_->setChecked(true);
        }
        else if (sender() == setRange_begin || sender() == setRange_end)
        {
            setRange_->setChecked(true);
        }
    }

    void validate()
    {
        bool error = true;

        if (setStart_->isChecked())
        {
                error = false;
        }

        if (setRange_->isChecked())
        {
            Time new_min = setRange_begin->time();
            Time new_max = setRange_end->time();
            Time min_resolution = model()->getMinResolution();
            if (new_min > new_max)
            {
                errorMessage->setText(tr("<b>Error: start > end</b>"));
                okButton->setEnabled(false);
            }
            else if (new_max-new_min < min_resolution)
            {
                QString msg = tr("Error: Time interval is too small."
                                 "The minimal interval is %1").arg(min_resolution.toString());

                errorMessage->setText("<b>"+msg+"</b>");
                okButton->setEnabled(false);
            }
            else
            {
                error = false;
            }
        }

        if (!error)
        {
            errorMessage->setText("");
            okButton->setEnabled(true);
        }
    }

    void setRange()
    {
        setStart_->setChecked(true);
    }

    void setStart()
    {
        setRange_->setChecked(true);
    }

private:

    void keyReleaseEvent(QKeyEvent* event)
    {
        if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && okButton->isEnabled())
        {
                okButton->animateClick();
        }
        else
        {
            Tool::keyReleaseEvent(event);
        }
    }

    QAction* selfAction;

    QRadioButton* setStart_;
    QRadioButton* setRange_;
    QRadioButton* showAllTrace;

    TimeEdit* setStart_time;
    TimeEdit* setRange_begin;
    TimeEdit* setRange_end;

    QLabel* errorMessage;
    QPushButton* okButton;

};

Tool* createGoto(QWidget* parent, Canvas* canvas)
{
    return new Goto(parent, canvas);
}


}

