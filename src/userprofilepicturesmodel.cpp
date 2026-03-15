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
        LOG("Loading initial chunk" << userId);
        tdLibWrapper->getUserFullInfo(userId);
        tdLibWrapper->getUserProfilePhotos(userId, 100, 0);
    }
}

void UserProfilePicturesModel::handleUserFullInfo(qlonglong userId, const QVariantMap &userFullInfo) {
    if (this->userId != userId)
        return;
    // FIXME: this can probably be done in a cleaner way

    /*bool hadPersonalPhoto = !personalPhoto.isEmpty();
    if (!hadPersonalPhoto)
        beginInsertColumns(QModelIndex(), 0, 0);

    personalPhoto = userFullInfo.value("personal_photo").toMap();
    if (hadPersonalPhoto) {
        const QModelIndex i = index(0);
        emit dataChanged(i, i);
    } else
        endInsertColumns();


    int publicPhotoIndex = -1;
    if (!publicPhoto.isEmpty())
        publicPhotoIndex = personalPhoto.isEmpty() ? 0 : 1;

    if (publicPhotoIndex > -1)
        beginInsertColumns(QModelIndex(), publicPhotoIndex, publicPhotoIndex);

    publicPhoto = userFullInfo.value("public_photo").toMap();
    if (publicPhotoIndex > -1) {
        const QModelIndex i = index(publicPhotoIndex);
        emit dataChanged(i, i);
    } else
        endInsertColumns();*/

    LOG("Handling user full info");

    bool containsPersonalPhoto = !profilePhotos.isEmpty() && profilePhotos.first().first == PersonalPhoto;
    if (containsPersonalPhoto) {
        if (userFullInfo.contains(PERSONAL_PHOTO)) {
            profilePhotos.replace(0, {PersonalPhoto, userFullInfo.value(PERSONAL_PHOTO).toMap()});
            const QModelIndex i = index(0);
            emit dataChanged(i, i);
        } else {
            beginRemoveRows(QModelIndex(), 0, 0);
            profilePhotos.removeFirst();
            endRemoveRows();
            containsPersonalPhoto = false;
        }
    } else if (userFullInfo.contains(PERSONAL_PHOTO)) {
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
            profilePhotos.replace(publicPhotoIndex, {PublicPhoto, userFullInfo.value(PERSONAL_PHOTO).toMap()});
            const QModelIndex i = index(publicPhotoIndex);
            emit dataChanged(i, i);
        } else {
            beginRemoveRows(QModelIndex(), publicPhotoIndex, publicPhotoIndex);
            profilePhotos.removeAt(publicPhotoIndex);
            endRemoveRows();
        }
    } else if (userFullInfo.contains(PUBLIC_PHOTO)) {
        const int insertIndex = containsPersonalPhoto ? 1 : 0;
        beginInsertRows(QModelIndex(), insertIndex, insertIndex);
        profilePhotos.prepend({PublicPhoto, userFullInfo.value(PUBLIC_PHOTO).toMap()});
        endInsertRows();
    }
}

void UserProfilePicturesModel::handleChatPhotosReceived(qlonglong chatId, const QVariantList &photos, int totalCount) {
    if (this->userId == chatId && !photos.isEmpty()) {
        LOG("User profile photos received" << chatId << totalCount);
        this->totalCount = totalCount;
        // TODO: first, add the currently set photo to the list, next remove it once we get it here
        // if (... CurrentPhoto) ... removeAt();
        beginInsertRows(QModelIndex(), this->profilePhotos.size(), this->profilePhotos.size() + photos.size() - 1);
        for (const QVariant &photo : photos)
            this->profilePhotos.append({Photo, photo.toMap()});
        endInsertRows();
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
    };
}

int UserProfilePicturesModel::rowCount(const QModelIndex &) const {
    return profilePhotos.size();
}

QVariant UserProfilePicturesModel::data(const QModelIndex &index, int role) const {
    /*const bool havePersonalPhoto = !personalPhoto.isEmpty();
    const int additionalPhotos = havePersonalPhoto + !publicPhoto.isEmpty();
    const int i = index.row();*/
    if (index.isValid() && index.row() < profilePhotos.size() /*+ additionalPhotos*/) {
        /*QVariantMap photo;
        if (additionalPhotos == 1 && i == 0) {
            return havePersonalPhoto ? personalPhoto : publicPhoto;
        } else if (additionalPhotos == 2 && i >= 0 && i <= 1)
            photo = i == 0 ? personalPhoto : publicPhoto;
        else
            photo = profilePhotos.at(index.row());*/

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
        }
    }
    return QVariant();
}

int UserProfilePicturesModel::additionalPhotosCount() {
    int result = 0;
    for (int i=0; i < qMin(2, profilePhotos.size()); i++)
        if (profilePhotos.at(0).first != Photo)
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
