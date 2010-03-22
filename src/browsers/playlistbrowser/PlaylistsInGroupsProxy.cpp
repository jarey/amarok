/****************************************************************************************
 * Copyright (c) 2009 Bart Cerneels <bart.cerneels@kde.org>                             *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "PlaylistsInGroupsProxy.h"

#include "AmarokMimeData.h"
#include "Debug.h"
#include "meta/Playlist.h"
#include "SvgHandler.h"
#include "UserPlaylistModel.h"
#include "playlist/PlaylistModelStack.h"

#include <KIcon>
#include <KInputDialog>

PlaylistsInGroupsProxy::PlaylistsInGroupsProxy( QAbstractItemModel *model )
    : QtGroupingProxy( model, QModelIndex(), PlaylistBrowserNS::UserModel::LabelColumn )
{
    m_renameFolderAction =  new QAction( KIcon( "media-track-edit-amarok" ),
                                         i18n( "&Rename Folder..." ), this );
    m_renameFolderAction->setProperty( "popupdropper_svg_id", "edit_group" );
    connect( m_renameFolderAction, SIGNAL( triggered() ), this,
             SLOT( slotRenameFolder() ) );

    m_deleteFolderAction = new QAction( KIcon( "media-track-remove-amarok" ),
                                        i18n( "&Delete Folder" ), this );
    m_deleteFolderAction->setProperty( "popupdropper_svg_id", "delete_group" );
    connect( m_deleteFolderAction, SIGNAL( triggered() ), this,
             SLOT( slotDeleteFolder() ) );

    connect( m_model, SIGNAL( renameIndex( QModelIndex ) ), SLOT( slotRename( QModelIndex ) ) );
}

PlaylistsInGroupsProxy::~PlaylistsInGroupsProxy()
{
}

QVariant
PlaylistsInGroupsProxy::data( const QModelIndex &idx, int role ) const
{
    if( idx.column() == 0 && isGroup( idx ) && role ==
        PlaylistBrowserNS::MetaPlaylistModel::ActionRole )
    {
        //wheter we use the list from m_deleteFolderAction or m_renameFolderAction does not matter
        //they are the same anyway
        QModelIndexList actionList = m_deleteFolderAction->data().value<QModelIndexList>();

        actionList << idx;
        QVariant value = QVariant::fromValue( actionList );
        m_deleteFolderAction->setData( value );
        m_renameFolderAction->setData( value );

        QList<QAction *> actions;
        actions << m_renameFolderAction << m_deleteFolderAction;
        return QVariant::fromValue( actions );
    }

    return QtGroupingProxy::data( idx, role );
}

bool
PlaylistsInGroupsProxy::removeRows( int row, int count, const QModelIndex &parent )
{
    DEBUG_BLOCK
    bool result;
    debug() << "in parent " << parent << "remove " << count << " starting at row " << row;
    if( !parent.isValid() )
    {
        QModelIndex folderIdx = index( row, 0, QModelIndex() );
        if( isGroup( folderIdx ) )
        {
            deleteFolder( folderIdx );
            return true;
        }

        //is a playlist not in a folder
        //FIXME: before confirming deletion of a playlist it already dissapears from the
        //view. The beginRemoveRows should not be called here but in the source model.
        QModelIndex childIdx = mapToSource( index( row, 0, m_rootNode ) );
        beginRemoveRows( QModelIndex(), row, row + count - 1 );
        result = m_model->removeRows( childIdx.row(), count, m_rootNode );
        endRemoveRows();
        return result;
    }

    if( isGroup( parent ) )
    {
        result = true;
        for( int i = row; i < row + count; i++ )
        {
            //individually remove all children of this group in the source model
            QModelIndex childIdx = mapToSource( index( i, 0, parent ) );
            //set success to false if removeRows returns false
            result = m_model->removeRow( childIdx.row(), QModelIndex() ) ? result : false;
        }
        return result;
    }

    //removing a track from a playlist
    beginRemoveRows( parent, row, row + count - 1 );
    QModelIndex originalIdx = mapToSource( parent );
    result = m_model->removeRows( row, count, originalIdx );
    endRemoveRows();

    return result;
}

QStringList
PlaylistsInGroupsProxy::mimeTypes() const
{
    QStringList mimeTypes = m_model->mimeTypes();
    mimeTypes << AmarokMimeData::PLAYLISTBROWSERGROUP_MIME;
    return mimeTypes;
}

QMimeData *
PlaylistsInGroupsProxy::mimeData( const QModelIndexList &indexes ) const
{
    DEBUG_BLOCK
    AmarokMimeData* mime = new AmarokMimeData();
    QModelIndexList sourceIndexes;
    foreach( const QModelIndex &idx, indexes )
    {
        debug() << idx;
        if( isGroup( idx ) )
        {
            debug() << "is a group, add mimeData of all children";
        }
        else
        {
            debug() << "is original item, add mimeData from source model";
            sourceIndexes << mapToSource( idx );
        }
    }

    if( !sourceIndexes.isEmpty() )
        return m_model->mimeData( sourceIndexes );

    return mime;
}

bool
PlaylistsInGroupsProxy::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent )
{
    DEBUG_BLOCK
    Q_UNUSED( row );
    Q_UNUSED( column );
    debug() << "dropped on " << QString("row: %1, column: %2, parent:").arg( row ).arg( column );
    debug() << parent;
    if( action == Qt::IgnoreAction )
    {
        debug() << "ignored";
        return true;
    }

    if( data->hasFormat( AmarokMimeData::PLAYLIST_MIME ) ||
        data->hasFormat( AmarokMimeData::PLAYLISTBROWSERGROUP_MIME ) )
    {
        debug() << "has amarok mime data";
        const AmarokMimeData *amarokMime = dynamic_cast<const AmarokMimeData *>(data);
        if( amarokMime == 0 )
        {
            error() << "could not cast to amarokMimeData";
            return false;
        }

        if( !parent.isValid() )
        {
            debug() << "dropped on the root";
            Meta::PlaylistList playlists = amarokMime->playlists();
            foreach( Meta::PlaylistPtr playlist, playlists )
                playlist->setGroups( QStringList() );
            buildTree();
            return true;
        }

        if( isGroup( parent ) )
        {
            debug() << "dropped on a group";
            if( data->hasFormat( AmarokMimeData::PLAYLIST_MIME ) )
            {
                debug() << "playlist dropped on group";
                if( parent.row() < 0 || parent.row() >= rowCount( QModelIndex() ) )
                {
                    debug() << "ERROR: something went seriously wrong in " << __FILE__ << __LINE__;
                    return false;
                }
                //apply the new groupname to the source index
                QString groupName = parent.data( Qt::DisplayRole ).toString();
                //TODO: apply the new groupname to the source index
                Meta::PlaylistList playlists = amarokMime->playlists();
                foreach( Meta::PlaylistPtr playlist, playlists )
                    playlist->setGroups( QStringList( groupName ) );
                buildTree();
                return true;
            }
            else if( data->hasFormat( AmarokMimeData::PLAYLISTBROWSERGROUP_MIME ) )
            {
                debug() << "playlistgroup dropped on group";
                //TODO: multilevel group support
                debug() << "ignore drop until we have multilevel group support";
                return false;
            }
        }
    }
    else
    {
        QModelIndex sourceIndex = mapToSource( parent );
        return m_model->dropMimeData( data, action, row, column,
                               sourceIndex );
    }

    return false;
}

Qt::DropActions
PlaylistsInGroupsProxy::supportedDropActions() const
{
    //always add MoveAction because playlists can be put into a different group
    return m_model->supportedDropActions() | Qt::MoveAction;
}

Qt::DropActions
PlaylistsInGroupsProxy::supportedDragActions() const
{
    //always add MoveAction because playlists can be put into a different group
    return m_model->supportedDragActions() | Qt::MoveAction;
}

void
PlaylistsInGroupsProxy::slotRename( QModelIndex sourceIdx )
{
    QModelIndex proxyIdx = mapFromSource( sourceIdx );
    emit renameIndex( proxyIdx );
}

void
PlaylistsInGroupsProxy::slotDeleteFolder()
{
    QAction *action = qobject_cast<QAction *>( QObject::sender() );
    if( action == 0 )
        return;

    QModelIndexList indexes = action->data().value<QModelIndexList>();

    foreach( const QModelIndex &groupIdx, indexes )
        deleteFolder( groupIdx );
}

void
PlaylistsInGroupsProxy::slotRenameFolder()
{
    QAction *action = qobject_cast<QAction *>( QObject::sender() );
    if( action == 0 )
        return;

    QModelIndexList indexes = action->data().value<QModelIndexList>();

    if( indexes.isEmpty() )
        return;

    //get the name for this new group
    //inline rename is handled by the view using setData()
    QModelIndex folder = indexes.first();
    QString folderName = folder.data( Qt::DisplayRole ).toString();
    bool ok;
    const QString newName = KInputDialog::getText( i18n("New name"),
                i18nc("Enter a new name for a folder that already exists",
                      "Enter new folder name:"),
                folderName,
                &ok );
    if( !ok || newName == folderName )
        return;

    for( int i = 0; i < rowCount( folder ); i++ )
    {
        QModelIndex idx = index( i, PlaylistBrowserNS::UserModel::LabelColumn, folder );
        setData( idx, newName, Qt::DisplayRole );
    }
    //remove the old foldername from the map
    m_groupMaps.removeAt( folder.row() );
    buildTree();
    emit layoutChanged();
}

void
PlaylistsInGroupsProxy::deleteFolder( const QModelIndex &groupIdx )
{
    int childCount = rowCount( groupIdx );
    if( childCount > 0 )
        removeRows( 0, childCount, groupIdx );
    removeGroup( groupIdx );
    buildTree();
}

QModelIndex
PlaylistsInGroupsProxy::createNewGroup( const QString &groupName )
{
    ColumnVariantMap data;
    RoleVariantMap roleData;
    roleData.insert( Qt::DisplayRole, groupName );
    roleData.insert( Qt::DecorationRole, QVariant( KIcon( "folder" ) ) );
    roleData.insert( Qt::EditRole, groupName );
    data.insert( 0, roleData );
    return addEmptyGroup( data );
}

#include "PlaylistsInGroupsProxy.moc"
