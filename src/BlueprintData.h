#ifndef SPACEENGINEERS_BLUEPRINTDUPLICATOR_BLUEPRINTDATA_H_
#define SPACEENGINEERS_BLUEPRINTDUPLICATOR_BLUEPRINTDATA_H_

#include <QByteArray>
#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include <optional>

class Options;

class BlueprintData {
public:
	BlueprintData(QString const& gridName, QString const& displayName, QString const& groupName, QString const& nameTag, QStringList const& itemNames, int id);

	QString const& getGridName() const;
	QString const& getDisplayName() const;
	QString const& getGroupName() const;
	QString const& getNameTag() const;
	QStringList const& getItemNames() const;
	int getId() const;

	static std::optional<BlueprintData> fromXml(QByteArray const& data, Options const& options);

	static QByteArray toXMLWithNewId(QByteArray const& data, BlueprintData const& blueprintData, qsizetype newId, Options const& options);
	static QString cutDigitsFromEnd(QString s);
	static bool isValidBlueprintLocation(QDir dir);
private:
	QString const m_gridName;
	QString const m_displayName;
	QString const m_groupName;
	QString const m_nameTag;
	QStringList const m_itemNames;
	int const m_id;

	static QRegularExpression const expressionCustomDataMissileNumber;
	static QRegularExpression const expressionCustomDataMissileNameTag;
};

#endif
