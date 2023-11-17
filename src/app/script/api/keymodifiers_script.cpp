#include "she/keys.h"
#include "script/engine.h"

using namespace she;

class KeyModifiersScriptObject : public script::ScriptObject {
public:
    KeyModifiersScriptObject() {
        addProperty("NONE", []{ return KeyModifiers::kKeyNoneModifier; });
        addProperty("SHIFT", []{ return KeyModifiers::kKeyShiftModifier; });
        addProperty("CTRL", []{ return KeyModifiers::kKeyCtrlModifier; });
        addProperty("ALT", []{ return KeyModifiers::kKeyAltModifier; });
        addProperty("CMD", []{ return KeyModifiers::kKeyCmdModifier; });
        addProperty("SPACE", []{ return KeyModifiers::kKeySpaceModifier; });
        addProperty("WIN", []{ return KeyModifiers::kKeyWinModifier; });
        addProperty("UNINITIALIZED", []{ return KeyModifiers::kKeyUninitializedModifier; });
        makeGlobal("KeyModifiers");
    }
};

static script::ScriptObject::Regular<KeyModifiersScriptObject> reg("KeyModifiersScriptObject", {"global"});
