// LibreSprite
// Copyright (C) 2021 LibreSprite contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "script/engine.h"
#include "app/document.h"
#include "app/ui_context.h"
#include "app/script/api/document_script.h"

DocumentScriptObject::DocumentScriptObject()
{
  addProperty("sprite", [this]{return m_sprite.get();});
}

static script::ScriptObject::Regular<DocumentScriptObject> reg("DocumentScriptObject");
