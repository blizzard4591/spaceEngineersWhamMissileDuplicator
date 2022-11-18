#include "BlueprintData.h"

#include <QFile>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <iostream>
#include <stack>

BlueprintData::BlueprintData(QString const& gridName, QString const& displayName, QString const& groupName, QStringList const& itemNames, int id) : m_gridName(gridName), m_displayName(displayName), m_groupName(groupName), m_itemNames(itemNames), m_id(id) {
	//
}

QString const& BlueprintData::getGridName() const {
	return m_gridName;
}

QString const& BlueprintData::getDisplayName() const {
	return m_displayName;
}

QString const& BlueprintData::getGroupName() const {
    return m_groupName;
}

QStringList const& BlueprintData::getItemNames() const {
	return m_itemNames;
}

int BlueprintData::getId() const {
    return m_id;
}

std::optional<BlueprintData> BlueprintData::fromXml(QByteArray const& data) {
    QXmlStreamReader reader(data);

    std::stack<QString> stack;

    bool haveIdSubType = false;
    QString idSubType;

    bool haveDisplayName = false;
    QString displayName;

    bool haveGroupName = false;
    QString groupName;

    bool haveCustomData = false;
    QString customData;

    QStringList itemNames;

    while (!reader.atEnd()) {
        auto const token = reader.readNext();
        if (token == QXmlStreamReader::Invalid) {
            continue;
        }

        // std::cout << "Found token: " << reader.tokenString().toStdString() << std::endl;
        switch (reader.tokenType()) {
            case QXmlStreamReader::StartElement: {
                QString const name = reader.name().toString();
                
                QString const top = (stack.empty() ? QString() : stack.top());
                stack.push(name);

                // std::cout << "Found start element: " << name.toStdString() << " (depth: " << stack.size() << ")" << std::endl;
                if ((!stack.empty()) && (top == QStringLiteral("ShipBlueprint")) && (name == QStringLiteral("Id"))) {
                    haveIdSubType = true;
                    auto const attrs = reader.attributes();
                    if (!attrs.hasAttribute("", "Subtype")) {
                        std::cerr << "Attr Subtype not defined?" << std::endl;
                        return std::nullopt;
                    }

                    idSubType = attrs.value("", "Subtype").toString();
                } else if ((!stack.empty()) && (top == QStringLiteral("CubeGrid")) && (name == QStringLiteral("DisplayName"))) {
                    haveDisplayName = true;
                    displayName = reader.readElementText();

                    // this operation consumed the EndElement
                    stack.pop();
                } else if ((!stack.empty()) && (top == QStringLiteral("MyObjectBuilder_BlockGroup")) && (name == QStringLiteral("Name"))) {
                    if (haveGroupName) {
                        std::cerr << "Error: More than one block group defined!" << std::endl;
                        return std::nullopt;
                    }
                    haveGroupName = true;
                    groupName = reader.readElementText();

                    // this operation consumed the EndElement
                    stack.pop();
                } else if ((!stack.empty()) && (top == QStringLiteral("MyObjectBuilder_CubeBlock")) && (name == QStringLiteral("CustomName"))) {
                    itemNames.push_back(reader.readElementText());

                    // this operation consumed the EndElement
                    stack.pop();
                }
                break;
            }
            case QXmlStreamReader::EndElement: {
                QString const name = reader.name().toString();
                
                // std::cout << "Found end element: " << name.toStdString() << " (depth: " << stack.size() << ")" << std::endl;
                if (stack.empty()) { std::cerr << "Invalid state, EndElement, but stack is empty!" << std::endl; return std::nullopt; }
                stack.pop();
                break;
            }
            case QXmlStreamReader::StartDocument:
                if (!stack.empty()) { std::cerr << "Invalid state, StartDocument but not looking for it!" << std::endl; return std::nullopt; }
                break;
            case QXmlStreamReader::EndDocument:
                if (!stack.empty()) { std::cerr << "Invalid state, EndDocument but not looking for it!" << std::endl; return std::nullopt; }
                break;
            case QXmlStreamReader::Characters: {
                auto const characters = reader.text();
                if (characters.contains(QStringLiteral("Missile number="))) {
                    if (haveCustomData) {
                        std::cerr << "Error: More than one custom data for WHAM defined!" << std::endl;
                        return std::nullopt;
                    }
                    haveCustomData = true;
                    customData = characters.toString();
                }
                break;
            }
            default:
                std::cout << "Found an unhandled token: " << reader.tokenString().toStdString() << std::endl;
                return std::nullopt;
        }
    }
    if (reader.hasError()) {
        std::cerr << "Error while parsing XML: " << reader.errorString().toStdString() << std::endl;
        return std::nullopt;
    }

    if (!haveIdSubType) {
        std::cerr << "Failed to find all relevant data, missing <Id Subtype=\"X\">!" << std::endl;
        return std::nullopt;
    } else if (!haveDisplayName) {
        std::cerr << "Failed to find all relevant data, missing DisplayName in CubeGrid!" << std::endl;
        return std::nullopt;
    } else if (!haveGroupName) {
        std::cerr << "Failed to find all relevant data, missing block group over items!" << std::endl;
        return std::nullopt;
    } else if (itemNames.size() == 0) {
        std::cerr << "Failed to find all relevant data, found no items!" << std::endl;
        return std::nullopt;
    } else if (!haveCustomData) {
        std::cerr << "Failed to find all relevant data, missing the WHAM custom data!" << std::endl;
        return std::nullopt;
    }

    QString const prefix = QString("(%1) ").arg(groupName);
    for (int i = 0; i < itemNames.size(); ++i) {
        if (!itemNames.at(i).startsWith(prefix)) {
            std::cerr << "Item '" << itemNames.at(i).toStdString() << "' is missing the prefix '" << prefix.toStdString() << "' based on the group name, this blueprint is broken." << std::endl;
            return std::nullopt;
        }
    }

    QRegularExpression expr(R"( (\d+)$)");
    auto matchGridName = expr.match(idSubType);
    if (!matchGridName.isValid() || !matchGridName.hasMatch()) {
        std::cerr << "Failed to match number at the end of Grid Name '" << idSubType.toStdString() << "'!" << std::endl;
        return std::nullopt;
    }
    int const numberGridName = matchGridName.captured(1).toInt();

    auto matchDisplayName = expr.match(displayName);
    if (!matchDisplayName.isValid() || !matchDisplayName.hasMatch()) {
        std::cerr << "Failed to match number at the end of Display Name '" << displayName.toStdString() << "'!" << std::endl;
        return std::nullopt;
    }
    int const numberDisplayName = matchDisplayName.captured(1).toInt();

    auto matchGroupName = expr.match(groupName);
    if (!matchGroupName.isValid() || !matchGroupName.hasMatch()) {
        std::cerr << "Failed to match number at the end of Group Name '" << groupName.toStdString() << "'!" << std::endl;
        return std::nullopt;
    }
    int const numberGroupName = matchGroupName.captured(1).toInt();

    QRegularExpression expressionCustomDataMissileNumber(R"(\nMissile number=(\d+)\n)", QRegularExpression::MultilineOption);
    auto matchCustomDataMissileNumber = expressionCustomDataMissileNumber.match(customData);
    if (!matchCustomDataMissileNumber.isValid() || !matchCustomDataMissileNumber.hasMatch()) {
        std::cerr << "Failed to match the missile number in the WHAM custom data!" << std::endl;
        std::cerr << "Custom Data: " << customData.toStdString() << std::endl;
        return std::nullopt;
    }
    int const numberMissileCustomData = matchCustomDataMissileNumber.captured(1).toInt();

    if ((numberGridName != numberDisplayName) || (numberDisplayName != numberGroupName) || (numberGridName != numberMissileCustomData)) {
        std::cerr << "Numbering on Grid Name, Display Name, Group Name and WHAM custom data does NOT match: " << numberGridName << " vs. " << numberDisplayName << " vs. " << numberGroupName << " vs. " << numberMissileCustomData << "!" << std::endl;
        return std::nullopt;
    }

    std::cout << "Info: Found GridName='" << idSubType.toStdString() << "', DisplayName='" << displayName.toStdString() << "', GroupName='" << groupName.toStdString() << "' and " << itemNames.size() << " items, which all have the group name prefix." << std::endl;

    return BlueprintData(idSubType, displayName, groupName, itemNames, numberGroupName);
}

QString BlueprintData::cutDigitsFromEnd(QString s) {
    while (s.size() > 0) {
        QChar const c = s.at(s.size() - 1);
        if (('0' <= c) && (c <= '9')) {
            s.chop(1);
        } else {
            break;
        }
    }
    return s;
}

QByteArray BlueprintData::toXMLWithNewId(QByteArray const& data, BlueprintData const& blueprintData, qsizetype newId) {
    // Replacement Data:
    QString const idSubType = cutDigitsFromEnd(blueprintData.getGridName()).append(QString::number(newId));
    QString const displayName = cutDigitsFromEnd(blueprintData.getDisplayName()).append(QString::number(newId));
    QString const groupName = cutDigitsFromEnd(blueprintData.getGroupName()).append(QString::number(newId));
    QString const oldItemPrefix = QString("(%1)").arg(blueprintData.getGroupName());
    QString const newItemPrefix = QString("(%1)").arg(groupName);

    QXmlStreamReader reader(data);
    QByteArray result;
    QXmlStreamWriter writer(&result);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    QRegularExpression const expressionCustomDataMissileNumber(R"(^Missile number=(\d+)$)", QRegularExpression::MultilineOption);

    std::stack<QString> stack;
    while (!reader.atEnd()) {
        auto const token = reader.readNext();
        switch (token) {
            case QXmlStreamReader::StartElement: {
                QString const name = reader.name().toString();
                QString const namespaceUri = reader.namespaceUri().toString();

                QString const top = (stack.empty() ? QString() : stack.top());
                stack.push(name);

                writer.writeStartElement(namespaceUri, name);
                auto const namespaces = reader.namespaceDeclarations();
                for (qsizetype i = 0; i < namespaces.size(); ++i) {
                    writer.writeNamespace(namespaces.at(i).namespaceUri().toString(), namespaces.at(i).prefix().toString());
                }

                // std::cout << "Found start element: " << name.toStdString() << " (depth: " << stack.size() << ")" << std::endl;
                if ((!stack.empty()) && (top == QStringLiteral("ShipBlueprint")) && (name == QStringLiteral("Id"))) {
                    auto const attrs = reader.attributes();
                    if (!attrs.hasAttribute("", "Subtype")) {
                        std::cerr << "Attr Subtype not defined?" << std::endl;
                        return QByteArray();
                    }

                    writer.writeAttribute(QStringLiteral("Type"), attrs.value("", QStringLiteral("Type")).toString());
                    writer.writeAttribute(QStringLiteral("Subtype"), idSubType);
                } else if ((!stack.empty()) && (top == QStringLiteral("CubeGrid")) && (name == QStringLiteral("DisplayName"))) {
                    writer.writeCharacters(displayName);
                    
                    // consume the characters to stop them from appearing next
                    reader.readElementText();

                    // this operation consumed the EndElement
                    writer.writeEndElement();
                    stack.pop();
                } else if ((!stack.empty()) && (top == QStringLiteral("MyObjectBuilder_BlockGroup")) && (name == QStringLiteral("Name"))) {
                    //writer.writeStartElement(QStringLiteral("Name"));
                    writer.writeCharacters(groupName);

                    // consume the characters to stop them from appearing next
                    reader.readElementText();
                    
                    // this operation consumed the EndElement
                    writer.writeEndElement();
                    stack.pop();
                } else if ((!stack.empty()) && (top == QStringLiteral("MyObjectBuilder_CubeBlock")) && (name == QStringLiteral("CustomName"))) {
                    QString oldItemName = reader.readElementText();
                    QString const newItemName = oldItemName.replace(oldItemPrefix, newItemPrefix);

                    //writer.writeStartElement(QStringLiteral("CustomName"));
                    writer.writeCharacters(newItemName);
                    writer.writeEndElement();

                    // this operation consumed the EndElement
                    stack.pop();
                } else {
                    writer.writeAttributes(reader.attributes());
                }
                break;
            }
            case QXmlStreamReader::EndElement: {
                QString const name = reader.name().toString();

                // std::cout << "Found end element: " << name.toStdString() << " (depth: " << stack.size() << ")" << std::endl;
                if (stack.empty()) { std::cerr << "Invalid state, EndElement, but stack is empty!" << std::endl; return QByteArray(); }
                stack.pop();
                writer.writeEndElement();
                break;
            }
            case QXmlStreamReader::StartDocument:
                if (!stack.empty()) { std::cerr << "Invalid state, StartDocument but not looking for it!" << std::endl; return QByteArray(); }
                writer.writeStartDocument();
                break;
            case QXmlStreamReader::EndDocument:
                if (!stack.empty()) { std::cerr << "Invalid state, EndDocument but not looking for it!" << std::endl; return QByteArray(); }
                writer.writeEndDocument();
                break;
            case QXmlStreamReader::Characters: {
                auto characters = reader.text().toString();
                if (characters.contains(QStringLiteral("Missile number="))) {
                    auto matchCustomDataMissileNumber = expressionCustomDataMissileNumber.match(characters);
                    if (!matchCustomDataMissileNumber.isValid() || !matchCustomDataMissileNumber.hasMatch()) {
                        std::cerr << "Failed to match the missile number in the WHAM custom data!" << std::endl;
                        std::cerr << "Custom Data: " << characters.toStdString() << std::endl;
                        return QByteArray();
                    }
                    int const numberMissileCustomData = matchCustomDataMissileNumber.captured(1).toInt();
                    std::cout << "Old missile number = " << numberMissileCustomData << std::endl;

                    characters = characters.replace(expressionCustomDataMissileNumber, QString("Missile number=%1").arg(newId));
                }
                writer.writeCharacters(characters);
                break;
            }
            default:
                std::cout << "Found an unhandled token: " << reader.tokenString().toStdString() << std::endl;
                return QByteArray();
        }
    }
    if (reader.hasError()) {
        std::cerr << "Error while parsing XML: " << reader.errorString().toStdString() << std::endl;
        return QByteArray();
    }

    QString fix = QString::fromUtf8(result);
    
    // Quick-and-Dirty fix for Qt removing the space from '" />' to '"/>'
    fix.replace(QStringLiteral("/>"), QStringLiteral(" />"));
    
    // Quick-and-Dirty fix for Qt replacing all " by &quot;
    fix.replace(QStringLiteral("&quot;"), QStringLiteral("\""));

    result = fix.toUtf8();

    return result;
}

bool BlueprintData::isValidBlueprintLocation(QDir dir) {
    // pick a folder and check if it contains bp.spc
    auto const list = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    if (list.size() < 1) {
        return false;
    } else if (!dir.cd(list.at(0))) {
        return false;
    }
    return QFile::exists(dir.absoluteFilePath(QStringLiteral("bp.sbc")));
}
