//Maintainer: Max Howell <max.howell@methylblue.com>
//Copyright:  GPL v2

//NOTE please show restraint when adding data members to this class!
//     some users have playlists with 20,000 items or more in, one 32 bit int adds up rapidly!


#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <klistview.h> //baseclass
#include <kurl.h>      //stack allocated
#include <qcolor.h>    //stack allocated

class QColorGroup;
class QDomNode;
class QListViewItem;
class QPainter;
class MetaBundle;
class Playlist;

class PlaylistItem : public KListViewItem
{
    public:
        PlaylistItem( Playlist*, QListViewItem*, const KURL&, const QString& = "", const int length = 0 );
        PlaylistItem( Playlist*, QListViewItem*, const KURL&, const QDomNode& );

        QString exactText( int col ) const { return KListViewItem::text( col ); }
        void setText( const MetaBundle& );
        void setText( int, const QString& );

        Playlist *listView() const { return (Playlist*)KListViewItem::listView(); }
        PlaylistItem *nextSibling() const { return (PlaylistItem*)KListViewItem::nextSibling(); }

        MetaBundle metaBundle();
        QString trackName() const { return KListViewItem::text( TrackName ); }
        QString title() const { return KListViewItem::text( Title ); }
        const KURL &url() const { return m_url; }
        QString seconds() const;

        static QColor glowText;
        static QColor glowBase;

        static const QString columnName(int n);

        enum Column  { TrackName = 0,
                       Title = 1,
                       Artist = 2,
                       Album = 3,
                       Year = 4,
                       Comment = 5,
                       Genre = 6,
                       Track = 7,
                       Directory = 8,
                       Length = 9,
                       Bitrate = 10 };

        void setHeight( int i ) { KListViewItem::setHeight( i ); }

    private:
        QString text( int column ) const;
        int     compare( QListViewItem*, int, bool ) const;
        void    paintCell( QPainter*, const QColorGroup&, int, int, int );

        static QString trackName( const KURL &u ) { return u.protocol() == "file" ? u.fileName() : u.prettyURL(); }

        const KURL m_url;
        int m_cachedHeight;

        static const uint STRING_STORE_SIZE = 80;
        static QString stringStore[STRING_STORE_SIZE];
        static const QString& attemptStore( const QString& );
};

#endif
