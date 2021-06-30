/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <string_view>

namespace Mayo {
namespace IO {

// Predefined I/O formats
enum Format {
    Format_Unknown,
    Format_AMF,
    Format_GLTF,
    Format_IGES,
    Format_OBJ,
    Format_OCCBREP,
    Format_STEP,
    Format_STL,
    Format_VRML,
    Format_DXF,
    //
    Format_3DS,
    Format_3MF,
    Format_COLLADA,
    Format_FBX,
    Format_IFC,
    Format_OFF,
    Format_PLY,
    Format_X3D
};

// Returns identifier(unique short name) corresponding to 'format'
std::string_view formatIdentifier(Format format);

// Returns name(eg ISO designation) corresponding to 'format'
std::string_view formatName(Format format);

// Returns array of applicable file suffixes(extensions) corresponding to 'format'
Span<std::string_view> formatFileSuffixes(Format format);

// Does 'format' provide BRep model ?
bool formatProvidesBRep(Format format);

// Does 'format' provide mesh model ?
bool formatProvidesMesh(Format format);

} // namespace IO
} // namespace Mayo
