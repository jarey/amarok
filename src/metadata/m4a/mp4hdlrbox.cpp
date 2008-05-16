/***************************************************************************
    copyright            : (C) 2002, 2003, 2006 by Jochen Issing
    email                : jochen.issing@isign-softart.de
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "mp4hdlrbox.h"

#include "boxfactory.h"
#include "mp4file.h"

#include <deque>
#include <iostream>

using namespace TagLib;

class MP4::Mp4HdlrBox::Mp4HdlrBoxPrivate
{
public:
  TagLib::uint   pre_defined;
  MP4::Fourcc    handler_type;
  TagLib::String hdlr_string;
}; // class Mp4HdlrBoxPrivate

MP4::Mp4HdlrBox::Mp4HdlrBox( TagLib::File* file, MP4::Fourcc fourcc, TagLib::uint size, long offset )
	: Mp4IsoFullBox( file, fourcc, size, offset )
{
  d = new MP4::Mp4HdlrBox::Mp4HdlrBoxPrivate();
}

MP4::Mp4HdlrBox::~Mp4HdlrBox()
{
  delete d;
}

MP4::Fourcc MP4::Mp4HdlrBox::hdlr_type() const
{
  return d->handler_type;
}

TagLib::String MP4::Mp4HdlrBox::hdlr_string() const
{
  return d->hdlr_string;
}

void MP4::Mp4HdlrBox::parse()
{
  TagLib::uint totalread = 12+20;

  TagLib::MP4::File* mp4file = static_cast<MP4::File*>( file() );
  if( mp4file->readInt( d->pre_defined ) == false )
    return;
  if( mp4file->readFourcc( d->handler_type ) == false )
    return;
  
  // read reserved into trash
  mp4file->seek( 3*4, TagLib::File::Current );

  // check if there are bytes remaining - used for hdlr string
  if( size() - totalread != 0 )
    d->hdlr_string = mp4file->readBlock( size()-totalread );
}
