#include <QtWidgets/QAction>
#include <QPainter>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QApplication>

#include <set>

#include "tool.h"
#include "find_tabs.h"
#include "canvas_item.h"
#include "canvas.h"
#include "trace_model.h"
#include "event_model.h"
#include "state_model.h"

namespace vis4 {

using std::set;

class Found_item_highlight : public CanvasItem
{
public:
    void setRect(QRect r)
    {
        rect_ = r;
        if (rect_.isValid())
        {
            r.adjust(-2, -2, 2, 2);
        }
        new_geometry(r);
    }

    QRect rect() const
    {
        return rect_;
    }

private:

    QRect rect_;

private:
    QRect xdraw(QPainter& painter)
    {
        if (!rect_.isValid())
        {
            return QRect();
        }

        painter.save();

        QColor halfRed(Qt::red);
        halfRed.setAlpha(75);
        painter.setBrush(halfRed);
        painter.setPen(QPen(Qt::red, 2));

        painter.drawRect(rect_);
        QRect r = rect_;
        r.adjust(-2, -2, 2, 2);

        painter.restore();

        return r;
    }
};

/* And item model where items can be conveniently disabled. */
class ItemModelWithDisabledItems : public QStandardItemModel
{
public:
    ItemModelWithDisabledItems(QObject* parent) :
        QStandardItemModel(parent)
    {
        insertColumns(0, 1);
    }

    int addItem(const QString& item, bool enabled,
        const QVariant& userData = QVariant())
    {
        int row = rowCount();
        insertRow(row);
        QModelIndex idx = index(row, 0, QModelIndex());
        setData(idx, item);
        setData(idx, userData, Qt::UserRole);

        enabled_.push_back(enabled);
        return row;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        return (enabled_[index.row()]) ? QStandardItemModel::flags(index) : Qt::NoItemFlags;
    }

private:
    std::vector<bool> enabled_;
};

class Find : public Tool
{
    Q_OBJECT
public:
    Find(QWidget* parent, Canvas* c) :
        Tool(parent, c),
        highlighted_component(-1),
      componentListModel_(0), state_restored(false)
    {
        setObjectName("find");
        setWindowTitle(tr("Find"));

        setWhatsThis(tr("<b>Find</b>"
                     "<p>Allows you to find events and states of interest."
                     "<p>There are three control groups on the tool."
                     "<p>The 'Search on' group allows you to request search on "
                     "all visible components, or just on one."
                     "<p>The 'Search for' group allows you to select search for "
                     "events or for states."
                     "<p>The last group has a list of event or state types that "
                     "must be searched for. For events, if an event is "
                     "globally filtered, it will be shown in gray and you "
                     "cannot search for it."));

        QAction* find_again = new QAction(parent);
        find_again->setShortcut(Qt::Key_F3);
        find_again->setShortcutContext(Qt::WindowShortcut);
        c->addAction(find_again);

        connect(find_again, SIGNAL(triggered(bool)), this,
                            SLOT(findNext()));

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* startTimeLayout = new QHBoxLayout();
        QLabel* l = new QLabel(tr("Search from:"), this);
        startTimeLayout->addWidget(l);

        startTimeLabel = new QLabel("0", this);
        startTimeLayout->addWidget(startTimeLabel);
        startTimeLayout->addStretch();

        QPushButton* resetButton = new QPushButton(tr("Reset"), this);
        startTimeLayout->addWidget(resetButton);
        connect(resetButton, SIGNAL(clicked(bool)), this,
                             SLOT(reset()));

        mainLayout->addLayout(startTimeLayout);

        QGroupBox* searchOnGroup = new QGroupBox(tr("Search on"), this);
        mainLayout->addWidget(searchOnGroup);

        QVBoxLayout* searchOnLayout = new QVBoxLayout(searchOnGroup);
        searchOnLayout->setSpacing(0);
        allComponents = new QRadioButton(tr("All components"));
        searchOnLayout->addWidget(allComponents);
        allComponents->setChecked(true);

        connect(allComponents, SIGNAL(toggled(bool)), this,
                               SLOT( updateSearchedComponent() ));

        singleComponent = new QRadioButton(tr("Selected component"));
        searchOnLayout->addWidget(singleComponent);
        connect(singleComponent, SIGNAL(toggled(bool)), this,
                                 SLOT( updateSearchedComponent() ));

        componentList = new QComboBox(searchOnGroup);
        searchOnLayout->addWidget(componentList);
        connect(componentList, SIGNAL(currentIndexChanged(int)), this,
                               SLOT( updateSearchedComponent() ));
        connect(componentList, SIGNAL(currentIndexChanged(int)),  singleComponent,
                               SLOT(click()));

        // Create find tabs
        findTabWidget = new QTabWidget(this);
        mainLayout->addWidget(findTabWidget);

        //queryTab = new FindQueryTab(this);
        findTabWidget->addTab(queryTab, queryTab->name());

        connect(queryTab, SIGNAL(showEvent(EventModel*)), this,
                          SLOT(eventFound(EventModel*)));
        connect(queryTab, SIGNAL(stateChanged()), this,
                          SLOT(tabStateChanged()));

        //eventsTab = new FindEventsTab(this);
        findTabWidget->addTab(eventsTab, eventsTab->name());

        connect(eventsTab, SIGNAL(showEvent(EventModel*)),this,
                           SLOT(eventFound(EventModel*)));
        connect(eventsTab, SIGNAL(stateChanged()), this,
                           SLOT(tabStateChanged()));

        //statesTab = new FindStatesTab(this);
        findTabWidget->addTab(statesTab, statesTab->name());
        connect(statesTab, SIGNAL(showState(StateModel*)), this,
                           SLOT(stateFound(StateModel*)));
        connect(statesTab, SIGNAL(stateChanged()), this,
                           SLOT(tabStateChanged()));

        connect(findTabWidget, SIGNAL(currentChanged(int)), this,
                               SLOT(switchTab(int)));

        //"Find" button

        QHBoxLayout* buttons = new QHBoxLayout(0);
        mainLayout->addLayout(buttons);

        buttons->addStretch();

        findButton = new QPushButton(tr("Find"), this);
        buttons->addWidget(findButton);
        connect(findButton, SIGNAL(clicked(bool)), this,
                            SLOT( findNext() ));

        highlight = new Found_item_highlight;
        getCanvas()->addItem(highlight);

        connect(getCanvas(), SIGNAL(modelChanged(TraceModel::TraceModelPtr &)), this,
                             SLOT(modelChanged(TraceModel::TraceModelPtr &)));
        connect(this, SIGNAL(messageBox(const QString &)), this,
                      SLOT(showMessageBox(const QString &)), Qt::QueuedConnection);

#ifndef _CONCISE_
        switchTab(1);
#else
        switchTab(0);
#endif
        modelChanged(model());
        restoreState();
    }

    QAction* createAction()
    {
        findAction = new QAction(QIcon(":/find.png"), tr("&Find"), this);
        findAction->setShortcut(QKeySequence(Qt::Key_F));
        return findAction;
    }

    void activate() {}
    void deactivate() {}

signals:
    void extraHelp(const QString& s);
    void messageBox(const QString& message);

private: /* methods */

    void maybeReposition(const Time& event_time)
    {
        // If the next found event is outside shown
        // part of the trace, we need to shift if.
        if (event_time < model()->getMinTime() || event_time > model()->getMaxTime())
        {
            Time shown_time_range = model()->getMaxTime() - model()->getMinTime();

            // Make new trace overlap with the previous
            // one by 1/10 of time.
            Time one_tenth = shown_time_range/10;

            Time desired_new_min_time = event_time - one_tenth;
            if (desired_new_min_time < model()->root()->getMinTime())
            {
                desired_new_min_time = model()->root()->getMinTime();
            }

            Time new_max_time = desired_new_min_time + shown_time_range;
            if (new_max_time > model()->root()->getMaxTime())
            {
                new_max_time = model()->root()->getMaxTime();
            }
            Time new_min_time = new_max_time - shown_time_range;

            getCanvas()->setModel(model()->setRange(new_min_time, new_max_time));
        }
    }

    bool mouseEvent(QEvent* event,
                          Canvas::clickTarget target,
                          int component,
                          StateModel* state,
                          const Time& time,
                          bool events_near)
    {
        if (event->type() != QEvent::MouseButtonRelease) return false;
        QMouseEvent * mEvent = static_cast<QMouseEvent*>(event);

        if (mEvent->button() == Qt::RightButton
            && mEvent->modifiers() == Qt::NoModifier)
        {
            /* Note: we respond both to click on component
               name and on lifeline.  */
            if (component != -1)
            {
                startTime = time;
                singleComponent->setChecked(true);
                // Since componentList may contains disabled(filtered) items,
                // component number is not the same as row number,
                // so we must convert component number to row.
                componentList->setCurrentIndex(base_model->getComponents().itemIndex(component));
                resetSearch(false);
                emit activateMe();
                return true;
            }
        }
        return false;
    }

    void setHighlight(int component, const Time& min, const Time& max)
    {
        highlighted_component = component;
        highlighted_min = min;
        highlighted_max = max;
        highlightChanged();

        getCanvas()->ensureVisible(0, highlight->rect().y());
    }

    void resetHightlight()
    {
        highlighted_component = -1;
        highlightChanged();
    }

    /* Populates the component list combo.  All possible
       components of the current component tree position
       are shown, with globally filtered ones disabled.
       If some model was previously used to populate the
       combo, try to select the same component that was selected
       previously.  */
    void updateComponentList(TraceModelPtr & model)
    {
        componentList->blockSignals(true);
        int currentlySelected = componentList->currentIndex();

        componentList->clear();

        std::set<QString> names_here_s;
        foreach(int comp, model->getVisibleComponents())
            names_here_s.insert(model->getComponentName(comp));

        std::vector<QString> all_names;
        foreach(int comp, model->getComponents().items(model->getParentComponent()))
            all_names.push_back(model->getComponentName(comp));

        delete componentListModel_;

        componentListModel_ = new ItemModelWithDisabledItems(this);

        componentList->setModel(componentListModel_);

        int first_enabled = -1;
        for (unsigned i = 0; i < all_names.size(); ++i)
        {
            bool enabled = names_here_s.count(all_names[i]) != 0;
            componentListModel_->addItem(all_names[i], enabled);
            if (enabled && first_enabled == -1)
                first_enabled = i;
            if (i == (unsigned)currentlySelected && !enabled)
                currentlySelected = -1;
        }

        bool tree_position_changed = (base_model.get() == 0) ||
            (base_model->getParentComponent() != model->getParentComponent());

        if (currentlySelected != -1 && !tree_position_changed)
            componentList->setCurrentIndex(currentlySelected);
        else if (first_enabled != -1)
            componentList->setCurrentIndex(first_enabled);

        base_model = model;
        componentList->blockSignals(false);
    }

    void applyComponentSelection()
    {
        if (allComponents->isChecked())
        {
            model_ = base_model; return;
        }

        int index = componentList->currentIndex();
        // FIXME: this is possible if all components
        // are filtered out. Not sure what do to.
        Q_ASSERT(index != -1);

        Selection component_filter = base_model->getComponents();
        int parent_component = base_model->getParentComponent();
        component_filter.disableAll(parent_component);
        component_filter.setEnabled(index, parent_component, true);

        model_ = base_model->filterComponents(component_filter);
    }

    void keyReleaseEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            if (findButton->isEnabled())
                findButton->animateClick();
        } else {
            Tool::keyReleaseEvent(event);
        }
    }

    void resetSearch(bool reset_time)
    {
        nothing_yet = true;
        resetHightlight();
        emit extraHelp(QString());

        if (reset_time)
        {
            if (startTime == model()->getMinTime())
                startTime = model()->root()->getMinTime();
            else
                startTime = model()->getMinTime();
        }

        queryTab->reset();
        eventsTab->reset();
        statesTab->reset();

        updateState();
    }

    void updateState()
    {
        applyComponentSelection();

        model_ = model_->setRange(startTime, model_->root()->getMaxTime());
        startTimeLabel->setText(startTime.toString());

        queryTab->setModel(model_);
        eventsTab->setModel(model_);
        statesTab->setModel(model_);

        highlightChanged();
        saveState();
    }

    void highlightChanged()
    {
        if (highlighted_component == -1)
        {
            highlight->setRect(QRect());
        }
        else
        {
            Time min = highlighted_min;
            if (min < model()->getMinTime())
                min = model()->getMinTime();

            Time max = highlighted_max;
            if (max > model()->getMaxTime())
                max = model()->getMaxTime();

            if (min <= max)
            {
                QRect r = getCanvas()->boundingRect(highlighted_component, min, max);
                r.adjust(-1, -1, 1, 1);
                highlight->setRect(r);
            }
            else
                highlight->setRect(QRect());
        }
    }

    void saveState()
    {
        if (!state_restored) return;

        QSettings settings;
        prepareSettings(settings);
        settings.beginGroup("find_tool");

        settings.setValue("search_on_single", singleComponent->isChecked());
        settings.setValue("search_on_component", componentList->currentText());
        settings.setValue("search_for", findTabWidget->currentWidget()->objectName());

        queryTab->saveState(settings);
        eventsTab->saveState(settings);
        statesTab->saveState(settings);
    }

private slots:

    void restoreState()
    {
        QSettings settings;
        prepareSettings(settings);
        settings.beginGroup("find_tool");

        // We must read and store all settings before applying some.
        bool single_comp = settings.value("search_on_single", false).toBool();
        QString component = settings.value("search_on_component").toString();
        QString search_for = settings.value("search_for").toString();

        if (!single_comp) allComponents->setChecked(true);
                     else singleComponent->setChecked(true);

        if (!component.isEmpty()) {
            int index = componentList->findText(component);
            if (index != -1) componentList->setCurrentIndex(index);
        }

        if (!search_for.isEmpty()) for (int i = 0; i < findTabWidget->count(); i++)
            if (findTabWidget->widget(i)->objectName() == search_for)
                { findTabWidget->setCurrentIndex(i); break; }

        queryTab->restoreState(settings);
        eventsTab->restoreState(settings);
        statesTab->restoreState(settings);

        state_restored = true;
    }

    void findNext()
    {
        if (!findButton->isEnabled()) return;

        QApplication::setOverrideCursor(Qt::WaitCursor);

        bool searched = active_tab->findNext();
        if (!searched) {
            if (nothing_yet)
            {
                emit messageBox(tr("Nothing was found."));
                resetSearch(false);
            }
            else
            {
                emit messageBox(tr("Search is complete."));
                emit extraHelp(QString());
            }
        } else {
            nothing_yet = false;
            emit extraHelp(tr("<b>Press F3 to continue search.</b>"));
        }

        QApplication::restoreOverrideCursor();
    }

    void reset()
    {
        resetSearch(true);
    }

    void modelChanged(TraceModelPtr & model)
    {
        bool need_reset = false;

        if (!base_model.get())
        {
            startTime = model->getMinTime();
        }
        else
        {
            int delta = vis4::delta(*base_model, *model);
            if (delta & Trace_model_delta::component_position ||
                delta & Trace_model_delta::components)
            {
                need_reset = true;
                startTime = model->getMinTime();
            }
        }

        updateComponentList(model);
        updateSearchedComponent();
        need_reset ? resetSearch(false) : updateState();
    }

    void updateSearchedComponent()
    {
        int newSearchedComponent =
            allComponents->isChecked() ? -1 : componentList->currentIndex();

        if (newSearchedComponent != -1)
        {
            /* Since componentList has a list of all components,
                and the model has some components hidden, we need
                to find the index inside the model. */
            Selection components = base_model->getComponents();
            for (int i = newSearchedComponent-1; i >= 0; i--)
                if (!components.isEnabled(i)) newSearchedComponent--;
        }

        if (newSearchedComponent != searchedComponent)
        {
            searchedComponent = newSearchedComponent;
            resetSearch(false);
        }
        else
        {
            updateState();
        }
    }

    void showMessageBox(const QString & message)
    {
        QMessageBox::information(getCanvas(), tr("Search"), message);
    }

    void tabStateChanged()
    {
        if (sender() == active_tab)
            findButton->setEnabled(active_tab->isSearchAllowed());

        saveState();
    }

    void switchTab(int tab)
    {
        switch (tab) {
            case 0:
                active_tab = queryTab;
            break;

            case 1:
                active_tab = eventsTab;
            break;

            case 2:
                active_tab = statesTab;
            break;
        }

        findTabWidget->setCurrentIndex(tab);
        findButton->setEnabled(active_tab->isSearchAllowed());
        saveState();
    }

    void eventFound(EventModel * e)
    {
        startTime = e->time;
        maybeReposition(e->time);

        setHighlight(e->component, e->time, e->time);
        emit showEvent(e);

    }

    void stateFound(StateModel * s)
    {
        startTime = s->start;
        maybeReposition(s->start);

        setHighlight(s->component, s->start, s->end);
        emit showState(s);
    }

private: /* memebers */

    QAction* findAction;

    TraceModelPtr base_model;
    TraceModelPtr model_;

    bool nothing_yet;

    Time startTime;
    int searchedComponent;

    Found_item_highlight* highlight;

    QLabel* startTimeLabel;
    QRadioButton* allComponents;
    QRadioButton* singleComponent;
    QComboBox* componentList;

    QTabWidget * findTabWidget;

    FindQueryTab * queryTab;
    FindEventsTab * eventsTab;
    FindStatesTab * statesTab;

    FindTab * active_tab;

    QPushButton* findButton;

    int highlighted_component;
    Time highlighted_min;
    Time highlighted_max;

    ItemModelWithDisabledItems* componentListModel_;

    bool state_restored;
};


Tool* createFind(QWidget* parent, Canvas* canvas)
{
    return new Find(parent, canvas);
}

}

