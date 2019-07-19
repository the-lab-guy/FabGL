/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once


/**
 * @file
 *
 * @brief This file contains all classes related to FabGL Sound System
 */




#include <stdint.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"

#include "fabglconf.h"
#include "fabutils.h"



namespace fabgl {


#define DEFAULT_SAMPLE_RATE 16000

// 512 samples, at 16KHz generate a send every 512/16000*1000 = 32ms (16000/512=31.25 sends per second)
// 200 samples, at 16Khz generate a send every 200/16000*1000 = 12.5ms (16000/200=80 sends per second)
#define I2S_SAMPLE_BUFFER_SIZE 200  // must be even

#define WAVEGENTASK_STACK_SIZE 1024


/** @brief Base abstract class for waveform generators. A waveform generator can be seen as an audio channel that will be mixed by SoundGenerator. */
class WaveformGenerator {
public:
  WaveformGenerator() : next(nullptr), m_sampleRate(0), m_volume(100), m_enabled(false) { }

  virtual ~WaveformGenerator() { }

  /**
   * @brief Sets output frequency
   *
   * @param value Frequency in Hertz
   */
  virtual void setFrequency(int value) = 0;

  /**
   * @brief Gets next sample
   *
   * @return Sample value as signed 8 bit (-128..127 range)
   */
  virtual int getSample() = 0;

  /**
   * @brief Sets volume of this generator
   *
   * @value Volume value. Minimum is 0, maximum is 127.
   */
  void setVolume(int value) { m_volume = value; }

  /**
   * @brief Determines current volume
   *
   * @return Current volume of this generator (0 = minimum, 127 = maximum)
   */
  int volume() { return m_volume; }

  /**
   * @brief Determines whether this generator is enabled or disabled
   *
   * @return True if this generator is enabled
   */
  bool enabled() { return m_enabled; }

  /**
   * @brief Enables or disabled this generator
   *
   * A generator is disabled for default and must be enabled in order to play sound
   *
   * @value True to enable the generator, False to disable
   */
  void enable(bool value) { m_enabled = value; }

  void setSampleRate(int value) { m_sampleRate = value; }

  uint16_t sampleRate() { return m_sampleRate; }

  WaveformGenerator * next;

private:
  uint16_t m_sampleRate;
  int8_t   m_volume;
  int8_t   m_enabled; // 0 = disabled, 1 = enabled
};


/** @brief Sine waveform generator */
class SineWaveformGenerator : public WaveformGenerator {
public:
  SineWaveformGenerator();

  void setFrequency(int value);

  int getSample();

private:
  uint32_t m_phaseInc;
  uint32_t m_phaseAcc;
  uint16_t m_frequency;
  int16_t  m_lastSample;
};


/** @brief Square waveform generator */
class SquareWaveformGenerator : public WaveformGenerator {
public:
  SquareWaveformGenerator();

  void setFrequency(int value);

  // dutyCycle: 0..255 (255=100%)
  /**
   * @brief Sets square wave duty cycle
   *
   * dutyCycle Duty cycle in 0..255 range. 255 = 100%
   */
  void setDutyCycle(int dutyCycle);

  int getSample();

private:
  uint32_t m_phaseInc;
  uint32_t m_phaseAcc;
  uint16_t m_frequency;
  int8_t   m_lastSample;
  uint8_t  m_dutyCycle;
};


/** @brief Triangle waveform generator */
class TriangleWaveformGenerator : public WaveformGenerator {
public:
  TriangleWaveformGenerator();

  void setFrequency(int value);

  int getSample();

private:
  uint32_t m_phaseInc;
  uint32_t m_phaseAcc;
  uint16_t m_frequency;
  int8_t   m_lastSample;
};


/** @brief Sawtooth waveform generator */
class SawtoothWaveformGenerator : public WaveformGenerator {
public:
  SawtoothWaveformGenerator();

  void setFrequency(int value);

  int getSample();

private:
  uint32_t m_phaseInc;
  uint32_t m_phaseAcc;
  uint16_t m_frequency;
  int8_t   m_lastSample;
};


/** @brief Noise generator */
class NoiseWaveformGenerator : public WaveformGenerator {
public:
  NoiseWaveformGenerator();

  void setFrequency(int value);

  int getSample();

private:
  uint16_t m_noise;
};


class SamplesGenerator : public WaveformGenerator {
public:
  SamplesGenerator(int8_t const * data, int length);

  void setFrequency(int value);

  int getSample();

private:
  int8_t const * m_data;
  int            m_length;
  int            m_index;
};



class SoundGenerator {

public:

  SoundGenerator(int sampleRate = DEFAULT_SAMPLE_RATE);

  ~SoundGenerator();

  void clear();

  // ret prev state
  bool play(bool value);

  bool playing();

  WaveformGenerator * channels() { return m_channels; }

  void attach(WaveformGenerator * value);

  void detach(WaveformGenerator * value);

  // value: 0..127
  void setVolume(int value) { m_volume = value; }

  int volume() { return m_volume; }


private:

  void i2s_audio_init();
  static void waveGenTask(void * arg);
  bool suspendPlay(bool value);
  void mutizeOutput();


  TaskHandle_t        m_waveGenTaskHandle;

  WaveformGenerator * m_channels;

  uint16_t *          m_sampleBuffer;

  int8_t              m_volume;

  uint16_t            m_sampleRate;

};




} // end of namespace

