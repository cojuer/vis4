#include "otf_main_window.h"

namespace vis4 {

OTF_main_window::OTF_main_window()
{
    setWindowTitle("Vis4");
}

void OTF_main_window::xinitialize(QWidget* toolContainer, Canvas* canvas)//? don't like this xinitialize things
{
    Browser* browser = createBrowser(toolContainer, canvas);
    installBrowser(browser);
    /*
    installTool(createGoto(toolContainer, canvas));
    installTool(createMeasure(toolContainer, canvas));
    installTool(createFilter(toolContainer, canvas));
    Tool* find = createFind(toolContainer, canvas);
    installTool(find);
    connect(find, SIGNAL(extraHelp(const QString&)), browser,
                  SLOT(extraHelp(const QString&)));
    */
}

}
