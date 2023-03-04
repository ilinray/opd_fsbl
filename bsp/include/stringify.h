/// @file
/// @brief stringification macros
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2019. All rights reserved.

#ifndef SCR_BSP_STRINGIFY_H
#define SCR_BSP_STRINGIFY_H

/// @bug Reserved identifiers
///@{
#define _TOSTR(s) __TOSTR(s)
#define __TOSTR(s) #s

#define _CONCAT(s1,s2) s1##s2
///@}

#endif // SCR_BSP_STRINGIFY_H
