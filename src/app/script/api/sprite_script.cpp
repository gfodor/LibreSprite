#include "sprite_script.h"
#include "app/commands/commands.h"
#include "app/script/api/layer_script.h"
#include "app/modules/gui.h"
#include <iostream>

using namespace app;

SpriteScriptObject::SpriteScriptObject() {
    if (m_document) {
      m_sprite = doc()->sprite();
      if (m_pal)
        m_pal->setWrapped(m_sprite->palette(app::frame_t(0)));
    }

    addProperty("layerCount", [this]{return (int) m_sprite->countLayers();})
      .doc("read-only. Returns the amount of layers in the sprite.");

    addProperty("filename", [this]{return doc()->filename();})
      .doc("read-only. Returns the file name of the sprite.");

    addProperty("width",
                [this]{return m_sprite->width();},
                [this](int width){
                  transaction().execute(new app::cmd::SetSpriteSize(m_sprite, width, m_sprite->height()));
                  return 0;
                })
      .doc("read+write. Returns and sets the width of the sprite.");

    addProperty("height",
                [this]{return m_sprite->height();},
                [this](int height){
                  transaction().execute(new app::cmd::SetSpriteSize(m_sprite, m_sprite->width(), height));
                  return 0;
                })
      .doc("read+write. Returns and sets the height of the sprite.");

    addProperty("colorMode", [this]{ return m_sprite->pixelFormat();})
      .doc("read-only. Returns the sprite's ColorMode.");

    addProperty("selection", [this]{ return this; })
      .doc("placeholder. Do not use.");

    addProperty("palette", [this]{ return m_pal.get(); });

    addMethod("layer", &SpriteScriptObject::layer)
      .doc("allows you to access a given layer.")
      .docArg("layerIndex", "The index of they layer, starting with zero from the bottom.")
      .docReturns("a Layer object or null if invalid.");

    addMethod("newLayer", &SpriteScriptObject::newLayer)
      .doc("creates a new layer.");

    addMethod("removeLayer", &SpriteScriptObject::removeLayer)
      .doc("removes a layer.")
      .docArg("layer", "The layer object");

    addMethod("newFrame", &SpriteScriptObject::newFrame)
      .doc("creates a new frame.");

    addMethod("removeFrameAtIndex", &SpriteScriptObject::removeFrameAtIndex)
      .doc("removes a frame.")
      .docArg("frameIndex", "The frame index");

    addMethod("commit", &SpriteScriptObject::commit)
      .doc("commits the current transaction.");

    addMethod("resize", &SpriteScriptObject::resize)
      .doc("resizes the sprite.")
      .docArg("width", "The new width.")
      .docArg("height", "The new height.");

    addMethod("crop", &SpriteScriptObject::crop)
      .doc("crops the sprite to the specified dimensions.")
      .docArg("x", "The left-most edge of the crop.")
      .docArg("y", "The top-most edge of the crop.")
      .docArg("width", "The width of the cropped area.")
      .docArg("height", "The height of the cropped area.");

    addMethod("save", &SpriteScriptObject::save)
      .doc("saves the sprite.");

    addMethod("saveAs", &SpriteScriptObject::saveAs)
      .doc("saves the sprite.")
      .docArg("fileName", "String. The new name of the file")
      .docArg("asCopy", "If true, the file is saved as a copy. Requires fileName to be specified.");

    addMethod("loadPalette", &SpriteScriptObject::loadPalette)
      .doc("loads a palette file.")
      .docArg("fileName", "The name of the palette file to load");
}

SpriteScriptObject::~SpriteScriptObject() {
    commit();
}

void* SpriteScriptObject::getWrapped() {
  return m_sprite;
}

void SpriteScriptObject::commit() {
    for (auto& entry : m_layers) {
      entry.second->call("commit");
    }
    if (m_transaction) {
      m_transaction->commit();
      m_transaction.reset();
    }
}

script::ScriptObject* SpriteScriptObject::layer(int i) {
    auto layer = m_sprite->indexToLayer(doc::LayerIndex(i));
    if (!layer)
      return nullptr;
    auto it = m_layers.find(layer);
    if (it == m_layers.end()) {
      it = m_layers.emplace(layer, "LayerScriptObject").first;
      it->second->setWrapped(layer);
    }
    return it->second.get();
}

script::ScriptObject* SpriteScriptObject::newLayer() {
    app::DocumentApi api(doc(), transaction());
    auto layer = api.newLayer(m_sprite, "");
    commit();

    update_screen_for_document(doc());

    auto it = m_layers.find(layer);
    if (it == m_layers.end()) {
      it = m_layers.emplace(layer, "LayerScriptObject").first;
      it->second->setWrapped(layer);
    }
    return it->second.get();
}

void SpriteScriptObject::removeLayer(script::ScriptObject* layerObj) {
    app::DocumentApi api(doc(), transaction());
    auto layer = (Layer *)static_cast<LayerScriptObject*>(layerObj)->getWrapped();
    api.removeLayer(layer);

    update_screen_for_document(doc());

    commit();
}

void SpriteScriptObject::newFrame() {
    app::DocumentApi api(doc(), transaction());
    api.addFrame(m_sprite, m_sprite->lastFrame() + 1);
    commit();

    update_screen_for_document(doc());
}

void SpriteScriptObject::removeFrameAtIndex(int frameIndex) {
    app::DocumentApi api(doc(), transaction());
    api.removeFrame(m_sprite, app::frame_t(frameIndex));
    commit();

    update_screen_for_document(doc());
}

void SpriteScriptObject::resize(int w, int h) {
    app::DocumentApi api(doc(), transaction());
    api.setSpriteSize(m_sprite, w, h);

    update_screen_for_document(doc());
}

void SpriteScriptObject::crop(script::Value x, script::Value y, script::Value w, script::Value h) {
    gfx::Rect bounds;
    commit();

    if (doc()->isMaskVisible())
      bounds = doc()->mask()->bounds();
    else
      bounds = m_sprite->bounds();

    if (x.type != script::Value::Type::UNDEFINED) bounds.x = x;
    if (y.type != script::Value::Type::UNDEFINED) bounds.y = y;
    if (w.type != script::Value::Type::UNDEFINED) bounds.w = w;
    if (h.type != script::Value::Type::UNDEFINED) bounds.h = h;

    if (!bounds.isEmpty()) {
      app::DocumentApi{doc(), transaction()}.cropSprite(m_sprite, bounds);
    }

    update_screen_for_document(doc());
}

void SpriteScriptObject::save() {
    commit();
    auto uiCtx = app::UIContext::instance();
    uiCtx->setActiveDocument(doc());
    auto saveCommand = app::CommandsModule::instance()->getCommandByName(app::CommandId::SaveFile);
    uiCtx->executeCommand(saveCommand);
}

void SpriteScriptObject::saveAs(const std::string& fileName, bool asCopy) {
    commit();
    if (fileName.empty()) asCopy = false;
    auto uiCtx = app::UIContext::instance();
    uiCtx->setActiveDocument(doc());
    auto commandName = asCopy ? app::CommandId::SaveFileCopyAs : app::CommandId::SaveFile;
    auto saveCommand = app::CommandsModule::instance()->getCommandByName(commandName);
    app::Params params;
    if (asCopy) params.set("filename", fileName.c_str());
    else if(!fileName.empty()) doc()->setFilename(fileName);
    uiCtx->executeCommand(saveCommand, params);
}

void SpriteScriptObject::loadPalette(const std::string& fileName) {
    std::unique_ptr<doc::Palette> palette(app::load_palette(fileName.c_str()));
    if (palette) {
      // TODO Merge this with the code in LoadPaletteCommand
      doc()->getApi(transaction()).setPalette(m_sprite, 0, palette.get());
    }

    update_screen_for_document(doc());
}

app::Document* SpriteScriptObject::doc() {
    return m_document->getWrapped<app::Document>();
}

app::Transaction& SpriteScriptObject::transaction() {
    if (!m_transaction) {
      m_transaction.reset(new app::Transaction(app::UIContext::instance(),
                                               "Script Execution",
                                               app::ModifyDocument));
    }
    return *m_transaction;
}

