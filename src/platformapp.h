#ifndef PLATFORMAPP_H
#define PLATFORMAPP_H

#include <QUrl>

#include "chatfoldersmodel.h"

namespace PlatformApp {
    QUrl pathTo(const QString &filename);
    QUrl pathToAppIcon();
    QUrl pathToChatFolderIcon(ChatFoldersModel::Icon icon);
}

#endif // PLATFORMAPP_H