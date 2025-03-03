/* 
	*************************************************************************

	Exports.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef Exports_H
#define Exports_H

#ifdef _WIN32
#    ifdef EXPORT_LIBRARY
#        define API __declspec(dllexport)
#    else
#        define API __declspec(dllimport)
#    endif
#elif
#    define API
#endif

#endif // Exports_H