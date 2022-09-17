// BSD 3-Clause License

// Copyright (c) 2022, Alex Tarasov
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <estd/ptr.hpp>
#include <estd/string_util.h>
#include <filesystem>
#include <vector>

namespace estd {
    namespace files {
        // class Path : public std::filesystem::path {
        class Path {
        private:
            std::string path;

        public:
            Path() noexcept {}
            Path(const Path& p) = default;
            Path(Path&& p) = default;
            Path(std::filesystem::path&& source) { path = source; }
            template <class Source>
            Path(const Source& source) {
                path = source;
            }
            Path(std::string source) { path = source; }
            Path(const char* source) { path = source; }
            Path(const std::filesystem::path& source) { path = source; }

            Path& operator=(const Path& p) = default;
            Path& operator=(Path&& p) = default;

            bool operator==(Path&& other) { return path == other.string(); }
            bool operator!=(Path&& other) { return path != other.string(); }
            bool operator==(const Path& other) { return path == other.string(); }
            bool operator!=(const Path& other) { return path != other.string(); }

            friend Path operator/(const Path& lhs, const Path& rhs) { return Path(lhs.string() + "/" + rhs.string()); }
            friend Path operator+(const Path& lhs, const Path& rhs) { return Path(lhs.string() + rhs.string()); }
            friend Path& operator/=(Path& lhs, const Path& rhs) { return lhs.path += "/" + rhs.string(), lhs; }
            friend Path& operator+=(Path& lhs, const Path& rhs) { return lhs.path += rhs.string(), lhs; }

            friend std::ostream& operator<<(std::ostream& stream, Path p) {
                return stream << "\"" << p.string() << "\"";
            }

            operator std::filesystem::path() const { return std::filesystem::path(path); }
            operator std::string() const { return path; }

            std::string string() const noexcept { return path; }

            Path normalize() {
                Path tmp = std::filesystem::path(path).lexically_normal();
                if (tmp == "." || tmp == "./") { tmp = ""; }
                return tmp;
            }

            Path normalizeSafe() = delete; // TODO: Not implemented yet

            // tells if this path is a parent of the passed path if(other is a tree in *this) (only for directories)
            bool contains(Path& other) {
                Path left = Path((*this) / "").normalize();
                Path right = Path(other / "").normalize();

                return estd::string_util::hasPrefix(right, left);
            }

            Path removeEmptySuffix() { return hasSuffix() ? *this : splitSuffix().first; }
            Path removeEmptyPrefix() { return hasPrefix() ? *this : splitPrefix().second; }
            bool hasPrefix() { return splitPrefix().first != ""; }
            bool hasSuffix() { return splitSuffix().second != ""; }
            Path getPrefix() { return splitPrefix().first; }
            Path getSuffix() { return splitSuffix().second; }
            Path replaceSuffix(Path s) { return splitSuffix().first / s; }
            Path replacePrefix(Path s) { return s / splitPrefix().second; }

            std::pair<Path, Path> splitPrefix() {
                size_t mid_pos = 0;
                std::string source = string();
                std::string psL = "";
                std::string psR = "";
                if ((mid_pos = source.find("/")) != std::string::npos) {
                    psL = source.substr(0, mid_pos);
                    psR = source.substr(mid_pos + 1);
                } else {
                    return {source, ""};
                }
                return {psL, psR};
            }

            std::pair<Path, Path> splitSuffix() {
                size_t mid_pos = 0;
                std::string source = string();
                std::string psL = "";
                std::string psR = "";
                if ((mid_pos = source.rfind("/")) != std::string::npos) {
                    psL = source.substr(0, mid_pos);
                    psR = source.substr(mid_pos + 1);
                } else {
                    return {"", source};
                }
                return {psL, psR};
            }

            estd::stack_ptr<Path> replacePrefix(Path from, Path to) {
                Path path = *this;

                path = ("." / path).normalize();
                from = ("." / from).normalize();
                to = ("." / to).normalize();

                bool pathIsDir = !path.hasSuffix();
                bool fromIsDir = !from.hasSuffix();
                bool toIsDir = !to.hasSuffix();

                to = (to / "").normalize();
                from = (from / "").normalize();
                path = (path / "").normalize();

                if (toIsDir && !fromIsDir) { // from is a file && to is a dir
                    to = to.replaceSuffix(from.splitSuffix().first.getSuffix());
                    to = (to / "").normalize();
                }

                if (from == "" || from == "." || from == "./") {
                    Path result = (to / path).normalize();
                    if (!pathIsDir) return result.splitSuffix().first; // remove slash at end
                    return result;
                }

                if (!estd::string_util::hasPrefix(path, from)) return nullptr;
                Path result = estd::string_util::replacePrefix(path, from, to);

                result = result.normalize();
                if (!pathIsDir) result = result.splitSuffix().first.normalize();

                return result;
            }
        };

        typedef std::filesystem::copy_options CopyOptions;
        class DirectoryIterator : public std::filesystem::directory_iterator {
        public:
            using std::filesystem::directory_iterator::directory_iterator;
        };
        class RecursiveDirectoryIterator : public std::filesystem::recursive_directory_iterator {
        public:
            using std::filesystem::recursive_directory_iterator::recursive_directory_iterator;
        };

        inline bool exists(Path p) { return std::filesystem::exists(p); }
        inline uintmax_t remove(Path p) { return std::filesystem::remove_all(p); }
        inline bool isDirectory(Path p) { return std::filesystem::is_directory(p); }
        inline Path followSoftLink(Path p) { return std::filesystem::read_symlink(p); }
        inline bool isSoftLink(Path p) { return std::filesystem::is_symlink(p); }
        inline bool isBlockFile(Path p) { return std::filesystem::is_block_file(p); }
        inline bool isCharacterFile(Path p) { return std::filesystem::is_character_file(p); }
        inline bool isEmptry(Path p) { return std::filesystem::is_empty(p); }
        inline bool isFIFO(Path p) { return std::filesystem::is_fifo(p); }
        inline bool isOther(Path p) { return std::filesystem::is_other(p); }
        inline bool isFile(Path p) { return std::filesystem::is_regular_file(p); }
        // returns if it is a directory or a softlink to a directory
        inline bool isSoftDirectory(Path p) {
            if (isDirectory(p)) return true;
            if (isSoftLink(p)) {
                Path link = followSoftLink(p);
                if (exists(link)) return isSoftDirectory(p);
                return !link.hasSuffix();
            }
            return false;
        }

        inline bool isSoftFile(Path p) {
            if (isFile(p)) return true;
            if (isSoftLink(p)) {
                Path link = followSoftLink(p);
                if (exists(link)) return isSoftFile(p);
                return link.hasSuffix();
            }
            return false;
        }
        inline bool isSocket(Path p) { return std::filesystem::is_socket(p); }

        inline void createHardLink(Path from, Path to) { return std::filesystem::create_hard_link(from, to); }
        inline void createSoftLink(Path from, Path to) {
            Path linkroot = to.removeEmptySuffix().splitSuffix().first;
            from = std::filesystem::relative(from, linkroot);
            to = to.removeEmptySuffix();
            std::filesystem::create_symlink(from, to);
        }
        //from path will be relative (the way it is in the OS)
        inline void createSoftLinkRelative(Path from, Path to) {
            to = to.removeEmptySuffix();
            std::filesystem::create_symlink(from, to);
        }

        inline void createDirectories(Path p) { std::filesystem::create_directories(p); }
        inline void createDirectory(Path p) { std::filesystem::create_directory(p); }
        // TODO: change to filesystem error exceptions
        inline void copy(
            Path from,
            Path to,
            const CopyOptions opt = CopyOptions::overwrite_existing | CopyOptions::recursive |
                                    CopyOptions::copy_symlinks
        ) {
            auto throwError = [](std::string description, Path dir1) {
                throw std::runtime_error(std::string("filesystem error: ") + description + " [" + dir1.string() + "]");
            };

            if (!exists(from.removeEmptySuffix())) throwError("cannot copy: No such file or directory", from);

            bool fromIsDir = !to.hasSuffix();

            if (fromIsDir != isSoftDirectory(from)) {
                if (fromIsDir) {
                    throwError("cannot copy: source not a directory", from);
                } else {
                    throwError("cannot copy: source not a file", from);
                }
            }

            bool toIsDir = !to.hasSuffix();

            if (exists(to.removeEmptySuffix())) {
                if (toIsDir != isSoftDirectory(to)) {
                    if (toIsDir) {
                        throwError("cannot copy: destination not a directory", from);
                    } else {
                        throwError("cannot copy: destination not a file", from);
                    }
                }
            }
            try {
                std::filesystem::copy(from, to, opt);
            } catch (std::exception& e) { throw std::runtime_error(e.what()); }
        }

        // sample error:
        // filesystem error: cannot copy: No such file or directory [...] [...]


        class TmpDir {
        private:
            std::filesystem::path iPath = "";
            inline static std::string generateUniqueTempDir(std::filesystem::path root) {
                while (true) {
                    std::filesystem::path name = "." + estd::string_util::gen_random(10);
                    name = root / name;
                    if (!std::filesystem::exists(name)) {
                        std::filesystem::create_directories(name);
                        return name.lexically_normal();
                    }
                }
            }

        public:
            TmpDir(std::filesystem::path root) { iPath = generateUniqueTempDir(root); }
            TmpDir() { iPath = generateUniqueTempDir(std::filesystem::current_path()); }
            ~TmpDir() { std::filesystem::remove_all(iPath); }

            std::filesystem::path path() { return iPath; }

            void discard() {
                for (const auto& entry : std::filesystem::directory_iterator(iPath)) std::filesystem::remove_all(entry);
            }
        };

        template <bool recursive = true, bool overwrite = true>
        void copy(Path from, Path to) {
            if (!std::filesystem::is_directory(from)) {
                std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
            } else {
                if (!std::filesystem::exists(to) || overwrite) {
                    std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
                }
            }
        }

        // Path findLargestCommonPrefix(Path p1, Path p2) {
        //     p1 = p1.lexically_normal();
        //     p2 = p2.lexically_normal();
        //     if (p1 == "./" || p1 == "." || p1 == "" || p2 == "./" || p2 == "." || p2 == "") return ".";

        //     return "";
        // }
    } // namespace files
} // namespace estd
