#include "otf_trace_data.h"

#include <cassert>

namespace temp {

OTF_TraceData::OTF_TraceData() {}

OTF_TraceData::~OTF_TraceData()
{
    OTF_Reader_close(readerPtr);
    OTF_HandlerArray_close(handlersPtr);
    OTF_FileManager_close(managerPtr);
}

bool OTF_TraceData::init(QString filename)
{
    info = (HandlerArgument){ 0, 0, nullptr, 0, 0};//? test
    
    managerPtr = OTF_FileManager_open(100);//? what if > 100?
    assert(managerPtr);

    handlersPtr  = OTF_HandlerArray_open();
    assert(handlersPtr);
    /* processes */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleDefProcessGroup, OTF_DEFPROCESSGROUP_RECORD );
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleDefProcess, OTF_DEFPROCESS_RECORD );
     /* functions */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleDefFunctionGroup, OTF_DEFFUNCTIONGROUP_RECORD );
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleDefFunction, OTF_DEFFUNCTION_RECORD );
     /* markers */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleDefMarker, OTF_DEFMARKER_RECORD );
    /* for reading enter/leave functions */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleEnter, OTF_ENTER_RECORD );
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleLeave, OTF_LEAVE_RECORD );
    /* for reading markers */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleMarker, OTF_MARKER_RECORD );
    /* messages */
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleSendMsg, OTF_SEND_RECORD );
    OTF_HandlerArray_setHandler( handlersPtr , (OTF_FunctionPointer*) handleRecvMsg, OTF_RECEIVE_RECORD );

    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_DEFPROCESSGROUP_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_DEFPROCESS_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_DEFFUNCTIONGROUP_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_DEFFUNCTION_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_DEFMARKER_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_ENTER_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_LEAVE_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_MARKER_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_SEND_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlersPtr, &info, OTF_RECEIVE_RECORD);

    readerPtr = OTF_Reader_open(filename.toLatin1().data(), managerPtr);
    assert(readerPtr);

    // read definitions and handle them
    OTF_Reader_readDefinitions( readerPtr, handlersPtr  );

    //? ???
    // чтение событий Events
    OTF_Reader_setRecordLimit(readerPtr, 1);
    OTF_Reader_readEvents( readerPtr, handlersPtr );
    OTF_Reader_setRecordLimit(readerPtr, 3);
    OTF_Reader_readMarkers( readerPtr, handlersPtr );

    return true;
}

OTF_FileManager* OTF_TraceData::getManagerPtr()
{
    return managerPtr;
}

OTF_Reader* OTF_TraceData::getReaderPtr()
{
    return readerPtr;
}

OTF_HandlerArray* OTF_TraceData::getHandlersPtr()
{
    return handlersPtr;
}

}
