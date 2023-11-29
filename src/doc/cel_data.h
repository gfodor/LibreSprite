// Aseprite Document Library
// Copyright (c) 2001-2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/shared_ptr.h"
#include "doc/image_ref.h"
#include "doc/object.h"
#include "doc/with_user_data.h"

namespace doc {

  class CelData : public WithUserData {
  public:
    CelData(const ImageRef& image);
    CelData(const CelData& celData);

    const gfx::Point& position() const { return m_position; }
    const uint32_t position_t() const { return m_position_t; }
    void setPosition_t(uint32_t t) { m_position_t = t; }

    int opacity() const { return m_opacity; }
    const uint32_t opacity_t() const { return m_opacity_t; }
    void setOpacity_t(uint32_t t) { m_opacity_t = t; }

    Image* image() const { return const_cast<Image*>(m_image.get()); };
    ImageRef imageRef() const { return m_image; }

    void setImage(const ImageRef& image);
    void setPosition(int x, int y, bool update_t = false) {
      if (update_t)
        m_position_t = get_new_t(m_position_t, m_position.x != x || m_position.y != y);

      m_position.x = x;
      m_position.y = y;
    }
    void setPosition(const gfx::Point& pos, bool update_t = false) { 
      if (update_t)
        m_position_t = get_new_t(m_position_t, m_position.x != pos.x || m_position.y != pos.y);

      m_position = pos;
    }
    void setOpacity(int opacity) {
      m_opacity_t = get_new_t(m_opacity_t, m_opacity != opacity);
      m_opacity = opacity;
    }

    virtual int getMemSize() const override {
      ASSERT(m_image);
      return sizeof(CelData) + m_image->getMemSize();
    }

  private:
    ImageRef m_image;
    gfx::Point m_position;      // X/Y screen position
    int m_opacity;              // Opacity level
    uint32_t m_position_t = 0;      // X/Y screen position changed at t
    uint32_t m_opacity_t = 0;       // Opacity level changed at t
  };

  typedef base::SharedPtr<CelData> CelDataRef;

} // namespace doc
