/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "dummystreamfactory.h"

StreamInfoPtr DummyStreamFactory::createDummyStreamInfo()
{
    return StreamInfoPtr(new StreamInfo());
}

StreamInfoPtr DummyStreamFactory::createDummyStreamInfo_Youtube()
{
    StreamInfoPtr info(new StreamInfo());
    info->_filename        = "TEST VIDEO-C0DPdy98e4c.webm";
    info->fulltitle        = "TEST VIDEO";
    info->title            = "TEST VIDEO";
    info->ext              = "webm";
    info->description      = "COUNTING LEADER AND TONE";
    info->thumbnail        = "https://i.ytimg.com/vi/C0DPdy98e4c/hqdefault.jpg";
    info->extractor        = "youtube";
    info->extractor_key    = "Youtube";
    info->format_id        = "244+140";
    info->formats.clear();
    info->formats << new StreamFormat("18", "mp4", "360p", 552999, "mp4a.40.2", 96, 44100, "avc1.42001E", 480, 360, 0, 0);
    info->formats << new StreamFormat("43", "webm", "360p", 287596, "vorbis", 128, 0, "vp8.0", 640, 360, 0, 0);
    info->formats << new StreamFormat("133", "mp4", "240p", 87155, "none", 0, 0, "avc1.4d400d", 320, 240, 25, 0);
    info->formats << new StreamFormat("134", "mp4", "360p", 142316, "none", 0, 0, "avc1.4d4015", 480, 360, 25, 0);
    info->formats << new StreamFormat("135", "mp4", "480p", 202392, "none", 0, 0, "avc1.4d401e", 640, 480, 25, 0);
    info->formats << new StreamFormat("140", "m4a", "tiny", 280597, "mp4a.40.2", 128, 44100, "none", 0, 0, 0, 0);
    info->formats << new StreamFormat("160", "mp4", "144p", 63901, "none", 0, 0, "avc1.4d400b", 192, 144, 25, 0);
    info->formats << new StreamFormat("242", "webm", "240p", 134629, "none", 0, 0, "vp9", 320, 240, 25, 0);
    info->formats << new StreamFormat("243", "webm", "360p", 205692, "none", 0, 0, "vp9", 480, 360, 25, 0);
    info->formats << new StreamFormat("244", "webm", "480p", 294311, "none", 0, 0, "vp9", 640, 480, 25, 0);
    info->formats << new StreamFormat("249", "webm", "tiny", 44319, "opus", 50, 48000, "none", 0, 0, 0, 0);
    info->formats << new StreamFormat("250", "webm", "tiny", 60843, "opus", 70, 48000, "none", 0, 0, 0, 0);
    info->formats << new StreamFormat("251", "webm", "tiny", 87201, "opus", 160, 48000, "none", 0, 0, 0, 0);
    info->formats << new StreamFormat("278", "webm", "144p", 53464, "none", 0, 0, "vp9", 192, 144, 13, 0);
    info->playlist         = "";
    info->playlist_index   = "";
    return info;
}

StreamInfoPtr DummyStreamFactory::createDummyStreamInfo_Dailymotion()
{
    StreamInfoPtr info(new StreamInfo());
    info->_filename        = "NBA - Beyoncé a rendu hommage à Kobe Bryant-x7s6a9i.mp4";
    info->fulltitle        = "NBA - Beyoncé a rendu hommage à Kobe Bryant";
    info->title            = "NBA - Beyoncé a rendu hommage à Kobe Bryant";
    info->ext              = "mp4";
    info->description      = "";
    info->thumbnail        = "https://s1.dmcdn.net/v/S395s1ULG5qKOgCMB/x1080";
    info->extractor        = "dailymotion";
    info->extractor_key    = "Dailymotion";
    info->format_id        = "http-1080-1";
    info->formats.clear();
    info->formats << new StreamFormat("hls-144-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info->formats << new StreamFormat("hls-144-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info->formats << new StreamFormat("http-144-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info->formats << new StreamFormat("http-144-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info->formats << new StreamFormat("hls-240-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info->formats << new StreamFormat("hls-240-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info->formats << new StreamFormat("http-240-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info->formats << new StreamFormat("http-240-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info->formats << new StreamFormat("hls-380-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info->formats << new StreamFormat("hls-380-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info->formats << new StreamFormat("http-380-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info->formats << new StreamFormat("http-380-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info->formats << new StreamFormat("hls-480-0"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info->formats << new StreamFormat("hls-480-1"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info->formats << new StreamFormat("http-480-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info->formats << new StreamFormat("http-480-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info->formats << new StreamFormat("hls-720-0"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info->formats << new StreamFormat("hls-720-1"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info->formats << new StreamFormat("http-720-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info->formats << new StreamFormat("http-720-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info->formats << new StreamFormat("hls-1080-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info->formats << new StreamFormat("hls-1080-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info->formats << new StreamFormat("http-1080-0", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info->formats << new StreamFormat("http-1080-1", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info->playlist         = "";
    info->playlist_index   = "";
    return info;
}

StreamInfoPtr DummyStreamFactory::createDummyStreamInfo_Other()
{
    StreamInfoPtr info(new StreamInfo());
    info->_filename        = "San Francisco-a1b2f3f4gh5t4.mp4";
    info->fulltitle        = "San Francisco";
    info->title            = "San Francisco";
    info->ext              = "mp4";
    info->description      = "";
    info->thumbnail        = "https://videos.com/videos/2019/10/27/SanFrancisco-27164.jpg";
    info->extractor        = "videos";
    info->extractor_key    = "videos";
    info->format_id        = "hls-703";
    info->formats.clear();
    info->formats << new StreamFormat("hls-287-0", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640015", 430, 240, 24,   0);
    info->formats << new StreamFormat("hls-287-1", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640015", 430, 240, 24,   0);
    info->formats << new StreamFormat("240p"     , "mp4", "", 0, ""         , 0, 0, ""           ,   0, 240,  0, 400);
    info->formats << new StreamFormat("hls-468"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001e", 860, 480, 24,   0);
    info->formats << new StreamFormat("480p"     , "mp4", "", 0, ""         , 0, 0, ""           ,   0, 480,  0, 600);
    info->formats << new StreamFormat("hls-703"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001e", 860, 480, 24,   0);
    info->playlist         = "";
    info->playlist_index   = "";
    return info;
}
