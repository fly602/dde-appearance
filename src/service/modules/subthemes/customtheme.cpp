/*
 * Copyright (C) 2021 ~ 2023 Deepin Technology Co., Ltd.
 *
 * Author:     caixiangrong <caixiangrong@uniontech.com>
 *
 * Maintainer: caixiangrong <caixiangrong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "customtheme.h"
#include "theme.h"
#include "../api/keyfile.h"
#include "../common/commondefine.h"

#include <QDir>
#include <QDebug>

#define CUSTOMTHEMEPATH ".cache/deepin/dde-appearance/deepin-themes/custom/"
#define THEMEFILE "index.theme"

CustomTheme::CustomTheme(QObject *parent)
    : QObject(parent)
    , m_customTheme(new KeyFile(','))
{
    openCustomTheme();
}

void CustomTheme::updateValue(const QString &type, const QString &value, const QString &oldTheme, const QVector<QSharedPointer<Theme>> &globalThemes)
{
    static const QMap<QString, QString> typekeyMap = {
        { TYPEBACKGROUND, "Wallpaper" },
        { TYPEGREETERBACKGROUND, "LockBackground" },
        { TYPEICON, "IconTheme" },
        { TYPECURSOR, "CursorTheme" },
        { TYPEGTK, "AppTheme" },
        { TYPESTANDARDFONT, "StandardFont" },
        { TYPEMONOSPACEFONT, "MonospaceFont" },
        { TYPEFONTSIZE, "FontSize" },
        { TYPEACTIVECOLOR, "ActiveColor" },
//        { TYPESTANDARDFONT, "DockBackground" },
        { TYPEDOCKOPACITY, "DockOpacity" },
//        { TYPESTANDARDFONT, "LauncherOpacity" },
        { TYPWINDOWRADIUS, "WindowRadius" },
        { TYPEWINDOWOPACITY, "WindowOpacity" }
    };
    if (!typekeyMap.contains(type))
        return;
    enum GolbalThemeMode {
        Light = 1,
        Dark = 2,
        Auto = 3,
    };
    QString themeId = oldTheme;
    GolbalThemeMode mode = Auto;
    QString modeStr;
    if (themeId.endsWith(".light")) {
        themeId.chop(6);
        mode = Light;
        modeStr = ".light";
    } else if (oldTheme.endsWith(".dark")) {
        themeId.chop(5);
        mode = Dark;
        modeStr = ".dark";
    }
    if (themeId != "custom") {
        QString themePath;
        for (auto iter : globalThemes) {
            if (iter->getId() == themeId) {
                themePath = iter->getPath();
                break;
            }
        }
        copyTheme(themePath, typekeyMap.values());
        Q_EMIT updateToCustom(modeStr);
    }

    if (mode & Light)
        m_customTheme->setKey("DefaultTheme", typekeyMap.value(type), value);
    if (mode & Dark)
        m_customTheme->setKey("DarkTheme", typekeyMap.value(type), value);
    saveCustomTheme();
}

void CustomTheme::openCustomTheme()
{
    QDir home = QDir::home();
    home.cd(CUSTOMTHEMEPATH);
    m_customTheme->loadFile(home.absoluteFilePath(THEMEFILE));
    if (m_customTheme->getStr("Deepin Theme", "Name").isEmpty()) {
        m_customTheme->setKey("Deepin Theme", "Name", "Custom");
        m_customTheme->setKey("Deepin Theme", "DefaultTheme", "DefaultTheme");
        m_customTheme->setKey("Deepin Theme", "DarkTheme", "DarkTheme");
        m_customTheme->setKey("Deepin Theme", "Example", "/usr/share/dde-appearance/custom.svg");
    }
}

void CustomTheme::saveCustomTheme()
{
    QDir home = QDir::home();
    home.mkdir(CUSTOMTHEMEPATH);
    home.cd(CUSTOMTHEMEPATH);
    m_customTheme->saveToFile(home.absoluteFilePath(THEMEFILE));
}

void CustomTheme::copyTheme(const QString &themePath, const QStringList &keys)
{
    if (themePath.isEmpty())
        return;
    KeyFile theme(',');
    theme.loadFile(themePath + "/index.theme");
    QString defaultTheme = theme.getStr("Deepin Theme", "DefaultTheme");
    if (defaultTheme.isEmpty())
        return;
    QString darkTheme = theme.getStr("Deepin Theme", "DarkTheme");
    if (darkTheme.isEmpty())
        darkTheme = defaultTheme;

    for (auto &&key : keys) {
        QString value = theme.getStr(defaultTheme, key);
        m_customTheme->setKey("DefaultTheme", key, value);

        value = theme.getStr(darkTheme, key, value);
        m_customTheme->setKey("DarkTheme", key, value);
    }
}