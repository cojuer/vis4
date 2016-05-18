#include "tracemodelimpl.h"

#include <QDebug>
#include <OTF_RBuffer.h>
#include <QElapsedTimer>

namespace vis4 {

TraceModelImpl::TraceModelImpl(const QString& filename, TraceReader* readerPtr)
{
    initialize();
    initialize_component_list();

    QElapsedTimer timer;
    timer.start();
    dataPtr = readerPtr->read(filename);
    qDebug() << timer.elapsed() << " time";

    minTime = dataPtr->getMinTime();
    maxTime = dataPtr->getMaxTime();

    components_ = dataPtr->getComponents();
    events_ = dataPtr->getEventTypes();
    states_ = dataPtr->getStateTypes();
}

TraceModelImpl::~TraceModelImpl() {}

void TraceModelImpl::initialize_component_list()
{
    components_.clear();
    int rootLink = components_.addItem("Stand", Selection::ROOT);
    parent_component_ = rootLink;
}

int TraceModelImpl::getParentComponent() const
{
    return parent_component_;
}

const QList<int> & TraceModelImpl::getVisibleComponents() const
{
    return visible_components_;
}

int TraceModelImpl::lifeline(int component) const
{
    return lifeline_map_.contains(component) ? lifeline_map_[component] : -1;
}

TraceModel::ComponentType TraceModelImpl::getComponentType(int component) const// deprecated
{
    return hasChildren(component) ? ComponentType::RCHM : ComponentType::CHM;
}

QString TraceModelImpl::getComponentName(int component, bool full) const
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

bool TraceModelImpl::hasChildren(int component) const
{
    return components_.hasChildren(component);
}

Time TraceModelImpl::getMinTime() const
{
    return minTime;
}

Time TraceModelImpl::getMaxTime() const
{
    return maxTime;
}

Time TraceModelImpl::getMinResolution() const
{
    return Time(3);
}

void TraceModelImpl::rewind()
{
    currentSubcomponent = -1;
}

StateModel* TraceModelImpl::getNextState()
{
    dataPtr->getNextState();
}

GroupModel* TraceModelImpl::getNextGroup()
{
    dataPtr->getNextGroup();
}

EventModel* TraceModelImpl::getNextEvent()
{
    dataPtr->getNextEvent();
}

TraceModelPtr TraceModelImpl::root()
{
    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->parent_component_ = Selection::ROOT;

    n->minTime = dataPtr->getMinTime();
    n->maxTime = dataPtr->getMaxTime();

    n->events_.enableAll(Selection::ROOT, true);
    n->adjust_components();

    return n;
}

TraceModelPtr TraceModelImpl::setParentComponent(int component)
{
    if (parent_component_ == component)
    {
        return shared_from_this();
    }
    if (component == Selection::ROOT)
    {
        TraceModelImplPtr n(new TraceModelImpl(*this));
        n->parent_component_ = Selection::ROOT;
        n->adjust_components();
        return n;
    }
    if (component == components_.itemParent(parent_component_))
    {
        TraceModelImplPtr n(new TraceModelImpl(*this));
        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    if (components_.itemParent(component) == parent_component_)
    {
        TraceModelImplPtr n(new TraceModelImpl(*this));

        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    TraceModelImplPtr n(new TraceModelImpl(*this));

    QList<int> parents; parents << component;
    while (parents.front() != component)
        parents.prepend(components_.itemParent(component));
    parents.pop_front();

    n->parent_component_ = component;
    n->adjust_components();
    return n;
}

TraceModelPtr TraceModelImpl::setRange(const Time& min, const Time& max)
{
    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->minTime = min;
    n->maxTime = max;
    return n;
}

const Selection & TraceModelImpl::getComponents() const
{
    return components_;
}

TraceModelPtr TraceModelImpl::filterComponents(const Selection & filter)
{
    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->components_ = filter;
    n->adjust_components();
    return n;
}

const Selection& TraceModelImpl::getEvents() const
{
    return events_;
}

const Selection& TraceModelImpl::getStates() const
{
    return states_;
}

const Selection& TraceModelImpl::getAvailableStates() const
{
    return available_states_;
}

TraceModelPtr TraceModelImpl::filterStates(const Selection& filter)
{
    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->states_ = filter;
    return n;
}

TraceModelPtr TraceModelImpl::filterEvents(const Selection& filter)
{
    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->events_ = filter;
    return n;
}

QString TraceModelImpl::save() const
{
    QString componentPos;
    componentPos = "/" + componentPos;

    return  componentPos + ":" +
        minTime.toString() + ":" + maxTime.toString();
}

bool TraceModelImpl::groupsEnabled() const
{
    return groups_enabled_;
}

TraceModelPtr TraceModelImpl::setGroupsEnabled(bool enabled)
{
    if (groups_enabled_ == enabled)
    {
        return shared_from_this();
    }

    TraceModelImplPtr n(new TraceModelImpl(*this));
    n->groups_enabled_ = enabled;
    return n;
}

void TraceModelImpl::restore(const QString& s)
{
    parent_component_ = Selection::ROOT;
    adjust_components();
}

void TraceModelImpl::initialize()
{
    // Initialize events
    events_.clear();
    events_.addItem("Update");
    events_.addItem("Delay");
    events_.addItem("Send");
    events_.addItem("Receive");
    events_.addItem("Stop");
}

void TraceModelImpl::adjust_components()
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

}

