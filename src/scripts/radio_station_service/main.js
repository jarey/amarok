/*#########################################################################
#                                                                         #
#   Simple script for testing the scriptable service browser              #
#   by creating a simple static browser with some cool radio              #
#   streams. URLs shamelessly stolen from Cool-Streams.xml.               #
#                                                                         #
#   Copyright                                                             #
#   (C) 2007, 2008 Nikolaj Hald Nielsen  <nhnFreespirit@gmail.com>        #
#   (C)       2008 Peter ZHOU <peterzhoulei@gmail.com>                    #
#   (C)       2008 Mark Kretschmann <kretschmann@kde.org>                 #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
##########################################################################*/

function Station( name, url )
{
    this.name = name;
    this.url = url;
}

var stationArray = new Array (
    new Station( "Afterhours.FM [Trance/Livesets]",                     "http://www.ah.fm/192k.m3u" ),
    new Station( "Bassdrive [Drum \'n Bass]",                           "http://www.bassdrive.com/v2/streams/BassDrive.m3u" ),
    new Station( "Bluemars [Ambient/Space-Music]",                      "http://207.200.96.225:8020/listen.pls" ),
    new Station( "Digitally Imported - Chillout [Chill-Out]",           "http://di.fm/mp3/classictechno.pls" ),
    new Station( "Digitally Imported - Classic Techno [Techno]",        "http://di.fm/mp3/classictechno.pls" ),
    new Station( "Digitally Imported - Trance [Trance]",                "http://di.fm/mp3/trance.pls" ),
    new Station( "Electronic Culture [Minimal Techno]",                 "http://www.shouted.fm/tunein/electro-dsl.m3u" ),
    new Station( "Frequence 3 [Pop]",                                   "http://streams.frequence3.net/hd-mp3.m3u" ),
    new Station( "Groove Salad [Chill-Out]",                            "http://www.somafm.com/groovesalad.pls" ),
    new Station( "Drone Zone [Ambient]",                                "http://somafm.com/dronezone.pls" ),
    new Station( "Tags Trance Trip [Trance]",                           "http://somafm.com/tagstrance.pls" ),
    new Station( "Indie Pop Rocks [Indie]",                             "http://www.somafm.com/indiepop.pls" ),
    new Station( "Kohina [Computer-Music]",                             "http://la.campus.ltu.se:8000/stream.ogg.m3u" ),
    new Station( "Mostly Classical [Classical]",                        "http://www.sky.fm/mp3/classical.pls" ),
    new Station( "MTH.House [House]",                                   "http://stream.mth-house.de:8500/listen.pls" ),
    new Station( "Nectarine Demoscene Radio [Computer-Music]",          "http://nectarine.sik.fi:8002/live.mp3.m3u" ),
    new Station( "Philosomatika [Psytrance]",                           "http://www.shoutcast.com/sbin/shoutcast-playlist.pls?rn=2933&file=filename.pls" ),
    new Station( "Proton Radio [House/Dance]",                          "http://protonradio.com/proton.m3u" ),
    new Station( "Pure DJ [Trance]",                                    "http://www.puredj.com/etc/pls/128K.pls" ),
    new Station( "Radio.BMJ.net [Trance/Livesets]",                     "http://radio.bmj.net:8000/listen.pls" ),
    new Station( "Radio Paradise [Rock/Pop/Alternative]",               "http://www.radioparadise.com/musiclinks/rp_128.m3u" ),
    new Station( "Raggakings [Reggae]",                                 "http://www.raggakings.net/listen.m3u" ),
    new Station( "Secret Agent [Downtempo/Lounge]",                     "http://somafm.com/secretagent.pls" ),
    new Station( "SLAY Radio [C64 Remixes]",                            "http://sc.slayradio.org:8000/listen.pls" ),
    new Station( "Virgin Radio [Rock/Pop]",                             "http://www.smgradio.com/core/audio/mp3/live.pls?service=vrbb" ),
    new Station( "X T C Radio [Techno/Trance]",                         "http://stream.xtcradio.com:8069/listen.pls" )
);

function CoolStream()
{
    this.serviceName = "Cool Streams";
    this.levels = 1;
    this.shortDescription = "List of some high quality radio streams";
    this.rootHtml = "Some really cool radio streams, hand picked for your listening pleasure by your friendly Amarok developers";
    this.showSearchBar = false;
    ScriptableServiceScript.call( this, this.serviceName, this.levels, this.shortDescription, this.rootHtml, this.showSearchBar );
}



function onConfigure()
{
    Amarok.alert( "This script does not require any configuration." );
}

CoolStream.prototype.populate = function( parent_label )
{
    print( " Populating station level..." );
/*
    //no callback string needed for leaf nodes
    callback_string = "";
    html_info = ""
    //add the station streams as leaf nodes
    for ( i = 0; i < stationArray.length; i++ )
    {
        name = stationArray[i].name;
        url = stationArray[i].url;
        html_info = "A cool stream called " + name;

        Amarok.ScriptableService.insertItem( service_name, 0, -1,  name, html_info, callback_string, url );
    }
*/
    //tell service that all items has been added to a parent item
//    Amarok.ScriptableService.donePopulating( "Cool Streams", parent_id );
}


//Amarok.ScriptableService.initService( service_name, levels, short_description, root_html, false );

Amarok.configured.connect( onConfigure );

script = new CoolStream();
