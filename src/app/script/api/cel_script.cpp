#include "cel_script.h"

CelScriptObject::CelScriptObject() {
    addProperty("x",
                [this]{return m_cel->x();},
                [this](int x){m_cel->setPosition(x, m_cel->y()); return x;});
    addProperty("y",
                [this]{return m_cel->y();},
                [this](int y){m_cel->setPosition(m_cel->x(), y); return y;});
    addProperty("image", [this]{return m_image.get();});
    addProperty("frame", [this]{return m_cel->frame();});
    addMethod("setPosition", &CelScriptObject::setPosition);
}

void CelScriptObject::setPosition(int x, int y) {
    m_cel->setPosition(x, y);
}

void* CelScriptObject::getWrapped() {
    return m_cel;
}

void CelScriptObject::setWrapped(void* cel) {
    m_cel = static_cast<doc::Cel*>(cel);
    auto image = m_cel->image();
    if (!image) {
      auto sprite = m_cel->sprite();
      doc::ImageRef imgref(doc::Image::create(sprite->pixelFormat(), sprite->width(), sprite->height()));
      m_cel->data()->setImage(imgref);
    }
    m_image->setWrapped(image);
}

