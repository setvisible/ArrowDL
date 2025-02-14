/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "fileaccessmanager.h"

#include <Core/File>

#include <QtCore/QDebug>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

FileAccessManager::FileAccessManager(QWidget *parent) : QObject(parent)
  , m_parent(parent)
{
    File::setFileAccessManager(this);
}

FileAccessManager::~FileAccessManager()
{
}

Settings *FileAccessManager::settings() const
{
    return m_settings;
}

void FileAccessManager::setSettings(Settings *settings)
{
    m_settings = settings;
}

ExistingFileOption FileAccessManager::aboutToModify(const QString &filename)
{
    QMessageBox msgBox(m_parent);
    msgBox.setWindowTitle(tr("Existing File"));
    msgBox.setText(QString("%0\n\n%1\n\n%2").arg(
                       tr("The file already exists:"),
                       filename,
                       tr("Do you want to Rename, Overwrite or Skip this file?")));
    msgBox.setIcon(QMessageBox::Icon::Question);

    QPushButton *renameButton = msgBox.addButton(tr("Rename"), QMessageBox::ActionRole);
    QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite"), QMessageBox::ActionRole);
    QPushButton *skipButton = msgBox.addButton(tr("Skip"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(skipButton);

    static bool dontAskAgain = false;
    QCheckBox *cb = new QCheckBox("Don't ask again");
    msgBox.setCheckBox(cb);
    QObject::connect(cb, &QCheckBox::stateChanged, [](int state){
        dontAskAgain = (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked);
    });

    msgBox.exec();
    ExistingFileOption answer = ExistingFileOption::Ask;

    if (msgBox.clickedButton() == renameButton) {
        answer = ExistingFileOption::Rename;
    } else if (msgBox.clickedButton() == overwriteButton) {
        answer = ExistingFileOption::Overwrite;
    } else if (msgBox.clickedButton() == skipButton) {
        answer = ExistingFileOption::Skip;
    }

    if (dontAskAgain) {
        m_settings->setExistingFileOption(answer);
    }
    return answer;
}
