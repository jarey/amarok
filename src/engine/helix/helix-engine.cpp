/***************************************************************************
 *   Copyright (C) 2005 Paul Cifarelli                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qthread.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "helix-engine.h"
#include "helix-configdialog.h"

AMAROK_EXPORT_PLUGIN( HelixEngine )

#define DEBUG_PREFIX "helix-engine"

#include <climits>
#include <cmath>
#include <iostream>

#include "debug.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <qapplication.h>
#include <qdir.h>

using namespace std;

extern "C"
{
    #include <unistd.h>
}

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif


///returns the configuration we will use
static inline QCString configPath() { return QFile::encodeName( QDir::homeDirPath() + "/.helix/config" ); }


HelixEngine::HelixEngine()
   : EngineBase(), HelixSimplePlayer(),
     m_state(Engine::Empty),
     m_coredir("/usr/local/RealPlayer/common"),
     m_pluginsdir("/usr/local/RealPlayer/plugins"),
     m_codecsdir("/usr/local/RealPlayer/codecs"),
     m_xfadeLength(0)
#ifdef DEBUG_PURPOSES_ONLY
     ,m_scopebufwaste(0), m_scopebufnone(0), m_scopebuftotal(0)
#endif
{
   addPluginProperty( "StreamingMode", "Socket" );
   addPluginProperty( "HasConfigure", "true" );
   addPluginProperty( "HasEqualizer", "true" );
   //addPluginProperty( "HasCrossfade", "true" );

}

HelixEngine::~HelixEngine()
{
}

amaroK::PluginConfig*
HelixEngine::configure() const
{
    return new HelixConfigDialog( (HelixEngine *)this );
}


bool
HelixEngine::init()
{
   //debug() << "Initializing HelixEngine\n";
   struct stat s;
   bool exists = false;

   m_numPlayers = 2;
   m_current = 1;

   if (!stat(m_coredir.utf8(), &s) && !stat(m_pluginsdir.utf8(), &s) && !stat(m_codecsdir.utf8(), &s))
   {
      HelixSimplePlayer::init(m_coredir.utf8(), m_pluginsdir.utf8(), m_codecsdir.utf8(), 2);
      exists = true;
   }

   if (!exists || HelixSimplePlayer::getError())
   {
      KMessageBox::error( 0, i18n("amaroK could not initialize helix-engine.") );
      return false;
   }

   debug() << "Succussful init\n";
   startTimer( 20 );

   return true;
}


bool
HelixEngine::load( const KURL &url, bool isStream )
{
   debug() << "In load " << url.url() << endl;

   cerr << "XFadeLength " << m_xfadeLength << endl;

   stop();

   int nextPlayer;

   nextPlayer = m_current ? 0 : 1;

   Engine::Base::load( url, isStream || url.protocol() == "http" );
   m_state = Engine::Idle;
   emit stateChanged( Engine::Idle );
   m_url = url;

   // most unfortunate...KURL represents a file with no leading slash, Helix uses a leading slash 'file:///home/abc.mp3' for example
   if (url.isLocalFile())
   {
      QString tmp;
      tmp ="file://" + url.directory() + "/" + url.filename();

      debug() << tmp << endl;
      HelixSimplePlayer::setURL( QFile::encodeName( tmp ), nextPlayer );      
   }
   else
      HelixSimplePlayer::setURL( QFile::encodeName( url.prettyURL() ), nextPlayer );

   return true;
}

bool
HelixEngine::play( uint offset )
{
   debug() << "In play" << endl;
   int nextPlayer;

   nextPlayer = m_current ? 0 : 1;

   HelixSimplePlayer::start(nextPlayer);
   if (offset)
      HelixSimplePlayer::seek( offset, nextPlayer );

   if (!HelixSimplePlayer::getError())
   {
      if (m_state != Engine::Playing)
      {
         m_state = Engine::Playing;
         emit stateChanged( Engine::Playing );
      }

      m_current = nextPlayer;
      return true;
   }

   HelixSimplePlayer::stop(); // stop all players
   m_state = Engine::Empty;
   emit stateChanged( Engine::Empty );

   return false;
}

void
HelixEngine::stop()
{
   debug() << "In stop\n";
   m_url = KURL();
   HelixSimplePlayer::stop(m_current);
   clearScopeQ();
   m_state = Engine::Empty;
   emit stateChanged( Engine::Empty );
}

void HelixEngine::play_finished(int /*playerIndex*/)
{
   debug() << "Ok, finished playing the track, so now I'm idle\n";
   m_state = Engine::Idle;
   emit trackEnded();
   //startTimer( 250 ); // should be resonable until we build the crossfader
}

void
HelixEngine::pause()
{
   debug() << "In pause\n";
   if( m_state == Engine::Playing )
   {
      HelixSimplePlayer::pause(m_current);
      m_state = Engine::Paused;
      emit stateChanged( Engine::Paused );
   }
   else if ( m_state == Engine::Paused )
   {
      HelixSimplePlayer::resume(m_current);
      m_state = Engine::Playing;
      emit stateChanged( Engine::Playing );
   }
}

Engine::State
HelixEngine::state() const
{
   //debug() << "In state, state is " << m_state << endl;

   if (m_url.isEmpty())
      return (Engine::Empty);

   return m_state;
}

uint
HelixEngine::position() const
{
   return HelixSimplePlayer::where(m_current);
}

uint
HelixEngine::length() const
{
   debug() << "In length\n";
   return HelixSimplePlayer::duration(m_current);
}

void
HelixEngine::seek( uint ms )
{
   debug() << "In seek\n";
   clearScopeQ();
   HelixSimplePlayer::seek(ms, m_current);
}

void
HelixEngine::setVolumeSW( uint vol )
{
   debug() << "In setVolumeSW\n";
   HelixSimplePlayer::setVolume(vol, m_current);
}


bool
HelixEngine::canDecode( const KURL &url ) const
{
   debug() << "In canDecode " << url.prettyURL() << endl;   
   //TODO check if the url really is supported by Helix
   return true;
}

void
HelixEngine::timerEvent( QTimerEvent * )
{
   HelixSimplePlayer::dispatch(); // dispatch the players
   if (m_state == Engine::Playing && HelixSimplePlayer::done(m_current))
      play_finished(m_current);
/*
   if (state() == Engine::Idle)
   {
      killTimers();
      debug() << "emitting trackEnded\n";
      emit trackEnded();
   }
*/
}


const Engine::Scope &HelixEngine::scope()
{
   int i, err;
   struct DelayQueue *item = 0;

   unsigned long w = position();
   unsigned long p;
   err = peekScopeTime(p);

   if (err || !w || w < p) // not enough buffers in the delay queue yet
      return m_scope;

   i = 0;
   while (!err && p < w)
   {
      if (item)
         delete item;
      
      item = getScopeBuf();
      err = peekScopeTime(p);

      i++;
   }

#ifdef DEBUG_PURPOSES_ONLY
   m_scopebuftotal += i;
   if (i > 1)
      m_scopebufwaste += (i-1);
#endif

   if (!item)
   {
#ifdef DEBUG_PURPOSES_ONLY
      m_scopebufnone++; // for tuning the scope... (scope is tuned for 44.1kHz sample rate)
#endif
      return m_scope;
   }

   for (i=0; i < 512; i++)
      m_scope[i] = (short int) item->buf[i];
   
   delete item;

#ifdef DEBUG_PURPOSES_ONLY
   if (!(m_scopebuftotal %100))
      cerr << "total " << m_scopebuftotal << " waste " << m_scopebufwaste << " no bufs " << m_scopebufnone << endl;
#endif

   return m_scope;
}

void
HelixEngine::setEqualizerEnabled( bool enabled ) //SLOT
{
   enableEQ(enabled);
}


// ok, this is lifted from gst... but why mess with what works?
void
HelixEngine::setEqualizerParameters( int preamp, const QValueList<int>& bandGains ) //SLOT
{
   m_preamp = ( preamp + 100 ) / 2;

   m_equalizerGains.resize( bandGains.count() );
   for ( uint i = 0; i < bandGains.count(); i++ )
      m_equalizerGains[i] = ( *bandGains.at( i ) + 100 ) / 2;

   updateEQgains();
}


namespace Debug
{
    #undef helix_indent
    QCString helix_indent;
}

#include "helix-engine.moc"
