/***************************************************************************
                         main.cpp  -  description
                            -------------------
   begin                : Mit Okt 23 14:35:18 CEST 2002
   copyright            : (C) 2002 by Mark Kretschmann
   email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "app.h"
#include <kaboutdata.h>


KAboutData aboutData( "amarok",
    I18N_NOOP( "amaroK" ), APP_VERSION,
    I18N_NOOP( "An audio player for KDE" ), KAboutData::License_GPL,
    I18N_NOOP( "(C) 2002-2003, Mark Kretschmann\n(C) 2003-2004, the amaroK developers" ),
    I18N_NOOP( "IRC:\nserver: irc.freenode.net / channel: #amarok\n\nFeedback:\namarok-devel@lists.sourceforge.net" ),
    I18N_NOOP( "http://amarok.sourceforge.net" ) );


int main( int argc, char *argv[] )
{
    aboutData.addAuthor( "Christian \"babe-magnet\" Muehlhaeuser", "developer, stud", "chris@chris.de", "http://www.chris.de" );
    aboutData.addAuthor( "Frederik \"ich bin kein Deustcher!\" Holljen", "developer, 733t code, OSD improvement, patches", "fh@ez.no" );
    aboutData.addAuthor( "Mark \"it's good, but it's not irssi\" Kretschmann", "project founder, developer, maintainer", "markey@web.de" );
    aboutData.addAuthor( "Max \"no sleep\" Howell", "developer, knight of the regression round-table", "max.howell@methylblue.com", "http://www.methyblue.com" );
    aboutData.addAuthor( "Stanislav \"did someone say DCOP?\" Karchebny", "developer, DCOP, improvements, cleanups, i18n", "berk@upnet.ru" );

    aboutData.addCredit( "Adam Pigg", I18N_NOOP( "analyzers, patches" ), "adam@piggz.fsnet.co.uk" );
    aboutData.addCredit( "Adeodato Simó", I18N_NOOP( "patches" ), "asp16@alu.ua.es" );
    aboutData.addCredit( "Alper Ayazoglu", I18N_NOOP( "graphics: buttons" ), "cubon@cubon.de", "http://cubon.de" );
    aboutData.addCredit( "Danny Allen", I18N_NOOP( "splash screen" ), "dannya40uk@yahoo.co.uk" );
    aboutData.addCredit( "Enrico Ros", I18N_NOOP( "analyzers, king of openGL" ), "eros.kde@email.it" );
    aboutData.addCredit( "Jarkko Lehti", I18N_NOOP( "tester, IRC channel operator, whipping" ), "grue@iki.fi" );
    aboutData.addCredit( "Josef Spillner", I18N_NOOP( "KDE RadioStation code" ), "spillner@kde.org" );
    aboutData.addCredit( "Markus A. Rykalski", I18N_NOOP( "graphics" ), "exxult@exxult.de" );
    aboutData.addCredit( "Melchior Franz", I18N_NOOP( "new FFT routine, bugfixes" ), "mfranz@kde.org" );
    aboutData.addCredit( "Mike Diehl", I18N_NOOP( "handbook" ), "madpenguin8@yahoo.com" );
    aboutData.addCredit( "Roman Becker", I18N_NOOP( "graphics: amaroK logo" ), "roman@formmorf.de", "http://www.formmorf.de" );
    aboutData.addCredit( "Scott Wheeler", "Taglib", "wheeler@kde.org" );
    aboutData.addCredit( "The Noatun Authors", I18N_NOOP( "code and inspiration" ), 0, "http://noatun.kde.org" );
    aboutData.addCredit( "Whitehawk Stormchaser", I18N_NOOP( "tester, patches" ), "zerokode@gmx.net" );


    KApplication::disableAutoDcopRegistration();

    QCString arg0 = argv[0];
    arg0.replace( "amarokapp", "amarok" );
    qstrcpy( argv[0], arg0 ); //should be safe since the new string is shorter than the old

    App::initCliArgs( argc, argv );
    App app;

    return app.exec();
}
