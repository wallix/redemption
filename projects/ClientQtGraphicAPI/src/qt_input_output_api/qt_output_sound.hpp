/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo

*/

#pragma once

#include <fstream>

#include "utils/log.hpp"
#include "utils/stream.hpp"
// #include "client_redemption/client_input_output_api/client_sound_api.hpp"
#include "client_redemption/client_channels/client_rdpsnd_channel.hpp"
#include "client_redemption/client_redemption_api.hpp"

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)

#if REDEMPTION_QT_VERSION == 4
#include <phonon/AudioOutput>
#include <phonon/MediaObject>
#else
#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>
#endif



class QtOutputSound : public QObject, public ClientOutputSoundAPI
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

    Phonon::MediaObject * media;
    Phonon::AudioOutput * audioOutput;
    Phonon::Path audio_path;

    int current_wav_index;
    int total_wav_files;
    std::string wave_file_to_write;

    std::string path;


public:
    QtOutputSound(const std::string & path, QWidget * parent)
      : QObject(parent), ClientOutputSoundAPI()
      , media(nullptr)
      , audioOutput(nullptr)
      , current_wav_index(0)
      , total_wav_files(0)
      , path(path)
    {
        this->media = new Phonon::MediaObject(this);
        this->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        this->audio_path = Phonon::createPath(this->media, this->audioOutput);

        this->QObject::connect(this->media, SIGNAL (finished()),  this, SLOT (call_playback_over()));
    }

    ~QtOutputSound() {
        for (int i = 0; i <= total_wav_files; i++) {
            remove(this->wav_file(i).c_str());
//             LOG(LOG_INFO, "delete wav: \"%s\"", wav_file_name);
        }
    }

    void init(size_t raw_total_size) override {

        this->total_wav_files++;

        this->wave_file_to_write = this->wav_file(this->total_wav_files);

        StaticOutStream<64> out_stream;
        out_stream.out_copy_bytes("RIFF", 4);
        out_stream.out_uint32_le(raw_total_size + 36);
        out_stream.out_copy_bytes("WAVEfmt ", 8);
        out_stream.out_uint32_le(16);
        out_stream.out_uint16_le(1);
        out_stream.out_uint16_le(this->n_channels);
        out_stream.out_uint32_le(this->n_sample_per_sec);
        out_stream.out_uint32_le(this->bit_per_sec);
        out_stream.out_uint16_le(this->n_block_align);
        out_stream.out_uint16_le(this->bit_per_sample);
        out_stream.out_copy_bytes("data", 4);
        out_stream.out_uint32_le(raw_total_size);

        std::ofstream file(this->wave_file_to_write.c_str(), std::ios::out| std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char *>(out_stream.get_data()), 44);
            file.close();
        }
    }

    void setData(const uint8_t * data, size_t size) override {
        std::ofstream file(this->wave_file_to_write.c_str(), std::ios::app | std::ios::out| std::ios::binary);
        if (file) {
            file.write(reinterpret_cast<const char *>(data), size);
            file.close();
        }
    }

    void play() override {
        if (this->media) {
            if (this->media->state() == Phonon::StoppedState) {

                if (this->total_wav_files > this->current_wav_index) {
                    this->current_wav_index++;
                    std::string wav_file_name = ":/DATA/sound_temp/sound";
                    wav_file_name += std::to_string(this->current_wav_index) + ".wav";

                    LOG(LOG_INFO, "play wav: \"%s\"", wav_file_name);

                    Phonon::MediaSource sources(QUrl(wav_file_name.c_str()));
                    this->media->setCurrentSource(sources);
                    this->media->play();
                }
            }
        }
    }

private Q_SLOTS:
    void call_playback_over() {
//         LOG(LOG_INFO, "remove wav: \"%s\"", wav_file_name);
//         remove(this->wav_file(this->current_wav_index).c_str());

        this->play();
    }

    std::string wav_file(int i) const
    {
        std::string wav_file_name = this->path + "/sound";
        wav_file_name += std::to_string(i);
        wav_file_name += ".wav";
        return wav_file_name;
    }
};

