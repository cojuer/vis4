#ifndef OTF_TRACE_DATA_H
#define OTF_TRACE_DATA_H

#include <QString>
#include <QDebug>

#include <otf.h>

#include "selection.h"

using vis4::Selection;

namespace temp {

typedef struct {
    uint64_t count;
    int parent_component;
    Selection* components;
    uint64_t countSend;
    uint64_t countRecv;
} HandlerArgument;

class OTF_TraceData
{
private:
    OTF_FileManager* managerPtr;
    OTF_Reader* readerPtr;
    OTF_HandlerArray* handlersPtr;
    
    HandlerArgument info;
public:
    OTF_TraceData();
    ~OTF_TraceData();

    //? what do we need?
    OTF_FileManager* getManagerPtr();
    OTF_Reader* getReaderPtr();
    OTF_HandlerArray* getHandlersPtr();

    bool init(QString filename);
};

/** Обработчики определений компонентов */
static int handleDefProcessGroup (void* userData, uint32_t stream, uint32_t procGroup, const char* name, uint32_t numberOfProcs, const uint32_t* procs)
{
    qDebug() << "stream = " << stream << " procGroup: " << procGroup << " name: " << name << " number: " << numberOfProcs;
    QString str = "";
    for (int i = 0; i < numberOfProcs; ++i)
    {
        str += QString::number(procs[i]) + " ";
    }
    qDebug() << "process list: " << str;
    return OTF_RETURN_OK;
}

static int handleDefProcess (void* userData, uint32_t stream, uint32_t process, const char* name, uint32_t parent)
{
    qDebug() << "stream = " << stream << " process: " << process << " name: " << name << " parent: " << parent;
    return OTF_RETURN_OK;
}

/** Обработчики определений состояний */
static int handleDefFunctionGroup (void* userData, uint32_t stream, uint32_t funcGroup, const char* name)
{
    qDebug() << "stream = " << stream << " funcGroup: " << funcGroup << " name: " << name;
    return OTF_RETURN_OK;
}

static int handleDefFunction (void* userData, uint32_t stream, uint32_t func, const char* name, uint32_t funcGroup, uint32_t source)
{
    qDebug() << "stream = " << stream << " func: " << func << " name: " << name << " funcGroup: " << funcGroup << " source: " << source;
    return OTF_RETURN_OK;
}

/** Обработчики определений событий */
static int handleDefMarker(void* userData, uint32_t stream, uint32_t token, const char* name, uint32_t type)
{
    qDebug() << "DEFMARKER: stream = " << stream << " token: " << token << " name: " << name << " type: " << type;
    return OTF_RETURN_OK;
}

/** Обработчики событий и состояний */
static int handleEnter (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList* list)
{
     //fprintf( stdout, "      Enter time: %u function name: '%u'\n", process, function );
     qDebug() << "ENTER: " << function;
     return OTF_RETURN_OK;
}

static int handleLeave (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList* list)
{
     qDebug() << "LEAVE: " << function;
     return OTF_RETURN_OK;
}

static int handleMarker(void* userData, uint64_t time, uint32_t process, uint32_t token, const char* text, OTF_KeyValueList* list)
{
     qDebug() << "MARKER: " << text;
     return OTF_RETURN_OK;
}

/** messages */
static int handleSendMsg(void* userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList* list)
{
     qDebug() << "SendMsg: ";
     reinterpret_cast<HandlerArgument*>(userData)->countSend++;
     return OTF_RETURN_OK;
}

static int handleRecvMsg(void* userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList* list)
{
     qDebug() << "RecvMsg: ";
     reinterpret_cast<HandlerArgument*>(userData)->countRecv++;
     return OTF_RETURN_OK;
}

}

#endif

