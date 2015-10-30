#include "otf_trace_data.h"

#include <cassert>

namespace temp {

OTF_trace_data::OTF_trace_data() {}

OTF_trace_data::~OTF_trace_data()
{
    OTF_Reader_close(reader_ptr);
    OTF_HandlerArray_close(handlers_ptr);
    OTF_FileManager_close(manager_ptr);
}

bool OTF_trace_data::init(QString filename)
{
    manager_ptr = OTF_FileManager_open(100);//? what if > 100?
    assert(manager_ptr );

    handlers_ptr  = OTF_HandlerArray_open();
    assert(handlers_ptr );

    /* processes */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleDefProcessGroup, OTF_DEFPROCESSGROUP_RECORD );
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleDefProcess, OTF_DEFPROCESS_RECORD );

     /* functions */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleDefFunctionGroup, OTF_DEFFUNCTIONGROUP_RECORD );
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleDefFunction, OTF_DEFFUNCTION_RECORD );

     /* markers */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleDefMarker, OTF_DEFMARKER_RECORD );

    /* for reading enter/leave functions */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleEnter, OTF_ENTER_RECORD );
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleLeave, OTF_LEAVE_RECORD );

    /* for reading markers */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleMarker, OTF_MARKER_RECORD );

    /* messages */
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleSendMsg, OTF_SEND_RECORD );
    OTF_HandlerArray_setHandler( handlers_ptr , (OTF_FunctionPointer*) handleRecvMsg, OTF_RECEIVE_RECORD );

    reader_ptr = OTF_Reader_open( filename.toLatin1().data(), manager_ptr );
    assert(reader_ptr);

    // read definitions and handle them
    OTF_Reader_readDefinitions( reader_ptr, handlers_ptr  );

    //? ???
    // чтение событий Events
    OTF_Reader_setRecordLimit(reader_ptr, 1);
    OTF_Reader_readEvents( reader_ptr, handlers_ptr );
    OTF_Reader_setRecordLimit(reader_ptr, 3);
    OTF_Reader_readMarkers( reader_ptr, handlers_ptr );

    return true;
}

OTF_FileManager* OTF_trace_data::get_manager_ptr()
{
    return manager_ptr;
}

OTF_Reader* OTF_trace_data::get_reader_ptr()
{
    return reader_ptr;
}

OTF_HandlerArray* OTF_trace_data::get_handlers_ptr()
{
    return handlers_ptr;
}

}
