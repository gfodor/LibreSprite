// Aseprite
// Copyright (C) 2015-2016  David Capello
// Copyright (C) 2021 LibreSprite contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/document.h"
#include "app/document_api.h"
#include "app/commands/commands.h"
#include "app/commands/params.h"
#include "app/script/api/document_script.h"
#include "app/script/api/sprite_script.h"
#include "app/script/api/layer_script.h"
#include "app/script/api/cel_script.h"
#include "app/tools/tool_box.h"
#include "app/ui_context.h"
#include "app/ui/document_view.h"
#include "doc/site.h"
#include "app/app.h"
#include "app/tools/active_tool.h"
#include "app/tools/tool.h"

#include "script/engine.h"
#include "script/engine_delegate.h"
#include "script/script_object.h"

#include <sstream>


class DudScriptObject : public script::InternalScriptObject {
public:
  void makeGlobal(const std::string& name) override {
    globalName = name;
  }
  std::string globalName;
};
static script::InternalScriptObject::Regular<DudScriptObject> dud("DudScriptObject");

namespace app {

class AppScriptObject : public script::ScriptObject {
public:
  inject<ScriptObject> m_pixelColor{"pixelColor"};
  std::vector<inject<ScriptObject>>  m_documents;

  AppScriptObject() {
    std::cout << "AppScriptObject() constructor" << std::endl;

    addProperty("activeFrameIndex",
        [this]{return updateSite() ? m_site.frame() : 0;},
        [] (const script::Value& value) {
            Params params = {
              { "frame", std::to_string(((int)value) + 1).c_str() }
            };

            UIContext::instance()->executeCommand(CommandsModule::instance()->getCommandByName(CommandId::GotoFrame), params);
            return (int)value;
        })
      .doc("read-only. Returns the number of the currently active animation frame.");

    addProperty("activeLayerIndex", 
        [this]{return updateSite() ? m_site.layerIndex() : 0;},
        [] (const script::Value& value) {
            Params params = {
              { "layer", std::to_string(((int)value)).c_str() }
            };

            UIContext::instance()->executeCommand(CommandsModule::instance()->getCommandByName(CommandId::GotoLayer), params);
            return (int)value;
        })
      .doc("read-only. Returns the number of the current layer.");

    addProperty("activeImage", [this]{
        DocumentScriptObject* doc = dynamic_cast<DocumentScriptObject*>((ScriptObject *)this->get("activeDocument"));
        SpriteScriptObject* sprite = dynamic_cast<SpriteScriptObject*>((ScriptObject *)doc->get("sprite"));
        LayerScriptObject* layer = dynamic_cast<LayerScriptObject*>((ScriptObject *)sprite->call("layer", (int)m_site.layerIndex()));
        CelScriptObject* cel = dynamic_cast<CelScriptObject*>((ScriptObject *)layer->call("cel", (int)m_site.frame()));
        return cel->get("image");
      })
      .doc("read-only, can be null. Returns the current layer/frame's image.");

    addProperty("activeSprite", [this]{
        DocumentScriptObject* doc = dynamic_cast<DocumentScriptObject*>((ScriptObject *)this->get("activeDocument"));
        return doc->get("sprite");
      })
      .doc("read-only. Returns the currently active Sprite.");

    addProperty("activeDocument",
        [this] () -> ScriptObject*{ 
          Document* activeDocument = UIContext::instance()->activeDocument();
          for (auto& doc : m_documents) {
            DocumentScriptObject* docObj = dynamic_cast<DocumentScriptObject*>((ScriptObject *)doc);
            if (docObj && docObj->getWrapped() == activeDocument) {
              return doc.get();
            }
          }

          return nullptr;
        },
        [] (const script::Value& documentObject) {
          DocumentScriptObject* doc = dynamic_cast<DocumentScriptObject*>((ScriptObject *)documentObject);
          if (doc) {
            Document* document = (Document *)doc->getWrapped();
            UIContext::instance()->setActiveDocument(document);
          }

          return true;
        })
      .doc("read-only. Returns the currently active Document.");

    addProperty("activeTool", 
        []{ return App::instance()->activeToolManager()->activeTool()->getId(); },
        [] (const script::Value& value) {
          auto tool = App::instance()->toolBox()->getToolById((std::string)value);

          if (tool) {
            App::instance()->activeToolManager()->setSelectedTool(tool);
            return true;
          }

          return false;
        }
        )
      .doc("read/write. Returns the currently active tool.");

    addProperty("pixelColor", [this]{return m_pixelColor.get();})
      .doc("read-only. Returns an object with functions for color conversion.");

    addProperty("version", []{return script::Value{VERSION};})
      .doc("read-only. Returns LibreSprite's current version as a string.");

    addMethod("documentation", &AppScriptObject::documentation)
      .doc("prints this text.");

    addMethod("newDocument", &AppScriptObject::newDocument)
      .doc("creates a new document.");

    addMethod("closeDocument", &AppScriptObject::closeDocument)
      .doc("closes the specified document");

    std::cout << "AppScriptObject() constructor make" << std::endl;
    makeGlobal("app");

    std::cout << "AppScriptObject() constructor init" << std::endl;
    init();
  }

  void documentation() {
    std::stringstream out;
    if (!this->get("activeDocument")) {
      return;
    }

    auto& internalRegistry = script::InternalScriptObject::getRegistry();
    auto originalDefault = internalRegistry[""];
    script::InternalScriptObject::setDefault("DudScriptObject");

    for (auto& entry : script::ScriptObject::getRegistry()) {
      if (entry.first.empty())
        continue;
      inject<ScriptObject> so{entry.first};
      auto internal = dynamic_cast<DudScriptObject*>(so->getInternalScriptObject());
      if (!internal)
        continue;

      out << "# ";
      if (!internal->globalName.empty())
        out << "global " << internal->globalName << " ";

      std::string className = entry.first;
      auto dot = className.rfind("ScriptObject");
      if (dot != std::string::npos)
        className.resize(dot);

      out << "[class " << className << "]" << std::endl;

      if (internal->properties.empty()) {
        out << "## No Properties." << std::endl;
      } else {
        out << "## Properties: " << std::endl;
        for (auto& propEntry : internal->properties) {
          auto& prop = propEntry.second;
          out << "   - `" << propEntry.first << "`: " << prop.docStr << std::endl;
        }
      }

      out << std::endl;

      if (internal->functions.empty()) {
        out << "## No Methods." << std::endl;
      } else {
        out << "## Methods: " << std::endl;
        for (auto& funcEntry : internal->functions) {
          auto& func = funcEntry.second;
          out << "   - `" << funcEntry.first << "(";
          bool first = true;
          for (auto& arg : func.docArgs) {
            if (!first) out << ", ";
            first = false;
            out << arg.name;
          }
          out << ")`: " << std::endl;

          for (auto& arg : func.docArgs) {
            out << "     - " << arg.name << ": " << arg.docStr << std::endl;
          }

          out << "      returns: " << func.docReturnsStr << std::endl;

          if (!func.docStr.empty())
            out << "      " << func.docStr << std::endl;

          out << std::endl;
        }
      }
      out << std::endl << std::endl;
    }

    std::cout << out.str() << std::endl;

    // inject<script::EngineDelegate>{}->onConsolePrint(out.str().c_str());

    internalRegistry[""] = originalDefault;
  }

  bool updateSite() {
    app::Document* doc = UIContext::instance()->activeDocument();
    app::DocumentView* m_view = UIContext::instance()->getFirstDocumentView(doc);
    if (!m_view)
      return false;
    m_view->getSite(&m_site);
    return true;
  }

  ScriptObject* init() {
    if (!updateSite()) {
      std::cout << "No site" << std::endl;
      return nullptr;
    }

    int layerIndex = m_site.layerIndex();
    int frameIndex = m_site.frame();

    std::cout << "init: " << layerIndex << ", " << frameIndex << std::endl;
    m_documents.emplace_back("DocumentScriptObject");

    auto sprite = m_documents.back()->get<ScriptObject*>("sprite");
    if (!sprite) {
      std::cout << "No sprite in document" << std::endl;
      return nullptr;
    }

    auto layer = sprite->call<ScriptObject*>("layer", layerIndex);
    if (!layer) {
      std::cout << "No layer in sprite" << std::endl;
      return nullptr;
    }

    return layer->call<ScriptObject*>("cel", frameIndex);
  }

  script::Value open(const std::string& fn) {
    if (fn.empty())
      return {};
    app::Document* oldDoc = UIContext::instance()->activeDocument();
    Command* openCommand = CommandsModule::instance()->getCommandByName(CommandId::OpenFile);
    Params params;
    params.set("filename", fn.c_str());
    UIContext::instance()->executeCommand(openCommand, params);

    app::Document* newDoc = UIContext::instance()->activeDocument();
    if (newDoc == oldDoc)
      return {};

    std::cout << "Opened " << fn << std::endl;
    m_documents.emplace_back("DocumentScriptObject");
    return inject<ScriptObject>{"activeSprite"}.get();
  }

  script::Value newDocument(int width, int height) {
    Command* newCommand = CommandsModule::instance()->getCommandByName(CommandId::NewFile);
    Params params;
    params.set("width", std::to_string(width).c_str());
    params.set("height", std::to_string(height).c_str());
    params.set("bg", "0");
    params.set("format", "0");

    std::cout << "New document" << std::endl;
    UIContext::instance()->executeCommand(newCommand, params);
    m_documents.emplace_back("DocumentScriptObject");
    return this->get("activeDocument");
  }

  script::Value closeDocument(const script::Value& documentObject) {
    DocumentScriptObject* doc = dynamic_cast<DocumentScriptObject*>((ScriptObject *)documentObject);
    if (!doc) return false;

    auto document = (Document *)doc->getWrapped();
    UIContext::instance()->setActiveDocument(document);
    UIContext::instance()->executeCommand(CommandsModule::instance()->getCommandByName(CommandId::CloseFile));

    // Remove it from m_documents, which will deregister
    auto it = std::find_if(m_documents.begin(), m_documents.end(),
      [doc](const auto& obj) {
        return obj.get() == doc;
      });
    if (it != m_documents.end()) {
      m_documents.erase(it);
      return true;
    }

    return false;
  }

  void App_exit() {
    Command* exitCommand = CommandsModule::instance()->getCommandByName(CommandId::Exit);
    UIContext::instance()->executeCommand(exitCommand);
  }

  doc::Site m_site;
};

static script::ScriptObject::Regular<AppScriptObject> reg("AppScriptObject", {"global"});

}
