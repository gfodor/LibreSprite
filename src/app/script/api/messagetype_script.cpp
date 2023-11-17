#include "ui/message_type.h"
#include "script/engine.h"

using namespace ui;

class MessageTypeScriptObject : public script::ScriptObject {
public:
    MessageTypeScriptObject() {
        addProperty("OPEN", []{ return MessageType::kOpenMessage; });
        addProperty("CLOSE", []{ return MessageType::kCloseMessage; });
        addProperty("CLOSE_DISPLAY", []{ return MessageType::kCloseDisplayMessage; });
        addProperty("RESIZE_DISPLAY", []{ return MessageType::kResizeDisplayMessage; });
        addProperty("PAINT", []{ return MessageType::kPaintMessage; });
        addProperty("TIMER", []{ return MessageType::kTimerMessage; });
        addProperty("DROP_FILES", []{ return MessageType::kDropFilesMessage; });
        addProperty("WIN_MOVE", []{ return MessageType::kWinMoveMessage; });
        addProperty("KEY_DOWN", []{ return MessageType::kKeyDownMessage; });
        addProperty("KEY_UP", []{ return MessageType::kKeyUpMessage; });
        addProperty("FOCUS_ENTER", []{ return MessageType::kFocusEnterMessage; });
        addProperty("FOCUS_LEAVE", []{ return MessageType::kFocusLeaveMessage; });
        addProperty("MOUSE_DOWN", []{ return MessageType::kMouseDownMessage; });
        addProperty("MOUSE_UP", []{ return MessageType::kMouseUpMessage; });
        addProperty("DOUBLE_CLICK", []{ return MessageType::kDoubleClickMessage; });
        addProperty("MOUSE_ENTER", []{ return MessageType::kMouseEnterMessage; });
        addProperty("MOUSE_LEAVE", []{ return MessageType::kMouseLeaveMessage; });
        addProperty("MOUSE_MOVE", []{ return MessageType::kMouseMoveMessage; });
        addProperty("SET_CURSOR", []{ return MessageType::kSetCursorMessage; });
        addProperty("MOUSE_WHEEL", []{ return MessageType::kMouseWheelMessage; });
        addProperty("TOUCH_MAGNIFY", []{ return MessageType::kTouchMagnifyMessage; });
        addProperty("FIRST_REGISTERED", []{ return MessageType::kFirstRegisteredMessage; });
        addProperty("LAST_REGISTERED", []{ return MessageType::kLastRegisteredMessage; });
        makeGlobal("MessageType");
    }
};

static script::ScriptObject::Regular<MessageTypeScriptObject> reg("MessageTypeScriptObject", {"global"});
