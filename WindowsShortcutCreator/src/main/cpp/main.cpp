/*
 * This C++ source file was generated by the Gradle 'init' task.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include "ShortcutCreator.h"

#include <objbase.h>
#include <winerror.h>
#include "shobjidl.h"
#include "objidl.h"
#include "shlguid.h"
#include "ComPtr.h"

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

#pragma comment(lib, "Ole32.lib")

class CCoInitialize {
public:
 CCoInitialize() : m_hr(CoInitialize(NULL)) { }
 ~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
 operator HRESULT() const { return m_hr; }
 HRESULT m_hr;
};

static std::wstring ToWideString(const std::string& str) {
    constexpr int numCharacters = 512;
    WCHAR buf[numCharacters];
    int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), buf, numCharacters);
    DWORD lastError = GetLastError();
    if (length != 0) {
        return std::wstring(buf, length);
    }
    else if (lastError == ERROR_INSUFFICIENT_BUFFER) {
        // Call to get length
        length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), nullptr, 0);
        auto bigBuf = std::make_unique<WCHAR[]>(length);
        length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), bigBuf.get(), length);
        return std::wstring(bigBuf.get(), length);
    } else {
        // Actual error, figure out what to do
        return L"";
    }
}

void from_json(const json& j, ShortcutInfo& s) {
    s.path = ToWideString(j.at("Path").get<std::string>());
    s.name = ToWideString(j.at("Name").get<std::string>());
    s.description = ToWideString(j.at("Description").get<std::string>());
}

void from_json(const json& j, ShortcutData& s) {
    j.at("IsAdmin").get_to(s.isAdmin);
    j.at("DesktopShortcuts").get_to(s.desktopShortcuts);
    j.at("StartMenuShortcuts").get_to(s.startMenuShortcuts);
    s.iconLocation = ToWideString(j.at("IconLocation").get<std::string>());
}

int main (int argc, char *argv[]) {
    // Initialize COM
    CCoInitialize init;

    if(FAILED(init)) {
        return -1;
    }

    // Initialize the shortcut creator
    ShortcutCreator shortcutCreator;

    if (FAILED(shortcutCreator)) {
        return -1;
    }

    json j;

    if (argc < 2) {
        std::cin >> j;
    } else {
        std::ifstream i(argv[1]);
        i >> j;
    }

    ShortcutData s = j.get<ShortcutData>();

    std::optional<std::wstring> desktopFolder;
    std::optional<std::wstring> startMenuFolder;

    if (s.isAdmin) {
        desktopFolder = shortcutCreator.GetPublicDesktopFolder();
        startMenuFolder = shortcutCreator.GetAllUsersStartMenuFolder();
    } else {
        desktopFolder = shortcutCreator.GetLocalDesktopFolder();
        startMenuFolder = shortcutCreator.GetLocalStartMenuFolder();
    }

    bool createdDesktop = false;

    if (desktopFolder) {
        createdDesktop = shortcutCreator.CreateShortcuts(s.desktopShortcuts, s.iconLocation, *desktopFolder);
    }

    bool createdStartMenu = false;

    if (startMenuFolder) {
        createdStartMenu = shortcutCreator.CreateShortcuts(s.startMenuShortcuts, s.iconLocation, *startMenuFolder);
    }



    return 0;
}
