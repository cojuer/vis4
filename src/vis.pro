QT += xml \
    widgets \
    printsupport
CONFIG += c++11
LIBS = -L/usr/lib \
    -lm \
    -lotf \
    -lotf2 \
    -lxml2 \
    -Wl,-rpath=/usr/lib \
    -L/opt/otf2/lib \
    -Wl,-rpath=/opt/otf2/lib
INCLUDEPATH += /opt/otf2/include \
    /usr/include/libxml2
SOURCES += vis4.cpp \
    trace_model.cpp \
    selection.cpp \
    otf_trace_model.cpp \
    event_list.cpp \
    canvas_item.cpp \
    main_window.cpp \
    canvas.cpp \
    trace_painter.cpp \
    timeline.cpp \
    timeunit_control.cpp \
    tools/tool.cpp \
    otf_main_window.cpp \
    tools/timeedit.cpp \
    tools/selection_widget.cpp \
    grx.cpp \
    otf_trace_data.cpp \
    time_vis.cpp \
    otf2_trace_data.cpp \
    otf2_tracemodel.cpp \
    message_model.cpp
HEADERS += trace_model.h \
    selection.h \
    otf_trace_model.h \
    state_model.h \
    group_model.h \
    event_model.h \
    event_list.h \
    canvas_item.h \
    main_window.h \
    canvas.h \
    trace_painter.h \
    timeline.h \
    timeunit_control.h \
    tools/tool.h \
    otf_main_window.h \
    tools/browser.h \
    tools/measure.h \
    tools/goto.h \
    tools/find.h \
    tools/filter.h \
    tools/timeedit.h \
    tools/selection_widget.h \
    grx.h \
    otf_trace_data.h \
    time_vis.h \
    otf2_trace_data.h \
    otf2_tracemodel.h \
    message_model.h

RESOURCES += vis3.qrc
OTHER_FILES += 
