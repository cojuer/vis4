#include "otf2tracemodel.h"

#include <QDebug>
#include <OTF_RBuffer.h>

namespace vis4 {

OTF2TraceModel::OTF2TraceModel(const QString& filename) :
    minTime(Time(0))
{
    initialize();
    initialize_component_list();
    dataPtr = (new OTF2Reader())->read(filename);

    components_ = dataPtr->getComponents();
    events_ = dataPtr->getEventTypes();
    states_ = dataPtr->getStateTypes();

    //TEST
    testAddMessages();
    updateTime();
}

OTF2TraceModel::~OTF2TraceModel() {}

void OTF2TraceModel:: initialize_component_list()
{
    components_.clear();
    int rootLink = components_.addItem("Stand", Selection::ROOT);
    parent_component_ = rootLink;
}

int OTF2TraceModel::getParentComponent() const
{
    return parent_component_;
}

const QList<int>& OTF2TraceModel::getVisibleComponents() const
{
    return visible_components_;
}

int OTF2TraceModel::lifeline(int component) const
{
    return lifeline_map_.contains(component) ? lifeline_map_[component] : -1;
}

TraceModel::ComponentType OTF2TraceModel::getComponentType(int component) const// deprecated
{
    return hasChildren(component) ? ComponentType::RCHM : ComponentType::CHM;
}

QString OTF2TraceModel::getComponentName(int component, bool full) const
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

bool OTF2TraceModel::hasChildren(int component) const
{
    return components_.hasChildren(component);
}

Time OTF2TraceModel::getMinTime() const
{
    return minTime;
}

Time OTF2TraceModel::getMaxTime() const
{
    return maxTime;
}

Time OTF2TraceModel::getMinResolution() const
{
    return Time(3);
}

void OTF2TraceModel::rewind()
{
    currentSubcomponent = -1;
}

void OTF2TraceModel::testAddMessages()
{

}

void OTF2TraceModel::updateTime()
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

StateModel* OTF2TraceModel::getNextState()
{
    dataPtr->getNextState();
}

GroupModel* OTF2TraceModel::getNextGroup()
{
    dataPtr->getNextGroup();
}

EventModel* OTF2TraceModel::getNextEvent()
{
    dataPtr->getNextEvent();
}

TraceModelPtr OTF2TraceModel::root()
{
    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->parent_component_ = Selection::ROOT;

    n->minTime = dataPtr->getMinTime();
    n->maxTime = dataPtr->getMaxTime();

    n->events_.enableAll(Selection::ROOT, true);
    n->adjust_components();

    return n;
}

TraceModelPtr OTF2TraceModel::setParentComponent(int component)
{
    if (parent_component_ == component)
    {
        return shared_from_this();
    }
    if (component == Selection::ROOT)
    {
        OTF2TraceModelPtr n(new OTF2TraceModel(*this));
        n->parent_component_ = Selection::ROOT;
        n->adjust_components();
        return n;
    }
    if (component == components_.itemParent(parent_component_))
    {
        OTF2TraceModelPtr n(new OTF2TraceModel(*this));
        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    if (components_.itemParent(component) == parent_component_)
    {
        OTF2TraceModelPtr n(new OTF2TraceModel(*this));

        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    OTF2TraceModelPtr n(new OTF2TraceModel(*this));

    QList<int> parents; parents << component;
    while (parents.front() != component)
        parents.prepend(components_.itemParent(component));
    parents.pop_front();

    n->parent_component_ = component;
    n->adjust_components();
    return n;
}

TraceModelPtr OTF2TraceModel::setRange(const Time& min, const Time& max)
{
    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->minTime = min;
    n->maxTime = max;
    return n;
}

const Selection& OTF2TraceModel::getComponents() const
{
    return components_;
}

TraceModelPtr OTF2TraceModel::filterComponents(const Selection & filter)
{
    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->components_ = filter;
    n->adjust_components();
    return n;
}

const Selection& OTF2TraceModel::getEvents() const
{
    return events_;
}

const Selection& OTF2TraceModel::getStates() const
{
    return states_;
}

const Selection& OTF2TraceModel::getAvailableStates() const
{
    return available_states_;
}

TraceModelPtr OTF2TraceModel::filterStates(const Selection& filter)
{
    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->states_ = filter;
    return n;
}

TraceModelPtr OTF2TraceModel::filterEvents(const Selection& filter)
{
    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->events_ = filter;
    return n;
}

QString OTF2TraceModel::save() const
{
    QString componentPos;
    componentPos = "/" + componentPos;

    return  componentPos + ":" +
        minTime.toString() + ":" + maxTime.toString();
}

bool OTF2TraceModel::groupsEnabled() const
{
    return groups_enabled_;
}

TraceModelPtr OTF2TraceModel::setGroupsEnabled(bool enabled)
{
    if (groups_enabled_ == enabled)
    {
        return shared_from_this();
    }

    OTF2TraceModelPtr n(new OTF2TraceModel(*this));
    n->groups_enabled_ = enabled;
    return n;
}

void OTF2TraceModel::restore(const QString& s)
{
    parent_component_ = Selection::ROOT;
    adjust_components();
}

void OTF2TraceModel::initialize()
{
    // Initialize events
    events_.clear();
    events_.addItem("Update");
    events_.addItem("Delay");
    events_.addItem("Send");
    events_.addItem("Receive");
    events_.addItem("Stop");
}

void OTF2TraceModel::adjust_components()
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

void OTF2TraceModel::findNextItem(const QString& elementName)
{

}
}

