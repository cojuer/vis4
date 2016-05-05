#include "otfreader.h"

namespace vis4 {

TraceData* read(QString tracePath)
{
    Selection* componentsPtr = new Selection();
    Selection* stateTypesPtr = new Selection();
    Selection* eventTypesPtr = new Selection();

    QVector<EventModel*>* eventsPtr;
    QVector<StateModel*>* statesPtr;

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
    OTF_Reader_setRecordLimit(reader, 1000);
    OTF_Reader_readEvents(reader, handlers);
    OTF_Reader_readMarkers(reader, handlers);
    OTF_Reader_readStatistics(reader, handlers);
    OTF_Reader_readSnapshots(reader, handlers);

    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr);
}

}

