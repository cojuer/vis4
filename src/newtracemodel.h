#ifndef NEWTRACEMODEL_H
#define NEWTRACEMODEL_H

#include <QVector>
#include <QDomDocument>
#include <QDomElement>
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
#include <otf.h>

#include "trace_model.h"
#include "state_model.h"
#include "event_model.h"
#include "message_model.h"
#include "canvas_item.h"
#include "group_model.h"
#include "event_list.h"
#include "trace_data.h"
#include "otfreader.h"
#include "otf2reader.h"

namespace vis4 {

class NewTraceModel :
    public TraceModel,
    public std::enable_shared_from_this<NewTraceModel>
{
public: /** members */
    typedef std::shared_ptr<NewTraceModel> NewTraceModelPtr;

public: /** methods */
    NewTraceModel(const QString& filename);
    ~NewTraceModel();

    int getParentComponent() const;
    const QList<int>& getVisibleComponents() const;
    int lifeline(int component) const;
    ComponentType getComponentType(int component) const;
    QString getComponentName(int component, bool full = false) const;
    bool hasChildren(int component) const;

    Time getMinTime() const override;
    Time getMaxTime() const override;
    Time getMinResolution() const override;

    void rewind() override;

    StateModel* getNextState() override;
    GroupModel* getNextGroup() override;
    EventModel* getNextEvent() override;

    TraceModelPtr root();
    TraceModelPtr setParentComponent(int component);
    TraceModelPtr setRange(const Time& min, const Time& max);

    const Selection& getComponents() const;
    TraceModelPtr filterComponents(const Selection & filter);

    const Selection& getEvents() const;
    const Selection& getStates() const;
    const Selection& getAvailableStates() const;

    TraceModelPtr filterStates(const Selection& filter);

    TraceModelPtr filterEvents(const Selection& filter);

    QString save() const override;
    bool groupsEnabled() const;
    TraceModelPtr setGroupsEnabled(bool enabled);
    void restore(const QString& s);

private:    /** members */
    TraceData* dataPtr;

    int parent_component_;
    Selection components_;
    Selection events_;
    bool groups_enabled_;
    Selection states_;
    Selection available_states_;

    Time minTime;
    Time maxTime;
private:    /** methods */
    Time getTime(int t) const;
    void initialize();
    void adjust_components();
    void initialize_component_list();

    void findNextItem(const QString& elementName);

    //TEST
    void testAddMessages();
    void updateTime();

    QList<int> visible_components_;
    QMap<int, int> lifeline_map_;

    int currentSubcomponent;
};

}

#endif // NEWTRACEMODEL_H
