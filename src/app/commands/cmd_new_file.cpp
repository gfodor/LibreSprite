// Aseprite
// Copyright (C) 2001-2015  David Capello
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/color.h"
#include "app/color_utils.h"
#include "app/commands/command.h"
#include "app/console.h"
#include "app/document.h"
#include "app/modules/editors.h"
#include "app/modules/palettes.h"
#include "app/pref/preferences.h"
#include "app/ui/button_set.h"
#include "app/ui/workspace.h"
#include "app/ui_context.h"
#include "app/util/clipboard.h"
#include "doc/cel.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/palette.h"
#include "doc/primitives.h"
#include "doc/sprite.h"
#include "ui/ui.h"

#include "new_sprite.xml.h"

#include <iostream>
#include <memory>

using namespace ui;

namespace app {

class NewFileCommand : public Command {
public:
  NewFileCommand();
  Command* clone() const override { return new NewFileCommand(*this); }

protected:
  void onLoadParams(const Params& params) override;
  void onExecute(Context* context) override;

private:
  int m_width = 0;
  int m_height = 0;
  int m_bg = -1;
  PixelFormat m_format = IMAGE_NONE;
};

static int _sprite_counter = 0;

NewFileCommand::NewFileCommand()
  : Command("NewFile",
            "New File",
            CmdRecordableFlag)
{
}

void NewFileCommand::onLoadParams(const Params& params)
{
  if (params.has_param("width")) {
    // Parse width from string
    m_width = params.get_as<int>("width");
  }

  if (params.has_param("height")) {
    // Parse height from string
    m_height = params.get_as<int>("height");
  }

  if (params.has_param("bg")) {
    // Parse background color from string
    m_bg = params.get_as<int>("bg");
  }

  if (params.has_param("format")) {
    m_format = params.get_as_enum<PixelFormat>("format");
  }
}

/**
 * Shows the "New Sprite" dialog.
 */
void NewFileCommand::onExecute(Context* context)
{
  std::cout << "NewFileCommand::onExecute" << std::endl;
  Preferences& pref = Preferences::instance();

  bool hasSettings = m_width != 0 && m_height != 0 && m_bg != -1 && m_format != IMAGE_NONE;

  PixelFormat format = m_format;
  int w = m_width;
  int h = m_height;
  int bg = m_bg;

  if (format == IMAGE_NONE) {
    // Default values: Indexed, 320x240, Background color
    PixelFormat format = pref.newFile.colorMode();
    // Invalid format in config file.
    if (format != IMAGE_RGB &&
        format != IMAGE_TRGB &&
        format != IMAGE_INDEXED &&
        format != IMAGE_GRAYSCALE) {
      format = IMAGE_INDEXED;
    }
  }

  if (w == 0)
    w = pref.newFile.width();

  if (h == 0)
    h = pref.newFile.height();

  if (bg == -1)
    bg = pref.newFile.backgroundColor();

  bg = MID(0, bg, 2);

  // If the clipboard contains an image, we can show the size of the
  // clipboard as default image size.
  gfx::Size clipboardSize;
  if (clipboard::get_image_size(clipboardSize)) {
    w = clipboardSize.w;
    h = clipboardSize.h;
  }

  if (hasSettings) {
    std::cout << "hasSettings" << std::endl;
    char buf[1024];
    int ncolors = get_default_palette()->size();
    app::Color bg_table[] = {
      app::Color::fromMask(),
      app::Color::fromRgb(255, 255, 255),
      app::Color::fromRgb(0, 0, 0),
    };

    w = MID(1, w, 65535);
    h = MID(1, h, 65535);
    bg = MID(0, bg, 2);

    // Select the color
    app::Color color = app::Color::fromMask();

    if (bg >= 0 && bg <= 3) {
      color = bg_table[bg];
    }

    std::unique_ptr<Sprite> sprite(Sprite::createBasicSprite(format, w, h, ncolors));

    if (sprite->pixelFormat() != IMAGE_GRAYSCALE)
      get_default_palette()->copyColorsTo(sprite->palette(frame_t(0)));

    // If the background color isn't transparent, we have to
    // convert the `Layer 1' in a `Background'
    if (color.getType() != app::Color::MaskType) {
      Layer* layer = sprite->folder()->getFirstLayer();

      if (layer && layer->isImage()) {
        LayerImage* layerImage = static_cast<LayerImage*>(layer);
        layerImage->configureAsBackground();

        Image* image = layerImage->cel(frame_t(0))->image();

        // TODO Replace this adding a new parameter to color utils
        Palette oldPal = *get_current_palette();
        set_current_palette(get_default_palette(), false);

        doc::clear_image(image,
          color_utils::color_for_target(color,
            ColorTarget(
              ColorTarget::BackgroundLayer,
              sprite->pixelFormat(),
              sprite->transparentColor())));

        set_current_palette(&oldPal, false);
      }
    }

    // Show the sprite to the user
    std::unique_ptr<Document> doc(new Document(sprite.get()));
    sprite.release();
    sprintf(buf, "Sprite-%04d", ++_sprite_counter);
    doc->setFilename(buf);
    doc->setContext(context);
    doc.release();
  } else {
    std::cout << "not hasSettings" << std::endl;
    std::shared_ptr<Window> windowPtr = std::make_shared<app::gen::NewSprite>();
    app::gen::NewSprite* window = static_cast<app::gen::NewSprite*>(windowPtr.get());

    window->width()->setTextf("%d", MAX(1, w));
    window->height()->setTextf("%d", MAX(1, h));

    // Select image-type
    window->colorMode()->setSelectedItem(format);

    // Select background color
    window->bgColor()->setSelectedItem(bg);

    Manager::getDefault()->openWindowInForeground(windowPtr, [context](ui::Window* windowPtr) -> void {
      app::gen::NewSprite* window = static_cast<app::gen::NewSprite*>(windowPtr);

      if (window->closer() == window->okButton()) {
        char buf[1024];
        int ncolors = get_default_palette()->size();
        app::Color bg_table[] = {
          app::Color::fromMask(),
          app::Color::fromRgb(255, 255, 255),
          app::Color::fromRgb(0, 0, 0),
        };

        Preferences& pref = Preferences::instance();
        bool ok = false;

        // Get the options
        int i_format = window->colorMode()->selectedItem();

        PixelFormat format;

        switch (i_format) {
          case 0: format = IMAGE_TRGB; break;
          case 1: format = IMAGE_RGB; break;
          case 2: format = IMAGE_GRAYSCALE; break;
          case 3: format = IMAGE_INDEXED; break;
        }

        int w = window->width()->textInt();
        int h = window->height()->textInt();
        int bg = window->bgColor()->selectedItem();

        static_assert(IMAGE_RGB == 0, "RGB pixel format should be 0");
        static_assert(IMAGE_INDEXED == 2, "Indexed pixel format should be 2");
        static_assert(IMAGE_TRGB == 4, "TRGB format should be 4");

        format = MID(IMAGE_RGB, format, IMAGE_TRGB);
        w = MID(1, w, 65535);
        h = MID(1, h, 65535);
        bg = MID(0, bg, 2);

        // Select the color
        app::Color color = app::Color::fromMask();

        if (bg >= 0 && bg <= 3) {
          color = bg_table[bg];
          ok = true;
        }

        if (ok) {
          // Save the configuration
          pref.newFile.width(w);
          pref.newFile.height(h);
          pref.newFile.colorMode(format);
          pref.newFile.backgroundColor(bg);

          // Create the new sprite
          ASSERT(format == IMAGE_RGB || format == IMAGE_TRGB || format == IMAGE_GRAYSCALE || format == IMAGE_INDEXED);
          ASSERT(w > 0 && h > 0);

          std::unique_ptr<Sprite> sprite(Sprite::createBasicSprite(format, w, h, ncolors));

          if (sprite->pixelFormat() != IMAGE_GRAYSCALE)
            get_default_palette()->copyColorsTo(sprite->palette(frame_t(0)));

          // If the background color isn't transparent, we have to
          // convert the `Layer 1' in a `Background'
          if (color.getType() != app::Color::MaskType) {
            Layer* layer = sprite->folder()->getFirstLayer();

            if (layer && layer->isImage()) {
              LayerImage* layerImage = static_cast<LayerImage*>(layer);
              layerImage->configureAsBackground();

              Image* image = layerImage->cel(frame_t(0))->image();

              // TODO Replace this adding a new parameter to color utils
              Palette oldPal = *get_current_palette();
              set_current_palette(get_default_palette(), false);

              doc::clear_image(image,
                color_utils::color_for_target(color,
                  ColorTarget(
                    ColorTarget::BackgroundLayer,
                    sprite->pixelFormat(),
                    sprite->transparentColor())));

              set_current_palette(&oldPal, false);
            }
          }

          // Show the sprite to the user
          std::unique_ptr<Document> doc(new Document(sprite.get()));
          sprite.release();
          sprintf(buf, "Sprite-%04d", ++_sprite_counter);
          doc->setFilename(buf);
          doc->setContext(context);
          doc.release();
        }
      }
    });
  }
}

Command* CommandFactory::createNewFileCommand()
{
  return new NewFileCommand;
}

} // namespace app
