#include <iostream>

using namespace std;

#include <otf2/otf2.h>
#include <stdlib.h>

static OTF2_TimeStamp
get_time( void )
{
    static uint64_t sequence;
    return sequence++;
}

static OTF2_FlushType
pre_flush( void*            userData,
           OTF2_FileType    fileType,
           OTF2_LocationRef location,
           void*            callerData,
           bool             final )
{
    return OTF2_FLUSH;
}
static OTF2_TimeStamp
post_flush( void*            userData,
            OTF2_FileType    fileType,
            OTF2_LocationRef location )
{
    return get_time();
}

static OTF2_FlushCallbacks flush_callbacks =
{
    .otf2_pre_flush  = pre_flush,
    .otf2_post_flush = post_flush
};

int
main( int    argc,
      char** argv )
{
    OTF2_Archive* archive = OTF2_Archive_Open( "ArchivePath",
                                               "ArchiveName",
                                               OTF2_FILEMODE_WRITE,
                                               1024 * 1024 /* event chunk size */,
                                               4 * 1024 * 1024 /* def chunk size */,
                                               OTF2_SUBSTRATE_POSIX,
                                               OTF2_COMPRESSION_NONE );

    OTF2_Archive_SetFlushCallbacks( archive, &flush_callbacks, NULL );

    OTF2_Archive_SetSerialCollectiveCallbacks( archive );

    OTF2_Archive_OpenEvtFiles( archive );

    for (int j = 0; j < 10; ++j)
    {
        OTF2_EvtWriter* evt_writer = OTF2_Archive_GetEvtWriter( archive, j );
        for (int i = 0;i < 100000; ++i)
        {
            OTF2_EvtWriter_Enter( evt_writer, nullptr, i, 0 /* region */ );

            if (i % 1000 == 0)
            {
                OTF2_EvtWriter_MpiSend(evt_writer, nullptr, i, 0, 0, 0, 100 * i + j);
            }
            OTF2_EvtWriter_Leave( evt_writer, nullptr, i + 1, 0 /* region */ );
            if (i % 1000 == 0)
            {
                OTF2_EvtWriter_MpiRecv(evt_writer, nullptr, i + 1, 0, 0, 0, 100 *  i + (9 - j));
            }
        }
        OTF2_Archive_CloseEvtWriter( archive, evt_writer );
    }

    OTF2_Archive_CloseEvtFiles( archive );

    OTF2_GlobalDefWriter* global_def_writer = OTF2_Archive_GetGlobalDefWriter( archive );

    OTF2_GlobalDefWriter_WriteClockProperties( global_def_writer,
                                               1 /* 1 tick per second */,
                                               0 /* epoch */,
                                               2 /* length */ );

    OTF2_GlobalDefWriter_WriteString( global_def_writer, 0, "" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 1, "Master Process" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 2, "Main Thread" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 3, "MyFunction" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 4, "Alternative function name (e.g. mangled one)" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 5, "Computes something" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 6, "MyHost" );
    OTF2_GlobalDefWriter_WriteString( global_def_writer, 7, "node" );

    OTF2_GlobalDefWriter_WriteRegion( global_def_writer,
                                      0 /* id */,
                                      3 /* region name  */,
                                      4 /* alternative name */,
                                      5 /* description */,
                                      OTF2_REGION_ROLE_FUNCTION,
                                      OTF2_PARADIGM_USER,
                                      OTF2_REGION_FLAG_NONE,
                                      0 /* source file */,
                                      0 /* begin lno */,
                                      0 /* end lno */ );

    OTF2_GlobalDefWriter_WriteSystemTreeNode( global_def_writer,
                                              0 /* id */,
                                              6 /* name */,
                                              7 /* class */,
                                              OTF2_UNDEFINED_SYSTEM_TREE_NODE /* parent */ );
    OTF2_GlobalDefWriter_WriteLocationGroup( global_def_writer,
                                             0 /* id */,
                                             1 /* name */,
                                             OTF2_LOCATION_GROUP_TYPE_PROCESS,
                                             0 /* system tree */ );
    for (int j = 0; j < 10; ++j)
    {
        OTF2_GlobalDefWriter_WriteLocation( global_def_writer,
                                            j /* id */,
                                            2 /* name */,
                                            OTF2_LOCATION_TYPE_CPU_THREAD,
                                            2 /* # events */,
                                            0 /* location group */ );
    }
    OTF2_Archive_Close( archive );
    return EXIT_SUCCESS;
}

