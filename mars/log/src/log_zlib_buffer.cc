// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * log_zlib_buffer.cpp
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */


#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "log/crypt/log_crypt.h"
#include "log_zlib_buffer.h"
#include "log_base_buffer.h"


#ifdef WIN32
#define snprintf _snprintf
#endif


LogZlibBuffer::LogZlibBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey)
    :LogBaseBuffer(_pbuffer, _len, _isCompress, ZLIB, _pubkey) {

    if (is_compress_) {
        memset(&cstream_, 0, sizeof(cstream_));
    }
}

LogZlibBuffer::~LogZlibBuffer() {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }
}

void LogZlibBuffer::Flush(AutoBuffer& _buff) {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }

    LogBaseBuffer::Flush(_buff);
}

size_t LogZlibBuffer::compress(const void* src, size_t inLen, void* dst, size_t outLen){

    
    cstream_.avail_in = (uInt)inLen;
    cstream_.next_in = (Bytef*)src;

    cstream_.next_out = (Bytef*)dst;
    cstream_.avail_out = (uInt)outLen;

    if (Z_OK != deflate(&cstream_, Z_SYNC_FLUSH)) {
        return -1;
    }
    
    return outLen - cstream_.avail_out;
}

bool LogZlibBuffer::__Reset() {
    
   __Clear();
   
   log_crypt_->SetHeaderInfo((char*)buff_.Ptr(), is_compress_, compress_mode_);
   buff_.Length(log_crypt_->GetHeaderLen(), log_crypt_->GetHeaderLen());

    if (is_compress_) {
        cstream_.zalloc = Z_NULL;
        cstream_.zfree = Z_NULL;
        cstream_.opaque = Z_NULL;

        if (Z_OK != deflateInit2(&cstream_, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
            return false;
        }
    }

    return true;
}
