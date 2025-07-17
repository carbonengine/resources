// Copyright © 2025 CCP ehf.

#pragma once
#ifndef Exports_H
#define Exports_H

#ifdef CARBON_RESOURCES_STATIC
#    define API
#else
#    ifdef _WIN32
#       ifdef EXPORT_LIBRARY
#           define API __declspec(dllexport)
#       else
#           define API __declspec(dllimport)
#       endif
#    else
#        ifdef EXPORT_LIBRARY
#            define API __attribute((visibility("default")))
#        else
#            define API
#        endif
#    endif
#endif

#endif // Exports_H