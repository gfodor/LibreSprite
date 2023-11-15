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

      val helperObj = val::object();
      librespriteObj.set("_", helperObj);

      helperObj.set("callFunc0", val::module_property("callFunc0"));
      helperObj.set("callFunc1", val::module_property("callFunc1"));
      helperObj.set("callFunc2", val::module_property("callFunc2"));
      helperObj.set("callFunc3", val::module_property("callFunc3"));
      helperObj.set("callFunc4", val::module_property("callFunc4"));
      helperObj.set("callFunc5", val::module_property("callFunc5"));
      helperObj.set("callGet0", val::module_property("callGet0"));
      helperObj.set("callSet1", val::module_property("callSet1"));
    }

    val obj = val::object();

    std::string handleId = this->getHandleId();

    librespriteObj.set("__", obj);

    for (auto& entry : functions) {
      std::string className = m_className;
      std::string fnName = entry.first;

      DocumentedFunction& fn = entry.second;
      std::string callFunc = "callFunc" + std::to_string(fn.getArity());

      std::string script = "libresprite.__." + fnName + " = (...arguments) => { arguments.unshift(\"" + handleId + "\", \""+ fnName + "\"); return libresprite._." + callFunc + "(...arguments); };";
      emscripten_run_script(script.c_str());
    }

    for (auto& entry : properties) {
      std::string className = m_className;
      std::string fnName = entry.first;
      std::string capitalized = entry.first;
      capitalized[0] = toupper(capitalized[0]);
      std::string callGetterFunc = "callGet0";
      std::string callSetterFunc = "callSet1";

      std::string script = "Object.defineProperty(libresprite.__, \"" + fnName + "\", { get: (...arguments) => { arguments.unshift(\"" + handleId + "\", \"" + fnName + "\"); return libresprite._." + callGetterFunc + "(...arguments); }, set: (...arguments) => { arguments.unshift(\"" + handleId + "\",\""+ fnName + "\"); return libresprite._." + callSetterFunc + "(...arguments); } });";

      emscripten_run_script(script.c_str());
    }

    emscripten_run_script("delete libresprite.__;");

    return obj;
  }

  void makeGlobal(const std::string& name) override {
    val obj = makeLocal();
    val window = val::global("window");
    val librespriteObj = window["libresprite"];
    librespriteObj.set(name, obj);
  }
};

static InternalScriptObject::Regular<BrowserScriptObject> browserSO("BrowserScriptObject");

static Engine::Regular<BrowserEngine> registration("browser", {"js"});

val returnValue(const Value& value) {
    switch (value.type) {
    case Value::Type::UNDEFINED:
      return val();

    case Value::Type::INT:
      return val((int)value);

    case Value::Type::DOUBLE:
      return val((double)value);

    case Value::Type::STRING:
      return val((std::string)value);

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

void pushValArgOntoFunc(val arg, Function& func) {
  val typeofArg = arg.typeof();
  std::string typeofArgString = typeofArg.as<std::string>();

  if (typeofArgString == "number") {
    func.arguments.push_back(arg.as<int>());
  } else if (typeofArgString == "string") {
    func.arguments.push_back(arg.as<std::string>());
  } else if (typeofArgString == "boolean") {
    func.arguments.push_back(arg.as<bool>());
  } else {
    printf("Unknown argument type: %s\n", typeofArgString.c_str());
  }
}

val callFunc0(std::string name, std::string handle) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  func();

  return returnValue(func.result);
}

val callFunc1(std::string name, std::string handle, val arg1) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  pushValArgOntoFunc(arg1, func);
  func();

  return returnValue(func.result);
}

val callFunc2(std::string name, std::string handle, val arg1, val arg2) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  pushValArgOntoFunc(arg1, func);
  pushValArgOntoFunc(arg2, func);
  func();

  return returnValue(func.result);
}

val callFunc3(std::string name, std::string handle, val arg1, val arg2, val arg3) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  pushValArgOntoFunc(arg1, func);
  pushValArgOntoFunc(arg2, func);
  pushValArgOntoFunc(arg3, func);
  func();

  return returnValue(func.result);
}

val callFunc4(std::string name, std::string handle, val arg1, val arg2, val arg3, val arg4) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  pushValArgOntoFunc(arg1, func);
  pushValArgOntoFunc(arg2, func);
  pushValArgOntoFunc(arg3, func);
  pushValArgOntoFunc(arg4, func);
  func();

  return returnValue(func.result);
}

val callFunc5(std::string name, std::string handle, val arg1, val arg2, val arg3, val arg4, val arg5) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(0);

  auto& func = it->second;
  pushValArgOntoFunc(arg1, func);
  pushValArgOntoFunc(arg2, func);
  pushValArgOntoFunc(arg3, func);
  pushValArgOntoFunc(arg4, func);
  pushValArgOntoFunc(arg5, func);
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

val callSet1(std::string name, std::string handle, val value) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->properties.find(name);

  if (it == obj->properties.end()) return val(0);

  auto& func = it->second.setter;
  pushValArgOntoFunc(value, func);
  func();
  return returnValue(func.result);
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("callFunc0", &callFunc0);
  function("callFunc1", &callFunc1);
  function("callFunc2", &callFunc2);
  function("callFunc3", &callFunc3);
  function("callFunc4", &callFunc4);
  function("callFunc5", &callFunc5);
  function("callGet0", &callGet0);
  function("callSet1", &callSet1);
}

#endif
