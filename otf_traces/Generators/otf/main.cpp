#include <cassert>
#include <otf.h>

#include <QDebug>

int main(int argc, char *argv[])
{
    OTF_FileManager* manager;
    OTF_Writer* writer;

    manager= OTF_FileManager_open( 100 );
    assert( manager );

    writer = OTF_Writer_open( "testotftrace", 1, manager );
    assert( writer );

    OTF_Writer_writeDefTimerResolution( writer, 0, 1 );
    OTF_Writer_writeDefProcess( writer, 0, 0, "proc 0", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 1, "proc 1", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 2, "proc 2", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 3, "proc 3", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 4, "proc 4", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 5, "proc 5", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 6, "proc 6", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 7, "proc 7", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 8, "proc 8", 0 );
    OTF_Writer_writeDefProcess( writer, 0, 9, "proc 9", 0 );
    OTF_Writer_writeDefFunctionGroup( writer, 0, 1000, "all functions" );
    OTF_Writer_writeDefFunction( writer, 0, 1, "main", 1000, 0 );

    int i = 0;
    while (i < 100000)
    {
        for (int j = 0; j < 10; ++j)
        {
            OTF_Writer_writeEnter( writer, i, 0, j, 0 );
            if (i % 1000 == 0)
            OTF_Writer_writeSendMsg(writer, i, j, 9 - j, 0, 0, 100 * i + j, 0);
        }

        for (int j = 0; j < 10; ++j)
        {
            OTF_Writer_writeLeave( writer, i + 1, 0, j, 0 );
            if (i % 1000 == 0)
            OTF_Writer_writeRecvMsg(writer, i + 1, 9 - j, j, 0, 0, 100 * i + j, 0);
        }
        ++i;
        qDebug() << i;
    }

    OTF_Writer_close( writer );
    OTF_FileManager_close( manager );

    return 0;
}


