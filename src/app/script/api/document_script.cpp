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
#include "app/file/file.h"

using namespace app;

DocumentScriptObject::DocumentScriptObject()
{
  addProperty("sprite", [this]{return m_sprite.get();});

  addMethod("saveAsAseprite", &DocumentScriptObject::saveAsAseprite)
    .doc("saves the document as an .aseprite file at the specified name");
}

static script::ScriptObject::Regular<DocumentScriptObject> reg("DocumentScriptObject");

void DocumentScriptObject::saveAsAseprite(std::string filename) {
  auto ctx = UIContext::instance();
  const app::Document* doc = dynamic_cast<const app::Document*>(m_doc);

  if (doc == nullptr) return;
  FileOp* fop = FileOp::createSaveDocumentOperation(ctx, doc, filename.c_str(), "");
  fop->operate();
}
