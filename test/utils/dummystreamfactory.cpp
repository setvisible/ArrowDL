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

StreamInfo DummyStreamFactory::createDummyStreamInfo()
{
    return StreamInfo();
}

StreamInfo DummyStreamFactory::createDummyStreamInfo_Youtube()
{
    StreamInfo info;
    info._filename        = "TEST VIDEO-C0DPdy98e4c.webm";
    info.fulltitle        = "TEST VIDEO";
    info.defaultTitle     = "TEST VIDEO";
    info.defaultSuffix    = "webm";
    info.description      = "COUNTING LEADER AND TONE";
    info.thumbnail        = "https://i.ytimg.com/vi/C0DPdy98e4c/hqdefault.jpg";
    info.extractor        = "youtube";
    info.extractor_key    = "Youtube";
    info.defaultFormatId  = "244+140";
    info.formats.clear();
    info.formats << StreamFormat("18", "mp4", "360p", 552999, "mp4a.40.2", 96, 44100, "avc1.42001E", 480, 360, 0, 0);
    info.formats << StreamFormat("43", "webm", "360p", 287596, "vorbis", 128, 0, "vp8.0", 640, 360, 0, 0);
    info.formats << StreamFormat("133", "mp4", "240p", 87155, "none", 0, 0, "avc1.4d400d", 320, 240, 25, 0);
    info.formats << StreamFormat("134", "mp4", "360p", 142316, "none", 0, 0, "avc1.4d4015", 480, 360, 25, 0);
    info.formats << StreamFormat("135", "mp4", "480p", 202392, "none", 0, 0, "avc1.4d401e", 640, 480, 25, 0);
    info.formats << StreamFormat("140", "m4a", "tiny", 280597, "mp4a.40.2", 128, 44100, "none", 0, 0, 0, 0);
    info.formats << StreamFormat("160", "mp4", "144p", 63901, "none", 0, 0, "avc1.4d400b", 192, 144, 25, 0);
    info.formats << StreamFormat("242", "webm", "240p", 134629, "none", 0, 0, "vp9", 320, 240, 25, 0);
    info.formats << StreamFormat("243", "webm", "360p", 205692, "none", 0, 0, "vp9", 480, 360, 25, 0);
    info.formats << StreamFormat("244", "webm", "480p", 294311, "none", 0, 0, "vp9", 640, 480, 25, 0);
    info.formats << StreamFormat("249", "webm", "tiny", 44319, "opus", 50, 48000, "none", 0, 0, 0, 0);
    info.formats << StreamFormat("250", "webm", "tiny", 60843, "opus", 70, 48000, "none", 0, 0, 0, 0);
    info.formats << StreamFormat("251", "webm", "tiny", 87201, "opus", 160, 48000, "none", 0, 0, 0, 0);
    info.formats << StreamFormat("278", "webm", "144p", 53464, "none", 0, 0, "vp9", 192, 144, 13, 0);
    info.playlist         = "";
    info.playlist_index   = "";
    return info;
}

StreamInfo DummyStreamFactory::createDummyStreamInfo_Dailymotion()
{
    StreamInfo info;
    info._filename        = "NBA - Beyoncé a rendu hommage à Kobe Bryant-x7s6a9i.mp4";
    info.fulltitle        = "NBA - Beyoncé a rendu hommage à Kobe Bryant";
    info.defaultTitle     = "NBA - Beyoncé a rendu hommage à Kobe Bryant";
    info.defaultSuffix    = "mp4";
    info.description      = "";
    info.thumbnail        = "https://s1.dmcdn.net/v/S395s1ULG5qKOgCMB/x1080";
    info.extractor        = "dailymotion";
    info.extractor_key    = "Dailymotion";
    info.defaultFormatId  = "http-1080-1";
    info.formats.clear();
    info.formats << StreamFormat("hls-144-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info.formats << StreamFormat("hls-144-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info.formats << StreamFormat("http-144-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info.formats << StreamFormat("http-144-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.42000b",  192,  112, 0, 0);
    info.formats << StreamFormat("hls-240-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info.formats << StreamFormat("hls-240-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info.formats << StreamFormat("http-240-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info.formats << StreamFormat("http-240-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d000d",  320,  184, 0, 0);
    info.formats << StreamFormat("hls-380-0"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info.formats << StreamFormat("hls-380-1"  , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info.formats << StreamFormat("http-380-0" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info.formats << StreamFormat("http-380-1" , "mp4", "", 0, "mp4a.40.5", 0, 0, "avc1.4d0016",  512,  288, 0, 0);
    info.formats << StreamFormat("hls-480-0"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info.formats << StreamFormat("hls-480-1"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info.formats << StreamFormat("http-480-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info.formats << StreamFormat("http-480-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f",  848,  480, 0, 0);
    info.formats << StreamFormat("hls-720-0"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info.formats << StreamFormat("hls-720-1"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info.formats << StreamFormat("http-720-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info.formats << StreamFormat("http-720-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001f", 1280,  720, 0, 0);
    info.formats << StreamFormat("hls-1080-0" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info.formats << StreamFormat("hls-1080-1" , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info.formats << StreamFormat("http-1080-0", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info.formats << StreamFormat("http-1080-1", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640028", 1920, 1080, 0, 0);
    info.playlist         = "";
    info.playlist_index   = "";
    return info;
}

StreamInfo DummyStreamFactory::createDummyStreamInfo_Other()
{
    StreamInfo info;
    info._filename        = "San Francisco-a1b2f3f4gh5t4.mp4";
    info.fulltitle        = "San Francisco";
    info.defaultTitle     = "San Francisco";
    info.defaultSuffix    = "mp4";
    info.description      = "";
    info.thumbnail        = "https://videos.com/videos/2019/10/27/SanFrancisco-27164.jpg";
    info.extractor        = "videos";
    info.extractor_key    = "videos";
    info.defaultFormatId  = "hls-703";
    info.formats.clear();
    info.formats << StreamFormat("hls-287-0", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640015", 430, 240, 24,   0);
    info.formats << StreamFormat("hls-287-1", "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.640015", 430, 240, 24,   0);
    info.formats << StreamFormat("240p"     , "mp4", "", 0, ""         , 0, 0, ""           ,   0, 240,  0, 400);
    info.formats << StreamFormat("hls-468"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001e", 860, 480, 24,   0);
    info.formats << StreamFormat("480p"     , "mp4", "", 0, ""         , 0, 0, ""           ,   0, 480,  0, 600);
    info.formats << StreamFormat("hls-703"  , "mp4", "", 0, "mp4a.40.2", 0, 0, "avc1.64001e", 860, 480, 24,   0);
    info.playlist         = "";
    info.playlist_index   = "";
    return info;
}

QByteArray DummyStreamFactory::dumpSingleVideo()
{
    return QByteArray(
                "{"
                "    \"title\": \"Fun Test: Which is real?\","
                "    \"format_id\": \"137+251\","
                "    \"artist\": null,"
                "    \"id\": \"YsYYO_fKxE0\","
                "    \"thumbnail\": \"https://i.ytimg.com/vi/maxresdefault.webp\","
                "    \"alt_title\": null,"
                "    \"creator\": null,"
                "    \"is_live\": null,"
                "    \"uploader_id\": \"homerta\","
                "    \"width\": 1920,"
                "    \"uploader\": \"Homer Ta\","
                "    \"track\": null,"
                "    \"album\": null,"
                "    \"chapters\": null,"
                "    \"like_count\": null,"
                "    \"end_time\": null,"
                "    \"vcodec\": \"avc1.640028\","
                "    \"webpage_url_basename\": \"watch\","
                "    \"series\": null,"
                "    \"_filename\": \"Fun Test - Which is real.mp4\","
                "    \"channel_id\": \"df4g5df654dg6d54\","
                "    \"stretched_ratio\": null,"
                "    \"height\": 1080,"
                "    \"extractor_key\": \"Youtube\","
                "    \"description\": \"hello \\n\\n  world \\n\\n\","
                "    \"duration\": 367,"
                "    \"age_limit\": 0,"
                "    \"episode_number\": null,"
                "    \"webpage_url\": \"https://www.youtube.com/watch?v=df4g5df654dg6d54\","
                "    \"extractor\": \"youtube\","
                "    \"channel_url\": \"http://www.youtube.com/channel/df4g5df654dg6d54\","
                "    \"ext\": \"mp4\","
                "    \"resolution\": null,"
                "    \"categories\": ["
                "        \"Howto & Style\""
                "    ],"
                "    \"license\": null,"
                "    \"display_id\": \"df4g5df654dg6d54\","
                "    \"requested_formats\": ["
                "        {"
                "            \"format_id\": \"137\","
                "            \"width\": 1920,"
                "            \"tbr\": 3582.387,"
                "            \"vcodec\": \"avc1.640028\","
                "            \"asr\": null,"
                "            \"filesize\": 66886259,"
                "            \"format\": \"137 - 1920x1080 (1080p)\","
                "            \"format_note\": \"1080p\","
                "            \"protocol\": \"https\","
                "            \"http_headers\": {"
                "                \"Accept-Encoding\": \"gzip, deflate\","
                "                \"Accept\": \"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\","
                "                \"Accept-Charset\": \"ISO-8859-1,utf-8;q=0.7,*;q=0.7\","
                "                \"Accept-Language\": \"en-us,en;q=0.5\","
                "                \"User-Agent\": \"Mozilla/5.0\""
                "            },"
                "            \"acodec\": \"none\","
                "            \"height\": 1080,"
                "            \"url\": \"https://123546789564\","
                "            \"player_url\": null,"
                "            \"fps\": 30,"
                "            \"downloader_options\": {"
                "                \"http_chunk_size\": 10485760"
                "            },"
                "            \"ext\": \"mp4\""
                "        },"
                "        {"
                "            \"vcodec\": \"none\","
                "            \"width\": null,"
                "            \"tbr\": 161.803,"
                "            \"format_id\": \"251\","
                "            \"asr\": 48000,"
                "            \"filesize\": 6878252,"
                "            \"format\": \"251 - audio only (tiny)\","
                "            \"format_note\": \"tiny\","
                "            \"acodec\": \"opus\","
                "            \"http_headers\": {"
                "                \"Accept-Encoding\": \"gzip, deflate\","
                "                \"Accept\": \"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\","
                "                \"Accept-Charset\": \"ISO-8859-1,utf-8;q=0.7,*;q=0.7\","
                "                \"Accept-Language\": \"en-us,en;q=0.5\","
                "                \"User-Agent\": \"Mozilla/5.0\""
                "            },"
                "            \"protocol\": \"https\","
                "            \"height\": null,"
                "            \"url\": \"https://123456\","
                "            \"abr\": 160,"
                "            \"player_url\": null,"
                "            \"fps\": null,"
                "            \"downloader_options\": {"
                "                \"http_chunk_size\": 10485760"
                "            },"
                "            \"ext\": \"webm\""
                "        }"
                "    ],"
                "    \"tags\": ["
                "        \"yoyo\","
                "        \"challenge\""
                "    ],"
                "    \"view_count\": 6105314,"
                "    \"uploader_url\": \"http://www.youtube.com/user/homerta\","
                "    \"formats\": ["
                "        {"
                "            \"acodec\": \"mp4a.40.2\","
                "            \"format_id\": \"18\","
                "            \"width\": 640,"
                "            \"protocol\": \"https\","
                "            \"vcodec\": \"avc1.42001E\","
                "            \"url\": \"https://123456\","
                "            \"filesize\": 20332068,"
                "            \"format\": \"18 - 640x360 (360p)\","
                "            \"format_note\": \"360p\","
                "            \"height\": 360,"
                "            \"http_headers\": {"
                "                \"Accept-Encoding\": \"gzip, deflate\","
                "                \"Accept\": \"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\","
                "                \"Accept-Charset\": \"ISO-8859-1,utf-8;q=0.7,*;q=0.7\","
                "                \"Accept-Language\": \"en-us,en;q=0.5\","
                "                \"User-Agent\": \"Mozilla/5.0\""
                "            },"
                "            \"asr\": 44100,"
                "            \"ext\": \"mp4\","
                "            \"tbr\": 443.609,"
                "            \"abr\": 96,"
                "            \"player_url\": null,"
                "            \"fps\": 30"
                "        },"
                "        {"
                "            \"acodec\": \"mp4a.40.2\","
                "            \"format_id\": \"22\","
                "            \"width\": 1280,"
                "            \"protocol\": \"https\","
                "            \"vcodec\": \"avc1.64001F\","
                "            \"url\": \"https://9897987978\","
                "            \"filesize\": null,"
                "            \"format\": \"22 - 1280x720 (720p)\","
                "            \"format_note\": \"720p\","
                "            \"height\": 720,"
                "            \"http_headers\": {"
                "                \"Accept-Encoding\": \"gzip, deflate\","
                "                \"Accept\": \"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\","
                "                \"Accept-Charset\": \"ISO-8859-1,utf-8;q=0.7,*;q=0.7\","
                "                \"Accept-Language\": \"en-us,en;q=0.5\","
                "                \"User-Agent\": \"Mozilla/5.0\""
                "            },"
                "            \"asr\": 44100,"
                "            \"ext\": \"mp4\","
                "            \"tbr\": 500.374,"
                "            \"abr\": 192,"
                "            \"player_url\": null,"
                "            \"fps\": 30"
                "        }"
                "    ],"
                "    \"release_year\": null,"
                "    \"annotations\": null,"
                "    \"season_number\": null,"
                "    \"release_date\": null,"
                "    \"abr\": 160,"
                "    \"upload_date\": \"20170519\","
                "    \"thumbnails\": ["
                "        {"
                "            \"url\": \"https://i.ytimg.com/vi/lowres.jpg\","
                "            \"width\": 168,"
                "            \"id\": \"0\","
                "            \"height\": 94,"
                "            \"resolution\": \"168x94\""
                "        },"
                "        {"
                "            \"url\": \"https://i.ytimg.com/vi/highres.webp\","
                "            \"width\": 1920,"
                "            \"id\": \"1\","
                "            \"height\": 1080,"
                "            \"resolution\": \"1920x1080\""
                "        }"
                "    ],"
                "    \"fps\": 30,"
                "    \"acodec\": \"opus\","
                "    \"average_rating\": 4.847651,"
                "    \"vbr\": null,"
                "    \"fulltitle\": \"Fun Test: Which is real?\","
                "    \"automatic_captions\": {},"
                "    \"format\": \"137 - 1920x1080 (1080p)+251 - audio only (tiny)\","
                "    \"start_time\": null,"
                "    \"dislike_count\": null,"
                "    \"playlist\": null,"
                "    \"playlist_index\": null,"
                "    \"requested_subtitles\": null,"
                "    \"subtitles\": {}"
                "}"
                "\n" // line carriage mandatory
                );
}

QByteArray DummyStreamFactory::dumpPlaylist()
{
    QByteArray ba;
    ba.append(dumpSingleVideo());
    ba.append(dumpSingleVideo());
    ba.append(dumpSingleVideo());
    return ba;
}
