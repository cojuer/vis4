#ifndef EVENT_MODEL_H
#define EVENT_MODEL_H

#include "time_vis.h"
#include <QString>

class QWidget;

namespace vis4 {

/**
 * Event description for visualization purposes.
 */
class EventModel
{
public:
    /** Event appearance time */
    Time time;

    /** Event type */
    QString kind;

    /**
     * Буква, используемая для показа события. Визуализатором всегда
     * показывается как заглавная.
     */
    char letter;

    /**
     * Дополнительная буква, используемая для показа события. Визуализатором всегда
     * показывается как строчная. Рисуется справа от основной буквы меньшим шрифтом.
     * Если равна нулю, не показывается.
     */
    char subletter;

    /** Позиция буквы относительно "засечки" события. */
    enum letter_position_t
    {
        left_top = 1,
        right_top,
        right_bottom,
        left_bottom
    } letter_position;

    /** Number of component, where event appeared */
    int component;

    /**
     * Event priority, used to decide which letter to draw
     * if letters from different event overlap.
     */
    unsigned priority;

    /** Returns short description of event. */
    virtual QString shortDescription() const
    {
        return kind;
    }

    EventModel() {}

    EventModel(Time time, int component, QString kind, char letter) :
        time(time),
        component(component),
        kind(kind),
        letter(letter)
    {}

    virtual ~EventModel() {}

    virtual bool operator==(const EventModel & other) const
    {
        // Very stupid implementation for vis_xml and vis_fake
        bool equal = true;
        equal &= (time == other.time);
        equal &= (kind == other.kind);
        equal &= (letter == other.letter);
        equal &= (subletter == other.subletter);
        equal &= (component == other.component);
        return equal;
    }
};

}
#endif
