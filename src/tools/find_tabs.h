#ifndef FIND_TABS_H
#define FIND_TABS_H

#include <QtWidgets/QWidget>
#include <QSettings>

#include <boost/shared_ptr.hpp>

#include "tool.h"
#include "selection_widget.h"

class QComboBox;
class QStackedLayout;

namespace vis4 {

class Trace_model;
class EventModel;
class State_model;

/** Base class for find tabs. */
class FindTab : public QWidget {
    Q_OBJECT
public: /** methods */

    FindTab(Tool* find_tool) :
        QWidget(find_tool),
        find_tool_(find_tool)
    {}

    virtual QString name() = 0;
    virtual void reset() = 0;
    virtual bool findNext() = 0;
    virtual void setModel(TraceModelPtr &) = 0;
    virtual bool isSearchAllowed() = 0;
    virtual void saveState(QSettings & settings) = 0;
    virtual void restoreState(QSettings & settings) = 0;

signals:
    void stateChanged();
protected:
    Tool* find_tool_;
};

/** Class for events find tab. */
class FindEventsTab : public FindTab {
    Q_OBJECT
public: /** methods */

    FindEventsTab(Tool* find_tool);

    QString name()
    {
        return tr("Events");
    }

    void reset();
    bool findNext();
    void setModel(TraceModelPtr& model);
    bool isSearchAllowed();
    void saveState(QSettings& settings);
    void restoreState(QSettings& settings);
signals:
    void showEvent(EventModel*);
private slots:
    void selectionChanged(const vis4::Selection& selection);
private: /** widgets */
    SelectionWidget* selector_;
    TraceModelPtr model_;
    TraceModelPtr filtered_model_;
};

/** Class for states find tab. */
class FindStatesTab : public FindTab {
    Q_OBJECT
public: /* methods */
    FindStatesTab(Tool* find_tool);

    QString name()
    {
        return tr("States");
    }

    void reset();
    bool findNext();
    void setModel(TraceModelPtr& model);
    bool isSearchAllowed();
    void saveState(QSettings& settings);
    void restoreState(QSettings& settings);
signals:
    void showState(State_model*);
private slots:
    void selectionChanged(const vis4::Selection & selection);
private: /** widgets */
    SelectionWidget* selector_;
    TraceModelPtr model_;
    TraceModelPtr filtered_model_;
};

/** Class for checkers find tab. */
class FindQueryTab : public FindTab {
    Q_OBJECT
public: /** methods */
    FindQueryTab(Tool* find_tool);

    QString name()
    {
        return tr("Query");
    }

    void reset();
    bool findNext();
    void setModel(TraceModelPtr & model);
    bool isSearchAllowed();
    void saveState(QSettings & settings);
    void restoreState(QSettings & settings);
private: /** methods */
    /** Creates checkers and ininialize event/subevent lists. */
    void initializeCheckers();
signals:
    void showEvent(EventModel*);
private slots:
    void updateChecker();
    void activateCheckerEvents();
    void checkerStateChanged();
private: /** widgets */
    TraceModelPtr model_;
    TraceModelPtr model_with_checker;

    //QList<pChecker> checkers;
    //Checker* active_checker;
    //bool active_checker_is_ready;

    QComboBox* checkerCombo;
    QStackedLayout* checkerWidgetContainer;
    QLayout* checkerSettings_layout;

};

}
#endif
