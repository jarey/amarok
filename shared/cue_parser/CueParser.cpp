/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.com>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Pulic License for more details.              *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/


#include "CueParser.h"

#include <KEncodingProber>

#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QTextCodec>

/**
* Parses a cue sheet file into CueFileItems and inserts them in a QMap
* @return a map of CueFileItems. If the cue file was not successfully loaded
*         the map is empty.
* @author (C) 2005 by Martin Ehmke <ehmke@gmx.de>
*/


CueFileItemMap CueParser::loadCueFile( const KUrl &cuefile, qint64 mediaLength  )
{

    CueFileItemMap cueItems;
    
    //qDebug() << "CUEFILE: " << cuefile.pathOrUrl();
    if ( QFile::exists ( cuefile.pathOrUrl() ) )
    {
        //qDebug() << "  EXISTS!";
        QFile file ( cuefile.pathOrUrl() );
        int track = 0;
        QString defaultArtist;
        QString defaultAlbum;
        QString artist;
        QString title;
        long length = 0;
        long prevIndex = -1;
        bool index00Present = false;
        long index = -1;

        int mode = BEGIN;
        if ( file.open ( QIODevice::ReadOnly ) )
        {
            QTextStream stream ( &file );
            QString line;
            KEncodingProber prober;

            KEncodingProber::ProberState result = prober.feed( file.readAll() );
            file.seek( 0 );

            if( result != KEncodingProber::NotMe )
                stream.setCodec( QTextCodec::codecForName( prober.encoding() ) );

            //qDebug() << "Encoding: " << prober.encoding();

            while ( !stream.atEnd() )
            {
                line = stream.readLine().simplified();

                if ( line.startsWith ( "title", Qt::CaseInsensitive ) )
                {
                    title = line.mid ( 6 ).remove ( '"' );
                    if ( mode == BEGIN )
                    {
                        defaultAlbum = title;
                        title.clear();
                        //qDebug() << "Album: " << defaultAlbum;
                    }
                    //else
                        //qDebug() << "Title: " << title;
                }

                else if ( line.startsWith ( "performer", Qt::CaseInsensitive ) )
                {
                    artist = line.mid ( 10 ).remove ( '"' );
                    if ( mode == BEGIN )
                    {
                        defaultArtist = artist;
                        artist.clear();
                        //qDebug() << "Album Artist: " << defaultArtist;
                    }
                    //else
                        //qDebug() << "Artist: " << artist;
                }

                else if ( line.startsWith ( "track", Qt::CaseInsensitive ) )
                {
                    if ( mode == TRACK_FOUND )
                    {
                        // not valid, because we have to have an index for the previous track
                        file.close();
                        //qDebug() << "Mode is TRACK_FOUND, abort.";
                        return CueFileItemMap();
                    }
                    if ( mode == INDEX_FOUND )
                    {
                        if ( artist.isNull() )
                            artist = defaultArtist;

                        //qDebug() << "Inserting item: " << title << " - " << artist << " on " << defaultAlbum << " (" << track << ")";
                        // add previous entry to map
                        cueItems.insert ( index, CueFileItem ( title, artist, defaultAlbum, track, index ) );
                        prevIndex = index;
                        title.clear();
                        artist.clear();
                        track  = 0;
                    }
                    track = line.section ( ' ',1,1 ).toInt();
                    //qDebug() << "Track: " << track;
                    mode = TRACK_FOUND;
                }
                else if ( line.startsWith ( "index", Qt::CaseInsensitive ) )
                {
                    if ( mode == TRACK_FOUND )
                    {
                        int indexNo = line.section ( ' ',1,1 ).toInt();

                        if ( indexNo == 1 )
                        {
                            QStringList time = line.section ( ' ', -1, -1 ).split ( ':' );

                            index = time[0].toLong() *60*1000 + time[1].toLong() *1000 + time[2].toLong() *1000/75; //75 frames per second

                            if ( prevIndex != -1 && !index00Present ) // set the prev track's length if there is INDEX01 present, but no INDEX00
                            {
                                length = index - prevIndex;
                                //qDebug() << "Setting length of track " << cueItems[prevIndex].title() << " to " << length << " msecs.";
                                cueItems[prevIndex].setLength ( length );
                            }

                            index00Present = false;
                            mode = INDEX_FOUND;
                            length = 0;
                        }

                        else if ( indexNo == 0 ) // gap, use to calc prev track length
                        {
                            QStringList time = line.section ( ' ', -1, -1 ).split ( ':' );

                            length = time[0].toLong() * 60 * 1000 + time[1].toLong() * 1000 + time[2].toLong() *1000/75; //75 frames per second

                            if ( prevIndex != -1 )
                            {
                                length -= prevIndex; //this[prevIndex].getIndex();
                                //qDebug() << "Setting length of track " << cueItems[prevIndex].title() << " to " << length << " msecs.";
                                cueItems[prevIndex].setLength ( length );
                                index00Present = true;
                            }
                            else
                                length =  0;
                        }
                        else
                        {
                            //qDebug() << "Skipping unsupported INDEX " << indexNo;
                        }
                    }
                    else
                    {
                        // not valid, because we don't have an associated track
                        file.close();
                        //qDebug() << "Mode is not TRACK_FOUND but encountered INDEX, abort.";
                        return CueFileItemMap();
                    }
                    //qDebug() << "index: " << index;
                }
            }

            if ( artist.isNull() )
                artist = defaultArtist;

            //qDebug() << "Inserting item: " << title << " - " << artist << " on " << defaultAlbum << " (" << track << ")";
            // add previous entry to map
            cueItems.insert ( index, CueFileItem ( title, artist, defaultAlbum, track, index ) );
            file.close();
        }

        /**
        *  Because there is no way to set the length for the last track in a normal way,
        *  we have to do some magic here. Having the total length of the media file given
        *  we can set the lenth for the last track after all the cue file was loaded into array.
        */

        cueItems[index].setLength ( mediaLength - index );
        //qDebug() << "Setting length of track " << cueItems[index].title() << " to " << mediaLength - index << " msecs.";

        return cueItems;
    }
    return CueFileItemMap();
}

KUrl CueParser::locateCueSheet ( const KUrl &trackurl )
{
    if ( !trackurl.isValid() || !trackurl.isLocalFile() )
        return KUrl();
    // look for the cue file that matches the media file
    QString path    = trackurl.path();
    QString cueFile = path.left ( path.lastIndexOf ( '.' ) ) + ".cue";

    if ( validateCueSheet ( cueFile ) )
    {
        //qDebug() << "[CUEFILE]: " << cueFile << " - Shoot blindly, found and loaded. ";
        return KUrl ( cueFile );
    }
    //qDebug() << "[CUEFILE]: " << cueFile << " - Shoot blindly and missed, searching for other cue files.";

    bool foundCueFile = false;
    QDir dir ( trackurl.directory() );
    QStringList filters;
    filters << "*.cue" << "*.CUE";
    dir.setNameFilters ( filters );

    QStringList cueFilesList = dir.entryList();

    if ( !cueFilesList.empty() )
        for ( QStringList::Iterator it = cueFilesList.begin(); it != cueFilesList.end() && !foundCueFile; ++it )
        {
            QFile file ( dir.filePath ( *it ) );
            if ( file.open ( QIODevice::ReadOnly ) )
            {
                //qDebug() << "[CUEFILE]: " << *it << " - Opened, looking for the matching FILE stanza." << endl;
                QTextStream stream ( &file );
                QString line;

                while ( !stream.atEnd() && !foundCueFile )
                {
                    line = stream.readLine().simplified();

                    if ( line.startsWith ( "file", Qt::CaseInsensitive ) )
                    {
                        line = line.mid ( 5 ).remove ( '"' );

                        if ( line.contains ( trackurl.fileName(), Qt::CaseInsensitive ) )
                        {
                            cueFile = dir.filePath ( *it );

                            if ( validateCueSheet ( cueFile ) )
                            {
                                //qDebug() << "[CUEFILE]: " << cueFile << " - Looked inside cue files, found and loaded proper one" << endl;
                                foundCueFile = true;
                            }
                        }
                    }
                }

                file.close();
            }
        }

    if ( foundCueFile )
        return KUrl ( cueFile );
    //qDebug() << "[CUEFILE]: - Didn't find any matching cue file." << endl;
    return KUrl();
}

bool CueParser::validateCueSheet ( const QString& cuefile )
{
    if ( !QFile::exists ( cuefile ) )
        return false;

    QFile file ( cuefile );
    int track = 0;
    QString defaultArtist;
    QString defaultAlbum;
    QString artist;
    QString title;
    long length = 0;
    long prevIndex = -1;
    bool index00Present = false;
    long index = -1;

    int mode = BEGIN;
    if ( file.open ( QIODevice::ReadOnly ) )
    {
        QTextStream stream ( &file );
        QString line;

        while ( !stream.atEnd() )
        {
            line = stream.readLine().simplified();

            if ( line.startsWith ( "title", Qt::CaseInsensitive ) )
            {
                title = line.mid ( 6 ).remove ( '"' );
                if ( mode == BEGIN )
                {
                    defaultAlbum = title;
                    title.clear();
                    //qDebug() << "Album: " << defaultAlbum;
                }
                //else
                    //qDebug() << "Title: " << title;
            }

            else if ( line.startsWith ( "performer", Qt::CaseInsensitive ) )
            {
                artist = line.mid ( 10 ).remove ( '"' );
                if ( mode == BEGIN )
                {
                    defaultArtist = artist;
                    artist.clear();
                    //qDebug() << "Album Artist: " << defaultArtist;
                }
                //else
                    //qDebug() << "Artist: " << artist;
            }

            else if ( line.startsWith ( "track", Qt::CaseInsensitive ) )
            {
                if ( mode == TRACK_FOUND )
                {
                    // not valid, because we have to have an index for the previous track
                    file.close();
                    //qDebug() << "Mode is TRACK_FOUND, abort.";
                    return false;
                }
                if ( mode == INDEX_FOUND )
                {
                    if ( artist.isNull() )
                        artist = defaultArtist;

                    prevIndex = index;
                    title.clear();
                    artist.clear();
                    track  = 0;
                }
                track = line.section ( ' ',1,1 ).toInt();
                //qDebug() << "Track: " << track;
                mode = TRACK_FOUND;
            }
            else if ( line.startsWith ( "index", Qt::CaseInsensitive ) )
            {
                if ( mode == TRACK_FOUND )
                {
                    int indexNo = line.section ( ' ',1,1 ).toInt();

                    if ( indexNo == 1 )
                    {
                        QStringList time = line.section ( ' ', -1, -1 ).split ( ':' );

                        index = time[0].toLong() *60*1000 + time[1].toLong() *1000 + time[2].toLong() *1000/75; //75 frames per second

                        if ( prevIndex != -1 && !index00Present ) // set the prev track's length if there is INDEX01 present, but no INDEX00
                        {
                            length = index - prevIndex;
                        }

                        index00Present = false;
                        mode = INDEX_FOUND;
                        length = 0;
                    }

                    else if ( indexNo == 0 ) // gap, use to calc prev track length
                    {
                        QStringList time = line.section ( ' ', -1, -1 ).split ( ':' );

                        length = time[0].toLong() *60*1000 + time[1].toLong() *1000 + time[2].toLong() *1000/75; //75 frames per second

                        if ( prevIndex != -1 )
                        {
                            length -= prevIndex; //this[prevIndex].getIndex();
                            index00Present = true;
                        }
                        else
                            length =  0;
                    }
                    else
                    {
                        //qDebug() << "Skipping unsupported INDEX " << indexNo;
                    }
                }
                else
                {
                    // not valid, because we don't have an associated track
                    file.close();
                    //qDebug() << "Mode is not TRACK_FOUND but encountered INDEX, abort.";
                    return false;
                }
                //qDebug() << "index: " << index;
            }
        }

        if( mode == BEGIN )
        {
            file.close();
            //qDebug() << "Cue file is invalid";
            return false;
        }

        if ( artist.isNull() )
            artist = defaultArtist;

        file.close();
    }
    return true;
}

QString CueParser::sourceFile( const QString& cuefile )
{
    if ( !QFile::exists ( cuefile ) )
      return QString();
    
    QString sourceFile;

    QFile file ( cuefile );
    
    if ( file.open ( QIODevice::ReadOnly ) )
    {
        QTextStream stream ( &file );
        QString line;

        while ( !stream.atEnd() )
        {
            line = stream.readLine().simplified();

            if ( line.startsWith ( "file", Qt::CaseInsensitive ) )
            {
	        int lastMark = line.lastIndexOf( '"' );
	      
                sourceFile = line.mid ( 6, lastMark - 6);
       
                qDebug() << "Found source file: " << sourceFile;
            }
	}
	
	file.close();
    }
    
    return sourceFile;
}
