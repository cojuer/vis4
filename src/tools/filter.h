#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>

#include "tool.h"
#include "canvas.h"
#include "trace_model.h"
#include "selection_widget.h"

namespace vis4 {

class Filter : public Tool
{
    Q_OBJECT
public:
    Filter(QWidget* parent, Canvas* c)
    : Tool(parent, c), state_restored(false)
    {
        setObjectName("filter");
        setWindowTitle(tr("Filter"));

        setWhatsThis(tr("<b>Filter</b>"
                     "<p>Allows you to select which components and event "
                     "types are shown."
                     "<p>There are two lists -- list of components and "
                     "list of events. You can individually enable and disable "
                     "each one. Changes are immediately reflected in the "
                     "diagram."));

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        tabWidget = new QTabWidget(this);
        mainLayout->addWidget(tabWidget);

        // Create components selector.
        components = new SelectionWidget(this, SelectionWidget::COMPONENTS_SELECTOR);
        components->setObjectName("components_tab");
        components->minimizeSize(true);
        tabWidget->addTab(components, tr("Components"));

        components->initialize(model()->components());

        connect(components, SIGNAL( selectionChanged(const vis4::Selection&) ),
                this, SLOT(filtersChanged()));
        connect(components, SIGNAL( itemDoubleClicked(int) ),
                this, SLOT( goToComponent(int) ));

        // Create tabs with events and states selectors.

        events = new SelectionWidget(this);
        events->minimizeSize(true);

        groupEventsCheckBox = new QCheckBox(tr("Show arrows for group events"), this);
        connect(groupEventsCheckBox, SIGNAL(  stateChanged(int) ),
                this, SLOT(filtersChanged()));

        QWidget* eventsTab = new QWidget(this);
        eventsTab->setObjectName("events_tab");
        eventsTab->setLayout(new QVBoxLayout());
        eventsTab->layout()->addWidget(groupEventsCheckBox);
        eventsTab->layout()->addWidget(events);
        tabWidget->addTab(eventsTab, tr("Events"));

        events->initialize(model()->events());

        states = new SelectionWidget(this);
        states->setObjectName("states_tab");
        states->minimizeSize(true);
        tabWidget->addTab(states, tr("States"));

        states->initialize(model()->states());

        connect(events, SIGNAL(selectionChanged(const vis4::Selection &)), this,
                        SLOT(filtersChanged()));

        connect(states, SIGNAL(selectionChanged(const vis4::Selection &)), this,
                        SLOT(filtersChanged()));

        connect(getCanvas(), SIGNAL(modelChanged(Trace_model::TraceModelPtr &)), this,
                             SLOT(modelChanged(Trace_model::TraceModelPtr &)));

        modelChanged(model());
        restoreState();

    }

    QAction* createAction()
    {
        QAction* filter_action = new QAction(QIcon(":/filter.png"),
                                             tr("Fi&lter"),//? strange
                                             this);
        filter_action->setShortcut(QKeySequence(Qt::Key_L));
        return filter_action;
    }

    void activate() {}
    void deactivate() {}

private: /* methods */

    void saveState()
    {
        if (!state_restored) return;

        QSettings settings;
        prepareSettings(settings);
        settings.beginGroup("filter_tool");

        settings.setValue("active_tab", tabWidget->currentWidget()->objectName());

        settings.beginGroup("components");
        components->saveState(settings);
        settings.endGroup();

        settings.beginGroup("events");
        events->saveState(settings);
        settings.setValue("show_arrows", groupEventsCheckBox->isChecked() ? "yes" : "no");
        settings.endGroup();

        settings.beginGroup("states");
        states->saveState(settings);
        settings.endGroup();
    }

    void restoreState()
    {
        QSettings settings;
        prepareSettings(settings);
        settings.beginGroup("filter_tool");

        QString active_tab = settings.value("active_tab", "components_tab").toString();
        if (active_tab == "events_tab")
            tabWidget->setCurrentIndex(1);
        else if (active_tab == "states_tab")
            tabWidget->setCurrentIndex(2);

        settings.beginGroup("components");
        components->restoreState(settings);
        settings.endGroup();

        settings.beginGroup("events");
        events->restoreState(settings);
        groupEventsCheckBox->setChecked(
            settings.value("show_arrows", "yes").toString() == "yes");
        settings.endGroup();

        settings.beginGroup("states");
        states->restoreState(settings);
        settings.endGroup();

        state_restored = true;
        filtersChanged();
    }

private slots:

    void modelChanged(TraceModelPtr & model)
    {
        components->initialize(model->components());
        events->initialize(model->events());
        states->initialize(model->available_states(), model->states());

        saveState();
    }

    void filtersChanged()
    {
        TraceModelPtr filtered = model();

        if (sender() == components || !sender())
            filtered = filtered->filter_components(components->selection());

        if (sender() == events || !sender())
            filtered = filtered->filter_events(events->selection());

        if (sender() == groupEventsCheckBox || !sender())
            filtered = filtered->setGroupsEnabled(groupEventsCheckBox->isChecked());

        if (sender() == states || !sender())
            filtered = filtered->filter_states(states->selection());

        getCanvas()->setModel(filtered);
    }

    void goToComponent(int component)
    {
        if (!model()->components().hasChildren(component))
            component = model()->components().itemParent(component);

        TraceModelPtr m = model()->set_parent_component(component);
        getCanvas()->setModel(m);
    }

private: /* members */

    SelectionWidget * components;

    QTabWidget * tabWidget;
    SelectionWidget * events;
    QCheckBox * groupEventsCheckBox;
    SelectionWidget * states;

    bool state_restored;
};

Tool* createFilter(QWidget* parent, Canvas* canvas)
{
    return new Filter(parent, canvas);
}

}

