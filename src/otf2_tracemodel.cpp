#include "otf2_tracemodel.h"
#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_ErrorCodes.h>

namespace vis4
{

static OTF2_CallbackCode
GlobDefLocation_Register( void*                 userData,
                          OTF2_LocationRef      location,
                          OTF2_StringRef        name,
                          OTF2_LocationType     locationType,
                          uint64_t              numberOfEvents,
                          OTF2_LocationGroupRef locationGroup )
{
    QVector<OTF2_LocationRef>* locations = static_cast<QVector<OTF2_LocationRef>*>(userData);
    locations->push_back(location);
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_TraceModel::OTF2_TraceModel(const QString& filename)
{
    //? TEST
    OTF2_Reader* reader = OTF2_Reader_Open("hello/hello_worlds.otf2");
    OTF2_Reader_SetSerialCollectiveCallbacks(reader);
    QVector<OTF2_LocationRef> locations;
    OTF2_GlobalDefReader* globalDefReader = OTF2_Reader_GetGlobalDefReader(reader);
    OTF2_GlobalDefReaderCallbacks* globalDefCallbacks = OTF2_GlobalDefReaderCallbacks_New();
    OTF2_GlobalDefReaderCallbacks_SetLocationCallback(globalDefCallbacks, &GlobDefLocation_Register);
    OTF2_Reader_RegisterGlobalDefCallbacks(reader,
                                           globalDefReader,
                                           globalDefCallbacks,
                                           &locations);
    OTF2_GlobalDefReaderCallbacks_Delete(globalDefCallbacks );
    uint64_t definitionsRead = 0;
    OTF2_Reader_ReadAllGlobalDefinitions(reader,
                                         globalDefReader,
                                         &definitionsRead);
}

}

