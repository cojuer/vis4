#ifndef OTF_MAIN_WINDOW_H
#define OTF_MAIN_WINDOW_H

#include "tools/tool.h"
#include "main_window.h"

namespace vis4 {

class OTF_main_window : public MainWindow
{
public:
    OTF_main_window();
private:
    void xinitialize(QWidget* toolContainer, Canvas* canvas);
};

}

#endif // OTF_MAIN_WINDOW_H
