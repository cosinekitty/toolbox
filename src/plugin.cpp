#include "plugin.hpp"

// Toolbox for VCV Rack 2, by Don Cross <cosinekitty@gmail.com>
// https://github.com/cosinekitty/toolbox


Plugin* pluginInstance;


void init(Plugin* p)
{
    pluginInstance = p;

    p->addModel(modelToolboxModuleScanner);
}
