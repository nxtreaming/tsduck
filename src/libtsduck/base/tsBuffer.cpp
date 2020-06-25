//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#include "tsBuffer.h"
#include "tsFatal.h"
TSDUCK_SOURCE;

#if defined(TS_NEED_STATIC_CONST_DEFINITIONS)
const size_t ts::Buffer::DEFAULT_SIZE;
const size_t ts::Buffer::MINIMUM_SIZE;
#endif


//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

ts::Buffer::~Buffer()
{
    if (_allocated && _buffer != nullptr) {
        delete[] _buffer;
    }
    _buffer = nullptr;
    _buffer_size = 0;
    _buffer_max = 0;
}


//----------------------------------------------------------------------------
// Check the validity of a buffer.
//----------------------------------------------------------------------------

bool ts::Buffer::valid() const
{
    assert(_state.rbyte <= _state.wbyte);
    assert(_buffer_max <= _buffer_size);
    assert(_state.wbyte <= _buffer_max);
    assert(_state.wbyte < _buffer_max || _state.wbit == 0);
    assert(8 * _state.rbyte + _state.rbit <= 8 * _state.wbyte + _state.wbit);
    return _buffer != nullptr;
}


//----------------------------------------------------------------------------
// Read/write state constructor.
//----------------------------------------------------------------------------

ts::Buffer::RWState::RWState() :
    rbyte(0),
    wbyte(0),
    rbit(0),
    wbit(0)
{
}


//----------------------------------------------------------------------------
// Default constructor
//----------------------------------------------------------------------------

ts::Buffer::Buffer(size_t size) :
    _buffer(new uint8_t[size]),
    _buffer_size(size),
    _buffer_max(size),
    _read_only(false),
    _allocated(true),
    _big_endian(true),
    _read_error(false),
    _write_error(false),
    _state(),
    _saved_max(),
    _saved_states()
{
    CheckNonNull(_buffer);
}


//----------------------------------------------------------------------------
// Reset the buffer using an internal buffer.
//----------------------------------------------------------------------------

void ts::Buffer::reset(size_t size)
{
    // Deallocate previous local resources.
    if (_allocated && _buffer != nullptr && _buffer_size < size) {
        delete[] _buffer;
        _buffer = nullptr;
        _buffer_size = 0;
        _buffer_max = 0;
    }

    // Allocate the new buffer.
    if (!_allocated || _buffer == nullptr) {
        _buffer = new uint8_t[size];
        _buffer_size = size;
        _buffer_max = size;
        CheckNonNull(_buffer);
    }

    // Reset other properties.
    _read_only = false;
    _allocated = true;
    _read_error = false;
    _write_error = false;
    _state.rbyte = 0;
    _state.rbit = 0;
    _state.wbyte = 0;
    _state.wbit = 0;
    _saved_max.clear();
    _saved_states.clear();
}


//----------------------------------------------------------------------------
// Constructor using an external memory area.
//----------------------------------------------------------------------------

ts::Buffer::Buffer(void* data, size_t size, bool read_only) :
    _buffer(reinterpret_cast<uint8_t*>(data)),
    _buffer_size(size),
    _buffer_max(size),
    _read_only(read_only),
    _allocated(false),
    _big_endian(true),
    _read_error(false),
    _write_error(false),
    _state(),
    _saved_max(),
    _saved_states()
{
    if (_read_only) {
        _state.wbyte = _buffer_size;
    }
}


//----------------------------------------------------------------------------
// Reset the buffer using an external memory area .
//----------------------------------------------------------------------------

void ts::Buffer::reset(void* data, size_t size, bool read_only)
{
    // Deallocate previous local resources.
    if (_allocated && _buffer != nullptr) {
        delete[] _buffer;
    }

    // Point to external buffer.
    _buffer = reinterpret_cast<uint8_t*>(data);
    _buffer_size = size;
    _buffer_max = size;
    _read_only = read_only;
    _allocated = false;

    // Reset other properties.
    _read_error = false;
    _write_error = false;
    _state.rbyte = 0;
    _state.rbit = 0;
    _state.wbyte = _read_only ? _buffer_size : 0;
    _state.wbit = 0;
    _saved_max.clear();
    _saved_states.clear();
}


//----------------------------------------------------------------------------
// Constructor using a read-only external memory area.
//----------------------------------------------------------------------------

ts::Buffer::Buffer(const void* data, size_t size) :
    _buffer(reinterpret_cast<uint8_t*>(const_cast<void*>(data))),
    _buffer_size(size),
    _buffer_max(size),
    _read_only(true),
    _allocated(false),
    _big_endian(true),
    _read_error(false),
    _write_error(false),
    _state(),
    _saved_max(),
    _saved_states()
{
    _state.wbyte = _buffer_size;
}


//----------------------------------------------------------------------------
// Reset the buffer using a read-only external memory area.
//----------------------------------------------------------------------------

void ts::Buffer::reset(const void* data, size_t size)
{
    // Deallocate previous local resources.
    if (_allocated && _buffer != nullptr) {
        delete[] _buffer;
    }

    // Point to external buffer.
    _buffer = reinterpret_cast<uint8_t*>(const_cast<void*>(data));
    _buffer_size = size;
    _buffer_max = size;
    _read_only = true;
    _allocated = false;

    // Reset other properties.
    _read_error = false;
    _write_error = false;
    _state.rbyte = 0;
    _state.rbit = 0;
    _state.wbyte = _buffer_size;
    _state.wbit = 0;
    _saved_max.clear();
    _saved_states.clear();
}


//----------------------------------------------------------------------------
// Copy constructor
//----------------------------------------------------------------------------

ts::Buffer::Buffer(const Buffer& other) :
    _buffer(other._buffer), // adjusted later
    _buffer_size(other._buffer_size),
    _buffer_max(other._buffer_max),
    _read_only(other._read_only),
    _allocated(other._allocated),
    _big_endian(other._big_endian),
    _read_error(other._read_error),
    _write_error(other._write_error),
    _state(other._state),
    _saved_max(other._saved_max),
    _saved_states(other._saved_states)
{
    if (_buffer != nullptr && _allocated) {
        // Private internal buffer, copy resources.
        _buffer = new uint8_t[_buffer_size];
        CheckNonNull(_buffer);
        ::memcpy(_buffer, other._buffer, _buffer_size);
    }
}


//----------------------------------------------------------------------------
// Move constructor
//----------------------------------------------------------------------------

ts::Buffer::Buffer(Buffer&& other) :
    _buffer(other._buffer),
    _buffer_size(other._buffer_size),
    _buffer_max(other._buffer_max),
    _read_only(other._read_only),
    _allocated(other._allocated),
    _big_endian(other._big_endian),
    _read_error(other._read_error),
    _write_error(other._write_error),
    _state(other._state),
    _saved_max(std::move(other._saved_max)),
    _saved_states(std::move(other._saved_states))
{
    // Clear state of moved buffer.
    other._buffer = nullptr;
    other._buffer_size = 0;
    other._buffer_max = 0;
}


//----------------------------------------------------------------------------
// Assignment operator.
//----------------------------------------------------------------------------

ts::Buffer& ts::Buffer::operator=(const Buffer& other)
{
    if (&other != this) {
        // Deallocate previous local resources.
        if (_allocated && _buffer != nullptr) {
            delete[] _buffer;
        }

        // Copy buffer properties.
        _buffer = other._buffer; // adjusted later
        _buffer_size = other._buffer_size;
        _buffer_max = other._buffer_max;
        _read_only = other._read_only;
        _allocated = other._allocated;
        _big_endian = other._big_endian;
        _read_error = other._read_error;
        _write_error = other._write_error;
        _state = other._state;
        _saved_max = other._saved_max;
        _saved_states = other._saved_states;

        // Process buffer content.
        if (_buffer != nullptr && _allocated) {
            // Private internal buffer, copy resources.
            _buffer = new uint8_t[_buffer_size];
            CheckNonNull(_buffer);
            ::memcpy(_buffer, other._buffer, _buffer_size);
        }
    }
    return *this;
}


//----------------------------------------------------------------------------
// Move-assignment operator.
//----------------------------------------------------------------------------

ts::Buffer& ts::Buffer::operator=(Buffer&& other)
{
    if (&other != this) {
        // Deallocate previous local resources.
        if (_allocated && _buffer != nullptr) {
            delete[] _buffer;
        }

        // Move resources between buffers.
        _buffer = other._buffer;
        _buffer_size = other._buffer_size;
        _buffer_max = other._buffer_max;
        _read_only = other._read_only;
        _allocated = other._allocated;
        _big_endian = other._big_endian;
        _read_error = other._read_error;
        _write_error = other._write_error;
        _state = other._state;
        _saved_max = std::move(other._saved_max);
        _saved_states = std::move(other._saved_states);

        // Clear state of moved buffer.
        other._buffer = nullptr;
        other._buffer_max = 0;
        other._buffer_size = 0;
    }
    return *this;
}


//----------------------------------------------------------------------------
// Push/pop the current state of the read/write streams.
//----------------------------------------------------------------------------

size_t ts::Buffer::pushReadWriteState()
{
    _saved_states.push_back(_state);
    return _saved_states.size() - 1;
}

bool ts::Buffer::popReadWriteState(size_t level)
{
    if (!_saved_states.empty() && level == NPOS) {
        _state = _saved_states.back();
        _saved_states.pop_back();
        return true;
    }
    else if (level >= _saved_states.size()) {
        return false;
    }
    else {
        _state = _saved_states.at(level);
        _saved_states.resize(level);
        return true;
    }
}

bool ts::Buffer::dropReadWriteState(size_t level)
{
    if (!_saved_states.empty() && level == NPOS) {
        _saved_states.pop_back();
        return true;
    }
    else if (level >= _saved_states.size()) {
        return false;
    }
    else {
        _saved_states.resize(level);
        return true;
    }
}


//----------------------------------------------------------------------------
// Set/push/pop the buffer size.
//----------------------------------------------------------------------------

bool ts::Buffer::resize(size_t size, bool reallocate)
{
    // Get the max write pointer in saved values.
    size_t new_size = _state.wbyte + (_state.wbit + 7) / 8;
    for (auto it = _saved_states.begin(); it != _saved_states.end(); ++it) {
        new_size = std::max<size_t>(new_size, it->wbyte + (it->wbit + 7) / 8);
    }
    assert(new_size <= _buffer_size);

    // We need at least the largest saved write pointer.
    new_size = std::max(new_size, size);

    // Reallocate (enlarge or shrink) if necessary.
    if (reallocate && _allocated && new_size != _buffer_size) {
        uint8_t* new_buffer = new uint8_t[new_size];
        CheckNonNull(new_buffer);
        if (_buffer != nullptr) {
            ::memcpy(new_buffer, _buffer, std::min(_buffer_size, new_size));
            delete[] _buffer;
        }
        _buffer = new_buffer;
        _buffer_size = new_size;
    }

    // We accept at most the physical buffer size.
    _buffer_max = std::min(new_size, _buffer_size);

    // Return success only if the requested size was granted.
    return size == _buffer_max;
}


//----------------------------------------------------------------------------
// Push/pop the buffer size.
//----------------------------------------------------------------------------

size_t ts::Buffer::pushSize(size_t size)
{
    _saved_max.push_back(_buffer_max);
    resize(size, false);
    return _saved_states.size() - 1;
}

bool ts::Buffer::popSize(size_t level)
{
    size_t size = 0;
    if (!_saved_max.empty() && level == NPOS) {
        size = _saved_max.back();
        _saved_max.pop_back();
    }
    else if (level >= _saved_max.size()) {
        return false;
    }
    else {
        size = _saved_max.at(level);
        _saved_max.resize(level);
    }
    return resize(size, size > _buffer_size);
}

bool ts::Buffer::dropSize(size_t level)
{
    if (!_saved_max.empty() && level == NPOS) {
        _saved_max.pop_back();
        return true;
    }
    else if (level >= _saved_max.size()) {
        return false;
    }
    else {
        _saved_max.resize(level);
        return true;
    }
}


//----------------------------------------------------------------------------
// Align the read or write pointer to the next byte boundary.
//----------------------------------------------------------------------------

ts::Buffer& ts::Buffer::readAlignByte()
{
    assert(_state.rbyte <= _state.wbyte);

    if (_state.rbit != 0) {
        if (_state.rbyte == _state.wbyte) {
            // Would go beyond write pointer
            _read_error = true;
        }
        else {
            _state.rbyte++;
            _state.rbit = 0;
        }
    }
    return *this;
}

ts::Buffer& ts::Buffer::writeAlignByte(int stuffing)
{
    assert(_buffer != nullptr);
    assert(_state.wbyte <= _buffer_max);
    assert(_state.wbyte < _buffer_max || _state.wbit == 0);

    if (_state.wbit != 0) {
        // Build a mask for partial byte ('1' in bits to overwrite).
        const uint8_t mask = _big_endian ? (0xFF >> _state.wbit) : uint8_t(0xFF << _state.wbit);
        if (stuffing == 0) {
            // Clear skipped bits.
            _buffer[_state.wbyte] &= ~mask;
        }
        else {
            // Set skipped bits.
            _buffer[_state.wbyte] |= mask;
        }
        _state.wbyte++;
        _state.wbit = 0;
    }
    return *this;
}


//----------------------------------------------------------------------------
// Reset reading or writing at the specified offset in the buffer.
//----------------------------------------------------------------------------

bool ts::Buffer::readSeek(size_t byte, size_t bit)
{
    assert(_state.rbyte <= _state.wbyte);

    // Forbid invalid values.
    if (bit > 7) {
        _read_error = true;
        return false;
    }

    // Forbid seeking beyond write pointer.
    if (byte > _state.wbyte || (byte == _state.wbyte && bit > _state.wbit)) {
        // Move to end of stream.
        _state.rbyte = _state.wbyte;
        _state.rbit = _state.wbit;
        _read_error = true;
        return false;
    }

    // Set read position.
    _state.rbyte = byte;
    _state.rbit = bit;
    return true;
}

bool ts::Buffer::writeSeek(size_t byte, size_t bit, uint8_t stuffing)
{
    assert(_state.rbyte <= _state.wbyte);
    assert(_buffer_max <= _buffer_size);
    assert(_state.wbyte <= _buffer_max);
    assert(_state.wbyte < _buffer_max || _state.wbit == 0);

    // Forbid invalid values.
    if (bit > 7) {
        _write_error = true;
        return false;
    }

    // Forbid seeking beyond read pointer.
    if (byte < _state.rbyte || (byte == _state.rbyte && bit < _state.rbit)) {
        // Move at read point, cannot got backward.
        _state.wbyte = _state.rbyte;
        _state.wbit = _state.rbit;
        _write_error = true;
        return false;
    }

    // Forbid seeking beyond end of buffer.
    if (byte > _buffer_max || (byte == _buffer_max && bit > 0)) {
        // File end of buffer with stuffing.
        writeAlignByte(stuffing);
        ::memset(_buffer + _state.wbyte, stuffing, _buffer_max - _state.wbyte);
        // Move to end of buffer.
        _state.wbyte = _buffer_max;
        _state.wbit = 0;
        _write_error = true;
        return false;
    }

    // If we seek forward, trash memory with stuffing bytes.
    if (byte == _state.wbyte && bit > _state.wbit) {
        // Only trash a few bits. In fact, trash the rest of the byte because the end is meaningless.
        const uint8_t mask = _big_endian ? (0xFF >> _state.wbit) : uint8_t(0xFF << _state.wbit);
        if (stuffing == 0) {
            // Clear trashed bits.
            _buffer[_state.wbyte] &= ~mask;
        }
        else {
            // Set trashed bits.
            _buffer[_state.wbyte] |= mask;
        }
    }
    else if (byte > _state.wbyte) {
        // Trash current partial byte and beyond.
        writeAlignByte(stuffing);
        ::memset(_buffer + _state.wbyte, stuffing, byte - _state.wbyte + (bit > 0 ? 1 : 0));
    }

    // Set write position.
    _state.wbyte = byte;
    _state.wbit = bit;
    return true;
}


//----------------------------------------------------------------------------
// Get positions and remaining space.
//----------------------------------------------------------------------------

size_t ts::Buffer::remainingReadBytes() const
{
    assert(_state.wbyte >= _state.rbyte);
    return _state.wbyte - _state.rbyte;
}

size_t ts::Buffer::remainingReadBits() const
{
    const size_t wpos = currentWriteBitOffset();
    const size_t rpos = currentReadBitOffset();
    assert(wpos >= rpos);
    return wpos - rpos;
}

size_t ts::Buffer::remainingWriteBytes() const
{
    assert(_buffer_max >= _state.wbyte);
    return _buffer_max - _state.wbyte; // ignore bit offset
}

size_t ts::Buffer::remainingWriteBits() const
{
    assert(_buffer_max > _state.wbyte || (_buffer_max == _state.wbyte && _state.wbit == 0));
    return 8 * (_buffer_max - _state.wbyte) - _state.wbit;
}


//----------------------------------------------------------------------------
// Skip read bits/bytes backward and forward.
//----------------------------------------------------------------------------

ts::Buffer& ts::Buffer::skipBytes(size_t bytes)
{
    if (_state.rbyte + bytes > _state.wbyte) {
        _read_error = true;
    }
    _state.rbyte = std::min(_state.rbyte + bytes, _state.wbyte);
    _state.rbit = 0;
    return *this;
}

ts::Buffer& ts::Buffer::skipBits(size_t bits)
{
    size_t rpos = 8 * _state.rbyte + _state.rbit + bits;
    size_t wpos = 8 * _state.wbyte + _state.wbit;
    if (rpos > wpos) {
        _read_error = true;
        rpos = wpos;
    }
    _state.rbyte = rpos >> 3;
    _state.rbit = rpos & 7;
    return *this;
}

ts::Buffer& ts::Buffer::backBytes(size_t bytes)
{
    if (bytes > _state.rbyte) {
        _read_error = true;
        bytes = _state.rbyte;
    }
    _state.rbyte -= bytes;
    _state.rbit = 0;
    return *this;
}

ts::Buffer& ts::Buffer::backBits(size_t bits)
{
    size_t rpos = 8 * _state.rbyte + _state.rbit + bits;
    if (bits > rpos) {
        _read_error = true;
        bits = rpos;
    }
    rpos -= bits;
    _state.rbyte = rpos >> 3;
    _state.rbit = rpos & 7;
    return *this;
}


//----------------------------------------------------------------------------
// Read the next bit and advance the bitstream pointer.
//----------------------------------------------------------------------------

uint8_t ts::Buffer::readBit(uint8_t def)
{
    if (endOfRead()) {
        _read_error = true;
        return def;
    }

    assert(_state.rbyte < _buffer_size);
    assert(_state.rbyte <= _state.wbyte);
    assert(_state.rbit < 8);

    const uint8_t bit = (_buffer[_state.rbyte] >> (_big_endian ? (7 - _state.rbit) : _state.rbit)) & 0x01;
    if (++_state.rbit > 7) {
        _state.rbyte++;
        _state.rbit = 0;
    }
    return bit;
}
