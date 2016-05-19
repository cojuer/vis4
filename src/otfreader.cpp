#include "otfreader.h"

namespace vis4 {

static int handleDefProcess (void* userData, uint32_t stream, uint32_t process, const char *name, uint32_t parent)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    int current_link = arg->components->addItem(QString(name), -1);//parent -1

    return OTF_RETURN_OK;
}

static int handleDefFunction (void* userData, uint32_t stream, uint32_t func, const char *name, uint32_t funcGroup, uint32_t source)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);

    arg->stateTypes->addItem(QString(name), -1);

    return OTF_RETURN_OK;
}

/** Обработчики событий и состояний */
static int handleEnter (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    //qDebug() << time << " E proc:" << process;
    auto arg = static_cast<NewHandlerArgument*>(userData);
    StateModel* sm = new StateModel(process, function, Time(time), Time(0), Qt::yellow);
    arg->states->push_back(sm);

    EventModel* em = new EventModel(Time(time), process, "ENTER", 'E');
    arg->events->push_back(em);

    return OTF_RETURN_OK;
}

static int handleLeave (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    //qDebug() << time << " L proc:" << process;
    auto arg = static_cast<NewHandlerArgument*>(userData);
    for (int i = arg->states->size() - 1; i > 0; --i)
    {
        if (process == (*arg->states)[i]->component)
        {
            (*arg->states)[i]->end = Time(time);
            break;
        }
    }

    EventModel* em = new EventModel(Time(time), process, "LEAVE", 'L');
    arg->events->push_back(em);

    return OTF_RETURN_OK;
}

static int handleSendMsg(void *userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);

    GroupModel* gm = new GroupModel();
    gm->id = length;
    QVector<GroupModel::Point> gmvec;
    GroupModel::Point sendPoint;
    sendPoint.component = sender;
    sendPoint.time = Time(time);
    gmvec.push_back(sendPoint);
    gm->points = gmvec;
    gm->type = GroupModel::arrow;

    arg->groups->push_back(gm);

    return OTF_RETURN_OK;
}

static int handleRecvMsg(void *userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);

    for (int i = arg->groups->size() - 1; i > 0; --i)
    {
        if ((*arg->groups)[i]->id == length)
        {
            GroupModel::Point recvPoint;
            recvPoint.component = recvProc;
            recvPoint.time = Time(time);
            (*arg->groups)[i]->points.push_back(recvPoint);
            break;
        }
    }

    return OTF_RETURN_OK;
}

TraceData* OTFReader::read(QString tracePath)
{
    Selection* componentsPtr = new Selection();
    Selection* stateTypesPtr = new Selection();
    Selection* eventTypesPtr = new Selection();

    QVector<EventModel*>* eventsPtr = new QVector<EventModel*>;
    QVector<StateModel*>* statesPtr = new QVector<StateModel*>;
    QVector<GroupModel*>* groupsPtr = new QVector<GroupModel*>;

    NewHandlerArgument ha = {componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr, groupsPtr};

    auto manager = OTF_FileManager_open(100);//? what if > 100?
    assert(manager);

    auto handlers = OTF_HandlerArray_open();
    assert(handlers);

    /* processes */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleDefProcess, OTF_DEFPROCESS_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFPROCESS_RECORD );

     /* functions */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleDefFunction, OTF_DEFFUNCTION_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_DEFFUNCTION_RECORD );

    /* for reading enter/leave functions */
    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleEnter, OTF_ENTER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_ENTER_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleLeave, OTF_LEAVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_LEAVE_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleSendMsg, OTF_SEND_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_SEND_RECORD );

    OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*)handleRecvMsg, OTF_RECEIVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &ha, OTF_RECEIVE_RECORD );

    auto reader = OTF_Reader_open( tracePath.toLatin1().data(), manager );
    assert(reader);

    // чтение определений и обработка их обработчикоми handlers
    auto ret = OTF_Reader_readDefinitions( reader, handlers );

    // чтение событий Events
    OTF_Reader_setRecordLimit(reader, 10000000);
    OTF_Reader_readEvents(reader, handlers);
    OTF_Reader_readMarkers(reader, handlers);
    OTF_Reader_readStatistics(reader, handlers);
    OTF_Reader_readSnapshots(reader, handlers);

    qDebug() << eventsPtr->size();

    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr, groupsPtr);
}

}

