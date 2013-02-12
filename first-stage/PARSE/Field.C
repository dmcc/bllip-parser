/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "Field.h"

Field::
Field( int length, const char mask[] )
:fragmentation_( length ),
 mask_( mask )
{
    if( fragmentation_ <= 0 )
	error( "fragmentation <= 0" );
}

bool
Field::
in( int integer ) const
{
    if( integer < fragmentation_ )
    {
	if( integer < 0 )
	    error( "Field given integer < 0" );
	return bool( mask_[ integer ] );
    }
    else
    {
	int	index = integer % fragmentation_;
	return bool( mask_[ index ] );
    }
}
