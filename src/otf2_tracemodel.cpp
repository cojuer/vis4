#include "otf2_tracemodel.h"
#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_ErrorCodes.h>

namespace vis4
{

struct OTF2Location
{
    OTF2_LocationRef      location;
    OTF2_StringRef        name;
    OTF2_LocationType     locationType;
    uint64_t              numberOfEvents;
    OTF2_LocationGroupRef locationGroup;
};

struct OTF2Region
{
    OTF2_RegionRef self;
    OTF2_StringRef name;
};

struct TestData
{
    QVector<OTF2Location> locations;
    QVector<OTF2Region> regions;
    QMap<int, QString> strings;
};

static OTF2_CallbackCode
Enter_print( OTF2_LocationRef    location,
             OTF2_TimeStamp      time,
             void*               userData,
             OTF2_AttributeList* attributes,
             OTF2_RegionRef      region )
{
    auto arg = static_cast<OTF2_HandlerArgument*>(userData);

    StateModel* sm = new StateModel(location, region, Time(time), Time(0), Qt::yellow);
    arg->states->push_back(sm);

    EventModel* em = new EventModel(Time(time), location, "ENTER", 'E');
    arg->events->push_back(em);

    std::cout << "Entering region " << region << " at location " << location << " at time " << time << std::endl;
    return OTF2_CALLBACK_SUCCESS;
}
static OTF2_CallbackCode
Leave_print( OTF2_LocationRef    location,
             OTF2_TimeStamp      time,
             void*               userData,
             OTF2_AttributeList* attributes,
             OTF2_RegionRef      region )
{
    auto arg = static_cast<OTF2_HandlerArgument*>(userData);

    for (int i = arg->states->size() - 1; i > 0; --i)
    {
        if (location == (*arg->states)[i]->component)
        {
            (*arg->states)[i]->end = Time(time);
        }
    }

    EventModel* em = new EventModel(Time(time), location, "LEAVE", 'L');
    arg->events->push_back(em);

    std::cout << "Leaving region " << region << " at location " << location << " at time " << time << std::endl;
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode
GlobDefLocation_Register( void*                 userData,
                          OTF2_LocationRef      location,
                          OTF2_StringRef        name,
                          OTF2_LocationType     locationType,
                          uint64_t              numberOfEvents,
                          OTF2_LocationGroupRef locationGroup )
{
    std::cout << name << std::endl;
    OTF2Location loc = {location, name, locationType, numberOfEvents, locationGroup};
    static_cast<TestData*>(userData)->locations.push_back(loc);
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode
StringReader(void *userData, OTF2_StringRef self, const char *string)
{
    static_cast<TestData*>(userData)->strings[self] = QString(string);
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode regionReader(void *userData, OTF2_RegionRef self, OTF2_StringRef name, OTF2_StringRef canonicalName, OTF2_StringRef description, OTF2_RegionRole regionRole, OTF2_Paradigm paradigm, OTF2_RegionFlag regionFlags, OTF2_StringRef sourceFile, uint32_t beginLineNumber, uint32_t endLineNumber)
{
    OTF2Region reg = {self, name};
    static_cast<TestData*>(userData)->regions.push_back(reg);
}

OTF2_TraceModel::OTF2_TraceModel(const QString& filename) :
    currentEvent(0),
    currentGroup(0),
    currentState(0)
{
    //? TEST
    states_.clear();

    TestData testData;
    ha = { 0, parent_component_, &components_, &states_, &allStates, &allEvents, 0, 0};
    initialize();
    min_time_ = Time(0);
    max_time_ = Time(1000);

    OTF2_Reader* reader = OTF2_Reader_Open(filename.toUtf8().constData());//should not use QString here
    OTF2_Reader_SetSerialCollectiveCallbacks(reader);

    OTF2_GlobalDefReader* globalDefReader = OTF2_Reader_GetGlobalDefReader(reader);
    OTF2_GlobalDefReaderCallbacks* globalDefCallbacks = OTF2_GlobalDefReaderCallbacks_New();
    OTF2_GlobalDefReaderCallbacks_SetRegionCallback(globalDefCallbacks, &regionReader);
    OTF2_GlobalDefReaderCallbacks_SetLocationCallback(globalDefCallbacks, &GlobDefLocation_Register);
    OTF2_GlobalDefReaderCallbacks_SetStringCallback(globalDefCallbacks, &StringReader);
    OTF2_Reader_RegisterGlobalDefCallbacks(reader,
                                           globalDefReader,
                                           globalDefCallbacks,
                                           &testData);
    OTF2_GlobalDefReaderCallbacks_Delete(globalDefCallbacks );
    uint64_t definitionsRead = 0;
    OTF2_Reader_ReadAllGlobalDefinitions(reader,
                                         globalDefReader,
                                         &definitionsRead);

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        OTF2_Reader_SelectLocation(reader, testData.locations[i].location);
    }
    bool localDefOpened = (OTF2_Reader_OpenDefFiles(reader) == OTF2_SUCCESS);

    OTF2_Reader_OpenEvtFiles(reader);

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        ha.components->addItem(testData.strings[testData.locations[i].name], -1);
    }
    for (unsigned int i = 0; i < testData.regions.size(); ++i)
    {
        ha.state_types->addItem(testData.strings[testData.regions[i].name], -1);
    }

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        if (localDefOpened)
        {
            OTF2_DefReader* def_reader = OTF2_Reader_GetDefReader(reader, testData.locations[i].location);
            if ( def_reader )
            {
                uint64_t def_reads = 0;
                OTF2_Reader_ReadAllLocalDefinitions(reader,
                                                    def_reader,
                                                    &def_reads);
                OTF2_Reader_CloseDefReader(reader, def_reader);
            }
        }
        OTF2_EvtReader* evt_reader = OTF2_Reader_GetEvtReader(reader, testData.locations[i].location);
    }
    if (localDefOpened)
    {
        OTF2_Reader_CloseDefFiles(reader);
    }

    OTF2_GlobalEvtReader* global_evt_reader = OTF2_Reader_GetGlobalEvtReader(reader);
    OTF2_GlobalEvtReaderCallbacks* event_callbacks = OTF2_GlobalEvtReaderCallbacks_New();
    OTF2_GlobalEvtReaderCallbacks_SetEnterCallback( event_callbacks,
                                                    &Enter_print );
    OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback( event_callbacks,
                                                    &Leave_print );
    OTF2_Reader_RegisterGlobalEvtCallbacks( reader,
                                            global_evt_reader,
                                            event_callbacks,
                                            &ha );
    OTF2_GlobalEvtReaderCallbacks_Delete( event_callbacks );
    uint64_t events_read = 0;
    OTF2_Reader_ReadAllGlobalEvents(reader,
                                    global_evt_reader,
                                    &events_read);
    OTF2_Reader_CloseGlobalEvtReader( reader, global_evt_reader );
    OTF2_Reader_CloseEvtFiles( reader );
    OTF2_Reader_Close( reader );

    updateTime();
}


OTF2_TraceModel::~OTF2_TraceModel() {}

void OTF2_TraceModel:: initialize_component_list()
{
    components_.clear();
    int rootLink = components_.addItem("Stand", Selection::ROOT);
    parent_component_ = rootLink;
}

int OTF2_TraceModel:: getParentComponent() const
{
    return parent_component_;
#if 0
    if (currentElement.tagName() == "component")
        return currentElement.attribute("name");
    else
        return "";
#endif
}

const QList<int> & OTF2_TraceModel:: getVisibleComponents() const
{
    return visible_components_;
}

int OTF2_TraceModel:: lifeline(int component) const
{
    return lifeline_map_.contains(component) ? lifeline_map_[component] : -1;
}

TraceModel::ComponentType OTF2_TraceModel::getComponentType(int component) const
{
    return hasChildren(component) ? ComponentType::RCHM : ComponentType::CHM;
}

QString OTF2_TraceModel::getComponentName(int component, bool full) const
{
    Q_ASSERT(component >= 0 && component < components_.size());

    if (!full) return components_.item(component);

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

bool OTF2_TraceModel::hasChildren(int component) const
{
    return components_.hasChildren(component);
}

Time OTF2_TraceModel::getMinTime() const
{
    return min_time_;
}

Time OTF2_TraceModel::getMaxTime() const
{
    return max_time_;
}

Time OTF2_TraceModel::getMinResolution() const
{
    return Time(3);
}

void OTF2_TraceModel::rewind()
{
    currentEvent = 0;
    currentSubcomponent = -1;
    currentGroup = 0;
    //testAddMessages();
}

void OTF2_TraceModel::testAddMessages()
{
    GroupModel* gm = new GroupModel();
    QVector<GroupModel::Point> gmvec;
    GroupModel::Point gp1;
    gp1.component = 4;
    gp1.time = Time(10000);
    GroupModel::Point gp2;
    gp2.component = 2;
    gp2.time = Time(10000);
    GroupModel::Point gp3;
    gp3.component = 3;
    gp3.time = Time(20000);
    gmvec.push_back(gp1);
    gmvec.push_back(gp2);
    gmvec.push_back(gp3);
    gm->points = gmvec;
    gm->type = GroupModel::arrow;
    allGroups.push_back(gm);
}

void OTF2_TraceModel::updateTime()
{
    max_time_ = allEvents[allEvents.size() - 1]->time;
    //? TEST
    for (int i = 0; i < allStates.size(); ++i)
    {
        if (allStates[i]->end.getData().tv_sec == 0 && allStates[i]->end.getData().tv_nsec == 0)
        {
            allStates[i]->end = max_time_;
        }
    }
}

StateModel* OTF2_TraceModel::getNextState()
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

GroupModel* OTF2_TraceModel::getNextGroup()
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

EventModel* OTF2_TraceModel::getNextEvent()
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

TraceModelPtr OTF2_TraceModel::root()
{
    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->parent_component_ = Selection::ROOT;

    n->min_time_ = Time(0);

    n->events_.enableAll(Selection::ROOT, true);
    n->adjust_components();

    return n;
}

TraceModelPtr OTF2_TraceModel::setParentComponent(int component)
{

    if (parent_component_ == component)
        return shared_from_this();

    if (component == Selection::ROOT)
    {
        OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
        n->parent_component_ = Selection::ROOT;
        n->adjust_components();
        return n;
    }

    if (component == components_.itemParent(parent_component_))
    {
        OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    if (components_.itemParent(component) == parent_component_)
    {
        OTF2TraceModelPtr n(new OTF2_TraceModel(*this));

        n->parent_component_ = component;
        n->adjust_components();
        return n;
    }

    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));

    QList<int> parents; parents << component;
    while (parents.front() != component)
        parents.prepend(components_.itemParent(component));
    parents.pop_front();

    n->parent_component_ = component;
    n->adjust_components();
    return n;
}

TraceModelPtr OTF2_TraceModel::setRange(const Time& min, const Time& max)
{
    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->min_time_ = min;
    n->max_time_ = max;
    return n;
}

const Selection & OTF2_TraceModel::getComponents() const
{
    return components_;
}

TraceModelPtr OTF2_TraceModel::filterComponents(const Selection & filter)
{
    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->components_ = filter;
    n->adjust_components();
    return n;
}

const Selection & OTF2_TraceModel::getEvents() const
{
    return events_;
}

const Selection & OTF2_TraceModel::getStates() const
{
    return states_;
}

const Selection& OTF2_TraceModel::getAvailableStates() const
{
    return available_states_;
}

TraceModelPtr OTF2_TraceModel::filterStates(const Selection& filter)
{
    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->states_ = filter;
    //n->adjust_components();
    return n;
}

TraceModelPtr OTF2_TraceModel::filterEvents(const Selection& filter)
{
    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->events_ = filter;
    return n;
}

QString OTF2_TraceModel::save() const
{
    QString componentPos;
    componentPos = "/" + componentPos;

    return  componentPos + ":" +
        min_time_.toString() + ":" + max_time_.toString();
}

bool OTF2_TraceModel::groupsEnabled() const
{
    return groups_enabled_;
}

TraceModelPtr OTF2_TraceModel::setGroupsEnabled(bool enabled)
{
    if (groups_enabled_ == enabled)
    {
        return shared_from_this();
    }

    OTF2TraceModelPtr n(new OTF2_TraceModel(*this));
    n->groups_enabled_ = enabled;
    return n;
}

void OTF2_TraceModel::restore(const QString& s)
{
    parent_component_ = Selection::ROOT;
    adjust_components();
}

void OTF2_TraceModel::initialize()
{
    // Initialize events
    events_.clear();
    events_.addItem("Update");
    events_.addItem("Delay");
    events_.addItem("Send");
    events_.addItem("Receive");
    events_.addItem("Stop");
}

void OTF2_TraceModel::adjust_components()
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

void OTF2_TraceModel::findNextItem(const QString& elementName)
{

}

}

