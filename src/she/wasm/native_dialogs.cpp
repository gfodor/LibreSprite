// SHE library
// Copyright (C) 2012-2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <vector>

#include "she/display.h"
#include "she/keys.h"
#include "she/native_cursor.h"
#include "she/wasm/native_dialogs.h"
#include <iostream>

#include "base/path.h"

#include <emscripten.h>

EM_JS(void, open_file_picker, (const char* acceptedExtensions), {
  Module.file_picker_done = false;
  Module.file_picker_tempfile = null;

  const inputElement = document.createElement('input');
  inputElement.type = 'file';

  if (acceptedExtensions) {
    acceptedExtensions = UTF8ToString(acceptedExtensions);
  }

  if (acceptedExtensions && acceptedExtensions.length > 0) {
    inputElement.accept = acceptedExtensions;
  }

  inputElement.onchange = function(event) {
    const file = event.target.files[0];
    if (!file) {
      Module.file_picker_done = true;
      return;
    }

    const reader = new FileReader();
    reader.onload = function(loadEvent) {
      const arrayBuffer = loadEvent.target.result;
      const file_data = new Uint8Array(arrayBuffer);

      // Generate a temporary file name and then write the bytes of the underlying file to MEMFS at that path.
      const filename = file.name;
      const ext = filename.substr(filename.lastIndexOf('.') + 1);
      const basename = filename.substr(0, filename.lastIndexOf('.'));
      const tempfile = Date.now() + '-temp-picker.' + ext;
      
      // Now write the file to MEMFS.
      FS.writeFile(tempfile, file_data);

      Module.file_picker_tempfile = tempfile;
      Module.file_picker_done = true;
    };

    reader.readAsArrayBuffer(file);
  };

  inputElement.click();

  // If window is focused again then set the picker done since the user cancelled

  window.addEventListener('focus', function() {
    if (!Module.file_picker_done) {
      Module.file_picker_done = true;
    }
  }, { once: true });
})

EM_JS(bool, file_picker_done, (), {
    return !!Module.file_picker_done;
});

EM_JS(bool, has_file_picker_tempfile, (), {
    return !!Module.file_picker_tempfile;
});

EM_JS(void, get_file_picker_tempfile, (char* buffer, int size), {
    stringToUTF8(Module.file_picker_tempfile, buffer, size);
});

namespace she {

class FileDialogWasm : public FileDialog {
public:
  FileDialogWasm()
    : m_save(false)
  {
  }

  void dispose() override {
    delete this;
  }

  void toOpenFile() override {
    m_save = false;
  }

  void toSaveFile() override {
    m_save = true;
  }

  void setTitle(const std::string& title) override {
    m_title = title;
  }

  void setDefaultExtension(const std::string& extension) override {
    m_defExtension = extension;
  }

  void addFilter(const std::string& extension, const std::string& description) override {
    if (m_defExtension.empty())
      m_defExtension = extension;

    m_filters.push_back(std::make_pair(description, extension));
  }

  std::string getAcceptedExtensionsString() {
      std::string extensions;
      for (const auto& filter : m_filters) {
          if (!extensions.empty())
              extensions += ",";
          extensions += "." + filter.second; // assuming the filter.second is the extension
      }
      return extensions.empty() ? "" : extensions;
  }

  std::string fileName() override {
    return m_filename;
  }

  void setFileName(const std::string& filename) override {
    m_filename = filename;
  }

  bool show(Display* display) override {
    std::string acceptedExtensions = getAcceptedExtensionsString();
    open_file_picker(acceptedExtensions.c_str());

    while (1) {
      if (file_picker_done()) {
        if (has_file_picker_tempfile()) {
          char buffer[1024];
          get_file_picker_tempfile(buffer, sizeof(buffer));
          m_filename = buffer;
          std::cout << "file_picker_tempfile: " << m_filename << std::endl;
          return true;
        }
        else
          return false;
      }

      emscripten_sleep(100);
    }

    return false;
  }

private:

  std::vector<std::pair<std::string, std::string>> m_filters;
  std::string m_defExtension;
  std::string m_filename;
  std::string m_title;
  bool m_save;
};

NativeDialogsWasm::NativeDialogsWasm()
{
}

FileDialog* NativeDialogsWasm::createFileDialog()
{
  return new FileDialogWasm();
}

} // namespace she
