/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

#include <fougtools/qttools/core/qstring_hfunc.h>
#include <QtCore/QSettings>
#include <gsl/gsl_util>
#include <unordered_map>

namespace Mayo {

namespace {

struct Settings_Setting {
    Property* property;
};

struct Settings_Section {
    QByteArray identifier; // Must be unique in the context of the parent group
    QString title;
    bool isDefault; // Default section in parent group
    std::vector<Settings_Setting> vecSetting;
};

struct Settings_Group {
    QByteArray identifier; // Must be unique in the context of the parent Settings object
    QString title;
    std::vector<Settings_Section> vecSection;
    Settings_Section defaultSection;
    std::function<void()> fnReset;
};

static bool isValidIdentifier(const QByteArray& identifier)
{
    return !identifier.isEmpty() && !identifier.simplified().isEmpty();
}

} // namespace

class Settings::Private {
public:
    Private()
        : m_locale(QLocale::system())
    {}

    Settings_Group& group(Settings::GroupIndex index) {
        return m_vecGroup.at(index.get());
    }

    Settings_Section& section(Settings::SectionIndex index) {
        return this->group(index.group()).vecSection.at(index.get());
    }

    QSettings m_settings;
    QLocale m_locale;
    std::vector<Settings_Group> m_vecGroup;
};

Settings::Settings(QObject* parent)
    : QObject(parent),
      d(new Private)
{
}

Settings::~Settings()
{
    delete d;
}

int Settings::groupCount() const
{
    return int(d->m_vecGroup.size());
}

QByteArray Settings::groupIdentifier(GroupIndex index) const
{
    return d->group(index).identifier;
}

QString Settings::groupTitle(GroupIndex index) const
{
    return d->group(index).title;
}

void Settings::setGroupResetFunction(GroupIndex index, std::function<void()> fn)
{
    d->group(index).fnReset = std::move(fn);
}

Settings::GroupIndex Settings::addGroup(TextId identifier)
{
    auto index = this->addGroup(static_cast<QByteArray>(identifier));
    this->setGroupTitle(index, identifier.tr());
    return index;
}

Settings::GroupIndex Settings::addGroup(QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));

    for (const Settings_Group& group : d->m_vecGroup) {
        if (group.identifier == identifier)
            return GroupIndex(&group - &d->m_vecGroup.front());
    }

    d->m_vecGroup.push_back({});
    Settings_Group& group = d->m_vecGroup.back();
    group.identifier = identifier;
    group.title = QString::fromUtf8(identifier);
    return GroupIndex(int(d->m_vecGroup.size()) - 1);
}

void Settings::setGroupTitle(GroupIndex index, const QString& title)
{
    d->group(index).title = title;
}

int Settings::sectionCount(GroupIndex index) const
{
    return int(d->group(index).vecSection.size());
}

QByteArray Settings::sectionIdentifier(SectionIndex index) const
{
    return d->section(index).identifier;
}

QString Settings::sectionTitle(SectionIndex index) const
{
    return d->section(index).title;
}

bool Settings::isDefaultGroupSection(SectionIndex index) const
{
    return d->section(index).isDefault;
}

Settings::SectionIndex Settings::addSection(GroupIndex index, TextId identifier)
{
    auto sectionIndex = this->addSection(index, static_cast<QByteArray>(identifier));
    this->setSectionTitle(sectionIndex, identifier.tr());
    return sectionIndex;
}

Settings::SectionIndex Settings::addSection(GroupIndex index, QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));
    // TODO Check identifier is unique

    Settings_Group& group = d->group(index);
    group.vecSection.push_back({});
    Settings_Section& section = group.vecSection.back();
    section.identifier = identifier;
    section.title = QString::fromUtf8(identifier);
    return SectionIndex(index, int(group.vecSection.size()) - 1);
}

void Settings::setSectionTitle(SectionIndex index, const QString& title)
{
    d->section(index).title = title;
}

int Settings::settingCount(SectionIndex index) const
{
    return int(d->section(index).vecSetting.size());
}

Property* Settings::property(SettingIndex index) const
{
    return d->section(index.section()).vecSetting.at(index.get()).property;
}

Settings::SettingIndex Settings::addSetting(Property* property, GroupIndex groupId)
{
    Settings_Group& group = d->group(groupId);
    Settings_Section* sectionDefault = nullptr;
    if (group.vecSection.empty()) {
        const SectionIndex sectionId = this->addSection(groupId, MAYO_TEXT_ID("Mayo::Settings", "DEFAULT"));
        sectionDefault = &d->section(sectionId);
    }
    else {
        if (group.vecSection.front().isDefault) {
            sectionDefault = &group.vecSection.front();
        }
        else {
            group.vecSection.insert(group.vecSection.begin(), {});
            sectionDefault = &group.vecSection.front();
        }
    }

//    sectionDefault->identifier = "DEFAULT";
//    sectionDefault->title = tr("DEFAULT");
    sectionDefault->isDefault = true;
    const SectionIndex sectionId(groupId, sectionDefault - &group.vecSection.front());
    return this->addSetting(property, sectionId);
}

Settings::SettingIndex Settings::addSetting(Property* property, SectionIndex index)
{
    // TODO Check identifier is unique
    Settings_Section& section = d->section(index);
    section.vecSetting.push_back({});
    Settings_Setting& setting = section.vecSetting.back();
    setting.property = property;
    return SettingIndex(index, int(section.vecSetting.size()) - 1);
}

void Settings::resetGroup(GroupIndex index)
{
    Settings_Group& group = d->group(index);
    if (group.fnReset)
        group.fnReset();
}

void Settings::resetAll()
{
    for (Settings_Group& group : d->m_vecGroup)
        this->resetGroup(GroupIndex(&group - &d->m_vecGroup.front()));
}

const QLocale& Settings::locale() const
{
    return d->m_locale;
}

void Settings::setLocale(const QLocale& locale)
{
    d->m_locale = locale;
}

void Settings::onPropertyChanged(Property* prop)
{
    PropertyGroup::onPropertyChanged(prop);
    emit this->changed(prop);
}

} // namespace Mayo