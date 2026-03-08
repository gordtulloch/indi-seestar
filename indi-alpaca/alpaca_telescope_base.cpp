/*
    INDI alpaca Telescope Base Library
    
    Copyright (C) 2026 Gord Tulloch
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

// This file exists to provide the alpaca telescope implementation as a library
// that can be linked by derived drivers. It includes the main implementation
// but NOT the INDI entry points or singleton instance.

#define INDI_ALPACA_BASE_LIBRARY
#include "indi_alpaca_telescope.cpp"
