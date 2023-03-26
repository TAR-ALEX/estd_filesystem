#include <iostream>

#include <estd/filesystem.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <estd/AnsiEscape.hpp>


using std::cout;
using std::endl;

class UnitTests {
private:
    uint32_t testNum = 0;
    uint32_t passedNum = 0;

public:
    bool verbose = true;
    void testBool(const char* file, int line, const char* func, bool val) {
        using std::cout;
        using std::endl;
        testNum++;
        if (val) {
            cout << "["<<estd::setTextColor(0,255,0)<<"PASS"<<estd::clearSettings<<"] test number ";
            passedNum++;
        } else {
            cout << "["<<estd::setTextColor(255,0,0)<<"FAIL"<<estd::clearSettings<<"] test number ";
        }
        cout << testNum << " (" << file << ":" << line << ")" << endl;
    }

    void testLambda(const char* file, int line, const char* func, std::function<bool()> test) {
        using std::cout;
        using std::endl;
        testNum++;
        bool testResult = false;
        try {
            testResult = test();
        } catch (std::exception& e) {
            if (verbose) { cout << e.what() << endl; }
        } catch (...) {};
        if (testResult) {
            cout << "["<<estd::setTextColor(0,255,0)<<"PASS"<<estd::clearSettings<<"] test number ";
            passedNum++;
        } else {
            cout << "["<<estd::setTextColor(255,0,0)<<"FAIL"<<estd::clearSettings<<"] test number ";
        }
        cout << testNum << " (" << file << ":" << line << ")" << endl;
    }

    std::string getStats() {
        return std::string() + "TEST RESULTS: " + std::to_string(passedNum) + "/" + std::to_string(testNum);
    }
};

#define testBool(...) testBool(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define testLambda(...) testLambda(__FILE__, __LINE__, __func__, __VA_ARGS__)



namespace fs = estd::files;

int main() {
    UnitTests test;
    cout << "[filesystem util tests]" << endl;


    test.testBool(fs::Path{} == "");
    test.testBool(fs::Path{}.normalize() == "./");
    test.testBool(fs::Path{"."}.normalize() == "./");
    test.testBool(fs::Path{"./"}.normalize() == "./");
    test.testBool(fs::Path{"./test/."}.normalize() == "test/");
    test.testBool(fs::Path{"./test/./././"}.normalize() == "test/");
    test.testBool(fs::Path{"./test/././../"}.normalize() == "./"); // todo: bad behavior

    test.testBool(fs::Path{"/home/user/Desktop"}.hasSuffix());
    test.testBool(!fs::Path{"/home/user/"}.hasSuffix());
    test.testBool(fs::Path{"home/user/Desktop"}.hasPrefix());
    test.testBool(!fs::Path{"/home/user/"}.hasPrefix());

    test.testBool(fs::Path{"home/user/Desktop"}.splitPrefix().first == "home");
    test.testBool(fs::Path{"home/user/Desktop"}.splitPrefix().second == "user/Desktop");

    test.testBool(fs::Path{"home/user/Desktop"}.splitSuffix().first == "home/user");
    test.testBool(fs::Path{"home/user/Desktop"}.splitSuffix().second == "Desktop");

    fs::Path p = "/some/root/path/";
    test.testBool(p.replaceSuffix("suffix").normalize() == "/some/root/path/suffix");
    test.testBool(p.replacePrefix("prefix").normalize() == "prefix/some/root/path/");
    test.testBool(p.replacePrefix("/some/", "/other/").value().normalize() == "/other/root/path/");
    test.testBool(p.replacePrefix("/some/root", "/other/root2").value().normalize() == "/other/root2/path/");
    test.testBool(p.replacePrefix("/some/root/", "/other/root2").value().normalize() == "/other/root2/path/");
    test.testBool(p.replacePrefix("/some/root", "/other/root2/").value().normalize() == "/other/root2/root/path/");
    test.testBool(p.removeEmptyPrefix() == "some/root/path/");
    test.testBool(p.removeEmptySuffix() == "/some/root/path");

    p = "./some/root/path/";
    test.testBool(p.replacePrefix("some/", "other/").value().normalize() == "other/root/path/");
    test.testBool(p.replacePrefix("some/root", "other/root2").value().normalize() == "other/root2/path/");
    test.testBool(p.replacePrefix("some/root/", "other/root2").value().normalize() == "other/root2/path/");
    test.testBool(p.replacePrefix("some/root", "other/root2/").value().normalize() == "other/root2/root/path/");

    test.testBool(p.removeEmptyPrefix() == "./some/root/path/");
    test.testBool(p.normalize().removeEmptyPrefix() == "some/root/path/");



    test.testLambda([&] {
        fs::remove("sandbox");
        fs::remove("sandbox_copy");
        return true;
    });

    test.testLambda([&] {
        fs::createDirectories("sandbox/dir/subdir/");
        std::ofstream("sandbox/dir/file1.txt").put('a');
        fs::createSoftLink("sandbox/dir/file1.txt", "sandbox/file2.txt");
        fs::createHardLink("sandbox/dir/file1.txt", "sandbox/file3.txt");

        fs::copy("sandbox/dir/", "sandbox/dir2/");
        fs::copy("sandbox/", "sandbox_copy/");
        return true;
    });

    fs::remove("sandbox");
    fs::remove("sandbox_copy");

    test.testLambda([&] {
        fs::createDirectories("sandbox/dir/subdir/");
        std::ofstream("sandbox/dir/subdir/file1.txt").put('a');
        fs::createSoftLink("sandbox/dir/subdir/", "sandbox/dir/sublink/");
        fs::copy("sandbox/dir/", "sandbox/dir2/");
        fs::copy("sandbox/", "sandbox_copy/");
        return true;
    });
    fs::remove("sandbox_copy");
    fs::remove("sandbox");
    test.testLambda([&] {
        try {
            fs::createDirectories("sandbox/dir/subdir/");
            std::ofstream("sandbox/dir/subdir/file1.txt").put('b');
            fs::createDirectories("sandbox/dir/subdir/file2.txt/");
            fs::copy("sandbox/dir/subdir/file1.txt", "sandbox/dir/subdir/file2.txt");

        } catch (...) { return true; }
        return false;
    });

    fs::remove("sandbox");
    test.testLambda([&] {
        try {
            fs::createDirectories("sandbox/dir/subdir/");
            std::ofstream("sandbox/dir/subdir/file1.txt").put('b');
            fs::createDirectories("sandbox/dir/subdir/file2.txt/");

            fs::createDirectories("sandbox/dir/subdir2/");
            std::ofstream("sandbox/dir/subdir2/file1.txt").put('b');
            std::ofstream("sandbox/dir/subdir2/file2.txt").put('a');

            fs::copy("sandbox/dir/subdir/", "sandbox/dir/subdir2/");
        } catch (...) { return true; }
        return false;
    });

    fs::remove("sandbox");
    test.testLambda([&] {
        try {
            fs::createDirectories("sandbox/dir/subdir/");
            std::ofstream("sandbox/dir/subdir/file1.txt").put('b');
            fs::createDirectories("sandbox/dir/subdir/file2.txt/");

            fs::createDirectories("sandbox/dir/subdir2/");
            std::ofstream("sandbox/dir/subdir2/file1.txt").put('b');
            std::ofstream("sandbox/dir/subdir2/file2.txt").put('a');

            fs::copy("sandbox/dir/subdir2/", "sandbox/dir/subdir/");
            if (fs::exists("sandbox/dir/subdir/file2.txt/file2.txt"))
                return false; // This test proves std copy function is not reliable
        } catch (...) { return true; }
        return true;
    });
    fs::remove("sandbox");
    test.testLambda([&] {
        fs::createDirectories("sandbox/dir/subdir/");
        std::ofstream("sandbox/dir/subdir/file1.txt").put('b');
        fs::createDirectories("sandbox/dir/subdir/file2.txt/");

        fs::createDirectories("sandbox/dir/subdir2/");
        std::ofstream("sandbox/dir/subdir2/file1.txt").put('b');
        std::ofstream("sandbox/dir/subdir2/file2.txt").put('a');

        fs::copy(
            "sandbox/dir/subdir2/",
            "sandbox/dir/subdir/",
            fs::CopyOptions::recursive | fs::CopyOptions::overwriteExisting
        );
        if (fs::exists("sandbox/dir/subdir/file2.txt/file2.txt"))
            return false; // This test proves std copy function is not reliable
        return true;
    });

    p = "./some/root/path/img112.jpeg";
    test.testBool(p.getExtention() == p.getLongExtention() && p.getExtention() == "jpeg");
    test.testBool(p.splitExtention().first == p.splitLongExtention().first && p.splitExtention().first == "./some/root/path/img112");
    p = "./some/root/path/fileone.txt.jpeg.zip";
    test.testBool(p.getExtention() == "zip" && p.splitExtention().first == "./some/root/path/fileone.txt.jpeg");
    test.testBool(p.getLongExtention() == "txt.jpeg.zip" && p.splitLongExtention().first == "./some/root/path/fileone");
    p = "./some/root/path/.fileone.txt.jpeg.zip";
    test.testBool(p.getExtention() == "zip" && p.splitExtention().first == "./some/root/path/.fileone.txt.jpeg");
    test.testBool(p.getLongExtention() == "txt.jpeg.zip" && p.splitLongExtention().first == "./some/root/path/.fileone");
    p = ".fileone.txt.jpeg.zip";
    test.testBool(p.getExtention() == "zip" && p.splitExtention().first == ".fileone.txt.jpeg");
    test.testBool(p.getLongExtention() == "txt.jpeg.zip" && p.splitLongExtention().first == ".fileone");
    p = "/.fileone.txt.jpeg.zip";
    test.testBool(p.getExtention() == "zip" && p.splitExtention().first == "/.fileone.txt.jpeg");
    test.testBool(p.getLongExtention() == "txt.jpeg.zip" && p.splitLongExtention().first == "/.fileone");
    p = "./.fileone.txt.jpeg.zip";
    test.testBool(p.getExtention() == "zip" && p.splitExtention().first == "./.fileone.txt.jpeg");
    test.testBool(p.getLongExtention() == "txt.jpeg.zip" && p.splitLongExtention().first == "./.fileone");
    p = "./";
    test.testBool(p.getExtention() == "" && p.splitExtention().first == "./");
    test.testBool(p.getLongExtention() == "" && p.splitLongExtention().first == "./");
    p = ".";
    test.testBool(p.getExtention() == "" && p.splitExtention().first == ".");
    test.testBool(p.getLongExtention() == "" && p.splitLongExtention().first == ".");

    // fs::remove("sandbox");

    cout << endl << test.getStats() << endl;

    // fs::copy("sandbox/dir/", "sandbox/dir2/");
    // fs::copy("sandbox/", "sandbox_copy/");

    return 0;
}