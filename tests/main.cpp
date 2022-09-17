#include <estd/filesystem.hpp>
#include <fstream>
#include <functional>
#include <iostream>

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
            cout << "[PASS] test number ";
            passedNum++;
        } else {
            cout << "[FAIL] test number ";
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
        } catch (std::exception& e) { cout << e.what() << endl; } catch (...) {
        };
        if (testResult) {
            cout << "[PASS] test number ";
            passedNum++;
        } else {
            cout << "[FAIL] test number ";
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
    test.testBool(fs::Path{"."}.normalize() == "");
    test.testBool(fs::Path{"./"}.normalize() == "");
    test.testBool(fs::Path{"./test/."}.normalize() == "test/");
    test.testBool(fs::Path{"./test/./././"}.normalize() == "test/");
    test.testBool(fs::Path{"./test/././../"}.normalize() == ""); // todo: bad behavior

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
    test.testBool(p.replacePrefix("/some/", "/other/").value().normalize() == "other/root/path/");
    test.testBool(p.replacePrefix("/some/root", "/other/root2").value().normalize() == "other/root2/path/");
    test.testBool(p.replacePrefix("/some/root/", "/other/root2").value().normalize() == "other/root2/path/");
    test.testBool(p.replacePrefix("/some/root", "/other/root2/").value().normalize() == "other/root2/root/path/");

    test.testBool(p.removeEmptyPrefix() == "some/root/path/");
    test.testBool(p.removeEmptySuffix() == "/some/root/path");

    fs::remove("sandbox");
    fs::remove("sandbox_copy");

    test.testLambda([&] {
        fs::createDirectories("sandbox/dir/subdir/");
        std::ofstream("sandbox/dir/file1.txt").put('a');
        fs::createSoftLink("sandbox/dir/file1.txt", "sandbox/file2.txt");
        fs::createHardLink("sandbox/dir/file1.txt", "sandbox/file3.txt");
        fs::copy("sandbox/dir/", "sandbox/dir2/");
        fs::copy("sandbox/", "sandbox_copy/");
        fs::remove("sandbox");
        fs::remove("sandbox_copy");
        return true;
    });

    test.testLambda([&] {
        fs::createDirectories("sandbox/dir/subdir/");
        std::ofstream("sandbox/dir/subdir/file1.txt").put('a');
        fs::createSoftLink("sandbox/dir/subdir/", "sandbox/dir/sublink/");
        fs::copy("sandbox/dir/", "sandbox/dir2/");
        fs::copy("sandbox/", "sandbox_copy/");
        fs::remove("sandbox");
        fs::remove("sandbox_copy");
        return true;
    });

    cout << endl << test.getStats() << endl;

    return 0;
}