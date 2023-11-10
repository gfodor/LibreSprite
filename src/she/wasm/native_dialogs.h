// SHE library
// Copyright (C) 2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "she/native_dialogs.h"

extern "C" {
  void jsOpenFilePicker(int callbackPtr, int userData);
  void jsWriteFileToMEMFS(const char* filename, const uint8_t* data, size_t size);
}

namespace she {

  class NativeDialogsWasm : public NativeDialogs {
  public:
    NativeDialogsWasm();
    FileDialog* createFileDialog() override;
  };

} // namespace she
