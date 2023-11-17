#include "ui/mouse_buttons.h"
#include "script/engine.h"

using namespace ui;

class MouseButtonsScriptObject : public script::ScriptObject {
public:
    MouseButtonsScriptObject() {
        addProperty("NONE", []{ return MouseButtons::kButtonNone; });
        addProperty("LEFT", []{ return MouseButtons::kButtonLeft; });
        addProperty("RIGHT", []{ return MouseButtons::kButtonRight; });
        addProperty("MIDDLE", []{ return MouseButtons::kButtonMiddle; });
        makeGlobal("MouseButtons");
    }
};

static script::ScriptObject::Regular<MouseButtonsScriptObject> reg("MouseButtonsScriptObject", {"global"});
