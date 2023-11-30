// LibreSprite
// Copyright (C) 2021 LibreSprite contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/document.h"
#include "app/ui_context.h"
#include "script/script_object.h"

class DocumentScriptObject : public script::ScriptObject {
public:
  DocumentScriptObject();
  void* getWrapped() override { return m_doc;}

  Provides provides{this, "activeDocument"};
  void saveToFile(std::string filename);
  void mergeWithAsepriteBytes(const std::string& bytes);

private:
  doc::Document* m_doc{app::UIContext::instance()->activeDocument()};
  inject<ScriptObject> m_sprite{"SpriteScriptObject"};
};
