// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
//
// This file is part of GtkMathView, a Gtk widget for MathML.
// 
// GtkMathView is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// GtkMathView is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GtkMathView; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// For details, see the GtkMathView World-Wide-Web page,
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#include "MathView_config.h"

#include <cassert>

#include <MathView/MathMLElement.hh>

#include "Abi_AreaFactory.h"
#include "Abi_MathGraphicDevice.h"

Abi_MathGraphicDevice::Abi_MathGraphicDevice()
  : abi_factory(Abi_AreaFactory::create())
{
  setFactory(abi_factory);

#if 0
  GObjectPtr<PangoContext> context = gtk_widget_create_pango_context(widget);
  SmartPtr<Abi_DefaultPangoShaper> defaultPangoShaper = Abi_DefaultPangoShaper::create();
  defaultPangoShaper->setPangoContext(context);
  getShaperManager()->registerShaper(defaultPangoShaper);

  getShaperManager()->registerShaper(SpaceShaper::create());

#if 1
  SmartPtr<Abi_PangoShaper> pangoShaper = Abi_PangoShaper::create();
  pangoShaper->setPangoContext(context);
  getShaperManager()->registerShaper(pangoShaper);
#else
  getShaperManager()->registerShaper(Abi_AdobeShaper::create());
#endif
#endif
}

Abi_MathGraphicDevice::~Abi_MathGraphicDevice()
{ }

