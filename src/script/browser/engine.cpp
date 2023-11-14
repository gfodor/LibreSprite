// LibreSprite Scripting Library
// Copyright (C) 2021  LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.


#include <cstring>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "script/engine.h"
#include "script/engine_delegate.h"

#include <map>
#include <iostream>
#include <string>
#include <unordered_map>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace script;
using namespace emscripten;

class BrowserEngine : public Engine {
public:
  inject<EngineDelegate> m_delegate;

  BrowserEngine() {
    InternalScriptObject::setDefault("BrowserScriptObject");
  }

  bool raiseEvent(const std::string& event) override {
    return eval("if (typeof onEvent === \"function\") onEvent(\"" + event + "\");");
  }

  bool eval(const std::string& code) override {
    initGlobals();

    return true;
  }
};

static std::map<std::string, InternalScriptObject*> browserScriptRegistry;

class BrowserScriptObject : public InternalScriptObject {
public:
  std::string getHandleId() {
    return std::to_string(reinterpret_cast<uintptr_t>(this));
  }

  BrowserScriptObject() : InternalScriptObject() {
    browserScriptRegistry[this->getHandleId()] = this;
  }

  ~BrowserScriptObject() {
    browserScriptRegistry.erase(this->getHandleId());
  }

  static BrowserScriptObject* getByHandle(std::string handle) {
      auto it = browserScriptRegistry.find(handle);
      if (it != browserScriptRegistry.end()) {
          return dynamic_cast<BrowserScriptObject*>(it->second);
      }
      return nullptr; // or handle differently if not found
  }

  val makeLocal() {
    val window = val::global("window");
    val librespriteObj = window["libresprite"];

    if (librespriteObj.isUndefined()) {
      librespriteObj = val::object();
      window.set("libresprite", librespriteObj);
    }

    val obj = val::object();

    std::string handleId = this->getHandleId();
    librespriteObj.set("__callFunc0", val::module_property("callFunc0"));
    librespriteObj.set("__callGet0", val::module_property("callGet0"));
    librespriteObj.set("__callSet0", val::module_property("callSet0"));

    librespriteObj.set("__temp", obj);

    for (auto& entry : functions) {
      std::string className = m_className;
      std::string fnName = entry.first;
      std::string callFunc = "callFunc0";

      std::string script = "libresprite.__temp." + fnName + " = (...arguments) => { arguments.unshift(\"" + handleId + "\"); arguments.unshift(\""+ fnName + "\"); return libresprite.__" + callFunc + "(...arguments); };";
      emscripten_run_script(script.c_str());
    }

    for (auto& entry : properties) {
      std::string className = m_className;
      std::string fnName = entry.first;
      std::string capitalized = entry.first;
      capitalized[0] = toupper(capitalized[0]);
      std::string callGetterFunc = "callGet0";
      std::string callSetterFunc = "callSet0";

      std::string script = "Object.defineProperty(libresprite.__temp, \"" + fnName + "\", { get: (...arguments) => { arguments.unshift(\"" + handleId + "\"); arguments.unshift(\"" + fnName + "\"); return libresprite.__" + callGetterFunc + "(...arguments); }, set: (...arguments) => { arguments.unshift(\"" + handleId + "\"); arguments.unshift(\""+ fnName + "\"); return libresprite.__" + callSetterFunc + "(...arguments); } });";

      emscripten_run_script(script.c_str());
    }

    librespriteObj.set("__temp", val::undefined());

    return obj;
  }

  void makeGlobal(const std::string& name) override {
    val window = val::global("window");
    val librespriteObj = window["libresprite"];

    if (librespriteObj.isUndefined()) {
      librespriteObj = val::object();
      window.set("libresprite", librespriteObj);
    }

    librespriteObj.set(name, makeLocal());
  }
};

static InternalScriptObject::Regular<BrowserScriptObject> browserSO("BrowserScriptObject");

static Engine::Regular<BrowserEngine> registration("browser", {"js"});

val returnValue(const Value& value) {
    switch (value.type) {
    case Value::Type::UNDEFINED:
      return val();

    case Value::Type::INT:
    case Value::Type::DOUBLE:
    case Value::Type::STRING:
      return val(value);

    case Value::Type::OBJECT:
      if (auto object = static_cast<ScriptObject*>(value)) {
        return static_cast<BrowserScriptObject*>(object->getInternalScriptObject())->makeLocal();
      }

    case Value::Type::BUFFER:
      // TODO

    default:
      printf("Unknown return type: %d\n", int(value.type));
      break;
    }
    return {};
}

val callFunc0(std::string name, std::string handle) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  func();

  return returnValue(func.result);
}

val callGet0(std::string name, std::string handle) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->properties.find(name);

  if (it == obj->properties.end()) return val(0);

  auto& func = it->second.getter;
  func();

  return returnValue(func.result);
}

val callSet0(std::string name, std::string handle, val value) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->properties.find(name);

  if (it == obj->properties.end()) return val(0);

  auto& func = it->second.setter;
  func();

  return returnValue(func.result);
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("callFunc0", &callFunc0);
  function("callGet0", &callGet0);
  function("callSet0", &callSet0);
}

#endif
