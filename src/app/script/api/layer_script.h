// LayerScriptObject.h
#pragma once

#include "script/script_object.h"
#include "doc/layer.h"
#include <unordered_map>
#include <string>

class LayerScriptObject : public script::ScriptObject {
public:
    LayerScriptObject();
    void* getWrapped() override;
    void setWrapped(void* layer) override;
    ScriptObject* cel(int i);

private:
    doc::Layer* m_layer;
    std::unordered_map<doc::Cel*, inject<ScriptObject>> m_cels;
};

static script::ScriptObject::Regular<LayerScriptObject> layerSO("LayerScriptObject");

