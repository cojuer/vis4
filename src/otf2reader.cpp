#include "otf2reader.h"

namespace vis4 {

struct OTF2Location
{
    OTF2_LocationRef      location;
    OTF2_StringRef        name;
    OTF2_LocationType     locationType;
    uint64_t              numberOfEvents;
    OTF2_LocationGroupRef locationGroup;
};

struct OTF2Region
{
    OTF2_RegionRef self;
    OTF2_StringRef name;
};

struct TestData
{
    QVector<OTF2Location> locations;
    QVector<OTF2Region> regions;
    QMap<int, QString> strings;
};

static OTF2_CallbackCode
Enter_print( OTF2_LocationRef    location,
             OTF2_TimeStamp      time,
             void*               userData,
             OTF2_AttributeList* attributes,
             OTF2_RegionRef      region )
{
    auto arg = static_cast<OTF2_NewHandlerArgument*>(userData);

    StateModel* sm = new StateModel(location, region, Time(time), Time(0), Qt::yellow);
    arg->states->push_back(sm);

    EventModel* em = new EventModel(Time(time), location, "ENTER", 'E');
    arg->events->push_back(em);

    std::cout << "Entering region " << region << " at location " << location << " at time " << time << std::endl;
    return OTF2_CALLBACK_SUCCESS;
}
static OTF2_CallbackCode
Leave_print( OTF2_LocationRef    location,
             OTF2_TimeStamp      time,
             void*               userData,
             OTF2_AttributeList* attributes,
             OTF2_RegionRef      region )
{
    auto arg = static_cast<OTF2_NewHandlerArgument*>(userData);

    for (int i = arg->states->size() - 1; i > 0; --i)
    {
        if (location == (*arg->states)[i]->component)
        {
            (*arg->states)[i]->end = Time(time);
        }
    }

    EventModel* em = new EventModel(Time(time), location, "LEAVE", 'L');
    arg->events->push_back(em);

    std::cout << "Leaving region " << region << " at location " << location << " at time " << time << std::endl;
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode
GlobDefLocation_Register( void*                 userData,
                          OTF2_LocationRef      location,
                          OTF2_StringRef        name,
                          OTF2_LocationType     locationType,
                          uint64_t              numberOfEvents,
                          OTF2_LocationGroupRef locationGroup )
{
    std::cout << name << std::endl;
    OTF2Location loc = {location, name, locationType, numberOfEvents, locationGroup};
    static_cast<TestData*>(userData)->locations.push_back(loc);
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode
StringReader(void *userData, OTF2_StringRef self, const char *string)
{
    static_cast<TestData*>(userData)->strings[self] = QString(string);
    return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode regionReader(void *userData, OTF2_RegionRef self, OTF2_StringRef name, OTF2_StringRef canonicalName, OTF2_StringRef description, OTF2_RegionRole regionRole, OTF2_Paradigm paradigm, OTF2_RegionFlag regionFlags, OTF2_StringRef sourceFile, uint32_t beginLineNumber, uint32_t endLineNumber)
{
    OTF2Region reg = {self, name};
    static_cast<TestData*>(userData)->regions.push_back(reg);
}

TraceData* OTF2Reader::read(QString tracePath)
{
    TestData testData;

    Selection* componentsPtr = new Selection();
    Selection* stateTypesPtr = new Selection();
    Selection* eventTypesPtr = new Selection();

    QVector<EventModel*>* eventsPtr;
    QVector<StateModel*>* statesPtr;

    OTF2_NewHandlerArgument ha = {componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr};

    auto reader = OTF2_Reader_Open(tracePath.toUtf8().constData());//should not use QString here
    OTF2_Reader_SetSerialCollectiveCallbacks(reader);

    auto globalDefReader = OTF2_Reader_GetGlobalDefReader(reader);
    auto globalDefCallbacks = OTF2_GlobalDefReaderCallbacks_New();
    OTF2_GlobalDefReaderCallbacks_SetRegionCallback(globalDefCallbacks, &regionReader);
    OTF2_GlobalDefReaderCallbacks_SetLocationCallback(globalDefCallbacks, &GlobDefLocation_Register);
    OTF2_GlobalDefReaderCallbacks_SetStringCallback(globalDefCallbacks, &StringReader);
    OTF2_Reader_RegisterGlobalDefCallbacks(reader,
                                           globalDefReader,
                                           globalDefCallbacks,
                                           &testData);
    OTF2_GlobalDefReaderCallbacks_Delete(globalDefCallbacks );
    uint64_t definitionsRead = 0;
    OTF2_Reader_ReadAllGlobalDefinitions(reader,
                                         globalDefReader,
                                         &definitionsRead);

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        OTF2_Reader_SelectLocation(reader, testData.locations[i].location);
    }
    bool localDefOpened = (OTF2_Reader_OpenDefFiles(reader) == OTF2_SUCCESS);

    OTF2_Reader_OpenEvtFiles(reader);

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        ha.components->addItem(testData.strings[testData.locations[i].name], -1);
    }
    for (unsigned int i = 0; i < testData.regions.size(); ++i)
    {
        ha.stateTypes->addItem(testData.strings[testData.regions[i].name], -1);
    }

    for (unsigned int i = 0; i < testData.locations.size(); ++i)
    {
        if (localDefOpened)
        {
            OTF2_DefReader* def_reader = OTF2_Reader_GetDefReader(reader, testData.locations[i].location);
            if ( def_reader )
            {
                uint64_t def_reads = 0;
                OTF2_Reader_ReadAllLocalDefinitions(reader,
                                                    def_reader,
                                                    &def_reads);
                OTF2_Reader_CloseDefReader(reader, def_reader);
            }
        }
        auto eventReader = OTF2_Reader_GetEvtReader(reader, testData.locations[i].location);
    }
    if (localDefOpened)
    {
        OTF2_Reader_CloseDefFiles(reader);
    }

    auto global_evt_reader = OTF2_Reader_GetGlobalEvtReader(reader);
    auto event_callbacks = OTF2_GlobalEvtReaderCallbacks_New();
    OTF2_GlobalEvtReaderCallbacks_SetEnterCallback(event_callbacks,
                                                   &Enter_print );
    OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback(event_callbacks,
                                                   &Leave_print);
    OTF2_Reader_RegisterGlobalEvtCallbacks(reader,
                                           global_evt_reader,
                                           event_callbacks,
                                           &ha);
    OTF2_GlobalEvtReaderCallbacks_Delete(event_callbacks);
    uint64_t events_read = 0;
    OTF2_Reader_ReadAllGlobalEvents(reader,
                                    global_evt_reader,
                                    &events_read);
    OTF2_Reader_CloseGlobalEvtReader(reader, global_evt_reader);
    OTF2_Reader_CloseEvtFiles(reader);
    OTF2_Reader_Close(reader);
    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr);
}

}

