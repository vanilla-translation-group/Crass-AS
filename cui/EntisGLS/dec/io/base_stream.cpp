// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "base_stream.h"
#include <memory>

using namespace au;
using namespace au::io;

BaseStream::~BaseStream() {}

uoff_t BaseStream::left() const
{
    return size() - pos();
}

io::BaseStream &BaseStream::skip(const soff_t offset)
{
    return seek(pos() + offset);
}

io::BaseStream &BaseStream::peek(
    const uoff_t offset, const std::function<void()> &func)
{
    const auto old_pos = pos();
    seek(offset);
    func();
    seek(old_pos);
    return *this;
}
