// Aseprite
// Copyright (C) 2001-2016  David Capello
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/add_cel.h"

#include "base/serialization.h"
#include "doc/cel.h"
#include "doc/cel_io.h"
#include "doc/cel_data_io.h"
#include "doc/document.h"
#include "doc/document_event.h"
#include "doc/image_io.h"
#include "doc/layer.h"
#include "doc/subobjects_io.h"

namespace app {
namespace cmd {

using namespace base::serialization;
using namespace base::serialization::little_endian;
using namespace doc;

AddCel::AddCel(Layer* layer, std::shared_ptr<Cel> cel)
  : WithLayer(layer)
  , WithCel(cel)
  , m_size(0)
{
}

void AddCel::onExecute()
{
  Layer* layer = this->layer();
  auto cel = this->cel();

  addCel(layer, cel);
}

void AddCel::onUndo()
{
  Layer* layer = this->layer();
  auto cel = this->cel();

  // Save the CelData only if the cel isn't linked
  bool has_data = (cel->links() == 0);
  write8(m_stream, has_data ? 1: 0);
  if (has_data) {
    write_image(m_stream, cel->image());
    write_celdata(m_stream, cel->data());
  }
  write_cel(m_stream, cel.get());
  m_size = size_t(m_stream.tellp());

  removeCel(layer, cel);
}

void AddCel::onRedo()
{
  Layer* layer = this->layer();

  SubObjectsFromSprite io(layer->sprite());
  bool has_data = (read8(m_stream) != 0);
  if (has_data) {
    ImageRef image(read_image(m_stream));
    Image *imagePtr = image.get();

    if (imagePtr->pixelFormat() == IMAGE_TRGB) {
      // Set all pixels in cel to current t
      const int w = image->width();
      const int h = image->height();
      uint32_t min_t = trgba_get_current_t();

      for (int y=0; y<h; ++y) {
        TrgbTraits::address_t addr = (TrgbTraits::address_t)imagePtr->getPixelAddress(0, y);
        for (int x=0; x<w; ++x, ++addr) {
          *addr = trgba_with_adjusted_t(0, *addr, min_t);
        }
      }
    }

    io.addImageRef(image);

    // TODO trgba update t

    CelDataRef celdata(read_celdata(m_stream, &io));
    io.addCelDataRef(celdata);
  }

  std::shared_ptr<Cel> cel{read_cel(m_stream, &io)};
  addCel(layer, cel);

  m_stream.str(std::string());
  m_stream.clear();
  m_size = 0;
}

void AddCel::addCel(Layer* layer, std::shared_ptr<Cel> cel)
{
  Image *image = cel->image();

  if (image->pixelFormat() == IMAGE_TRGB) {
    // Set all pixels in cel to current t
    const int w = image->width();
    const int h = image->height();
    uint32_t min_t = trgba_get_current_t();

    for (int y=0; y<h; ++y) {
      TrgbTraits::address_t addr = (TrgbTraits::address_t)image->getPixelAddress(0, y);
      for (int x=0; x<w; ++x, ++addr) {
        *addr = trgba_with_adjusted_t(0, *addr, min_t);
      }
    }
  }

  static_cast<LayerImage*>(layer)->addCel(cel);
  layer->incrementVersion();

  Document* doc = cel->document();
  DocumentEvent ev(doc);
  ev.sprite(layer->sprite());
  ev.layer(layer);
  ev.cel(cel);
  doc->notifyObservers<DocumentEvent&>(&DocumentObserver::onAddCel, ev);
}

void AddCel::removeCel(Layer* layer, std::shared_ptr<Cel> cel)
{
  Document* doc = cel->document();
  DocumentEvent ev(doc);
  ev.sprite(layer->sprite());
  ev.layer(layer);
  ev.cel(cel);
  doc->notifyObservers<DocumentEvent&>(&DocumentObserver::onRemoveCel, ev);

  static_cast<LayerImage*>(layer)->removeCel(cel);
  layer->incrementVersion();
}

} // namespace cmd
} // namespace app
