
#ifndef TRACE_MODEL_HPP_VP_2006_03_20
#define TRACE_MODEL_HPP_VP_2006_03_20

#include "time_vis3.h"
#include "selection.h"

#include <boost/shared_ptr.hpp>

#include <memory>
#include <vector>

class Trace;

namespace vis4 {

class Event_model;
class State_model;
class Group_model;
class Checker;

/** ���������� ������������� ������ ��� �������������.

    ���� ����������� ����� ��������� ��, ��� ������ ���� �������� �� ������ (����� �����,
    �������, ���������), � ������������� ���������� ��� ��������� �� �������� ������������.
    ����� ����������� ������������ ������ ������������ ��������������� ��������, ������
    ����� �� ������������ ������ �����������.

    ��������� ���� ������ � ������ ������ Vis4 �������� ����� ���� ���������, ������� ��� ��������������
    ������������.

    ��� ������-������� ������ ���� "�������", ����� ������������ ������ �� ������ �����������
    ������ ���� �����������, � �����������/��������� ������ ���� �������� ����������. ��� ��������,
    ��� ��� ������������� ������� �������� ������ ��� ������������� ������ ���������� �������-��������
    ����� ��������� �� ���������� � ������, ������� ������ ���� ������ ��������� �������/�������.

    ��������������� ���������� �� ���� ����������� �������� ������. ��� ������ ��������� ������� �����
    ���������. ���, � ��������� � ����������� "��������" ��������, ��������� ��������� �������, �����
    ����� � ��� ����� ���� ������ ���������. ��� �������, ��������, ��� �������� "back".

    ��������� ����������� �� ���� ����.

    ������ ��� -- ��������� � ��������� ���������. ��������������, ��� ���� �������� ����� �������
    � ��������� �����������. ��� ������ ������� ���� ����� ������������ ������� ����� �����, �������
    � ���������. ����� ����� ������� ������������ ���������� ����������� ���������� ����������
    ���������� Trace_model.

    ����� ��������� �������� �������� ���� �����
    - ������� "������"
    - "���������" ����� �� ����� ����� � ������ � ����
    ��� ������� ��������� ����������� ���������� ���� ��������� ���������� ���������� ����������
    ���������� Trace_model.

    ������ ��� ��������� -- ������� ����� �����, ������� � ���������. ���� ��������� � ���������
    ��������� ���������� ������������� ����� ���� ��������, �� �������������� ���������� ����������
    ������� ������� �����.

    ������ ��� ��������� -- ��� ����������� � ������������ ������������ �����. ���� ���� ������ �
    ������������ ������� -- ����� � ������ ������� ��������� ���������. �������, ������������ Trace_model,
    ����� �������� �� ���� ��������, ��� �������� ��������� ��� "�����������" ���������
    (��������� � ��������� �������). ����� �������, ��� ��� ������� �������� �� ����������� ���������,
    ����� ���� ��������� ���������� ��������� ��������� �������.

    ��������� �� ���� ���� ���� ����������� � ������� ������� ������� ������ Trace_model, �������
    ������� ����� ������ ����� Trace_model � ����� �������������� ������.
*/
class Trace_model
{
public:

    typedef boost::shared_ptr<Trace_model> Ptr;

    enum ComponentType { RCHM = 1, CHM, CHANNEL, INTERFACE, EXTERNAL_OBJECTS };

    /** ���������� ������ Trace_model ��������������� "����������" ���������
    � ��������� ����������, ������������� ���������� ��������� ������ �
    �� ������� ������� ��������. */
    virtual Trace_model::Ptr root() = 0;

/** @defgroup hierarchy Methods for managing components hierarchy */
/// @{

    /** Returns the link of the parent component.
        Link may be used to refer to parent component in components selection. */
    virtual int parent_component() const = 0;

    /** Returns Trace_model with given component as current parent. */
    virtual Trace_model::Ptr set_parent_component(int component) = 0;

    /** ���������� true ���� �������� component ����� "�����" � � ���� ����� ������.
        @sa set_parent_component. */
    virtual bool has_children(int component) const = 0;

/// @}

/** @defgroup time Methods for managing model time. */
/// @{

    /** ���������� ����������� �����, ������� ������ ������������ �� ��������� ���������. */
    virtual common::Time min_time() const = 0;

    /** ���������� ������������ �����, ������� ������ ������������ �� ��������� ���������. */
    virtual common::Time max_time() const = 0;

    /** ���������� ����������� �������� �������, ������� ����� ���������� ��. */
    virtual common::Time min_resolution() const = 0;

    /** ��������� ���������� ��������� �������, ��������� � ��������� �������
       �� ����������� �����. */
    virtual void rewind() = 0;

    /** ���������� ����� ������ Trace_model � ��������� ���������� ������. */
    virtual Trace_model::Ptr set_range(const common::Time& min, const common::Time& max) = 0;

/// @}

/** @defgroup data Methods for obtaining trace data. */
/// @{

    /** ���������� ��������� ������� � ������ � ����������� ���������� ���������
        �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<Event_model> next_event() = 0;

    /** ���������� ��������� ��������� � ������ � ����������� ���������� ���������
    �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<State_model> next_state() = 0;

    /** ���������� ��������� ��������� � ������ � ����������� ���������� ���������
    �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<Group_model> next_group() = 0;

/// @}

/** @defgroup filters Methods for managing filters. */
/// @{

    /** Returns new object with given selection of components. */
    virtual Trace_model::Ptr filter_components(
        const common::Selection & filter) = 0;

    /** Returns current component selection. */
    virtual const common::Selection & components() const = 0;

    /** Returns the list with links of visible components.
        Visibility means that component is not filtered and
        it's a child of current parent. */
    virtual const QList<int> & visible_components() const = 0;

    /** Returns a lifeline number for given component.
        Returns -1 if there is no corresponding lifeline. */
    virtual int lifeline(int component) const = 0;

    /** Returns a name for given component.
        If "fullname" is true returns full component name,
        which includes all its parents. */
    virtual QString component_name(int component, bool fullname = false) const = 0;
    virtual int component_type(int component) const = 0;

    /** Returns new object with given selection of event. */
    virtual Trace_model::Ptr filter_events(
        const common::Selection & filter) = 0;

    virtual bool groupsEnabled() const = 0;
    virtual Trace_model::Ptr setGroupsEnabled(bool enabled) = 0;

    /** Returns current event selection. */
    virtual const common::Selection & events() const = 0;

    /** Returns new object with given selection of states. */
    virtual Trace_model::Ptr filter_states(
        const common::Selection & filter) = 0;

    /** Returns current state selection. */
    virtual const common::Selection & states() const = 0;

    /** Returns selection with only available states enabled.
        State is available when though one of corresponding
        components are enabled. */
    virtual const common::Selection & available_states() const = 0;

    /** Return new model with given checker installed into. */
    virtual Trace_model::Ptr install_checker(Checker * checker) = 0;

/// @}

    /** ��������� ������� ��������� � ������ � ���������� ���. ��������� ������ ��
       ����������. */
    virtual QString save() const = 0;

    /** �� ������, ����� ������������ �������� save, �������� ������������
       ���������. ������� �� ����������� ���-�� ������, ���� �������� ������ ������
       ������� ���������� �� ������ ������ � ������ ������ save, ������� ����� ������
       �� ������.

       �������� �������������� -- ��� �������������� ��������� ������������ ����������
       ��� ����������� ��������. */
    virtual void restore(const QString& s) = 0;

    virtual ~Trace_model() {}

};


struct Trace_model_delta
{
    enum {
        component_position = 1,
        components = component_position << 1,
        event_types = components << 1,
        state_types = event_types << 1,
        time_range = state_types << 1
    };
};

/* This is experimental trace comparison interface.
   It is not yet clear if we want to be able to compare
   "inner" state of trace models. We also don't
   compare time unit. This interface may change any
   second. */
int delta(const Trace_model& a, const Trace_model& b);

}
#endif
