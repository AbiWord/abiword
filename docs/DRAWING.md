Drawing in AbiWord
==================

Author: Hubert Figuière
Date: 9 October 2021

This document is written as the drawing code is being rearchitectured a bit.

Concept
-------

queueDraw() or invalidate() methods: these method invalidate the
drawing area (`GR_Graphics::queueDraw()` is the platform
implementation). This is needed on Gtk3 and Cocoa as the drawing can only done
in the proper context.

`drawImmediate` is the drawing code to be called in the drawing
context. In gtk3, this the "draw" signal (`GdkDrawingArea`) or the
`draw` class function `GtkWidget`. On Cocoa this is `-[NSView
drawRect:]`.

XAP implementation
------------------

Anything that is "drawable" should subclass `XAP_Drawable`. This
class define a few pure virtual functions.

Usually `queueDraw()` should just call `getGraphics().queueDraw()`.

Cocoa implementation
--------------------

The pattern in the Cocoa based macOS code is to set the `drawable`
property from the `XAP_CocoaNSView` instance.  On most dialogs this
view is the `preview` property, with the notable exception the the
styles dialog. The preview is usually created after the "GC".

In AP_Ruler the drawable is `this`.