#ifndef SPACEENGINEERS_BLUEPRINTDUPLICATOR_OPTIONS_H_
#define SPACEENGINEERS_BLUEPRINTDUPLICATOR_OPTIONS_H_

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>

class Options {
public:
	Options(
		bool haveBlueprintLocation,	QString const& userBlueprintLocation,
		bool haveBlueprintName, QString const& userBlueprintName,
		bool haveFirstIndex, qsizetype userFirstIndex,
		bool haveNumCopies, qsizetype userNumCopies,
		bool force
	) :
		haveBlueprintLocation(haveBlueprintLocation), userBlueprintLocation(userBlueprintLocation),
		haveBlueprintName(haveBlueprintName), userBlueprintName(userBlueprintName),
		haveFirstIndex(haveFirstIndex), userFirstIndex(userFirstIndex),
		haveNumCopies(haveNumCopies), userNumCopies(userNumCopies),
		force(force) {
	}

	bool const haveBlueprintLocation;
	QString const userBlueprintLocation;

	bool const haveBlueprintName;
	QString const userBlueprintName;

	bool const haveFirstIndex;
	qsizetype const userFirstIndex;

	bool const haveNumCopies;
	qsizetype const userNumCopies;

	bool const force;

	static Options parseOptions(QCoreApplication const& app);
};

#endif
