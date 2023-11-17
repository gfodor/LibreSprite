// SHE library
// Copyright (C) 2016  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace she {

  // Source of a mouse like event
  enum PointerType {
    Unknown = 0,
    Mouse = 1,                      // A regular mouse
    Multitouch = 2,                 // Trackpad/multitouch surface
    Pen = 3,                        // Stylus pen
    Cursor = 4,                     // Puck like device
    Eraser = 5                      // Eraser end of a stylus pen
  };

} // namespace she
