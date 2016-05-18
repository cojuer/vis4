#include <QtWidgets/QApplication>

#include "main_window.h"
#include "tracemodelimpl.h"
#include "xmlreader.h"
#include "otfreader.h"
#include "otf2reader.h"

int main(int ac, char* av[])
{
    using namespace vis4;
    QApplication app(ac, av);
    app.setOrganizationName("Computer Systems Laboratory");
    app.setOrganizationDomain("lvk.cs.msu.su");
    app.setApplicationName("vis4");

    //TraceModelPtr model(new TraceModelImpl("../otf_traces/ArchivePath2/ArchiveName.otf2", new OTF2Reader()));
    //TraceModelPtr model(new TraceModelImpl("../otf_traces/testotftrace/testotftrace.otf", new OTFReader()));
    TraceModelPtr model(new TraceModelImpl("../otf_traces/trace.xml", new XMLReader()));

    MainWindow mw;
    mw.initialize(model);
    mw.show();

    app.exec();
    return 0;
}
