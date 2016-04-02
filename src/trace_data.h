#ifndef TRACE_DATA_H
#define TRACE_DATA_H

class TraceData
{
public:
    TraceData(const QString& filename);
    ~TraceData();

    /** Returns parent location number. */
    int getParentLocation() const;

    /** Returns numbers of visible locations in locations selection. */
    const QList<int>& getVisibleLocations() const;

    /** Returns number of lifeline adjusted to location number. */
    int getLifeline(int location) const;

    ComponentType getComponentType(int component) const;//?

    /** Returns location name. Full name contains all parents of the location. */
    QString getLocationName(int location, bool full = false) const;

    bool hasChildren(int location) const;

    Time getMinTime() const;
    Time getMaxTime() const;

    Time getMinResolution();//?

    void rewind() override;//?

    StateModel* getNextState();
    GroupModel* getNextGroup();
    EventModel* getNextEvent();

    TraceModelPtr root();//?
    TraceModelPtr set_parent_component(int component);//?
    TraceModelPtr set_range(const Time& min, const Time& max);//?

    const Selection& components() const;//?
    TraceModelPtr filter_components(const Selection & filter);//?

    const Selection& events() const;
    const Selection& states() const;
    const Selection& available_states() const;

    TraceModelPtr filter_states(const Selection& filter);//?
    TraceModelPtr filter_events(const Selection& filter);//?

    QString save() const override;//?
    bool groupsEnabled() const;//?
    TraceModelPtr setGroupsEnabled(bool enabled);//?
    void restore(const QString& s);//?
private:
    Selection* locationsPtr;
    Selection* statesPtr;
    Time start, end;
    QVector<StateModel*> states;
    QVector<EventModel*> events;
};

#endif // TRACE_DATA

