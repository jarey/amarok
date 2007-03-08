/* This file is part of the KDE project
   Copyright (C) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef SQLMETA_H
#define SQLMETA_H

#include "meta.h"

#include <QMutex>
#include <QStringList>

using namespace Meta;

class SqlTrack : public Track
{
    public:
        SqlTrack( const QStringList &queryResult );

        virtual QString name() const { return m_title; }
        virtual QString prettyName() const;

        virtual KUrl playableUrl() const { return m_url; }
        virtual QString prettyUrl() const { return m_url.path(); }
        virtual QString url() const { return m_url.path(); }

        virtual bool isPlayable() const;
        virtual bool isEditable() const;

        virtual AlbumPtr album() const { return m_album; }
        virtual void setAlbum( const QString &newAlbum );
        virtual void setArtist( const QString &newArtist );
        virtual ArtistPtr artist() const { return m_artist; }
        virtual ComposerPtr composer() const { return m_composer; }
        virtual void setComposer( const QString &newComposer );
        virtual YearPtr year() const { return m_year; }
        virtual void setYear( const QString &newYear );
        virtual GenrePtr genre() const { return m_genre; }
        virtual void setGenre( const QString &newGenre );

        virtual QString type() const;

        //helper functions
        static QString prettyTitle( const QString &filename );

        virtual void subscribe( TrackObserver *observer );
        virtual void unsubscribe( TrackObserver *observer );

        virtual double score() const { return m_score; }
        virtual void setScore( double newScore );
        virtual int rating() const { return m_rating; }
        virtual void setRating( int newRating );
        virtual int length() const { return m_length; }
        virtual int filesize() const { return m_filesize; }
        virtual int sampleRate() const { return m_sampleRate; }
        virtual int bitrate() const { return m_bitrate; }
        virtual int trackNumber() const { return m_trackNumber; }
        virtual void setTrackNumber( int newTrackNumber );
        virtual int discNumber() const { return m_discNumber; }
        virtual void setDiscNumber( int newDiscNumber );
        virtual uint lastPlayed() const { return m_lastPlayed; }
        virtual int playCount() const { return m_playCount; }

    protected:
        void notifyObservers();

    private:

        QString m_title;
        KUrl m_url;

        int m_length;
        int m_filesize;
        int m_trackNumber;
        int m_discNumber;
        uint m_lastPlayed;
        int m_playCount;
        int m_bitrate;
        int m_sampleRate;
        int m_rating;
        double m_score;

        AlbumPtr m_album;
        ArtistPtr m_artist;
        GenrePtr m_genre;
        ComposerPtr m_composer;
        YearPtr m_year;

        QList<TrackObserver*> m_observers;
};

class SqlArtist : public Artist
{
    public:
        SqlArtist( const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; } //change if necessary

        virtual void invalidateCache();

        virtual TrackList tracks();

    private:
        QString m_name;
        bool m_tracksLoaded;
        TrackList m_tracks;
        //QReadWriteLock doesnt support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;

};

class SqlAlbum : public Album
{
    public:
        SqlAlbum( const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        virtual void invalidateCache();

        virtual TrackList tracks();

        virtual bool isCompilation() const { return false; } //TODO: fixme

        virtual bool hasAlbumArtist() const { return false; } //TODO: fixme
        virtual ArtistPtr albumArtist() const { return ArtistPtr(); }

        //updating album images is possible or local tracks, but let's ignore it for now
        virtual bool canUpdateImage() const { return false; }
        virtual void image() const { }  //TODO: fixme
        virtual void updateImage() { }

    private:
        QString m_name;
        bool m_tracksLoaded;
        TrackList m_tracks;
        //QReadWriteLock doesnt support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;

        //TODO: add album artist
};

class SqlComposer : public Composer
{
    public:
        SqlComposer( const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        virtual void invalidateCache();

        virtual TrackList tracks();

    private:
        QString m_name;
        bool m_tracksLoaded;
        TrackList m_tracks;
        //QReadWriteLock doesnt support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

class SqlGenre : public Genre
{
    public:
        SqlGenre( const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        virtual void invalidateCache();

        virtual TrackList tracks();

    private:
        QString m_name;
        bool m_tracksLoaded;
        TrackList m_tracks;
        //QReadWriteLock doesnt support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

class SqlYear : public Year
{
    public:
        SqlYear( const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        virtual void invalidateCache();

        virtual TrackList tracks();

    private:
        QString m_name;
        bool m_tracksLoaded;
        TrackList m_tracks;
        //QReadWriteLock doesnt support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

#endif /* SQLMETA_H */
