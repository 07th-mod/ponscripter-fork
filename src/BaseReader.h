/* -*- C++ -*-
 *
 *  BaseReader.h - Base class of archive reader
 *
 *  Copyright (c) 2001-2006 Ogapee (original ONScripter, of which this
 *  is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

#ifndef __BASE_READER_H__
#define __BASE_READER_H__

#include "defs.h"

#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifdef WIN32
#define DELIMITER "\\"
#else
#define DELIMITER "/"
#endif

struct BaseReader {
    enum {
        NO_COMPRESSION   = 0,
        SPB_COMPRESSION  = 1,
        LZSS_COMPRESSION = 2,
        NBZ_COMPRESSION  = 4
    };

    enum {
        ARCHIVE_TYPE_NONE = 0,
        ARCHIVE_TYPE_SAR  = 1,
        ARCHIVE_TYPE_NSA  = 2,
        ARCHIVE_TYPE_NS2  = 3,
        ARCHIVE_TYPE_NS3  = 4
    };

    struct FileInfo {
        pstring name;
        int compression_type;
        size_t offset;
        size_t length;
        size_t original_length;
    };

    struct ArchiveInfo {
	// FIXME: convert to an STL container
        ArchiveInfo* next;
        FILE* file_handle;
        pstring file_name;
        FileInfo* fi_list;
        unsigned int num_of_files;
        unsigned long base_offset;

        ArchiveInfo() {
            next = NULL;
            file_handle = NULL;
            fi_list = NULL;
            num_of_files = 0;
        }
        ~ArchiveInfo(){
            if (file_handle) fclose( file_handle );
            if (fi_list) delete[] fi_list;
        }
    };

    virtual ~BaseReader() { };

    virtual int open(const pstring& name = "",
		     int archive_type = ARCHIVE_TYPE_NONE) = 0;
    
    virtual int close() = 0;

    virtual pstring getArchiveName() const = 0;

    virtual int getNumFiles() = 0;

    virtual void registerCompressionType(const pstring& ext, int type) = 0;

    virtual FileInfo getFileByIndex(unsigned int index) = 0;

    virtual size_t getFileLength(const pstring& file_name) = 0;

    virtual size_t getFile(const pstring& file_name, unsigned char* buffer,
			   int* location = NULL) = 0;

    pstring getFile(const pstring& file_name, int* location = NULL);
};


inline pstring
BaseReader::getFile(const pstring& file_name, int* location)
{
    size_t length = getFileLength(file_name);
    if (!length) return pstring();
    char* buf = new char[length];
    length = getFile(file_name, (unsigned char*) buf, location);
    
    // roto 20100227 (fixing memory leak)
    pstring data(buf, length);
    delete[] buf;
    return data;
}

#endif // __BASE_READER_H__
