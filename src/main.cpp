#include "NexusPch.h"
#include <Windows.h>

#include <QtWidgets/QApplication>
#include <QPalette>
#include <QFile>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    WCHAR path[MAX_PATH] = { 0 };
    DWORD size = GetModuleFileName(NULL, path, MAX_PATH);

    if (size == 0) {
        std::cerr << "Failed to get the executable path." << std::endl;
        return 1;
    }

    // Convert WCHAR to std::string
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    if (utf8Size == 0) {
        std::cerr << "Failed to convert WCHAR to UTF-8." << std::endl;
        return 1;
    }

    std::string exePath(utf8Size, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, path, -1, &exePath[0], utf8Size, NULL, NULL) == 0) {
        std::cerr << "Failed to convert WCHAR to UTF-8." << std::endl;
        return 1;
    }
    // Create fs::path from std::wstring
    std::filesystem::path exe_directory(exePath);

    // Extract the directory path (excluding the executable name)
    exe_directory.remove_filename();
   
    // arrow dll
    auto arrow_dll = exe_directory / "arrow.dll";
	HMODULE hDLL = LoadLibrary(StringToLPCWSTR(arrow_dll.string()));
    if (!hDLL) {
        std::cerr << "Failed to load 'arrow.dll'." << std::endl;
    }


	QCoreApplication::setApplicationName("Nexus");
	QCoreApplication::setOrganizationName("Agis Systems");
	QApplication::setStyle("Fusion");

    QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(true);

	QFile styleSheetFile("./styles/NexusTree.qss");
	styleSheetFile.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(styleSheetFile.readAll());
	a.setStyleSheet(styleSheet);

	MainWindow w;
    w.resize(1280, 720);
	w.show();

    return a.exec();
}
