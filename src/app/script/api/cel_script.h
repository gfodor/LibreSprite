#pragma once

#include "doc/cel.h"
#include "doc/image.h"
#include "doc/sprite.h"
#include "script/script_object.h"
#include <memory>

class CelScriptObject : public script::ScriptObject {
public:
    CelScriptObject();
    void setPosition(int x, int y);
    void* getWrapped() override;
    void setWrapped(void* cel) override;

private:
    inject<ScriptObject> m_image{"ImageScriptObject"};
    doc::Cel* m_cel;
};

static script::ScriptObject::Regular<CelScriptObject> celSO("CelScriptObject");

