/* 
	*************************************************************************

	Macros.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef Macros_H
#define Macros_H

#define ATTEMPT_LOAD_MEMBER( structName, fromStruct, member )     \
	if( offsetof( struct structName, member ) < fromStruct.size ) \
	{                                                             \
		member = fromStruct.member;                               \
	}

#define MEMBER_IS_IN_STRUCT( structName, fromStruct, member )     \
	offsetof( struct structName, member ) < fromStruct.size 

#endif // Macros_H