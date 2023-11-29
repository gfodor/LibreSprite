// Aseprite Document Library
// Copyright (c) 2001-2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/cel.h"

#include "gfx/rect.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/sprite.h"

namespace doc {

Cel::Cel(frame_t frame, const ImageRef& image)
  : Object(ObjectType::Cel)
  , m_layer(NULL)
  , m_frame(frame)
  , m_data(new CelData(image))
{
}

Cel::Cel(frame_t frame, const CelDataRef& celData)
  : Object(ObjectType::Cel)
  , m_layer(NULL)
  , m_frame(frame)
  , m_data(celData)
{
}

// static
std::shared_ptr<Cel> Cel::createCopy(std::shared_ptr<const Cel> other)
{
  auto cel = std::make_shared<Cel>(other->frame(), ImageRef(Image::createCopy(other->image())));
  cel->setPosition(other->position());
  cel->setOpacity(other->opacity());
  return cel;
}

// static
std::shared_ptr<Cel> Cel::createLink(std::shared_ptr<const Cel> other)
{
  return std::make_shared<Cel>(other->frame(), other->dataRef());
}

void Cel::setFrame(frame_t frame)
{
  ASSERT(m_layer == NULL);
  m_frame = frame;
}

void Cel::setDataRef(const CelDataRef& celData)
{
  ASSERT(celData);
  m_data = celData;
}

void Cel::setPosition(int x, int y, bool update_t)
{
  setPosition(gfx::Point(x, y), update_t);
}

void Cel::setPosition(const gfx::Point& pos, bool update_t)
{
  m_data->setPosition(pos, update_t);
}

void Cel::setOpacity(int opacity)
{
  m_data->setOpacity(opacity);
}

Document* Cel::document() const
{
  ASSERT(m_layer);
  if (m_layer && m_layer->sprite())
    return m_layer->sprite()->document();
  else
    return NULL;
}

Sprite* Cel::sprite() const
{
  ASSERT(m_layer);
  if (m_layer)
    return m_layer->sprite();
  else
    return NULL;
}

std::shared_ptr<Cel> Cel::link() const
{
  ASSERT(m_data);
  if (m_data.get() == NULL)
    return NULL;

  if (!m_data.unique()) {
    for (frame_t fr=0; fr<m_frame; ++fr) {
      auto possible = m_layer->cel(fr);
      if (possible && possible->dataRef().get() == m_data.get())
        return possible;
    }
  }

  return NULL;
}

std::size_t Cel::links() const
{
  std::size_t links = 0;

  Sprite* sprite = this->sprite();
  for (frame_t fr=0; fr<sprite->totalFrames(); ++fr) {
    auto cel = m_layer->cel(fr);
    if (cel && cel.get() != this && cel->dataRef().get() == m_data.get())
      ++links;
  }

  return links;
}

gfx::Rect Cel::bounds() const
{
  auto image = this->image();
  ASSERT(image);
  if (image)
    return gfx::Rect(
      position().x, position().y,
      image->width(), image->height());
  else
    return gfx::Rect();
}

void Cel::setParentLayer(LayerImage* layer)
{
  m_layer = layer;
  fixupImage();
}

void Cel::fixupImage()
{
  // Change the mask color to the sprite mask color
  if (m_layer && image())
    image()->setMaskColor(m_layer->sprite()->transparentColor());
}

} // namespace doc
