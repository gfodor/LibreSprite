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

EM_JS(void, bind_local_function, (const char *handleId, const char *fnName, const char *callFunc), {
    handleId = UTF8ToString(handleId);
    fnName = UTF8ToString(fnName);
    callFunc = UTF8ToString(callFunc);

    libresprite.__[fnName] = (...args) => { return libresprite._[callFunc](handleId, fnName, ...args); };
});

EM_JS(void, bind_local_property, (const char *handleId, const char *fnName, const char *callGetterFunc, const char* callSetterFunc), {
    handleId = UTF8ToString(handleId);
    fnName = UTF8ToString(fnName);
    callGetterFunc = UTF8ToString(callGetterFunc);
    callSetterFunc = UTF8ToString(callSetterFunc);

    Object.defineProperty(libresprite.__, fnName, {
      get: () => { return libresprite._[callGetterFunc](handleId, fnName); },
      set: (...args) => { return libresprite._[callSetterFunc](handleId, fnName, ...args); }
    });
});

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
      helperObj.set("callFunc6", val::module_property("callFunc6"));
      helperObj.set("callGet0", val::module_property("callGet0"));
      helperObj.set("callSet1", val::module_property("callSet1"));
    }

    val obj = val::object();

    std::string handleId = this->getHandleId();

    librespriteObj.set("__", obj);
    obj.set("_", handleId);

    for (auto& entry : functions) {
      std::string fnName = entry.first;

      DocumentedFunction& fn = entry.second;
      std::string callFunc = "callFunc" + std::to_string(fn.getArity());

      bind_local_function(handleId.c_str(), fnName.c_str(), callFunc.c_str());
    }

    for (auto& entry : properties) {
      std::string fnName = entry.first;
      std::string capitalized = entry.first;
      capitalized[0] = toupper(capitalized[0]);
      std::string callGetterFunc = "callGet0";
      std::string callSetterFunc = "callSet1";

      bind_local_property(handleId.c_str(), fnName.c_str(), callGetterFunc.c_str(), callSetterFunc.c_str());
    }

    librespriteObj.delete_("__");

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

bool pushValArgOntoFunc(val arg, Function& func) {
  val typeofArg = arg.typeof();
  std::string typeofArgString = typeofArg.as<std::string>();

  if (typeofArgString == "number") {
    func.arguments.push_back(arg.as<int>());
  } else if (typeofArgString == "string") {
    func.arguments.push_back(arg.as<std::string>());
  } else if (typeofArgString == "boolean") {
    func.arguments.push_back(arg.as<bool>());
  } else {
    std::string handleId = arg["_"].as<std::string>();
    BrowserScriptObject *obj = BrowserScriptObject::getByHandle(handleId);

    if (obj != nullptr) {
      func.arguments.push_back(Value(obj->getScriptObject()));
    } else {
      return false;
    }
  }

  return true;
}

val callFunc0(std::string handle, std::string name) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  func();

  return returnValue(func.result);
}

val callFunc1(std::string handle, std::string name, val arg1) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);

  func();

  return returnValue(func.result);
}

val callFunc2(std::string handle, std::string name, val arg1, val arg2) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);
  if (!pushValArgOntoFunc(arg2, func)) return val(false);
  func();

  return returnValue(func.result);
}

val callFunc3(std::string handle, std::string name, val arg1, val arg2, val arg3) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);
  if (!pushValArgOntoFunc(arg2, func)) return val(false);
  if (!pushValArgOntoFunc(arg3, func)) return val(false);
  func();

  return returnValue(func.result);
}

val callFunc4(std::string handle, std::string name, val arg1, val arg2, val arg3, val arg4) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);
  if (!pushValArgOntoFunc(arg2, func)) return val(false);
  if (!pushValArgOntoFunc(arg3, func)) return val(false);
  if (!pushValArgOntoFunc(arg4, func)) return val(false);
  func();

  return returnValue(func.result);
}

val callFunc5(std::string handle, std::string name, val arg1, val arg2, val arg3, val arg4, val arg5) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);
  if (!pushValArgOntoFunc(arg2, func)) return val(false);
  if (!pushValArgOntoFunc(arg3, func)) return val(false);
  if (!pushValArgOntoFunc(arg4, func)) return val(false);
  if (!pushValArgOntoFunc(arg5, func)) return val(false);
  func();

  return returnValue(func.result);
}

val callFunc6(std::string handle, std::string name, val arg1, val arg2, val arg3, val arg4, val arg5, val arg6) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->functions.find(name);

  if (it == obj->functions.end()) return val(false);

  auto& func = it->second;
  if (!pushValArgOntoFunc(arg1, func)) return val(false);
  if (!pushValArgOntoFunc(arg2, func)) return val(false);
  if (!pushValArgOntoFunc(arg3, func)) return val(false);
  if (!pushValArgOntoFunc(arg4, func)) return val(false);
  if (!pushValArgOntoFunc(arg5, func)) return val(false);
  if (!pushValArgOntoFunc(arg6, func)) return val(false);
  func();

  return returnValue(func.result);
}

val callGet0(std::string handle, std::string name) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->properties.find(name);

  if (it == obj->properties.end()) return val(false);

  auto& func = it->second.getter;
  func();

  return returnValue(func.result);
}

val callSet1(std::string handle, std::string name, val value) {
  auto obj = BrowserScriptObject::getByHandle(handle);
  auto it = obj->properties.find(name);

  if (it == obj->properties.end()) return val(false);

  auto& func = it->second.setter;
  if (!pushValArgOntoFunc(value, func)) return val(false);
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
  function("callFunc6", &callFunc6);
  function("callGet0", &callGet0);
  function("callSet1", &callSet1);
}

#endif
