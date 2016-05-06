#include "newtracemodel.h"

#include <QDebug>
#include <OTF_RBuffer.h>

namespace vis4 {

NewTraceModel::NewTraceModel(const QString& filename) :
    minTime(Time(0))
{
    initialize();
    initialize_component_list();
    dataPtr = (new OTFReader())->read(filename);

    for (unsigned int i = 0; i < dataPtr->getComponents().size(); ++i)
    {
        components_.addItem(dataPtr->getComponents().item(i), dataPtr->getComponents().itemParent(i));
    }

    //components_ = dataPtr->getComponents();
    events_ = dataPtr->getEventTypes();
    states_ = dataPtr->getStateTypes();

    //TEST
    testAddMessages();
    updateTime();
}

NewTraceModel::~NewTraceModel() {}

void NewTraceModel:: initialize_component_list()
{
    components_.clear();
    int rootLink = components_.addItem("Stand", Selection::ROOT);
    parent_component_ = rootLink;
}

int NewTraceModel:: getParentComponent() const
{
    return parent_component_;
}

const QList<int> & NewTraceModel::getVisibleComponents() const
{
    return visible_components_;
}

int NewTraceModel::lifeline(int component) const
{
    return lifeline_map_.contains(component) ? lifeline_map_[component] : -1;
}

TraceModel::ComponentType NewTraceModel::getComponentType(int component) const// deprecated
{
    return hasChildren(component) ? ComponentType::RCHM : ComponentType::CHM;
}

QString NewTraceModel::getComponentName(int component, bool full) const
{
    Q_ASSERT(component >= 0 && component < components_.size());

    if (!full)
    {
        return components_.item(component);
    }

    QString fullname = components_.item(component).trimmed();
    for(;;)
    {
        QString splitter = "::";
        if (getComponentType(component) == ComponentType::INTERFACE) splitter = ":";

        component = components_.itemParent(component);
        if (component == Selection::ROOT) break;
        if (component == getParentComponent()) break;

        fullname = components_.item(component).trimmed() + splitter + fullname;
    }

    return fullname;
}

bool NewTraceModel::hasChildren(int component) const
{
    return components_.hasChildren(component);
}

Time NewTraceModel::getMinTime() const
{
    return minTime;
}

Time NewTraceModel::getMaxTime() const
{
    return maxTime;
}

Time NewTraceModel::getMinResolution() const
{
    return Time(3);
}

void NewTraceModel::rewind()
{
    currentSubcomponent = -1;
}

void NewTraceModel::testAddMessages()
{

}

void NewTraceModel::updateTime()
{
    maxTime = dataPtr->getMaxTime();
    //? TEST
    /*
    for (int i = 0; i < allStates.size(); ++i)
    {
        if (allStates[i]->end.getData().tv_sec == 0 &&
             allStates[i]->end.getData().tv_nsec == 0)
        {
            allStates[i]->end = maxTime;
        }
    }
    */
}

StateModel* NewTraceModel::getNextState()
{
    dataPtr->getNextState();
}

GroupModel* NewTraceModel::getNextGroup()
{
    dataPtr->getNextGroup();
}

EventModel* NewTraceModel::getNextEvent()
{
    dataPtr->getNextEvent();
}

TraceModelPtr NewTraceModel::root()
{
    NewTraceModelPtr n(new NewTraceModel(*this));
    n->parent_component_ = Selection::ROOT;

    n->minTime = dataPtr->getMinTime();
    n->maxTime = dataPtr->getMaxTime();

    n->events_.enableAll(Selection::ROOT, true);
    n->adjust_components();

    return n;
}

TraceModelPtr NewTraceModel::setParentComponent(int component)
{
    if (parent_component_ == component)
    {
        return shared_from_this();
    }
    if (component == Selection::ROOT)
    {
        NewTraceModelPtr n(new NewTraceModel(*this));
        n->parent_component_ = Selection::ROOT;
        n->adjust_components();
        return n;
    }
    if (component == components_.itemParent(parent_component_))
    {
        NewTraceModelPtr n(new NewTraceModel(*this));
        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    if (components_.itemParent(component) == parent_component_)
    {
        NewTraceModelPtr n(new NewTraceModel(*this));

        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    NewTraceModelPtr n(new NewTraceModel(*this));

    QList<int> parents; parents << component;
    while (parents.front() != component)
        parents.prepend(components_.itemParent(component));
    parents.pop_front();

    n->parent_component_ = component;
    n->adjust_components();
    return n;
}

TraceModelPtr NewTraceModel::setRange(const Time& min, const Time& max)
{
    NewTraceModelPtr n(new NewTraceModel(*this));
    n->minTime = min;
    n->maxTime = max;
    return n;
}

const Selection & NewTraceModel::getComponents() const
{
    return components_;
}

TraceModelPtr NewTraceModel::filterComponents(const Selection & filter)
{
    NewTraceModelPtr n(new NewTraceModel(*this));
    n->components_ = filter;
    n->adjust_components();
    return n;
}

const Selection& NewTraceModel::getEvents() const
{
    return events_;
}

const Selection& NewTraceModel::getStates() const
{
    return states_;
}

const Selection& NewTraceModel::getAvailableStates() const
{
    return available_states_;
}

TraceModelPtr NewTraceModel::filterStates(const Selection& filter)
{
    NewTraceModelPtr n(new NewTraceModel(*this));
    n->states_ = filter;
    return n;
}

TraceModelPtr NewTraceModel::filterEvents(const Selection& filter)
{
    NewTraceModelPtr n(new NewTraceModel(*this));
    n->events_ = filter;
    return n;
}

QString NewTraceModel::save() const
{
    QString componentPos;
    componentPos = "/" + componentPos;

    return  componentPos + ":" +
        minTime.toString() + ":" + maxTime.toString();
}

bool NewTraceModel::groupsEnabled() const
{
    return groups_enabled_;
}

TraceModelPtr NewTraceModel::setGroupsEnabled(bool enabled)
{
    if (groups_enabled_ == enabled)
    {
        return shared_from_this();
    }

    NewTraceModelPtr n(new NewTraceModel(*this));
    n->groups_enabled_ = enabled;
    return n;
}

void NewTraceModel::restore(const QString& s)
{
    parent_component_ = Selection::ROOT;
    adjust_components();
}

void NewTraceModel::initialize()
{
    // Initialize events
    events_.clear();
    events_.addItem("Update");
    events_.addItem("Delay");
    events_.addItem("Send");
    events_.addItem("Receive");
    events_.addItem("Stop");
}

void NewTraceModel::adjust_components()
{
    visible_components_ = components_.enabledItems(parent_component_);
    components_.setItemProperty(0, "current_parent", parent_component_);

    lifeline_map_.clear();
    for (int ll = 0; ll < visible_components_.size(); ll++)
    {
        QList<int> queue;
        queue << visible_components_[ll];
        while (!queue.isEmpty())
        {
            int comp = queue.takeFirst();
            lifeline_map_[comp] = ll;

            foreach(int child, components_.enabledItems(comp))
            {
                queue << child;
            }
        }
    }

}

void NewTraceModel::findNextItem(const QString& elementName)
{

}
}

