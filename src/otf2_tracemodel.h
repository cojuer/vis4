#ifndef OTF2_TRACEMODEL_H
#define OTF2_TRACEMODEL_H

#include <QVector>
#include <QFile>
#include <QMap>
#include <QDebug>
#include <QTextCodec>

#include <sstream>
#include <set>
#include <queue>
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <memory>

#include <boost/enable_shared_from_this.hpp>
#include <otf2/otf2.h>
#include <otf.h>
#include "trace_model.h"
#include "state_model.h"
#include "event_model.h"
#include "canvas_item.h"
#include "group_model.h"
#include "event_list.h"

namespace vis4 {

class OTF2_TraceModel;

typedef struct
{
    uint64_t count;
    int parent_component;
    Selection* components;
    Selection* state_types;
    QVector<State_model*>* states;
    QVector<EventModel*>* events;
    uint64_t countSend;
    uint64_t countRecv;
} OTF2_HandlerArgument;

class OTF2_TraceModel :
    public boost::enable_shared_from_this<OTF2_TraceModel>
{
public: /** members */
    typedef boost::shared_ptr<OTF2_TraceModel> OTF2TraceModelPtr;

public: /** methods */
    OTF2_TraceModel(const QString& filename);
    ~OTF2_TraceModel();

    int getParentComponent() const;
    const QList<int>& getVisibleComponents() const;
    int lifeline(int component) const;
    QString componentName(int component, bool full = false) const;
    bool hasChildren(int component) const;

    Time getMinTime() const;
    Time getMaxTime() const;
    Time getMinResolution() const;

    void rewind();

    State_model* nextState();
    GroupModel* nextGroup();
    EventModel* nextEvent();
    EventModel* nextEventUnsorted();//?

    TraceModelPtr root();
    TraceModelPtr setParentComponent(int component);
    TraceModelPtr setRange(const Time& min, const Time& max);

    const Selection& getComponents() const;
    TraceModelPtr filterComponents(const Selection & filter);

    const Selection& getEvents() const;
    const Selection& getStates() const;
    const Selection& getAvailableStates() const;

    TraceModelPtr filterStates(const Selection& filter);
    TraceModelPtr installChecker(Checker* checker);
    TraceModelPtr filterEvents(const Selection& filter);

    QString save() const;
    bool areGroupsEnabled() const;
    TraceModelPtr setGroupsEnabled(bool enabled);
    void restore(const QString& s);

private:    /** members */
    OTF_FileManager* manager;
    OTF2_Reader* reader;

    int parentComponent;
    Selection components;//? processes?
    Selection events;
    bool groupsEnabled;
    Selection states;
    Selection availableStates;

    Time minTime;
    Time maxTime;

    OTF_HandlerArray* handlers;
    OTF2_HandlerArgument ha;
    uint64_t ret;

private:    /** methods */
    Time getTime(int t) const;
    void initialize();
    void adjustComponents();
    void initializeComponentList();

    void updateTime();


    QList<int> visibleComponents;
    QMap<int, int> lifelineMap;

    int currentSubcomponent;

    QVector<EventModel*> allEvents;
    int currentEvent;

    QVector<State_model*> allStates;
    int currentState;

    QVector<GroupModel*> allGroups;
    int currentGroup;
};

/** OTF2 callbacks */
static OTF2_CallbackCode
handleSetState( OTF2_LocationRef    location,
                OTF2_TimeStamp      time,
                void*               userData,
                OTF2_AttributeList* attributes,
                OTF2_RegionRef      region )
{
    std::cout << "Entering region " << region << " at location " << location <<
                 " at time " << time << std::endl;
    return OTF2_CALLBACK_SUCCESS;
}

}   //namespace

#endif // OTF2_TRACEMODEL_H
