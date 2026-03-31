#include "userprofilepicturesmodel.h"
#include "utilities.h"

namespace {
    const QString ID("id");
    const QString SMALL_ANIMATION("small_animation");
    const QString ANIMATION("animation");
    const QString PERSONAL_PHOTO("personal_photo");
    const QString PUBLIC_PHOTO("public_photo");
    const QString SIZES("sizes");
    const QString PHOTO("photo");
}

#define DEBUG_MODULE UserProfilePicturesModel
#include "debuglog.h"

UserProfilePicturesModel::UserProfilePicturesModel(QObject *parent) : QAbstractListModel(parent) {}

void UserProfilePicturesModel::setTDLibWrapper(QObject *obj) {
    TDLibWrapper *wrapper = qobject_cast<TDLibWrapper*>(obj);
    if (tdLibWrapper != wrapper) {
        tdLibWrapper = wrapper;
        emit tdlibChanged();

        if (tdLibWrapper) {
            connect(tdLibWrapper, &TDLibWrapper::userFullInfoReceived, this, &UserProfilePicturesModel::handleUserFullInfo);
            connect(tdLibWrapper, &TDLibWrapper::userFullInfoUpdated, this, &UserProfilePicturesModel::handleUserFullInfo);
            connect(tdLibWrapper, &TDLibWrapper::chatPhotosReceived, this, &UserProfilePicturesModel::handleChatPhotosReceived);
        }

        setup();
    }
}

void UserProfilePicturesModel::setUserId(qlonglong userId) {
    if (this->userId != userId) {
        this->userId = userId;
        emit userIdChanged();
        setup();
    }
}

void UserProfilePicturesModel::setup() {
    if (tdLibWrapper && userId) {
        if (!profilePhotos.isEmpty()) {
            beginResetModel();
            profilePhotos.clear();
            endResetModel();
        }

        if (userId == tdLibWrapper->myUserId())
            connect(tdLibWrapper, &TDLibWrapper::okReceived, this, &UserProfilePicturesModel::handleOkReceived);

        LOG("Loading initial chunk" << userId);
        tdLibWrapper->getUserFullInfo(userId);
        tdLibWrapper->getUserProfilePhotos(userId, 100, 0);
    }
}

void UserProfilePicturesModel::handleUserFullInfo(qlonglong userId, const QVariantMap &userFullInfo) {
    if (this->userId != userId)
        return;

    LOG("Handling user full info");
    // FIXME: this can probably be done in a cleaner way

    const qlonglong newCurrentPhotoId = userFullInfo.value(PHOTO).toMap().value(ID).toLongLong();
    if (this->currentPhotoId != newCurrentPhotoId) {
        const qlonglong oldCurrentPhotoId = this->currentPhotoId;
        this->currentPhotoId = newCurrentPhotoId;
        LOG("Current photo ID changed" << oldCurrentPhotoId << currentPhotoId);

        QModelIndex i;
        const int additionalCount = this->additionalPhotosCount();
        if (indexMap.contains(oldCurrentPhotoId)) {
            i = index(indexMap.value(oldCurrentPhotoId) + additionalCount);
            emit dataChanged(i, i, {RoleIsCurrent});
        }
        if (indexMap.contains(newCurrentPhotoId)) {
            i = index(indexMap.value(newCurrentPhotoId) + additionalCount);
            emit dataChanged(i, i, {RoleIsCurrent});
        }
    }

    bool containsPersonalPhoto = !profilePhotos.isEmpty() && profilePhotos.first().first == PersonalPhoto;
    if (containsPersonalPhoto) {
        if (userFullInfo.contains(PERSONAL_PHOTO)) {
            LOG("Personal photo updated");
            profilePhotos.replace(0, {PersonalPhoto, userFullInfo.value(PERSONAL_PHOTO).toMap()});
            const QModelIndex i = index(0);
            emit dataChanged(i, i);
        } else {
            LOG("Personal photo removed");
            beginRemoveRows(QModelIndex(), 0, 0);
            profilePhotos.removeFirst();
            endRemoveRows();
            containsPersonalPhoto = false;
        }
    } else if (userFullInfo.contains(PERSONAL_PHOTO)) {
        LOG("Personal photo added");
        beginInsertRows(QModelIndex(), 0, 0);
        profilePhotos.prepend({PersonalPhoto, userFullInfo.value(PERSONAL_PHOTO).toMap()});
        endInsertRows();
        containsPersonalPhoto = true;
    }


    int publicPhotoIndex = -1;
    if (!profilePhotos.isEmpty()) {
        if (profilePhotos.first().first == PublicPhoto)
            publicPhotoIndex = 0;
        else if (profilePhotos.size() > 1 && profilePhotos.at(1).first == PublicPhoto)
            publicPhotoIndex = 1;
    }

    if (publicPhotoIndex > -1) {
        if (userFullInfo.contains(PUBLIC_PHOTO)) {
            LOG("Public photo updated");
            profilePhotos.replace(publicPhotoIndex, {PublicPhoto, userFullInfo.value(PUBLIC_PHOTO).toMap()});
            const QModelIndex i = index(publicPhotoIndex);
            emit dataChanged(i, i);
        } else {
            LOG("Public photo removed");
            beginRemoveRows(QModelIndex(), publicPhotoIndex, publicPhotoIndex);
            profilePhotos.removeAt(publicPhotoIndex);
            endRemoveRows();
        }
    } else if (userFullInfo.contains(PUBLIC_PHOTO)) {
        LOG("Public photo added");
        const int insertIndex = containsPersonalPhoto ? 1 : 0;
        beginInsertRows(QModelIndex(), insertIndex, insertIndex);
        profilePhotos.insert(insertIndex, {PublicPhoto, userFullInfo.value(PUBLIC_PHOTO).toMap()});
        endInsertRows();
    }
}

void UserProfilePicturesModel::handleChatPhotosReceived(qlonglong chatId, const QVariantList &photos, int totalCount) {
    if (this->userId == chatId && !photos.isEmpty()) {
        LOG("User profile photos received" << chatId << totalCount);
        this->totalCount = totalCount;
        const int additionalCount = this->additionalPhotosCount();

        // TODO: first, add the currently set photo to the list, next remove it once we get it here
        // if (... CurrentPhoto) ... removeAt();
        beginInsertRows(QModelIndex(), this->profilePhotos.size(), this->profilePhotos.size() + photos.size() - 1);
        for (const QVariant &photoVariant : photos) {
            const QVariantMap photo = photoVariant.toMap();
            this->profilePhotos.append({Photo, photo});
            indexMap.insert(photo.value(ID).toLongLong(), profilePhotos.size() - 1 - additionalCount);
            LOG("Inserting photo" << photo.value(ID).toLongLong() << "with relative index" << profilePhotos.size() - 1 - additionalCount);
        }
        endInsertRows();
    }
}

void UserProfilePicturesModel::handleOkReceived(const QVariant &extraVariant) {
    if (extraVariant.userType() == QMetaType::QString) {
        const QString extra = extraVariant.toString();
        if (extra.startsWith("deleteProfilePhoto:")) {
            qlonglong id = extra.mid(19).toLongLong();
            if (indexMap.contains(id)) {
                const int additionalCount = this->additionalPhotosCount();
                const int index = indexMap.take(id) + additionalCount;
                beginRemoveRows(QModelIndex(), index, index);
                profilePhotos.removeAt(index);
                LOG("Removed profile photo at" << index << additionalCount << additionalPhotosCount());
                for (int i = index; i < profilePhotos.size(); i++)
                    indexMap.insert(profilePhotos.at(i).second.value(ID).toLongLong(), i - additionalCount);
                endRemoveRows();
            }
        }
    }
}

QHash<int, QByteArray> UserProfilePicturesModel::roleNames() const {
    return {
        {RoleDisplay, "display"},
        {RoleType, "type"},
        {RoleId, "photo_id"},
        {RoleAddedDate, "added_date"},
        {RoleMinithumbnail, "photo_minithumbnail"},
        {RoleSmallPhoto, "small_photo"},
        {RoleBigPhoto, "big_photo"},
        {RoleAnimation, "photo_animation"},
        {RoleIsCurrent, "is_current"},
    };
}

int UserProfilePicturesModel::rowCount(const QModelIndex &) const {
    return profilePhotos.size();
}

QVariant UserProfilePicturesModel::data(const QModelIndex &index, int role) const {
    if (index.isValid() && index.row() < profilePhotos.size()) {
        auto pair = profilePhotos.at(index.row());
        const QVariantMap &photo = pair.second;
        switch (static_cast<Role>(role)) {
        case RoleDisplay: return photo;
        case RoleType: return pair.first;
        case RoleId: return photo.value(ID).toString();
        case RoleAddedDate: return photo.value("added_date").toInt();
        case RoleMinithumbnail: return photo.value("minithumbnail").toMap();
        case RoleSmallPhoto:
            return Utilities::findSmallestPhotoSize(photo.value(SIZES).toList());
        case RoleBigPhoto:
            return Utilities::findBiggestPhotoSize(photo.value(SIZES).toList());
        case RoleAnimation:
            if (photo.contains(ANIMATION))
                return photo.value(ANIMATION).toMap();
            else if (photo.contains(SMALL_ANIMATION))
                return photo.value(SMALL_ANIMATION).toMap();
            return QVariant();
        case RoleIsCurrent:
            return photo.value(ID).toLongLong() == this->currentPhotoId;
        }
    }
    return QVariant();
}

int UserProfilePicturesModel::additionalPhotosCount() {
    int result = 0;
    for (int i=0; i < qMin(2, profilePhotos.size()); i++)
        if (profilePhotos.at(i).first != Photo)
            result++;

    return result;
}

void UserProfilePicturesModel::loadMore() {
    if (tdLibWrapper && userId) {
        const int count = profilePhotos.size() - additionalPhotosCount();
        if (totalCount == -1 || totalCount > count) {
            LOG("Loading more" << userId);
            totalCount = 0; // don't allow loading more until the next chunk
            tdLibWrapper->getUserProfilePhotos(userId, 100, count);
        }
    }
}
