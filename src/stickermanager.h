/*
    Copyright (C) 2020 Sebastian J. Wolf and other contributors

    This file is part of Fernschreiber.

    Fernschreiber is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fernschreiber is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fernschreiber. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <QVariantList>

#include "tdlib/tdlibwrapper.h"

class StickerManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QList<int> recentStickerIds MEMBER recentStickerIds NOTIFY recentStickersChanged)
    Q_PROPERTY(QList<int> favoriteStickerIds MEMBER favoriteStickerIds NOTIFY favoriteStickersChanged)

public:
    explicit StickerManager(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);
    ~StickerManager();

    //Q_INVOKABLE QVariantList getInstalledStickerSets();
    Q_INVOKABLE QVariantMap getStickerSet(const QString &stickerSetId);
    Q_INVOKABLE bool hasStickerSet(const QString &stickerSetId);

signals:
    void recentStickersChanged();
    void favoriteStickersChanged();
    void stickerSetUpdated(const QString &stickerSetId);
    void stickerSetStickersUpdated(const QString &stickerSetId);
    void stickerSetsReceived();

private slots:
    void handleRecentStickersUpdated(bool isAttached, const QList<int> &stickerIds);
    void handleFavoriteStickersUpdated(const QList<int> &stickerIds);
    void handleStickerSetUpdated(const QString &stickerSetId, const QVariantMap &stickerSet);
    //void handleStickersReceived(const QVariantList &stickers);
    void handleStickerSetReceived(const QString &stickerSetId, const QVariantMap &stickerSet);

private:
    void handleStickerSet(const QString &stickerSetId, const QVariantMap &stickerSet);

private:
    TDLibWrapper *tdLibWrapper;

    QList<int> recentStickerIds;
    QList<int> favoriteStickerIds;
    QMap<QString, QVariantMap> stickerSets;
};
