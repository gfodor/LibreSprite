// SpriteScriptObject.h
#pragma once

#include "app/cmd/set_sprite_size.h"
#include "app/commands/commands.h"
#include "app/document.h"
#include "app/document_api.h"
#include "app/file/palette_file.h"
#include "app/transaction.h"
#include "app/ui_context.h"
#include "doc/document_observer.h"
#include "doc/mask.h"
#include "doc/palette.h"
#include "script/script_object.h"

#include <memory>
#include <unordered_map>
#include <string>

class SpriteScriptObject : public script::ScriptObject {
public:
    SpriteScriptObject();
    ~SpriteScriptObject();

    void* getWrapped() override;

    void commit();
    script::ScriptObject* layer(int i);
    void resize(int w, int h);
    void crop(script::Value x, script::Value y, script::Value w, script::Value h);
    void save();
    void saveAs(const std::string& fileName, bool asCopy);
    void loadPalette(const std::string& fileName);
    ScriptObject* newLayer();
    void removeLayer(ScriptObject* layer);

private:
    app::Document* doc();
    app::Transaction& transaction();

    Provides provides{this, "activeSprite"};
    inject<ScriptObject> m_document{"activeDocument"};
    inject<ScriptObject> m_pal{"PaletteScriptObject"};
    doc::Sprite* m_sprite;
    std::unordered_map<doc::Layer*, inject<ScriptObject>> m_layers;
    std::unique_ptr<app::Transaction> m_transaction;
};

static script::ScriptObject::Regular<SpriteScriptObject> spriteSO("SpriteScriptObject");
