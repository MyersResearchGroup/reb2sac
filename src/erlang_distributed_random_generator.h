/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_ERLANG_DISTRIBUTED_RANDOM_GENERATOR)
#define HAVE_ERLANG_DISTRIBUTED_RANDOM_GENERATOR


#include "common.h"
#include "random_number_generator.h"

BEGIN_C_NAMESPACE

/*
struct _ERLANG_DISTRIBUTED_RANDOM_GENERATOR;
typedef struct _ERLANG_DISTRIBUTED_RANDOM_GENERATOR  ERLANG_DISTRIBUTED_RANDOM_GENERATOR;


struct _ERLANG_DISTRIBUTED_RANDOM_GENERATOR {
    RANDOM_T (*GetNext)( ERLANG_DISTRIBUTED_RANDOM_GENERATOR *generator );            
    RET_VAL (*SetSeed)( ERLANG_DISTRIBUTED_RANDOM_GENERATOR *generator, long seed );
    RET_VAL (*SetInput)( ERLANG_DISTRIBUTED_RANDOM_GENERATOR *generator, double rate, long scale );
    long seed;
    double rate;
    long scale;
};

*/

END_C_NAMESPACE

#endif
