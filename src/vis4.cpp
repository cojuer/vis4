#include <QtWidgets/QApplication>
#include <boost/enable_shared_from_this.hpp>

#include "otf_trace_model.h"
#include "otf2_tracemodel.h"
#include "newtracemodel.h"
#include "main_window.h"

int main(int ac, char* av[])
{
    using namespace vis4;
    QApplication app(ac, av);
    app.setOrganizationName("Computer Systems Laboratory");
    app.setOrganizationDomain("lvk.cs.msu.su");
    app.setApplicationName("vis4");

    TraceModelPtr model(new NewTraceModel("../otf_traces/philosophers/philosophers.otf"));
    //TraceModelPtr model(new OTF_trace_model("../otf_traces/philosophers/philosophers.otf"));
    //TraceModelPtr model(new OTF2_TraceModel("../otf_traces/ArchivePath/ArchiveName.otf2"));

    MainWindow mw;
    mw.initialize(model);
    mw.show();

    app.exec();
    return 0;
}
