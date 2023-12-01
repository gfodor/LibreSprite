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
#include "app/ui/editor/editor.h"
#include "app/modules/editors.h"

using namespace app;

DocumentScriptObject::DocumentScriptObject()
{
  addProperty("sprite", [this]{return m_sprite.get();});

  addMethod("saveToFile", &DocumentScriptObject::saveToFile)
    .doc("saves the document to a file at the specified name");

  addMethod("mergeWithAsespriteBytes", &DocumentScriptObject::mergeWithAsepriteBytes)
    .doc("merges the document with the specified .aseprite file bytes");
}

static script::ScriptObject::Regular<DocumentScriptObject> reg("DocumentScriptObject");

void DocumentScriptObject::saveToFile(std::string filename) {
  auto ctx = UIContext::instance();
  const app::Document* doc = dynamic_cast<const app::Document*>(m_doc);

  if (doc == nullptr) return;
  std::unique_ptr<FileOp> fop(FileOp::createSaveDocumentOperation(ctx, doc, filename.c_str(), ""));
  fop->operate();
}

void DocumentScriptObject::mergeWithAsepriteBytes(const std::string& bytes) {
  auto ctx = UIContext::instance();
  auto doc = dynamic_cast<app::Document*>(m_doc);

  if (doc == nullptr) return;

  std::unique_ptr<FileOp> fop(FileOp::createLoadDocumentOperation(ctx, "file.ase", FILE_LOAD_SEQUENCE_NONE, bytes));
  fop->operate();
  std::unique_ptr<doc::Document> loadedDoc(fop->document());
  bool didChange = m_doc->sprite()->mergeWith(loadedDoc->sprite());

  if (didChange) {
    current_editor->invalidate();
  }
}
