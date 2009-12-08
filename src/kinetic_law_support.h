/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#if !defined(HAVE_KINETIC_LAW_SUPPORT)
#define HAVE_KINETIC_LAW_SUPPORT

#include "float.h"
#include "common.h"
#include "linked_list.h"
#include "species_node.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

typedef struct {
    LINKED_LIST *supportList;
} KINETIC_LAW_SUPPORT_ELEMENT;


struct _KINETIC_LAW_SUPPORT;
typedef struct _KINETIC_LAW_SUPPORT KINETIC_LAW_SUPPORT;

struct _KINETIC_LAW_SUPPORT {
    LINKED_LIST* (*Support)( KINETIC_LAW_SUPPORT *support, KINETIC_LAW *kineticLaw );       
};

KINETIC_LAW_SUPPORT *CreateKineticLawSupport();
RET_VAL FreeKineticLawSupport( KINETIC_LAW_SUPPORT **support );

END_C_NAMESPACE

#endif


