// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_STORAGE_FILE_HPP
#define OUZEL_STORAGE_FILE_HPP

#include <string>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>
#  undef WIN32_LEAN_AND_MEAN
#  undef NOMINMAX
#endif

#if defined(__ANDROID__)
#  include <android/asset_manager_jni.h>
#endif

namespace ouzel
{
    namespace storage
    {
        class File final
        {
        public:
            enum Mode
            {
                READ = 0x01,
                WRITE = 0x02,
                CREATE = 0x04,
                APPEND = 0x08,
                TRUNCATE = 0x10
            };

            enum Seek
            {
                BEGIN,
                CURRENT,
                END
            };

            File() {}
            File(const std::string& filename, int mode);
            ~File();

            File(const File&) = delete;
            File& operator=(const File&) = delete;

            File(File&&);
            File& operator=(File&&);

            inline bool isOpen() const
            {
#if defined(_WIN32)
                return file != INVALID_HANDLE_VALUE;
#else
                return file != -1;
#endif
            }

            void close();

            uint32_t read(void* buffer, uint32_t size, bool all = false) const;
            uint32_t write(const void* buffer, uint32_t size, bool all = false) const;
            void seek(int32_t offset, int method) const;
            uint32_t getOffset() const;

        private:
#if defined(_WIN32)
            HANDLE file = INVALID_HANDLE_VALUE;
#else
            int file = -1;
#endif
        };
    } // namespace storage
} // namespace ouzel

#endif // OUZEL_STORAGE_FILE_HPP
