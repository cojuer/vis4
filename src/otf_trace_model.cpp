#include <QDebug>
#include <OTF_RBuffer.h>

#include "otf_trace_model.h"

namespace vis4 {

OTF_trace_model::OTF_trace_model(const QString& filename) :
    min_time_(getTime(0)),
    currentState(0)
{
    states_.clear();

    ha = (HandlerArgument){ 0, parent_component_, &components_, &states_, &allStates, &allEvents, 0, 0};

    manager = OTF_FileManager_open(100);//? what if > 100?
    assert( manager );

    initialize();
    max_time_ = getTime(1000);

    handlers = OTF_HandlerArray_open();
    assert(handlers);

    /* processes */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleDefProcessGroup, OTF_DEFPROCESSGROUP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFPROCESSGROUP_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleDefProcess, OTF_DEFPROCESS_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFPROCESS_RECORD );

     /* functions */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleDefFunctionGroup, OTF_DEFFUNCTIONGROUP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFFUNCTIONGROUP_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleDefFunction, OTF_DEFFUNCTION_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFFUNCTION_RECORD );

     /* markers */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleDefMarker, OTF_DEFMARKER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFMARKER_RECORD );

    /* for reading enter/leave functions */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleEnter, OTF_ENTER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_ENTER_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleLeave, OTF_LEAVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_LEAVE_RECORD );

    /* for reading markers */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleMarker, OTF_MARKER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_MARKER_RECORD );

    /* messages */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleSendMsg, OTF_SEND_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_SEND_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handleRecvMsg, OTF_RECEIVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_RECEIVE_RECORD );

    initialize_component_list();

    reader = OTF_Reader_open( filename.toLatin1().data(), manager );
    assert(reader);

    // чтение определений и обработка их обработчикоми handlers
    ret = OTF_Reader_readDefinitions( reader, handlers );

    // чтение событий Events
    OTF_Reader_setRecordLimit(reader, 1000);
    OTF_Reader_readEvents( reader, handlers);
    OTF_Reader_readMarkers( reader, handlers);
    OTF_Reader_readStatistics( reader, handlers);
    OTF_Reader_readSnapshots( reader, handlers);

    qDebug() << "read definition records: " << (unsigned long long int)ret;
    qDebug() << "countSend=" << ha.countSend << " countRecv=" << ha.countRecv;

    //TEST
    testAddMessages();
    updateTime();
}

OTF_trace_model::~OTF_trace_model() {}

void OTF_trace_model:: initialize_component_list()
{
    components_.clear();
    int rootLink = components_.addItem("Stand", Selection::ROOT);
    parent_component_ = rootLink;
}

int OTF_trace_model:: parent_component() const
{
    return parent_component_;
#if 0
    if (currentElement.tagName() == "component")
        return currentElement.attribute("name");
    else
        return "";
#endif
}

const QList<int> & OTF_trace_model:: visible_components() const
{
    return visible_components_;
}

int OTF_trace_model:: lifeline(int component) const
{
    return lifeline_map_.contains(component) ? lifeline_map_[component] : -1;
}

Trace_model::ComponentType OTF_trace_model::component_type(int component) const
{
    return has_children(component) ? ComponentType::RCHM : ComponentType::CHM;
}

QString OTF_trace_model::component_name(int component, bool full) const
{
    Q_ASSERT(component >= 0 && component < components_.totalItemsCount());

    if (!full) return components_.item(component);

    QString fullname = components_.item(component).trimmed();
    for(;;)
    {
        QString splitter = "::";
        if (component_type(component) == ComponentType::INTERFACE) splitter = ":";

        component = components_.itemParent(component);
        if (component == Selection::ROOT) break;
        if (component == parent_component()) break;

        fullname = components_.item(component).trimmed() + splitter + fullname;
    }

    return fullname;
}

bool OTF_trace_model::has_children(int component) const
{
    return components_.hasChildren(component);
}

Time OTF_trace_model::min_time() const
{
    return min_time_;
}

Time OTF_trace_model::max_time() const
{
    return max_time_;
}

Time OTF_trace_model::min_resolution() const
{
    return getTime(3);
}

void OTF_trace_model::rewind()
{
    currentEvent = 0;
    currentSubcomponent = -1;
    currentGroup = 0;
    //testAddMessages();
}

void OTF_trace_model::testAddMessages()
{
    GroupModel* gm = new GroupModel();
    QVector<GroupModel::Point> gmvec;
    GroupModel::Point gp1;
    gp1.component = 1;
    gp1.time = getTime(0);
    GroupModel::Point gp2;
    gp2.component = 2;
    gp2.time = getTime(1000);
    GroupModel::Point gp3;
    gp3.component = 3;
    gp3.time = getTime(1000);
    gmvec.push_back(gp1);
    gmvec.push_back(gp2);
    gmvec.push_back(gp3);
    gm->points = gmvec;
    gm->type = GroupModel::arrow;
    allGroups.push_back(gm);
}

void OTF_trace_model::updateTime()
{
    max_time_ = allEvents[allEvents.size() - 1]->time;
    //? TEST
    for (int i = 0; i < allStates.size(); ++i)
    {
        if (boost::any_cast<int>(allStates[i]->end.data()) == -1)
        {
            allStates[i]->end = max_time_;
        }
    }
}

State_model* OTF_trace_model::next_state()
{
    if (currentState < allStates.size())
    {
        return allStates[currentState++];
    }
    else
    {
        currentState = 0;
        return nullptr;
    }
}

GroupModel* OTF_trace_model::next_group()
{
    if (currentGroup < allGroups.size())
    {
        return allGroups[currentGroup++];
    }
    else
    {
        currentGroup = 0;
        return nullptr;
    }
}

std::auto_ptr<EventModel> OTF_trace_model:: next_event_unsorted()
{
    return std::auto_ptr<EventModel>();
}

EventModel* OTF_trace_model::next_event()
{
    if (currentEvent >= allEvents.count())
    {
        return nullptr;
    }
    else
    {
        return allEvents[currentEvent++];
    }
}

TraceModelPtr OTF_trace_model::root()
{
    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->parent_component_ = Selection::ROOT;

    n->min_time_ = getTime(0);

    n->events_.enableAll(Selection::ROOT, true);
    n->adjust_components();

    return n;
}

TraceModelPtr OTF_trace_model::set_parent_component(int component)
{

    if (parent_component_ == component)
        return shared_from_this();

    if (component == Selection::ROOT)
    {
        OTFTraceModelPtr n(new OTF_trace_model(*this));
        n->parent_component_ = Selection::ROOT;
        n->adjust_components();
        return n;
    }

    if (component == components_.itemParent(parent_component_))
    {
        OTFTraceModelPtr n(new OTF_trace_model(*this));
        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    if (components_.itemParent(component) == parent_component_)
    {
        OTFTraceModelPtr n(new OTF_trace_model(*this));

        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    OTFTraceModelPtr n(new OTF_trace_model(*this));

    QList<int> parents; parents << component;
    while (parents.front() != component)
        parents.prepend(components_.itemParent(component));
    parents.pop_front();

    n->parent_component_ = component;
    n->adjust_components();
    return n;
}

TraceModelPtr OTF_trace_model::set_range(const Time& min, const Time& max)
{
    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->min_time_ = min;
    n->max_time_ = max;
    return n;
}

const Selection & OTF_trace_model::components() const
{
    return components_;
}

TraceModelPtr OTF_trace_model::filter_components(const Selection & filter)
{
    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->components_ = filter;
    n->adjust_components();
    return n;
}

const Selection & OTF_trace_model::events() const
{
    return events_;
}

const Selection & OTF_trace_model::states() const
{
    return states_;
}

const Selection& OTF_trace_model::available_states() const
{
    return available_states_;
}

TraceModelPtr OTF_trace_model::filter_states(const Selection& filter)
{
    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->states_ = filter;
    //n->adjust_components();
    return n;
}

TraceModelPtr OTF_trace_model::install_checker(Checker* checker)
{
    return shared_from_this();
}

TraceModelPtr OTF_trace_model::filter_events(const Selection& filter)
{
    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->events_ = filter;
    return n;
}

QString OTF_trace_model::save() const
{
    QString componentPos;
    componentPos = "/" + componentPos;

    return  componentPos + ":" +
        min_time_.toString() + ":" + max_time_.toString();
}

bool OTF_trace_model::groupsEnabled() const
{
    return groups_enabled_;
}

TraceModelPtr OTF_trace_model::setGroupsEnabled(bool enabled)
{
    if (groups_enabled_ == enabled)
    {
        return shared_from_this();
    }

    OTFTraceModelPtr n(new OTF_trace_model(*this));
    n->groups_enabled_ = enabled;
    return n;
}

void OTF_trace_model::restore(const QString& s)
{
    parent_component_ = Selection::ROOT;
    adjust_components();
}

Time OTF_trace_model::getTime(int t) const//? it must not be here
{
    return scalarTime<int>(t);
}

void OTF_trace_model::initialize()
{
    // Initialize events
    events_.clear();
    events_.addItem("Update");
    events_.addItem("Delay");
    events_.addItem("Send");
    events_.addItem("Receive");
    events_.addItem("Stop");
}

void OTF_trace_model::adjust_components()
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

void OTF_trace_model::findNextItem(const QString& elementName)
{

}
}
