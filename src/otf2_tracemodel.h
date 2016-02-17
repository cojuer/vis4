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
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
    uint64_t countSend;
    uint64_t countRecv;
} OTF2_HandlerArgument;

class OTF2_TraceModel :
    public Trace_model,
    public boost::enable_shared_from_this<OTF2_TraceModel>
{
public: /** members */
    typedef boost::shared_ptr<OTF2_TraceModel> OTF2TraceModelPtr;

public: /** methods */
    OTF2_TraceModel(const QString& filename);
    ~OTF2_TraceModel();

    int parent_component() const;
    const QList<int>& visible_components() const;
    int lifeline(int component) const;
    ComponentType component_type(int component) const;
    QString component_name(int component, bool full = false) const;
    bool has_children(int component) const;

    Time min_time() const override;
    Time max_time() const override;
    Time min_resolution() const override;

    void rewind() override;

    StateModel* next_state() override;
    GroupModel* next_group() override;
    std::auto_ptr<EventModel> next_event_unsorted();
    EventModel* next_event() override;

    TraceModelPtr root();
    TraceModelPtr set_parent_component(int component);
    TraceModelPtr set_range(const Time& min, const Time& max);

    const Selection& components() const;
    TraceModelPtr filter_components(const Selection & filter);

    const Selection& events() const;
    const Selection& states() const;
    const Selection& available_states() const;

    TraceModelPtr filter_states(const Selection& filter);
    TraceModelPtr install_checker(Checker* checker);
    TraceModelPtr filter_events(const Selection& filter);

    QString save() const override;
    bool groupsEnabled() const;
    TraceModelPtr setGroupsEnabled(bool enabled);
    void restore(const QString& s);

private:    /** members */
    OTF_FileManager* manager;
    OTF2_Reader* reader;

    int parent_component_;
    Selection components_;//? processes?
    Selection events_;
    bool groups_enabled_;
    Selection states_;
    Selection available_states_;

    Time min_time_;
    Time max_time_;

    OTF_HandlerArray* handlers;
    OTF2_HandlerArgument ha;
    uint64_t ret;

private:    /** methods */
    Time getTime(int t) const;
    void initialize();
    void adjust_components();
    void initialize_component_list();

    void findNextItem(const QString& elementName);

    void testAddMessages();
    void updateTime();

    QList<int> visible_components_;
    QMap<int, int> lifeline_map_;

    int currentSubcomponent;

    QVector<EventModel*> allEvents;
    int currentEvent;

    QVector<StateModel*> allStates;
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
