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
#if !defined(HAVE_NARY_ORDER_DECIDER)
#define HAVE_NARY_ORDER_DECIDER

#include "common.h"
#include "IR.h"
#include "abstraction_method_manager.h"
#include "nary_order_transformation_method.h"

BEGIN_C_NAMESPACE


struct _NARY_ORDER_DECIDER;
typedef struct _NARY_ORDER_DECIDER NARY_ORDER_DECIDER;

struct _NARY_ORDER_DECIDER {
    RET_VAL (*Decide)( ABSTRACTION_METHOD *method, CRITICAL_CONCENTRATION_INFO *info, double *criticalCons, int len );
};

NARY_ORDER_DECIDER *GetNaryOrderDeciderInstance( ABSTRACTION_METHOD_MANAGER *manager );

END_C_NAMESPACE

#endif
