#include "canvas_item.h"

#include <QtWidgets/QWidget>

namespace vis4 {

CanvasItem::CanvasItem() {}

void CanvasItem::draw(QPainter& painter)
{
    currentRect = xdraw(painter);
}

void CanvasItem::new_geometry(const QRect& rect)
{
    static_cast<QWidget*>(parent())->update(rect | currentRect);
}

}
