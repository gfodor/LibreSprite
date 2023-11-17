// Aseprite UI Library
// Copyright (C) 2001-2016  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace ui {

    // Message types.
    enum MessageType {
      // General messages.
      kOpenMessage = 1,           // Windows is open.
      kCloseMessage = 2,          // Windows is closed.
      kCloseDisplayMessage = 3,   // The user wants to close the entire application.
      kResizeDisplayMessage = 4,  // Resize display message.
      kPaintMessage = 5,          // Widget needs to be repainted.
      kTimerMessage = 6,          // A timer timeout.
      kDropFilesMessage = 7,      // Drop files in the manager.
      kWinMoveMessage = 8,        // Window movement.

      // Keyboard related messages.
      kKeyDownMessage = 9,        // When any key is pressed.
      kKeyUpMessage = 10,         // When any key is released.
      kFocusEnterMessage = 11,    // Widget gets the focus.
      kFocusLeaveMessage = 12,    // Widget loses the focus.

      // Mouse related messages.
      kMouseDownMessage = 13,     // User makes a click inside a widget.
      kMouseUpMessage = 14,       // User releases the mouse button in a widget.
      kDoubleClickMessage = 15,   // User makes a double click in some widget.
      kMouseEnterMessage = 16,    // A widget gets the mouse pointer.
      kMouseLeaveMessage = 17,    // A widget loses the mouse pointer.
      kMouseMoveMessage = 18,     // User moves the mouse on some widget.
      kSetCursorMessage = 19,     // A widget needs to set up the mouse cursor.
      kMouseWheelMessage = 20,    // User moves the wheel.

      // Touch related messages.
      kTouchMagnifyMessage = 21,

      // TODO Drag'n'drop messages...
      // k...DndMessage

      // User widgets.
      kFirstRegisteredMessage = 22,
      kLastRegisteredMessage = 0x7fffffff
  };


} // namespace ui
