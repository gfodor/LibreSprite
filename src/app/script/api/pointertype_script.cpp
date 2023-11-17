#include "she/pointer_type.h"
#include "script/engine.h"

using namespace she;

class PointerTypeScriptObject : public script::ScriptObject {
public:
  PointerTypeScriptObject() {
    addProperty("UNKNOWN", []{ return PointerType::Unknown; });
    addProperty("MOUSE", []{ return PointerType::Mouse; });
    addProperty("PEN", []{ return PointerType::Pen; });
    addProperty("ERASER", []{ return PointerType::Eraser; });
    addProperty("MULTITOUCH", []{ return PointerType::Multitouch; });
    addProperty("CURSOR", []{ return PointerType::Cursor; });
    makeGlobal("PointerType");
  }
};

static script::ScriptObject::Regular<PointerTypeScriptObject> reg("PointerTypeScriptObject", {"global"});
