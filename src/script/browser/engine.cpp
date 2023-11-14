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
    obj.set("__handle", this->getHandleId());

    // Add the functions
    for (auto& entry : functions) {
      std::string className = m_className;
      std::string fnName = entry.first;
      std::string path = className + "." + fnName;

      //val fn = val::module_property(path.c_str());
      val fn = val::module_property("callFuncV0");

      std::cout << "Adding function " << path << std::endl;
      obj.set("__" + entry.first, fn);
    }

    // Add the properties
    //for (auto& entry : properties) {
    //  // entry first is the name, entry second has .getter and .setter, add set_ and get_ to obj
    //  obj.set("set_" + entry.first, val::function(entry.second.setter));
    //  obj.set("get_" + entry.first, val::function(entry.second.getter));
    //}

    librespriteObj.set("__temp", obj);

    for (auto& entry : functions) {
      std::string className = m_className;
      std::string fnName = entry.first;
      std::string path = className + "." + fnName;

      std::string script = "let v = window.libresprite.__temp; v." + fnName + " = (...arguments) => { arguments.unshift(v.__handle); arguments.unshift(\""+ fnName + "\"); return v.__" + fnName + "(...arguments); };";
      emscripten_run_script(script.c_str());
    }

    librespriteObj.set("__temp", val::undefined());
    emscripten_run_script("delete window.__temp;");

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

val callFuncV0(std::string name, std::string handle) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  std::cout << "Calling function [" << name << "] on object " << handle << std::endl;
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) {
    std::cout << "Function " << name << " not found" << std::endl;
    return val(0);
  }

  std::cout << "Found function " << name << std::endl;
  auto& func = it->second;
  func();
  std::cout << "Called function " << name << std::endl;

  return returnValue(func.result);
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("callFuncV0", &callFuncV0);
}

#endif
