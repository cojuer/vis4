#ifndef OTF_TRACE_MODEL_H
#define OTF_TRACE_MODEL_H

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

namespace vis4 {

class OTF_trace_model;

typedef struct {
    uint64_t count;
    int parent_component;
    Selection* components;
    Selection* state_types;
    QVector<State_model*>* states;
    QVector<EventModel*>* events;
    uint64_t countSend;
    uint64_t countRecv;
} HandlerArgument;

class OTF_trace_model : 
    public Trace_model,
    public boost::enable_shared_from_this<OTF_trace_model>
{
public: /** members */
    typedef boost::shared_ptr<OTF_trace_model> OTFTraceModelPtr;

public: /** methods */
    OTF_trace_model(const QString& filename);
    ~OTF_trace_model();

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

    State_model* next_state() override;
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
    OTF_Reader* reader;

    int parent_component_;
    Selection components_;//? processes?
    Selection events_;
    bool groups_enabled_;
    Selection states_;
    Selection available_states_;

    Time min_time_;
    Time max_time_;

    OTF_HandlerArray* handlers;
    HandlerArgument ha;
    uint64_t ret;

private:    /** methods */
    Time getTime(int t) const;
    void initialize();
    void adjust_components();
    void initialize_component_list();

//?
    void findNextItem(const QString& elementName);

    //TEST
    void testAddMessages();
    void updateTime();

    QList<int> visible_components_;
    QMap<int, int> lifeline_map_;

    int currentSubcomponent;

    // All events sorted by the time.
    QVector<EventModel*> allEvents;
    int currentEvent;

    QVector<State_model*> allStates;
    int currentState;

    QVector<GroupModel*> allGroups;
    int currentGroup;

    QVector<MessageModel*> allMessages;
    int currentMessage;
};



/** Обработчики определений компонентов */
static int handleDefProcessGroup (void *userData, uint32_t stream, uint32_t procGroup, const char *name, uint32_t numberOfProcs, const uint32_t *procs)
{
    qDebug() << "stream = " << stream << " procGroup: " << procGroup << " name: " << name << " number: " << numberOfProcs;
    QString str = "";
    for (int i=0; i<numberOfProcs; i++) str += QString::number(procs[i]) + " ";
    qDebug() << "process list: " << str;
    return OTF_RETURN_OK;
}

static int handleDefProcess (void *userData, uint32_t stream, uint32_t process, const char *name, uint32_t parent)
{
    int current_link = ((HandlerArgument*)userData)->components->addItem( QString(name), (int) parent);
    qDebug() << "stream = " << stream << " process: " << process << " name: " << name << " parent: " << parent;
    return OTF_RETURN_OK;
}

/** Обработчики определений состояний */
static int handleDefFunctionGroup (void *userData, uint32_t stream, uint32_t funcGroup, const char *name)
{
    qDebug() << "stream = " << stream << " funcGroup: " << funcGroup << " name: " << name;
    return OTF_RETURN_OK;
}

static int handleDefFunction (void *userData, uint32_t stream, uint32_t func, const char *name, uint32_t funcGroup, uint32_t source)
{
    QTextCodec *codec = QTextCodec::codecForName( "KOI8-R" );
    QString tr_name = codec->toUnicode(name);
    QTextCodec *codec1 = QTextCodec::codecForName( "UTF-8" );
    tr_name = codec1->fromUnicode(tr_name);
    ((HandlerArgument*)userData)->state_types->addItem(tr_name, -1);
    qDebug() << "stream = " << stream << " func: " << func << " name: " << tr_name << " funcGroup: " << funcGroup << " source: " << source;
    return OTF_RETURN_OK;
}

/** Обработчики определений событий */
static int handleDefMarker(void *userData, uint32_t stream, uint32_t token, const char *name, uint32_t type)
{
    qDebug() << "DEFMARKER: stream = " << stream << " token: " << token << " name: " << name << " type: " << type;
    return OTF_RETURN_OK;
}

/** Обработчики событий и состояний */
static int handleEnter (void *userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    State_model* sm = new State_model();
    sm->component = process;
    sm->type = function;
    sm->begin = scalarTime<int>(time);
    sm->end = scalarTime<int>(-1);//TEST
    sm->color = Qt::yellow;
    ((HandlerArgument*)userData)->states->push_back(sm);
    EventModel* em = new EventModel();
    em->time = scalarTime<int>(time);
    em->component = process;
    em->kind = "ENTER";
    em->letter = 'E';
    ((HandlerArgument*)userData)->events->push_back(em);
    qDebug() << "ENTER: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

static int handleLeave (void *userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    for (int i = ((HandlerArgument*)userData)->states->size() - 1; i > 0; --i)
    {
        //TEST
        if (process == (*((HandlerArgument*)userData)->states)[i]->component)
        {
            (*((HandlerArgument*)userData)->states)[i]->end = scalarTime<int>(time);
        }
    }
    EventModel* em = new EventModel();
    em->time = scalarTime<int>(time);
    em->component = process;
    em->kind = "LEAVE";
    em->letter = 'L';
    ((HandlerArgument*)userData)->events->push_back(em);
    qDebug() << "LEAVE: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

 static int handleMarker(void *userData, uint64_t time, uint32_t process, uint32_t token, const char *text, OTF_KeyValueList *list)
{
     qDebug() << "MARKER: " << text;
     return OTF_RETURN_OK;
}


/** Обработчики сообщений */
static int handleSendMsg(void *userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
{
     qDebug() << "SendMsg: ";
     ((HandlerArgument*)userData)->countSend++;
     return OTF_RETURN_OK;
}

 static int handleRecvMsg(void *userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
{
     qDebug() << "RecvMsg: ";
     ((HandlerArgument*)userData)->countRecv++;
     return OTF_RETURN_OK;
}

}   // End of Namespace

#endif //
