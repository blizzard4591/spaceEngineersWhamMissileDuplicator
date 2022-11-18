#include "Options.h"

#include "BlueprintData.h"

#include <iostream>

qsizetype parseInt(QString const& name, QCommandLineParser& parser) {
    QString const s = parser.value(name);
    bool ok = false;
    qsizetype const result = s.toInt(&ok);
    if ((!ok) || (result < 0)) {
        std::cerr << "Option '" << name.toStdString() << "' could not be parsed: '" << s.toStdString() << "'" << std::endl;
        QCoreApplication::exit(-1);
    }
    return result;
}

Options Options::parseOptions(QCoreApplication const& app) {
    QCommandLineParser parser;
    parser.setApplicationDescription("A utility for duplicating missiles made with the WHAM (Whip's Homing Advanced Missile) script.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("blueprintFolder", "The folder containing your Blueprints, usually %APPDATA%/SpaceEngineers/Blueprints/local", "path", ""));
    parser.addOption(QCommandLineOption("blueprint", "Name of the Blueprint to copy", "name", ""));
    parser.addOption(QCommandLineOption("firstIndex", "First index that the copies will take", "number", ""));
    parser.addOption(QCommandLineOption("numCopies", "How many copies will be created", "number", ""));
    parser.addOption(QCommandLineOption("force", "Yes to all overwrite questions"));

    parser.process(app);

    bool ok = false;

    // Check Argument validity
    bool const haveBlueprintLocation = parser.isSet("blueprintFolder");
    QString const userBlueprintLocation = parser.value("blueprintFolder");
    if (haveBlueprintLocation) {
        if (!BlueprintData::isValidBlueprintLocation(QDir(userBlueprintLocation))) {
            std::cerr << "Your specified Blueprint location '" << userBlueprintLocation.toStdString() << "' is invalid!" << std::endl;
            QCoreApplication::exit(-1);
        }
    }

    bool const haveBlueprintName = parser.isSet("blueprint");
    QString const userBlueprintName = parser.value("blueprint");

    bool const haveFirstIndex = parser.isSet("firstIndex");
    qsizetype const userFirstIndex = (haveFirstIndex) ? parseInt("firstIndex", parser) : -1;

    bool const haveNumCopies = parser.isSet("numCopies");
    qsizetype const userNumCopies = (haveNumCopies) ? parseInt("numCopies", parser) : -1;

    bool const force = parser.isSet("force");

    return Options(haveBlueprintLocation, userBlueprintLocation, haveBlueprintName, userBlueprintName, haveFirstIndex, userFirstIndex, haveNumCopies, userNumCopies, force);
}
