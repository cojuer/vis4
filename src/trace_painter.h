#ifndef TRACE_PAINTER_H
#define TRACE_PAINTER_H

#include "time_vis.h"

#include <QPainter>
#include <QMap>

#include <memory>

using std::shared_ptr;

namespace vis4 {

class TraceModel;
class TraceGeometry;
class StateModel;

/** Trace painter.
    Class encapsulates all common methods for drawing on screen
    and printing on printer.

    For screen case class has some relations with class
    Contents_widget. All common variables now placing in
    Trace_geometry object.

    For printer case method drawTrace make it possible to
    print not only visible, but any number of pages.
*/
class TracePainter {

public: /* types */
    enum StateEnum { Ready, Active, Background, Canceled };

public: /* methods */
    TracePainter();
    ~TracePainter();

    void setModel(std::shared_ptr<TraceModel> & model);
    void setPaintDevice(QPaintDevice* paintDevice);
    void setState(StateEnum state);

    void drawTrace(const Time& timePerPage, bool start_in_background);
    int getState();


    /** Draws text in a nice frame.
        Uses current pen and brush for the frame, and black for text.

       @param text          Text for draw.
       @param painter       Painter for drawing.
       @param x             Absciss of top left corner of the frame.
       @param y             Ordinate of vertical middle of the frame.
       @param width         Width of the frame. If text does not fit,
                            it's truncated. Value may be -1, then width
                            will be auto-computed.
       @param height        Height of the frame.
       @param text_start_x  If the value is not -1, text is drawing starting
                            on that x position

       This function is public because one used not only in class Trace_painter.
    */
    static QRect drawTextBox(const QString& text,
                      QPainter* painterPtr,
                      int x, int y, int width, int height,
                      int text_start_x = -1);

    /** Draws a time line using given painter and coordinates.

       @param painter       Painter for drawing.
       @param x             Absciss of the top left corner of the time line.
       @param y             Ordinate of the top left corner of the time line.
    */
    void drawTimeline(QPainter* painterPtr, int x, int y);

    /** Calculates x coordinate corresponding to given time on timeline. */
    int pixelPositionForTime(const Time& time) const;

    /** Calculates Time corresponding to given pixel coordinate. */
    Time timeForPixel(int pixel_x) const;

    std::auto_ptr<TraceGeometry> traceGeometry() const;//to shared

private: /* methods */

    /** Group of methods that really draw parts of trace diagram. */
    void drawComponentsList(int from_component, int to_component, bool drawLabels);
    void drawEvents(int from_component, int to_component);
    void drawStates(int from_component, int to_component);
    void drawGroups(int from_component, int to_component);

    /** Calculates the number of pages, that must be printed. */
    void splitToPages();//? void func calculating some number seems strange

    /** Draw(or print) the page with given number.
        @param i Page number by horizontal.
        @param j Page number by vertical.
    */
    void drawPage(int i, int j);

    /** Draws an arrow from (x1, y1) to (x2, y2) on 'painter'.
       The primary issue is that often, there are several arrows
       with the same start position, and zero delta_y. If we draw
       them as straight lines, they will overlap, and look ugly.
       We can't avoid overlap in general, but this case is very
       comment and better be handled.

       The approach is to make arrows with zero delta y curved.
       When delta y is large enough (or, more specifically, the
       angle to horisontal is small enough, arrow will be drawn as
       straigh line.
       We compute the angle of straight line between start and end
       to horisontal, and the compute "delta angle" -- angle to that
       straight line that will have start and end segments of the curve.
       For angle of 90 (delta y is zero), delta angle is 30.
       For a "limit angle" of 10, delta angle is 0 (the line is straight)
       For intermediate angles, we interporal.
    */
    void draw_unified_arrow(int x1, int y1, int x2, int y2, QPainter* painterPtr,
                            bool always_straight = false,
                            bool start_arrowhead = false);

    /** Draw a marker with a page number.
       @param painter   Painter for drawing.
       @param page      Page number.
       @param count     Count of the pages.
       @param x         Marker X coordinate.
       @param y         Marker Y coordinate.
       @param horiz     Marker orientation (true - horizontal, false - vertical).
    */
    QRect drawPageMarker(QPainter* painterPtr,
                        int page, int count,
                        int x, int y, bool horiz);

public: /** members */

    /** These values used for text drawing. */
    unsigned int text_elements_height;
    unsigned int text_height;
    unsigned int text_letter_width;

    /** Width of left and right margins at the current page.*/
    int right_margin, left_margin;

    /** Number of pages for printing (sets by splitToPages() function). */
    int pages_horizontally, pages_vertically;

    std::vector<unsigned int> lifeline_position;
    unsigned int lifeline_stepping;

    /** The height of timeline's markers */
    static const int timeline_text_top = 13;

private: /** members */

    shared_ptr<TraceModel> model;      ///< Model for drawing.
    QPainter* painter;                 ///< Active painter.
    TraceGeometry* tg;                ///< Trace geometry (coords of component labels,
                                        ///< lifelines, states, events etc.

    bool printer_flag;          ///< Indicates paint device is QPrinter or not.

    int left_margin1;           ///< Width of margin at first page.
    int left_margin2;           ///< Width of margin at other pages.

    Time timePerFirstPage;
    Time timePerFullPage;
    Time timePerPage;                       ///< Trace scalling.

    uint components_per_page;

    int width, height;                      ///< Full paper (or screen widget) size, including margins.
    uint y_unparented;                      ///< Top margin for the components (however a parent label is placed above)
    uint timeline_height;

    /* How much even line stands out on top of states.  */
    static const int event_line_extra_height = 5;

    /* The difference in vertical coordinate of
       event line end point and the baseline of the
       letter drawn above it.  */
    static const int event_line_and_letter_spacing = 2;

    StateEnum state_;

    QMap<int, QColor> componentLabelColors;

};

/**
 * Class holds all methods and members to manipulate
 * layouts of trace parts. Members of this class are set by
 * TracePainter class. And it's used by Content_widget class
 * for handling mouse events.
 */
class TraceGeometry
{
public:
    /**
     * Returns the pointer to the number of clickable component
     * if point is withing the clickable area, and null otherwise.
     */
    bool clickable_component(const QPoint& point, int& component) const;
    StateModel* clickable_state(const QPoint& point) const;
    int componentAtPosition(const QPoint& point);
    /**
     * Function identify component's label, which rectangle contains point.
     * @param point Interesting position.
     * @return Index of component in model_->component_names array,
     *         -1 when actual component is root, and -2 when no actual
     *         components around.
     *
     * Function used by tooltips mechanism
     */
    int componentLabelAtPos(const QPoint& p);
public:
    QVector<QVector<bool>> eventsNear;
private:
    /** Used by tooltips mechanism. */
    QVector<QPair<QRect,int>> componentlabel_rects;
    QVector<QPair<QRect,int>> lifeline_rects;
    QVector<QPair<QRect,int>> clickable_components;
    // FIXME: it might be bad to store all states here,
    // and better solution would be to re-get them from trace
    // on click.
    QVector<QPair<QRect, shared_ptr<StateModel>>> states;

    friend class TracePainter;
};

}

#endif
