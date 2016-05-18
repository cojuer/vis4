#include "otfreader.h"

namespace vis4 {

static int handleDefProcess (void* userData, uint32_t stream, uint32_t process, const char *name, uint32_t parent)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    int current_link = arg->components->addItem(QString(name), -1);//parent -1
    //qDebug() << "stream = " << stream << " process: " << process << " name: " << name << " parent: " << parent;
    return OTF_RETURN_OK;
}

static int handleDefFunction (void* userData, uint32_t stream, uint32_t func, const char *name, uint32_t funcGroup, uint32_t source)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    QTextCodec* codec = QTextCodec::codecForName( "KOI8-R" );
    QString tr_name = codec->toUnicode(name);
    QTextCodec* codec1 = QTextCodec::codecForName( "UTF-8" );
    tr_name = codec1->fromUnicode(tr_name);
    arg->stateTypes->addItem(tr_name, -1);
    //qDebug() << "stream = " << stream << " func: " << func << " name: " << tr_name << " funcGroup: " << funcGroup << " source: " << source;
    return OTF_RETURN_OK;
}

/** Обработчики событий и состояний */
static int handleEnter (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    StateModel* sm = new StateModel(process, function, Time(time), Time(0), Qt::yellow);
    arg->states->push_back(sm);

    EventModel* em = new EventModel(Time(time), process, "ENTER", 'E');
    arg->events->push_back(em);

    //qDebug() << "ENTER: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

static int handleLeave (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    for (int i = arg->states->size() - 1; i > 0; --i)
    {
        //TEST
        if (process == (*arg->states)[i]->component)
        {
            (*arg->states)[i]->end = Time(time);
            break;
        }
    }

    EventModel* em = new EventModel(Time(time), process, "LEAVE", 'L');
    arg->events->push_back(em);

    //qDebug() << "LEAVE: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

TraceData* OTFReader::read(QString tracePath)
{
    Selection* componentsPtr = new Selection();
    Selection* stateTypesPtr = new Selection();
    Selection* eventTypesPtr = new Selection();

    QVector<EventModel*>* eventsPtr = new QVector<EventModel*>;
    QVector<StateModel*>* statesPtr = new QVector<StateModel*>;

    NewHandlerArgument ha = {componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr};

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

    auto reader = OTF_Reader_open( tracePath.toLatin1().data(), manager );
    assert(reader);

    // чтение определений и обработка их обработчикоми handlers
    auto ret = OTF_Reader_readDefinitions( reader, handlers );

    // чтение событий Events
    OTF_Reader_setRecordLimit(reader, 100000);
    OTF_Reader_readEvents(reader, handlers);
    OTF_Reader_readMarkers(reader, handlers);
    OTF_Reader_readStatistics(reader, handlers);
    OTF_Reader_readSnapshots(reader, handlers);

    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr);
}

}

