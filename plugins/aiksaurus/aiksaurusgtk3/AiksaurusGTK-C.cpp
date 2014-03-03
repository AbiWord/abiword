/*
 * AiksaurusGTK - A GTK interface to the Aiksaurus library
 * Copyright (C) 2001 by Jared Davis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "AiksaurusGTK-C.h"
#include "AiksaurusGTK.h"
#include <new>

static AiksaurusGTK *instance = 0;

int AiksaurusGTK_init()
{
    if (!instance)
        instance = new(std::nothrow) AiksaurusGTK;

    return (instance != 0);
}


void AiksaurusGTK_destroy()
{
    delete instance;
    instance = 0;
}


void AiksaurusGTK_setTitle(const char* title)
{
    if (instance)
        instance->setTitle(title);
}


void AiksaurusGTK_setInitialMessage(const char* message)
{
    if (instance)
        instance->setInitialMessage(message);
}


void AiksaurusGTK_showReplacebar()
{
    if (instance)
        instance->showReplacebar();
}


void AiksaurusGTK_hideReplacebar()
{
    if (instance)
        instance->hideReplacebar();
}

const char* AiksaurusGTK_runThesaurus(const char* word)
{
    return (instance) ? (instance->runThesaurus(word)) : (word);
}

