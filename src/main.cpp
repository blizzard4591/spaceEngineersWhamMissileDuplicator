#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <iomanip>
#include <string>

#include "BlueprintData.h"

QString readInputFromConsoleWithDefault(std::string const& text, QString const& defaultValue) {
    std::cout << text << " [" << defaultValue.toStdString() << "]: ";
    std::string str;
    std::getline(std::cin, str);
    if (str.empty()) {
        return defaultValue;
    }
    return QString::fromStdString(str);
}

bool isOnlyDigitsAndAtLeastOne(QString const& s) {
    if (s.size() < 1) {
        return false;
    }
    for (qsizetype i = 0; i < s.size(); ++i) {
        auto const c = s.at(i);
        if ((c < '0') || (c > '9')) {
            return false;
        }
    }
    return true;
}

bool readNumericInputOrQuit(std::string const& text, qsizetype& choice, qsizetype min, qsizetype max) {
    while (true) {
        std::cout << text << "Enter any number from " << min << " to " << max << ", or q to quit: " << std::endl;

        std::string str;
        std::getline(std::cin, str);

        QString const reply = QString::fromStdString(str);
        if ((reply == QStringLiteral("q")) || (reply == QStringLiteral("quit"))) {
            return false;
        } else if (!isOnlyDigitsAndAtLeastOne(reply)) {
            std::cerr << "Did not understand your reply." << std::endl;
            continue;
        }

        bool ok = false;
        choice = reply.toInt(&ok);
        if (!ok) {
            std::cerr << "Did not understand your reply, could not parse it as a number." << std::endl;
            continue;
        }

        if (choice < min || choice > max) {
            std::cerr << "Did not understand your reply, it was not in the given range." << std::endl;
            continue;
        }
        return true;
    }
    
}

bool isValidBlueprintLocation(QDir dir) {
    // pick a folder and check if it contains bp.spc
    auto const list = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    if (list.size() < 1) {
        return false;
    } else if (!dir.cd(list.at(0))) {
        return false;
    }
    return QFile::exists(dir.absoluteFilePath(QStringLiteral("bp.sbc")));
}

QString getDefaultLocalBlueprintFolder() {
    QString const baseDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, "Blueprints", QStandardPaths::LocateOption::LocateDirectory);
    if (baseDir.isEmpty()) {
        std::cerr << "Warning: Failed to locate your SpaceEngineers Blueprints folder in %APPDATA%/SpaceEngineers/Blueprints." << std::endl;
        return QString();
    }
    QDir dir(baseDir);
    if (!dir.cd("local")) {
        std::cerr << "Warning: Failed to locate your SpaceEngineers folder in in %APPDATA%/SpaceEngineers/Blueprints/local!" << std::endl;
        return QString();
    }
    return dir.absolutePath();
}

QStringList scanBlueprints(QString const& path) {
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    if (!dir.exists()) {
        std::cerr << "Error: The selected folder '" << dir.absolutePath().toStdString() << "' is invalid!" << std::endl;
        throw;
    }

    return dir.entryList();
}

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    a.setApplicationName("SpaceEngineers"); // To allow easy access to AppData/Roaming/SpaceEngineers
    
    // 1. Select location for blueprints
    QString blueprintLocation;
    QString const defaultPath = getDefaultLocalBlueprintFolder();
    while (true) {
        blueprintLocation = readInputFromConsoleWithDefault("Please select a blueprint location", defaultPath);
        if ((blueprintLocation == QStringLiteral("q")) || (blueprintLocation == QStringLiteral("quit"))) {
            std::cout << "Quitting as requested..." << std::endl;
            return -1;
        } else if (isValidBlueprintLocation(QDir(blueprintLocation))) {
            break;
        }

        std::cerr << "The selected location '" << blueprintLocation.toStdString() << "' is not valid. It should contain a set of folders, each containing a file called 'bp.spc'." << std::endl;
    }

    // 2. Present a list of Blueprints
    auto const list = scanBlueprints(blueprintLocation);
    if (list.size() < 1) {
        std::cerr << "The selected location '" << blueprintLocation.toStdString() << "' does not contain any Blueprints! It should contain a set of folders, each containing a file called 'bp.spc'." << std::endl;
        return -1;
    }
    std::cout << "Available Blueprints:" << std::endl;
    for (qsizetype i = 0; i < list.size(); ++i) {
        std::cout << std::setw(3) << (i + 1) << ": " << list.at(i).toStdString() << std::endl;
    }

    qsizetype choiceIndex = -1;
    if (!readNumericInputOrQuit("", choiceIndex, 1, list.size())) {
        std::cout << "Quitting as requested..." << std::endl;
        return -1;
    }

    // Correct for the offset
    choiceIndex -= 1;
    QString const choice = list.at(choiceIndex);
    std::cout << "You selected: " << choice.toStdString() << std::endl;

    // 3. Load Blueprint
    QDir blueprintFolder(blueprintLocation);
    if (!blueprintFolder.cd(choice)) {
        std::cerr << "Error: Failed to open your chosen blueprint?!" << std::endl;
        return -1;
    }
    QFile file(blueprintFolder.absoluteFilePath(QStringLiteral("bp.sbc")));
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Could not open selected blueprint for reading!" << std::endl;
        return -1;
    }
    QByteArray const data = file.readAll();
    file.close();
    auto const blueprintData = BlueprintData::fromXml(data);
    if (!blueprintData) {
        return -1;
    } else if (choice != blueprintData->getDisplayName()) {
        std::cerr << "The selected blueprint should be in a folder called '" << blueprintData->getDisplayName().toStdString() << "', not in '" << choice.toStdString() << "'..." << std::endl;
    }

    // 4. Ask how many copies and duplicate them
    qsizetype firstIndex = -1;
    if (!readNumericInputOrQuit("Choose the starting ID of your copies. ", firstIndex, 1, 9999)) {
        std::cout << "Quitting as requested..." << std::endl;
        return -1;
    }
    qsizetype copyCount = -1;
    if (!readNumericInputOrQuit("Choose how many copies you would like. ", copyCount, 1, 9999)) {
        std::cout << "Quitting as requested..." << std::endl;
        return -1;
    }
    std::cout << "We will create " << copyCount << " cop" << ((copyCount == 1) ? "y" : "ies") << ", starting at " << firstIndex << "." << std::endl;

    for (qsizetype i = 0; i < copyCount; ++i) {
        QByteArray const copyData = BlueprintData::toXMLWithNewId(data, *blueprintData, firstIndex);
        if (copyData.isNull() || copyData.isEmpty()) {
            std::cerr << "Failed to produce a viable copy, quitting..." << std::endl;
            return -1;
        }
        
        QString const copyName = BlueprintData::cutDigitsFromEnd(blueprintData->getDisplayName()).append(QString::number(firstIndex));

        QDir copyDir(blueprintLocation);
        if (!copyDir.cd(copyName)) {
            copyDir.mkdir(copyName);
            copyDir.cd(copyName);
        }

        QString const copyBpName = copyDir.absoluteFilePath(QStringLiteral("bp.sbc"));
        if (QFile::exists(copyBpName)) {
            QString const removeReply = readInputFromConsoleWithDefault("Are you sure you want to replace all contents of the existing Blueprint '" + copyName.toStdString() + "'? (y or yes to confirm)", QStringLiteral("no"));
            if ((removeReply == QStringLiteral("y")) || (removeReply == QStringLiteral("yes"))) {
                QFile::remove(copyBpName);
                QFile::remove(copyDir.absoluteFilePath(QStringLiteral("bp.sbcB5")));
                QFile::remove(copyDir.absoluteFilePath(QStringLiteral("thumb.png")));
            } else {
                std::cout << "Will not override, quitting..." << std::endl;
                return -1;
            }
        }

        QFile fileBlueprint(copyBpName);
        if (!fileBlueprint.open(QFile::WriteOnly)) {
            std::cerr << "Error: Failed to write file '" << copyBpName.toStdString() << "', not writable!" << std::endl;
            return -1;
        }
        fileBlueprint.write(copyData);
        fileBlueprint.close();

        // Copy the Thumbnail
        QFile::copy(blueprintFolder.absoluteFilePath(QStringLiteral("thumb.png")), copyDir.absoluteFilePath(QStringLiteral("thumb.png")));

        ++firstIndex;
    }

    std::cout << "Done! Happy Engineering!" << std::endl;
    return 0;
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__* hInstance, struct HINSTANCE__* hPrevInstance, char* lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
