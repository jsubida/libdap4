// XDRStreamMarshaller.cc

// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: Patrick West <pwest@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>

#include "config.h"

#include <cassert>

#include <iostream>
#include <sstream>
#include <iomanip>

//#define DODS_DEBUG 1

#include "XDRStreamMarshaller.h"
#include "Vector.h"
#include "util.h"
#include "debug.h"

using namespace std;

namespace libdap {

char *XDRStreamMarshaller::d_buf = 0;

#define XDR_DAP_BUFF_SIZE 256

/** Build an instance of XDRStreamMarshaller. Bind the C++ stream out to this
 * instance. If the checksum parameter is true, initialize a checksum buffer
 * and enable the use of the reset_checksum() and get_checksum() methods.
 *
 * @param out Write to this stream object.
 * @param checksum If true, compute checksums. False by default
 * @param write_data If true, write data values. True by default
 */
XDRStreamMarshaller::XDRStreamMarshaller(ostream &out) :
    d_out(out), d_partial_put_element_count(0)
{
    if (!d_buf) d_buf = (char *) malloc(XDR_DAP_BUFF_SIZE);
    if (!d_buf) throw Error("Failed to allocate memory for data serialization.");

    xdrmem_create(&d_sink, d_buf, XDR_DAP_BUFF_SIZE, XDR_ENCODE);
}

XDRStreamMarshaller::XDRStreamMarshaller() :
    Marshaller(), d_out(cout), d_partial_put_element_count(0)
{
    throw InternalErr(__FILE__, __LINE__, "Default constructor not implemented.");
}

XDRStreamMarshaller::XDRStreamMarshaller(const XDRStreamMarshaller &m) :
    Marshaller(m), d_out(cout), d_partial_put_element_count(0)
{
    throw InternalErr(__FILE__, __LINE__, "Copy constructor not implemented.");
}

XDRStreamMarshaller &
XDRStreamMarshaller::operator=(const XDRStreamMarshaller &)
{
    throw InternalErr(__FILE__, __LINE__, "Copy operator not implemented.");

    return *this;
}

XDRStreamMarshaller::~XDRStreamMarshaller()
{
    xdr_destroy(&d_sink);
}

void XDRStreamMarshaller::put_byte(dods_byte val)
{
    DBG( std::cerr << "put_byte: " << val << std::endl );

    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send byte data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!xdr_char(&d_sink, (char *) &val))
        throw Error(
            "Network I/O Error. Could not send byte data.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send byte data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_int16(dods_int16 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send int 16 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!XDR_INT16(&d_sink, &val))
        throw Error(
            "Network I/O Error. Could not send int 16 data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send int 16 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_int32(dods_int32 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send int 32 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!XDR_INT32(&d_sink, &val))
        throw Error(
            "Network I/O Error. Culd not read int 32 data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send int 32 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_float32(dods_float32 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send float 32 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!xdr_float(&d_sink, &val))
        throw Error(
            "Network I/O Error. Could not send float 32 data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send float 32 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_float64(dods_float64 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send float 64 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!xdr_double(&d_sink, &val))
        throw Error(
            "Network I/O Error. Could not send float 64 data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send float 64 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_uint16(dods_uint16 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send uint 16 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!XDR_UINT16(&d_sink, &val))
        throw Error(
            "Network I/O Error. Could not send uint 16 data. This may be due to a\nbug in libdap or a problem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send uint 16 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_uint32(dods_uint32 val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send uint 32 data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!XDR_UINT32(&d_sink, &val))
        throw Error(
            "Network I/O Error. Could not send uint 32 data. This may be due to a\nbug in libdap or a problem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send uint 32 data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_str(const string &val)
{
    int size = val.length() + 8;

    XDR str_sink;
    vector<char> str_buf(size);
    try {
        xdrmem_create(&str_sink, &str_buf[0], size, XDR_ENCODE);

        if (!xdr_setpos(&str_sink, 0))
            throw Error(
                "Network I/O Error. Could not send string data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        const char *out_tmp = val.c_str();
        if (!xdr_string(&str_sink, (char **) &out_tmp, size))
            throw Error(
                "Network I/O Error. Could not send string data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

        unsigned int bytes_written = xdr_getpos(&str_sink);
        if (!bytes_written)
            throw Error(
                "Network I/O Error. Could not send string data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        d_out.write(&str_buf[0], bytes_written);

        xdr_destroy(&str_sink);
    }
    catch (...) {
        xdr_destroy(&str_sink);
        throw;
    }
}

void XDRStreamMarshaller::put_url(const string &val)
{
    put_str(val);
}

void XDRStreamMarshaller::put_opaque(char *val, unsigned int len)
{
    if (len > XDR_DAP_BUFF_SIZE)
        throw Error("Network I/O Error. Could not send opaque data - length of opaque data larger than allowed");

    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send opaque data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!xdr_opaque(&d_sink, val, len))
        throw Error(
            "Network I/O Error. Could not send opaque data.\nThis may be due to a bug in libdap, on the server or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send opaque data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_int(int val)
{
    if (!xdr_setpos(&d_sink, 0))
        throw Error(
            "Network I/O Error. Could not send int data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    if (!xdr_int(&d_sink, &val))
        throw Error(
            "Network I/O Error(1). Could not send int data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

    unsigned int bytes_written = xdr_getpos(&d_sink);
    if (!bytes_written)
        throw Error(
            "Network I/O Error. Could not send int data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

    d_out.write(d_buf, bytes_written);
}

void XDRStreamMarshaller::put_vector(char *val, int num, Vector &)
{
    if (!val) throw InternalErr(__FILE__, __LINE__, "Could not send byte vector data. Buffer pointer is not set.");

    // write the number of members of the array being written and then set the position to 0
    put_int(num);

    // this is the word boundary for writing xdr bytes in a vector.
    const unsigned int add_to = 8;

    vector<char> byte_buf(num + add_to);
    XDR byte_sink;
    try {
        xdrmem_create(&byte_sink, &byte_buf[0], num + add_to, XDR_ENCODE);
        if (!xdr_setpos(&byte_sink, 0))
            throw Error(
                "Network I/O Error. Could not send byte vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        if (!xdr_bytes(&byte_sink, (char **) &val, (unsigned int *) &num, num + add_to))
            throw Error(
                "Network I/O Error(2). Could not send byte vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

        unsigned int bytes_written = xdr_getpos(&byte_sink);
        if (!bytes_written)
            throw Error(
                "Network I/O Error. Could not send byte vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        d_out.write(&byte_buf[0], bytes_written);

        xdr_destroy(&byte_sink);
    }
    catch (...) {
        xdr_destroy(&byte_sink);
        throw;
    }
}

void XDRStreamMarshaller::put_vector(char *val, int num, int width, Vector &vec)
{
    put_vector(val, num, width, vec.var()->type());
}

/* private */
/**
 * Write elements of a Vector (i.e. an Array) to the stream using XDR encoding.
 * Encoding is performed on 'num' values that use 'width' bytes. The parameter
 * 'type' is used to choose the XDR encoding function.
 *
 * @param val Pointer to the values to write
 * @param num The number of elements in the memory referenced by 'val'
 * @param width The number of bytes in each element
 * @param type The DAP type of the elements
 */
void XDRStreamMarshaller::put_vector(char *val, unsigned int num, int width, Type type)
{
    if (!val) throw InternalErr(__FILE__, __LINE__, "Buffer pointer is not set.");

    // write the number of array members being written, then set the position back to 0
    put_int(num);

    int use_width = width;
    if (use_width < 4) use_width = 4;

    // the size is the number of elements num times the width of each
    // element, then add 4 bytes for the number of elements
    int size = (num * use_width) + 4;

    // allocate enough memory for the elements
    vector<char> vec_buf(size);
    XDR vec_sink;
    try {
        xdrmem_create(&vec_sink, &vec_buf[0], size, XDR_ENCODE);

        // set the position of the sink to 0, we're starting at the beginning
        if (!xdr_setpos(&vec_sink, 0))
            throw Error(
                "Network I/O Error. Could not send vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        // write the array to the buffer
        if (!xdr_array(&vec_sink, (char **) &val, (unsigned int *) &num, size, width, XDRUtils::xdr_coder(type)))
            throw Error(
                "Network I/O Error(2). Could not send vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

        // how much was written to the buffer
        unsigned int bytes_written = xdr_getpos(&vec_sink);
        if (!bytes_written)
            throw Error(
                "Network I/O Error. Could not send vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

        // write that much out to the output stream
        d_out.write(&vec_buf[0], bytes_written);

        xdr_destroy(&vec_sink);
    }
    catch (...) {
        xdr_destroy(&vec_sink);
        throw;
    }
}

void XDRStreamMarshaller::put_vector_size_prefix(int num)
{
    put_int(num);
    put_int(num);

    d_partial_put_element_count = 0;
}

void XDRStreamMarshaller::put_vector_part(char *val, unsigned int num, int width, Type /*type*/)
{
    if (width == 1) {
        // Add space for the 4 bytes of length info and 4 bytes for padding, even though
        // we will not send either of those.
        const unsigned int add_to = 8;

        vector<char> byte_buf(num + add_to);
        XDR byte_sink;
        try {
            xdrmem_create(&byte_sink, &byte_buf[0], num + add_to, XDR_ENCODE);
            if (!xdr_setpos(&byte_sink, 0))
                throw Error(
                    "Network I/O Error. Could not send byte vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            if (!xdr_bytes(&byte_sink, (char **) &val, (unsigned int *) &num, num + add_to))
                throw Error(
                    "Network I/O Error(2). Could not send byte vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

            unsigned int bytes_written = xdr_getpos(&byte_sink);
            if (!bytes_written)
                throw Error(
                    "Network I/O Error. Could not send byte vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // The bytes written minus the 4 length bytes should be no more than 3 more than num
            assert(bytes_written - num - 4 < 4);

            // Only send the num bytes that follow the 4 bytes of length info - we skip the
            // length info because it's already been sent and we don't send any trailing padding
            // bytes in this method (see put_vector_last() for that).
            unsigned int bytes_sent = d_out.write(&byte_buf[4], num);

            if (bytes_sent != num)
                throw Error ("Network I/O Error. Could not send byte vector data");

            // Now increment the element count so we can figure out about the padding in put_vector_last()
            d_partial_put_element_count += num;

            xdr_destroy(&byte_sink);
        }
        catch (...) {
            xdr_destroy(&byte_sink);
            throw;
        }
    }
    else {
#if 0
        // broken code jhrg 8/14/15
        int use_width = width;
        if (width < 4) use_width = 4;

        // the size is the number of elements num times the width of each
        // element, then add 4 bytes for the number of elements
        int size = (num * use_width) + 4;

        // allocate enough memory for the elements
        vector<char> vec_buf(size);
        XDR vec_sink;
        try {
            xdrmem_create(&vec_sink, &vec_buf[0], size, XDR_ENCODE);

            // set the position of the sink to 0, we're starting at the beginning
            if (!xdr_setpos(&vec_sink, 0))
                throw Error(
                    "Network I/O Error. Could not send vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // write the array to the buffer
            if (!xdr_array(&vec_sink, (char **) &val, (unsigned int *) &num, size, width, XDRUtils::xdr_coder(type)))
                throw Error(
                    "Network I/O Error(2). Could not send vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

            // how much was written to the buffer
            unsigned int bytes_written = xdr_getpos(&vec_sink);
            if (!bytes_written)
                throw Error(
                    "Network I/O Error. Could not send vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // write that much out to the output stream, skipping the length data that
            // XDR writes since we have already written the length info.
            d_out.write(&vec_buf[4], bytes_written - 4);

            xdr_destroy(&vec_sink);
        }
        catch (...) {
            xdr_destroy(&vec_sink);
            throw;
        }
#endif
    }
}

void XDRStreamMarshaller::put_vector_last(char *val, unsigned int num, int width, Type /*type*/)
{
    if (width == 1) {
        // this is the word boundary for writing xdr bytes in a vector.
        const unsigned int add_to = 8;

        vector<char> byte_buf(num + add_to);
        XDR byte_sink;
        try {
            xdrmem_create(&byte_sink, &byte_buf[0], num + add_to, XDR_ENCODE);
            if (!xdr_setpos(&byte_sink, 0))
                throw Error(
                    "Network I/O Error. Could not send byte vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            if (!xdr_bytes(&byte_sink, (char **) &val, (unsigned int *) &num, num + add_to))
                throw Error(
                    "Network I/O Error(2). Could not send byte vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

            unsigned int bytes_written = xdr_getpos(&byte_sink);
            if (!bytes_written)
                throw Error(
                    "Network I/O Error. Could not send byte vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // The bytes written minus the 4 length bytes should be no more than 3 more than num
            assert(bytes_written - num - 4 < 4);

            // Only send the num bytes that follow the 4 bytes of length info - we skip the
            // length info because it's already been sent and we don't send any trailing padding
            // bytes in this method (see put_vector_last() for that).
            unsigned int bytes_sent = d_out.write(&byte_buf[4], num);

            if (bytes_sent != num)
                throw Error ("Network I/O Error. Could not send byte vector data");

            // Now increment the element count so we can figure out about the padding in put_vector_last()
            d_partial_put_element_count += num;

            // now compute the trailing (padding) bytes
            unsigned int pad = d_partial_put_element_count % 4;
            if (pad) {
                vector<char> padding(4, 0); // 4 zeros
                unsigned int bytes_sent = d_out.write(&padding[0], pad);
                if (bytes_sent != pad)
                    throw Error ("Network I/O Error. Could not send byte vector data padding");

            }

            xdr_destroy(&byte_sink);
        }
        catch (...) {
            xdr_destroy(&byte_sink);
            throw;
        }
    }
    else {
#if 0
        int use_width = width;
        if (width < 4) use_width = 4;

        // the size is the number of elements num times the width of each
        // element, then add 4 bytes for the number of elements
        int size = (num * use_width) + 4;

        // allocate enough memory for the elements
        vector<char> vec_buf(size);
        XDR vec_sink;
        try {
            xdrmem_create(&vec_sink, &vec_buf[0], size, XDR_ENCODE);

            // set the position of the sink to 0, we're starting at the beginning
            if (!xdr_setpos(&vec_sink, 0))
                throw Error(
                    "Network I/O Error. Could not send vector data - unable to set stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // write the array to the buffer
            if (!xdr_array(&vec_sink, (char **) &val, (unsigned int *) &num, size, width, XDRUtils::xdr_coder(type)))
                throw Error(
                    "Network I/O Error(2). Could not send vector data.\nThis may be due to a bug in libdap or a\nproblem with the network connection.");

            // how much was written to the buffer
            unsigned int bytes_written = xdr_getpos(&vec_sink);
            if (!bytes_written)
                throw Error(
                    "Network I/O Error. Could not send vector data - unable to get stream position.\nThis may be due to a bug in DODS, on the server or a\nproblem with the network connection.");

            // write that much out to the output stream
            d_out.write(&vec_buf[4], bytes_written - 4);

            xdr_destroy(&vec_sink);
        }
        catch (...) {
            xdr_destroy(&vec_sink);
            throw;
        }
#endif
    }
}

void XDRStreamMarshaller::dump(ostream &strm) const
{
    strm << DapIndent::LMarg << "XDRStreamMarshaller::dump - (" << (void *) this << ")" << endl;
}

} // namespace libdap

