// Aseprite
// Copyright (C) 2001-2015  David Capello
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <map>
#include <string>
#include <sstream>

namespace app {

  class Params {
  public:
    typedef std::map<std::string, std::string> Map;
    typedef Map::iterator iterator;
    typedef Map::const_iterator const_iterator;

    Params() {}
    Params(std::initializer_list<Map::value_type> init) : m_params(init) {}

    iterator begin() { return m_params.begin(); }
    iterator end() { return m_params.end(); }
    const_iterator begin() const { return m_params.begin(); }
    const_iterator end() const { return m_params.end(); }

    bool empty() const {
      return m_params.empty();
    }

    void clear() {
      return m_params.clear();
    }

    bool has_param(const char* name) const {
      return m_params.find(name) != m_params.end();
    }

    bool operator==(const Params& params) const {
      return m_params == params.m_params;
    }

    bool operator!=(const Params& params) const {
      return m_params != params.m_params;
    }

    std::string& set(const char* name, const char* value) {
      return m_params[name] = value;
    }

    std::string& set(const char* name, const std::string& value) {
      return m_params[name] = value;
    }

    const std::string& get(const char* name) const {
      return m_params[name];
    }

    void operator|=(const Params& params) const {
      for (const auto& p : params)
        m_params[p.first] = p.second;
    }

    template<typename T>
    const T get_as(const char* name) const {
      std::istringstream stream(m_params[name]);
      T value = T();
      stream >> value;
      return value;
    }

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, T>::type
    get_as_enum(const char* name) const {
        std::istringstream stream(m_params[name]);
        std::underlying_type_t<T> intValue;
        stream >> intValue;
        return static_cast<T>(intValue);
    }

  private:
    mutable Map m_params;
  };

} // namespace app
