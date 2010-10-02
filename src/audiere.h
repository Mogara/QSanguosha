/**
 * @file
 *
 * Audiere Sound System
 * Version 1.9.4
 * (c) 2001-2003 Chad Austin
 *
 * This API uses principles explained at
 * http://aegisknight.org/cppinterface.html
 *
 * This code licensed under the terms of the LGPL.  See doc/license.txt.
 *
 *
 * Note: When compiling this header in gcc, you may want to use the
 * -Wno-non-virtual-dtor flag to get rid of those annoying "class has
 * virtual functions but no virtual destructor" warnings.
 *
 * This file is structured as follows:
 * - includes, macro definitions, other general setup
 * - interface definitions
 * - DLL-safe entry points (not for general use)
 * - inline functions that use those entry points
 */

#ifndef AUDIERE_H
#define AUDIERE_H


#include <vector>
#include <string>

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#ifndef __cplusplus
  #error Audiere requires C++
#endif


// DLLs in Windows should use the standard (Pascal) calling convention
#ifndef ADR_CALL
  #if defined(WIN32) || defined(_WIN32)
    #define ADR_CALL __stdcall
  #else
    #define ADR_CALL
  #endif
#endif

// Export functions from the DLL
#ifndef ADR_DECL
#  if defined(WIN32) || defined(_WIN32)
#    ifdef AUDIERE_EXPORTS
#      define ADR_DECL __declspec(dllexport)
#    else
#      define ADR_DECL __declspec(dllimport)
#    endif
#  else
#    define ADR_DECL
#  endif
#endif



#define ADR_FUNCTION(ret) extern "C" ADR_DECL ret ADR_CALL
#define ADR_METHOD(ret) virtual ret ADR_CALL


namespace audiere {

  class RefCounted {
  protected:
    /**
     * Protected so users of refcounted classes don't use std::auto_ptr
     * or the delete operator.
     *
     * Interfaces that derive from RefCounted should define an inline,
     * empty, protected destructor as well.
     */
    ~RefCounted() { }

  public:
    /**
     * Add a reference to the internal reference count.
     */
    ADR_METHOD(void) ref() = 0;

    /**
     * Remove a reference from the internal reference count.  When this
     * reaches 0, the object is destroyed.
     */
    ADR_METHOD(void) unref() = 0;
  };


  template<typename T>
  class RefPtr {
  public:
    RefPtr(T* ptr = 0) {
      m_ptr = 0;
      *this = ptr;
    }

    RefPtr(const RefPtr<T>& ptr) {
      m_ptr = 0;
      *this = ptr;
    }

    ~RefPtr() {
      if (m_ptr) {
        m_ptr->unref();
        m_ptr = 0;
      }
    }
 
    RefPtr<T>& operator=(T* ptr) {
      if (ptr != m_ptr) {
        if (m_ptr) {
          m_ptr->unref();
        }
        m_ptr = ptr;
        if (m_ptr) {
          m_ptr->ref();
        }
      }
      return *this;
    }

    RefPtr<T>& operator=(const RefPtr<T>& ptr) {
      *this = ptr.m_ptr;
      return *this;
    }

    T* operator->() const {
      return m_ptr;
    }

    T& operator*() const {
      return *m_ptr;
    }

    operator bool() const {
      return (m_ptr != 0);
    }

    T* get() const {
      return m_ptr;
    }

  private:
    T* m_ptr;
  };


  template<typename T, typename U>
  bool operator==(const RefPtr<T>& a, const RefPtr<U>& b) {
      return (a.get() == b.get());
  }

  template<typename T>
  bool operator==(const RefPtr<T>& a, const T* b) {
      return (a.get() == b);
  }

  template<typename T>
  bool operator==(const T* a, const RefPtr<T>& b) {
      return (a == b.get());
  }
  

  template<typename T, typename U>
  bool operator!=(const RefPtr<T>& a, const RefPtr<U>& b) {
      return (a.get() != b.get());
  }

  template<typename T>
  bool operator!=(const RefPtr<T>& a, const T* b) {
      return (a.get() != b);
  }

  template<typename T>
  bool operator!=(const T* a, const RefPtr<T>& b) {
      return (a != b.get());
  }


  /**
   * A basic implementation of the RefCounted interface.  Derive
   * your implementations from RefImplementation<YourInterface>.
   */
  template<class Interface>
  class RefImplementation : public Interface {
  protected:
    RefImplementation() {
      m_ref_count = 0;
    }

    /**
     * So the implementation can put its destruction logic in the destructor,
     * as natural C++ code does.
     */
    virtual ~RefImplementation() { }

  public:
    void ADR_CALL ref() {
      ++m_ref_count;
    }

    void ADR_CALL unref() {
      if (--m_ref_count == 0) {
        delete this;
      }
    }

  private:
    int m_ref_count;
  };


  /**
   * Represents a random-access file, usually stored on a disk.  Files
   * are always binary: that is, they do no end-of-line
   * transformations.  File objects are roughly analogous to ANSI C
   * FILE* objects.
   *
   * This interface is not synchronized.
   */
  class File : public RefCounted {
  protected:
    ~File() { }

  public:
    /**
     * The different ways you can seek within a file.
     */
    enum SeekMode {
      BEGIN,
      CURRENT,
      END,
    };

    /**
     * Read size bytes from the file, storing them in buffer.
     *
     * @param buffer  buffer to read into
     * @param size    number of bytes to read
     *
     * @return  number of bytes successfully read
     */
    ADR_METHOD(int) read(void* buffer, int size) = 0;

    /**
     * Jump to a new position in the file, using the specified seek
     * mode.  Remember: if mode is END, the position must be negative,
     * to seek backwards from the end of the file into its contents.
     * If the seek fails, the current position is undefined.
     *
     * @param position  position relative to the mode
     * @param mode      where to seek from in the file
     *
     * @return  true on success, false otherwise
     */
    ADR_METHOD(bool) seek(int position, SeekMode mode) = 0;

    /**
     * Get current position within the file.
     *
     * @return  current position
     */
    ADR_METHOD(int) tell() = 0;
  };
  typedef RefPtr<File> FilePtr;


  /// Storage formats for sample data.
  enum SampleFormat {
    SF_U8,  ///< unsigned 8-bit integer [0,255]
    SF_S16, ///< signed 16-bit integer in host endianness [-32768,32767]
  };


  /// Supported audio file formats.
  enum FileFormat {
    FF_AUTODETECT,
    FF_WAV,
    FF_OGG,
    FF_FLAC,
    FF_MP3,
    FF_MOD,
    FF_AIFF,
    FF_SPEEX,
  };


  /**
   * Source of raw PCM samples.  Sample sources have an intrinsic format
   * (@see SampleFormat), sample rate, and number of channels.  They can
   * be read from or reset.
   *
   * Some sample sources are seekable.  Seekable sources have two additional
   * properties: length and position.  Length is read-only.
   *
   * This interface is not synchronized.
   */
  class SampleSource : public RefCounted {
  protected:
    ~SampleSource() { }

  public:
    /**
     * Retrieve the number of channels, sample rate, and sample format of
     * the sample source.
     */
    ADR_METHOD(void) getFormat(
      int& channel_count,
      int& sample_rate,
      SampleFormat& sample_format) = 0;

    /**
     * Read frame_count samples into buffer.  buffer must be at least
     * |frame_count * GetSampleSize(format) * channel_count| bytes long.
     *
     * @param frame_count  number of frames to read
     * @param buffer       buffer to store samples in
     *
     * @return  number of frames actually read
     */
    ADR_METHOD(int) read(int frame_count, void* buffer) = 0;

    /**
     * Reset the sample source.  This has the same effect as setPosition(0)
     * on a seekable source.  On an unseekable source, it resets all internal
     * state to the way it was when the source was first created.
     */
    ADR_METHOD(void) reset() = 0;

    /**
     * @return  true if the stream is seekable, false otherwise
     */
    ADR_METHOD(bool) isSeekable() = 0;

    /**
     * @return  number of frames in the stream, or 0 if the stream is not
     *          seekable
     */
    ADR_METHOD(int) getLength() = 0;
    
    /**
     * Sets the current position within the sample source.  If the stream
     * is not seekable, this method does nothing.
     *
     * @param position  current position in frames
     */
    ADR_METHOD(void) setPosition(int position) = 0;

    /**
     * Returns the current position within the sample source.
     *
     * @return  current position in frames
     */
    ADR_METHOD(int) getPosition() = 0;

    /**
     * @return  true if the sample source is set to repeat
     */
    ADR_METHOD(bool) getRepeat() = 0;

    /**
     * Sets whether the sample source should repeat or not.  Note that not
     * all sample sources repeat by starting again at the beginning of the
     * sound.  For example MOD files can contain embedded loop points.
     *
     * @param repeat  true if the source should repeat, false otherwise
     */
    ADR_METHOD(void) setRepeat(bool repeat) = 0;

    /// Returns number of metadata tags present in this sample source.
    ADR_METHOD(int) getTagCount() = 0;

    /**
     * Returns the key of the i'th tag in the source.  If the tag is
     * "author=me", the key is "author".
     */
    virtual const char* ADR_CALL getTagKey(int i) = 0;

    /**
     * Returns the value of the i'th tag in the source.  If the tag is
     * "author=me", the value is "me".
     */
    virtual const char* ADR_CALL getTagValue(int i) = 0;

    /**
     * Returns the type of the i'th tag in the source.  The type is where
     * the tag comes from, i.e. "ID3v1", "ID3v2", or "vorbis".
     */
    virtual const char* ADR_CALL getTagType(int i) = 0;
  };
  typedef RefPtr<SampleSource> SampleSourcePtr;


  /**
   * LoopPointSource is a wrapper around another SampleSource, providing
   * custom loop behavior.  LoopPointSource maintains a set of links
   * within the sample stream and whenever the location of one of the links
   * (i.e. a loop point) is reached, the stream jumps to that link's target.
   * Each loop point maintains a count.  Every time a loop point comes into
   * effect, the count is decremented.  Once it reaches zero, that loop point
   * is temporarily disabled.  If a count is not a positive value, it
   * cannot be disabled.  Calling reset() resets all counts to their initial
   * values.
   *
   * Loop points only take effect when repeating has been enabled via the
   * setRepeat() method.
   *
   * Loop points are stored in sorted order by their location.  Each one
   * has an index based on its location within the list.  A loop point's
   * index will change if another is added before it.
   *
   * There is always one implicit loop point after the last sample that
   * points back to the first.  That way, this class's default looping
   * behavior is the same as a standard SampleSource.  This loop point
   * does not show up in the list.
   */
  class LoopPointSource : public SampleSource {
  protected:
    ~LoopPointSource() { }

  public:
    /**
     * Adds a loop point to the stream.  If a loop point at 'location'
     * already exists, the new one replaces it.  Location and target are
     * clamped to the actual length of the stream.
     *
     * @param location   frame where loop occurs
     * @param target     frame to jump to after loop point is hit
     * @param loopCount  number of times to execute this jump.
     */
    ADR_METHOD(void) addLoopPoint(
      int location, int target, int loopCount) = 0;

    /**
     * Removes the loop point at index 'index' from the stream.
     *
     * @param index  index of the loop point to remove
     */
    ADR_METHOD(void) removeLoopPoint(int index) = 0;

    /**
     * Returns the number of loop points in this stream.
     */
    ADR_METHOD(int) getLoopPointCount() = 0;

    /**
     * Retrieves information about a specific loop point.
     *
     * @param index      index of the loop point
     * @param location   frame where loop occurs
     * @param target     loop point's target frame
     * @param loopCount  number of times to loop from this particular point
     *
     * @return  true if the index is valid and information is returned
     */
    ADR_METHOD(bool) getLoopPoint(
      int index, int& location, int& target, int& loopCount) = 0;
  };
  typedef RefPtr<LoopPointSource> LoopPointSourcePtr;


  /**
   * A connection to an audio device.  Multiple output streams are
   * mixed by the audio device to produce the final waveform that the
   * user hears.
   *
   * Each output stream can be independently played and stopped.  They
   * also each have a volume from 0.0 (silence) to 1.0 (maximum volume).
   */
  class OutputStream : public RefCounted {
  protected:
    ~OutputStream() { }

  public:
    /**
     * Start playback of the output stream.  If the stream is already
     * playing, this does nothing.
     */
    ADR_METHOD(void) play() = 0;

    /**
     * Stop playback of the output stream.  If the stream is already
     * stopped, this does nothing.
     */
    ADR_METHOD(void) stop() = 0;

    /**
     * @return  true if the output stream is playing, false otherwise
     */
    ADR_METHOD(bool) isPlaying() = 0;

    /**
     * Reset the sample source or buffer to the beginning. On seekable
     * streams, this operation is equivalent to setPosition(0).
     *
     * On some output streams, this operation can be moderately slow, as up to
     * several seconds of PCM buffer must be refilled.
     */
    ADR_METHOD(void) reset() = 0;

    /**
     * Set whether the output stream should repeat.
     *
     * @param repeat  true if the stream should repeat, false otherwise
     */
    ADR_METHOD(void) setRepeat(bool repeat) = 0;

    /**
     * @return  true if the stream is repeating
     */
    ADR_METHOD(bool) getRepeat() = 0;

    /**
     * Sets the stream's volume.
     *
     * @param  volume  0.0 = silence, 1.0 = maximum volume (default)
     */
    ADR_METHOD(void) setVolume(float volume) = 0;

    /**
     * Gets the current volume.
     *
     * @return  current volume of the output stream
     */
    ADR_METHOD(float) getVolume() = 0;

    /**
     * Set current pan.
     *
     * @param pan  -1.0 = left, 0.0 = center (default), 1.0 = right
     */
    ADR_METHOD(void) setPan(float pan) = 0;

    /**
     * Get current pan.
     */
    ADR_METHOD(float) getPan() = 0;

    /**
     * Set current pitch shift.
     *
     * @param shift  can range from 0.5 to 2.0.  default is 1.0.
     */
    ADR_METHOD(void) setPitchShift(float shift) = 0;

    /**
     * Get current pitch shift.  Defaults to 1.0.
     */
    ADR_METHOD(float) getPitchShift() = 0;

    /**
     * @return  true if the stream is seekable, false otherwise
     */
    ADR_METHOD(bool) isSeekable() = 0;

    /**
     * @return  number of frames in the stream, or 0 if the stream is not
     *          seekable
     */
    ADR_METHOD(int) getLength() = 0;
    
    /**
     * Sets the current position within the sample source.  If the stream
     * is not seekable, this method does nothing.
     *
     * @param position  current position in frames
     */
    ADR_METHOD(void) setPosition(int position) = 0;

    /**
     * Returns the current position within the sample source.
     *
     * @return  current position in frames
     */
    ADR_METHOD(int) getPosition() = 0;
  };
  typedef RefPtr<OutputStream> OutputStreamPtr;


  /// An integral code representing a specific type of event.
  enum EventType {
    ET_STOP, ///< See StopEvent and StopCallback
  };


  /// Base interface for event-specific data passed to callbacks.
  class Event : public RefCounted {
  protected:
    ~Event() { }

  public:
    /// Returns the EventType code for this event.
    ADR_METHOD(EventType) getType() = 0;
  };
  typedef RefPtr<Event> EventPtr;


  /**
   * An event object that gets passed to implementations of StopCallback
   * when a stream has stopped playing.
   */
  class StopEvent : public Event {
  protected:
    ~StopEvent() { }

  public:
    EventType ADR_CALL getType() { return ET_STOP; }

    /// A code representing the reason the stream stopped playback.
    enum Reason {
      STOP_CALLED,  ///< stop() was called from an external source.
      STREAM_ENDED, ///< The stream reached its end.
    };

    /**
     * @return Pointer to the OutputStream that stopped playback.
     */
    ADR_METHOD(OutputStream*) getOutputStream() = 0;

    /**
     * @return Reason for the stop event.
     */
    ADR_METHOD(Reason) getReason() = 0;
  };
  typedef RefPtr<StopEvent> StopEventPtr;


  /**
   * Base interface for all callbacks.  See specific callback implementations
   * for descriptions.
   */  
  class Callback : public RefCounted {
  protected:
    ~Callback() { }

  public:
    /**
     * Returns the event type that this callback knows how to handle.
     */
    ADR_METHOD(EventType) getType() = 0;

    /**
     * Actually executes the callback with event-specific data.  This is
     * only called if event->getType() == this->getType().
     */
    ADR_METHOD(void) call(Event* event) = 0;
  };
  typedef RefPtr<Callback> CallbackPtr;

  
  /**
   * To listen for stream stopped events on a device, implement this interface
   * and call registerStopCallback() on the device, passing your
   * implementation.  streamStopped() will be called whenever a stream on that
   * device stops playback.
   *
   * WARNING: StopCallback is called from another thread.  Make sure your
   * callback is thread-safe.
   */
  class StopCallback : public Callback {
  protected:
    ~StopCallback() { }

  public:
    EventType ADR_CALL getType() { return ET_STOP; }
    void ADR_CALL call(Event* event) {
      streamStopped(static_cast<StopEvent*>(event));
    }

    /**
     * Called when a stream has stopped.
     *
     * @param event  Information pertaining to the event.
     */
    ADR_METHOD(void) streamStopped(StopEvent* event) = 0;
  };
  typedef RefPtr<StopCallback> StopCallbackPtr;


  /**
   * AudioDevice represents a device on the system which is capable
   * of opening and mixing multiple output streams.  In Windows,
   * DirectSound is such a device.
   *
   * This interface is synchronized.  update() and openStream() may
   * be called on different threads.
   */
  class AudioDevice : public RefCounted {
  protected:
    ~AudioDevice() { }

  public:
    /**
     * Tell the device to do any internal state updates.  Some devices
     * update on an internal thread.  If that is the case, this method
     * does nothing.
     */
    ADR_METHOD(void) update() = 0;

    /**
     * Open an output stream with a given sample source.  If the sample
     * source ever runs out of data, the output stream automatically stops
     * itself.
     *
     * The output stream takes ownership of the sample source, even if
     * opening the output stream fails (in which case the source is
     * immediately deleted).
     *
     * @param  source  the source used to feed the output stream with samples
     *
     * @return  new output stream if successful, 0 if failure
     */
    ADR_METHOD(OutputStream*) openStream(SampleSource* source) = 0;

    /**
     * Open a single buffer with the specified PCM data.  This is sometimes
     * more efficient than streaming and works on a larger variety of audio
     * devices.  In some implementations, this may download the audio data
     * to the sound card's memory itself.
     *
     * @param samples  Buffer containing sample data.  openBuffer() does
     *                 not take ownership of the memory.  The application
     *                 is responsible for freeing it.  There must be at
     *                 least |frame_count * channel_count *
     *                 GetSampleSize(sample_format)| bytes in the buffer.
     *
     * @param frame_count  Number of frames in the buffer.
     *
     * @param channel_count  Number of audio channels.  1 = mono, 2 = stereo.
     *
     * @param sample_rate  Number of samples per second.
     *
     * @param sample_format  Format of samples in buffer.
     *
     * @return  new output stream if successful, 0 if failure
     */
    ADR_METHOD(OutputStream*) openBuffer(
      void* samples,
      int frame_count,
      int channel_count,
      int sample_rate,
      SampleFormat sample_format) = 0;

    /**
     * Gets the name of the audio device.  For example "directsound" or "oss".
     *
     * @return name of audio device
     */
    ADR_METHOD(const char*) getName() = 0;

    /**
     * Registers 'callback' to receive events.  Callbacks can be
     * registered multiple times.
     */
    ADR_METHOD(void) registerCallback(Callback* callback) = 0;
    
    /**
     * Unregisters 'callback' once.  If it is registered multiple times,
     * each unregisterStopCallback call unregisters one of the instances.
     */
    ADR_METHOD(void) unregisterCallback(Callback* callback) = 0;

    /// Clears all of the callbacks from the device.
    ADR_METHOD(void) clearCallbacks() = 0;
  };
  typedef RefPtr<AudioDevice> AudioDevicePtr;


  /**
   * A readonly sample container which can open sample streams as iterators
   * through the buffer.  This is commonly used in cases where a very large
   * sound effect is loaded once into memory and then streamed several times
   * to the audio device.  This is more efficient memory-wise than loading
   * the effect multiple times.
   *
   * @see CreateSampleBuffer
   */
  class SampleBuffer : public RefCounted {
  protected:
    ~SampleBuffer() { }

  public:

    /**
     * Return the format of the sample data in the sample buffer.
     * @see SampleSource::getFormat
     */
    ADR_METHOD(void) getFormat(
      int& channel_count,
      int& sample_rate,
      SampleFormat& sample_format) = 0;

    /**
     * Get the length of the sample buffer in frames.
     */
    ADR_METHOD(int) getLength() = 0;

    /**
     * Get a readonly pointer to the samples contained within the buffer.  The
     * buffer is |channel_count * frame_count * GetSampleSize(sample_format)|
     * bytes long.
     */
    virtual const void* ADR_CALL getSamples() = 0;

    /**
     * Open a seekable sample source using the samples contained in the
     * buffer.
     */
    ADR_METHOD(SampleSource*) openStream() = 0;
  };
  typedef RefPtr<SampleBuffer> SampleBufferPtr;


  /**
   * Defines the type of SoundEffect objects.  @see SoundEffect
   */
  enum SoundEffectType {
    SINGLE,
    MULTIPLE,
  };


  /**
   * SoundEffect is a convenience class which provides a simple
   * mechanism for basic sound playback.  There are two types of sound
   * effects: SINGLE and MULTIPLE.  SINGLE sound effects only allow
   * the sound to be played once at a time.  MULTIPLE sound effects
   * always open a new stream to the audio device for each time it is
   * played (cleaning up or reusing old streams if possible).
   */
  class SoundEffect : public RefCounted {
  protected:
    ~SoundEffect() { }

  public:
    /**
     * Trigger playback of the sound.  If the SoundEffect is of type
     * SINGLE, this plays the sound if it isn't playing yet, and
     * starts it again if it is.  If the SoundEffect is of type
     * MULTIPLE, play() simply starts playing the sound again.
     */
    ADR_METHOD(void) play() = 0;

    /**
     * If the sound is of type SINGLE, stop the sound.  If it is of
     * type MULTIPLE, stop all playing instances of the sound.
     */
    ADR_METHOD(void) stop() = 0;

    /**
     * Sets the sound's volume.
     *
     * @param  volume  0.0 = silence, 1.0 = maximum volume (default)
     */
    ADR_METHOD(void) setVolume(float volume) = 0;

    /**
     * Gets the current volume.
     *
     * @return  current volume of the output stream
     */
    ADR_METHOD(float) getVolume() = 0;

    /**
     * Set current pan.
     *
     * @param pan  -1.0 = left, 0.0 = center (default), 1.0 = right
     */
    ADR_METHOD(void) setPan(float pan) = 0;

    /**
     * Get current pan.
     */
    ADR_METHOD(float) getPan() = 0;

    /**
     * Set current pitch shift.
     *
     * @param shift  can range from 0.5 to 2.0.  default is 1.0.
     */
    ADR_METHOD(void) setPitchShift(float shift) = 0;

    /**
     * Get current pitch shift.  Defaults to 1.0.
     */
    ADR_METHOD(float) getPitchShift() = 0;
  };
  typedef RefPtr<SoundEffect> SoundEffectPtr;


  /**
   * Represents a device capable of playing CD audio. Internally, this
   * uses the MCI subsystem in windows and libcdaudio on other platforms.
   * MCI subsystem: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_multimedia_command_strings.asp
   * libcdaudio: http://cdcd.undergrid.net/libcdaudio/
   */
  class CDDevice : public RefCounted {
  protected:
    virtual ~CDDevice() { }

  public:
    /**
     * Returns the name of this CD Device, often just the device name
     * it was created with.
     */
    virtual const char* ADR_CALL getName() = 0;

    /**
     * Returns the number of audio tracks on the disc.
     */
    ADR_METHOD(int) getTrackCount() = 0;

    /**
     * Starts playback of the given track. If another track was
     * already playing, the previous track is stopped.  IMPORTANT: Tracks are
     * indexed from 0 to getTrackCount() - 1.
     */
    ADR_METHOD(void) play(int track) = 0;

    /**
     * Stops the playback, if the playback was already stopped, this
     * does nothing.
     */
    ADR_METHOD(void) stop() = 0;
    
    /**
     * pauses playback of the track that is currently playing (if any)
     * This does nothing if no track is playing
     */
    ADR_METHOD(void) pause() = 0;

    /**
     * Resumes playback of the track that is currently paused (if any).
     * This does nothing if no track is paused.
     */
    ADR_METHOD(void) resume() = 0;

    /**
     * Returns true if the CD is currently playing a sound, this could
     * be through us, or through some other program.
     */
    ADR_METHOD(bool) isPlaying() = 0;

    /**
     * Returns true if the drive contains a cd. This might be slow
     * on some systems, use with care.
     */
    ADR_METHOD(bool) containsCD() = 0;

    /// Returns true if the door is open.
    ADR_METHOD(bool) isDoorOpen() = 0;

    /// Opens this device's door.
    ADR_METHOD(void) openDoor() = 0;

    /// Closes this device's door.
    ADR_METHOD(void) closeDoor() = 0;
  };
  typedef RefPtr<CDDevice> CDDevicePtr;


  /**
   * An opened MIDI song that can be played, stopped, and seeked within.
   * Uses MCI under Windows and is not supported in other platforms.
   */
  class MIDIStream : public RefCounted {
  protected:
    virtual ~MIDIStream() { }

  public:
    /**
     * Begins playback of the song and does nothing if the song is already
     * playing.
     */
    ADR_METHOD(void) play() = 0;

    /// Stops playback of the song and seeks to the beginning.
    ADR_METHOD(void) stop() = 0;

    /**
     * Stops playback of the song and does not change its current position.
     * A subsequent play() will resume the song where it left off.
     */
    ADR_METHOD(void) pause() = 0;

    /// Returns true if the song is currently playing, false otherwise.
    ADR_METHOD(bool) isPlaying() = 0;

    /// Returns the length of the song in milliseconds.
    ADR_METHOD(int) getLength() = 0;

    /// Returns the current position of the song in milliseconds.
    ADR_METHOD(int) getPosition() = 0;

    /// Sets the current position of the song.
    ADR_METHOD(void) setPosition(int position) = 0;

    /// Returns true if this song is set to repeat.
    ADR_METHOD(bool) getRepeat() = 0;

    /// Sets whether the song should repeat on completion.  Defaults to false.
    ADR_METHOD(void) setRepeat(bool repeat) = 0;
  };
  typedef RefPtr<MIDIStream> MIDIStreamPtr;


  /**
   * A MIDIDevice must be instantiated in order to open MIDIStreams.
   */
  class MIDIDevice : public RefCounted {
  protected:
    virtual ~MIDIDevice() { }

  public:
    /**
     * Returns the name of the device.
     */
    ADR_METHOD(const char*) getName() = 0;

    /**
     * openStream() creates and returns a new MIDIStream object from the
     * file with the specified name, which then can be queried and played.
     * This method returns NULL if the stream cannot be opened.
     *
     * Note: MCI subsystem limitations do not allow loading MIDIStream
     * objects from an audiere File implementation.  This may be addressed
     * in future versions of this API.
     */
    ADR_METHOD(MIDIStream*) openStream(const char* filename) = 0;
  };
  typedef RefPtr<MIDIDevice> MIDIDevicePtr;


  /// PRIVATE API - for internal use only
  namespace hidden {

    // these are extern "C" so we don't mangle the names

    ADR_FUNCTION(const char*) AdrGetVersion();

    /**
     * Returns a formatted string that lists the file formats that Audiere
     * supports.  This function is DLL-safe.
     *
     * It is formatted in the following way:
     *
     * description1:ext1,ext2,ext3;description2:ext1,ext2,ext3
     */
    ADR_FUNCTION(const char*) AdrGetSupportedFileFormats();

    /**
     * Returns a formatted string that lists the audio devices Audiere
     * supports.  This function is DLL-safe.
     *
     * It is formatted in the following way:
     *
     * name1:description1;name2:description2;...
     */
    ADR_FUNCTION(const char*) AdrGetSupportedAudioDevices();

    ADR_FUNCTION(int) AdrGetSampleSize(SampleFormat format);

    ADR_FUNCTION(AudioDevice*) AdrOpenDevice(
      const char* name,
      const char* parameters);

    ADR_FUNCTION(SampleSource*) AdrOpenSampleSource(
      const char* filename,
      FileFormat file_format);
    ADR_FUNCTION(SampleSource*) AdrOpenSampleSourceFromFile(
      File* file,
      FileFormat file_format);
    ADR_FUNCTION(SampleSource*) AdrCreateTone(double frequency);
    ADR_FUNCTION(SampleSource*) AdrCreateSquareWave(double frequency);
    ADR_FUNCTION(SampleSource*) AdrCreateWhiteNoise();
    ADR_FUNCTION(SampleSource*) AdrCreatePinkNoise();

    ADR_FUNCTION(LoopPointSource*) AdrCreateLoopPointSource(
      SampleSource* source);

    ADR_FUNCTION(OutputStream*) AdrOpenSound(
      AudioDevice* device,
      SampleSource* source,
      bool streaming);

    ADR_FUNCTION(SampleBuffer*) AdrCreateSampleBuffer(
      void* samples,
      int frame_count,
      int channel_count,
      int sample_rate,
      SampleFormat sample_format);
    ADR_FUNCTION(SampleBuffer*) AdrCreateSampleBufferFromSource(
      SampleSource* source);

    ADR_FUNCTION(SoundEffect*) AdrOpenSoundEffect(
      AudioDevice* device,
      SampleSource* source,
      SoundEffectType type);

    ADR_FUNCTION(File*) AdrOpenFile(
      const char* name,
      bool writeable);

    ADR_FUNCTION(File*) AdrCreateMemoryFile(
      const void* buffer,
      int size);

    ADR_FUNCTION(const char*) AdrEnumerateCDDevices();

    ADR_FUNCTION(CDDevice*) AdrOpenCDDevice(
      const char* name);  // Parameters?

    ADR_FUNCTION(MIDIDevice*) AdrOpenMIDIDevice(
      const char* name);  // Parameters?
  }




  /*-------- PUBLIC API FUNCTIONS --------*/


  /**
   * Returns the Audiere version string.
   *
   * @return  Audiere version information
   */
  inline const char* GetVersion() {
    return hidden::AdrGetVersion();
  }


  inline void SplitString(
    std::vector<std::string>& out,
    const char* in,
    char delim)
  {
    out.clear();
    while (*in) {
      const char* next = strchr(in, delim);
      if (next) {
        out.push_back(std::string(in, next));
      } else {
        out.push_back(in);
      }

      in = (next ? next + 1 : "");
    }
  }


  /// Describes a file format that Audiere supports.
  struct FileFormatDesc {
    /// Short description of format, such as "MP3 Files" or "Mod Files"
    std::string description;

    /// List of support extensions, such as {"mod", "it", "xm"}
    std::vector<std::string> extensions;
  };

  /// Populates a vector of FileFormatDesc structs.
  inline void GetSupportedFileFormats(std::vector<FileFormatDesc>& formats) {
    std::vector<std::string> descriptions;
    SplitString(descriptions, hidden::AdrGetSupportedFileFormats(), ';');

    formats.resize(descriptions.size());
    for (unsigned i = 0; i < descriptions.size(); ++i) {
      const char* d = descriptions[i].c_str();
      const char* colon = strchr(d, ':');
      formats[i].description.assign(d, colon);

      SplitString(formats[i].extensions, colon + 1, ',');
    }
  }


  /// Describes a supported audio device.
  struct AudioDeviceDesc {
    /// Name of device, i.e. "directsound", "winmm", or "oss"
    std::string name;

    // Textual description of device.
    std::string description;
  };

  /// Populates a vector of AudioDeviceDesc structs.
  inline void GetSupportedAudioDevices(std::vector<AudioDeviceDesc>& devices) {
    std::vector<std::string> descriptions;
    SplitString(descriptions, hidden::AdrGetSupportedAudioDevices(), ';');

    devices.resize(descriptions.size());
    for (unsigned i = 0; i < descriptions.size(); ++i) {
      std::vector<std::string> d;
      SplitString(d, descriptions[i].c_str(), ':');
      devices[i].name        = d[0];
      devices[i].description = d[1];
    }
  }


  /**
   * Get the size of a sample in a specific sample format.
   * This is commonly used to determine how many bytes a chunk of
   * PCM data will take.
   *
   * @return  Number of bytes a single sample in the specified format
   *          takes.
   */
  inline int GetSampleSize(SampleFormat format) {
    return hidden::AdrGetSampleSize(format);
  }

  /**
   * Open a new audio device. If name or parameters are not specified,
   * defaults are used. Each platform has its own set of audio devices.
   * Every platform supports the "null" audio device.
   *
   * @param  name  name of audio device that should be used
   * @param  parameters  comma delimited list of audio-device parameters;
   *                     for example, "buffer=100,rate=44100"
   *
   * @return  new audio device object if OpenDevice succeeds, and 0 in case
   *          of failure
   */
  inline AudioDevice* OpenDevice(
    const char* name = 0,
    const char* parameters = 0)
  {
    return hidden::AdrOpenDevice(name, parameters);
  }

  /**
   * Create a streaming sample source from a sound file.  This factory simply
   * opens a default file from the system filesystem and calls
   * OpenSampleSource(File*).
   *
   * @see OpenSampleSource(File*)
   */
  inline SampleSource* OpenSampleSource(
    const char* filename,
    FileFormat file_format = FF_AUTODETECT)
  {
    return hidden::AdrOpenSampleSource(filename, file_format);
  }

  /**
   * Opens a sample source from the specified file object.  If the sound file
   * cannot be opened, this factory function returns 0.
   *
   * @note  Some sound files support seeking, while some don't.
   *
   * @param file         File object from which to open the decoder
   * @param file_format  Format of the file to load.  If FF_AUTODETECT,
   *                     Audiere will try opening the file in each format.
   *
   * @return  new SampleSource if OpenSampleSource succeeds, 0 otherwise
   */
  inline SampleSource* OpenSampleSource(
    const FilePtr& file,
    FileFormat file_format = FF_AUTODETECT)
  {
    return hidden::AdrOpenSampleSourceFromFile(file.get(), file_format);
  }

  /**
   * Create a tone sample source with the specified frequency.
   *
   * @param  frequency  Frequency of the tone in Hz.
   *
   * @return  tone sample source
   */
  inline SampleSource* CreateTone(double frequency) {
    return hidden::AdrCreateTone(frequency);
  }

  /**
   * Create a square wave with the specified frequency.
   *
   * @param  frequency  Frequency of the wave in Hz.
   *
   * @return  wave sample source
   */
  inline SampleSource* CreateSquareWave(double frequency) {
    return hidden::AdrCreateSquareWave(frequency);
  }

  /**
   * Create a white noise sample source.  White noise is just random
   * data.
   *
   * @return  white noise sample source
   */
  inline SampleSource* CreateWhiteNoise() {
    return hidden::AdrCreateWhiteNoise();
  }

  /**
   * Create a pink noise sample source.  Pink noise is noise with equal
   * power distribution among octaves (logarithmic), not frequencies.
   *
   * @return  pink noise sample source
   */
  inline SampleSource* CreatePinkNoise() {
    return hidden::AdrCreatePinkNoise();
  }

  /**
   * Create a LoopPointSource from a SampleSource.  The SampleSource must
   * be seekable.  If it isn't, or the source isn't valid, this function
   * returns 0.
   */
  inline LoopPointSource* CreateLoopPointSource(
    const SampleSourcePtr& source)
  {
    return hidden::AdrCreateLoopPointSource(source.get());
  }

  /**
   * Creates a LoopPointSource from a source loaded from a file.
   */
  inline LoopPointSource* CreateLoopPointSource(
    const char* filename,
    FileFormat file_format = FF_AUTODETECT)
  {
    return CreateLoopPointSource(OpenSampleSource(filename, file_format));
  }

  /**
   * Creates a LoopPointSource from a source loaded from a file.
   */
  inline LoopPointSource* CreateLoopPointSource(
    const FilePtr& file,
    FileFormat file_format = FF_AUTODETECT)
  {
    return CreateLoopPointSource(OpenSampleSource(file, file_format));
  }

  /**
   * Try to open a sound buffer using the specified AudioDevice and
   * sample source.  If the specified sample source is seekable, it
   * loads it into memory and uses AudioDevice::openBuffer to create
   * the output stream.  If the stream is not seekable, it uses
   * AudioDevice::openStream to create the output stream.  This means
   * that certain file types must always be streamed, and therefore,
   * OpenSound will hold on to the file object.  If you must guarantee
   * that the file on disk is no longer referenced, you must create
   * your own memory file implementation and load your data into that
   * before calling OpenSound.
   *
   * @param device  AudioDevice in which to open the output stream.
   *
   * @param source  SampleSource used to generate samples for the sound
   *                object.  OpenSound takes ownership of source, even
   *                if it returns 0.  (In that case, OpenSound immediately
   *                deletes the SampleSource.)
   *
   * @param streaming  If false or unspecified, OpenSound attempts to
   *                   open the entire sound into memory.  Otherwise, it
   *                   streams the sound from the file.
   *
   * @return  new output stream if successful, 0 otherwise
   */
  inline OutputStream* OpenSound(
    const AudioDevicePtr& device,
    const SampleSourcePtr& source,
    bool streaming = false)
  {
    return hidden::AdrOpenSound(device.get(), source.get(), streaming);
  }

  /**
   * Calls OpenSound(AudioDevice*, SampleSource*) with a sample source
   * created via OpenSampleSource(const char*).
   */
  inline OutputStream* OpenSound(
    const AudioDevicePtr& device,
    const char* filename,
    bool streaming = false,
    FileFormat file_format = FF_AUTODETECT)
  {
    SampleSource* source = OpenSampleSource(filename, file_format);
    return OpenSound(device, source, streaming);
  }

  /**
   * Calls OpenSound(AudioDevice*, SampleSource*) with a sample source
   * created via OpenSampleSource(File* file).
   */
  inline OutputStream* OpenSound(
    const AudioDevicePtr& device,
    const FilePtr& file,
    bool streaming = false,
    FileFormat file_format = FF_AUTODETECT)
  {
    SampleSource* source = OpenSampleSource(file, file_format);
    return OpenSound(device, source, streaming);
  }

  /**
   * Create a SampleBuffer object using the specified samples and formats.
   *
   * @param samples  Pointer to a buffer of samples used to initialize the
   *                 new object.  If this is 0, the sample buffer contains
   *                 just silence.
   *
   * @param frame_count  Size of the sample buffer in frames.
   *
   * @param channel_count  Number of channels in each frame.
   *
   * @param sample_rate  Sample rate in Hz.
   *
   * @param sample_format  Format of each sample.  @see SampleFormat.
   *
   * @return  new SampleBuffer object
   */
  inline SampleBuffer* CreateSampleBuffer(
    void* samples,
    int frame_count,
    int channel_count,
    int sample_rate,
    SampleFormat sample_format)
  {
    return hidden::AdrCreateSampleBuffer(
      samples, frame_count,
      channel_count, sample_rate, sample_format);
  }

  /**
   * Create a SampleBuffer object from a SampleSource.
   *
   * @param source  Seekable sample source used to create the buffer.
   *                If the source is not seekable, then the function
   *                fails.
   *
   * @return  new sample buffer if success, 0 otherwise
   */
  inline SampleBuffer* CreateSampleBuffer(const SampleSourcePtr& source) {
    return hidden::AdrCreateSampleBufferFromSource(source.get());
  }

  /**
   * Open a SoundEffect object from the given sample source and sound
   * effect type.  @see SoundEffect
   *
   * @param device  AudioDevice on which the sound is played.
   *
   * @param source  The sample source used to feed the sound effect
   *                with data.
   *
   * @param type  The type of the sound effect.  If type is MULTIPLE,
   *              the source must be seekable.
   *
   * @return  new SoundEffect object if successful, 0 otherwise
   */
  inline SoundEffect* OpenSoundEffect(
    const AudioDevicePtr& device,
    const SampleSourcePtr& source,
    SoundEffectType type)
  {
    return hidden::AdrOpenSoundEffect(device.get(), source.get(), type);
  }

  /**
   * Calls OpenSoundEffect(AudioDevice*, SampleSource*,
   * SoundEffectType) with a sample source created from the filename.
   */
  inline SoundEffect* OpenSoundEffect(
    const AudioDevicePtr& device,
    const char* filename,
    SoundEffectType type,
    FileFormat file_format = FF_AUTODETECT)
  {
    SampleSource* source = OpenSampleSource(filename, file_format);
    return OpenSoundEffect(device, source, type);
  }

  /**
   * Calls OpenSoundEffect(AudioDevice*, SampleSource*,
   * SoundEffectType) with a sample source created from the file.
   */
  inline SoundEffect* OpenSoundEffect(
    const AudioDevicePtr& device,
    const FilePtr& file,
    SoundEffectType type,
    FileFormat file_format = FF_AUTODETECT)
  {
    SampleSource* source = OpenSampleSource(file, file_format);
    return OpenSoundEffect(device, source, type);
  }

  /**
   * Opens a default file implementation from the local filesystem.
   *
   * @param filename   The name of the file on the local filesystem.
   * @param writeable  Whether the writing to the file is allowed.
   */
  inline File* OpenFile(const char* filename, bool writeable) {
    return hidden::AdrOpenFile(filename, writeable);
  }

  /**
   * Creates a File implementation that reads from a buffer in memory.
   * It stores a copy of the buffer that is passed in.
   *
   * The File object does <i>not</i> take ownership of the memory buffer.
   * When the file is destroyed, it will not free the memory.
   *
   * @param buffer  Pointer to the beginning of the data.
   * @param size    Size of the buffer in bytes.
   *
   * @return  0 if size is non-zero and buffer is null. Otherwise,
   *          returns a valid File object.
   */
  inline File* CreateMemoryFile(const void* buffer, int size) {
    return hidden::AdrCreateMemoryFile(buffer, size);
  }

  /**
   * Generates a list of available CD device names.
   *
   * @param devices A vector of strings to be filled.
   */
  inline void EnumerateCDDevices(std::vector<std::string>& devices) {
    const char* d = hidden::AdrEnumerateCDDevices();
    while (d && *d) {
      devices.push_back(d);
      d += strlen(d) + 1;
    }
  }

  /**
   * Opens the specified CD playback device.
   * 
   * @param device  The filesystem device to be played.
   *                e.g. Linux: "/dev/cdrom", Windows: "D:"
   *
   * @return  0 if opening device failed, valid CDDrive object otherwise.
   */
  inline CDDevice* OpenCDDevice(const char* device) {
    return hidden::AdrOpenCDDevice(device);
  }

  /**
   * Opens the specified MIDI synthesizer device.
   *
   * @param device  The name of the device.  Unused for now.
   *
   * @return  0 if opening device failed, valid MIDIDevice object otherwise.
   */
  inline MIDIDevice* OpenMIDIDevice(const char* device) {
    return hidden::AdrOpenMIDIDevice(device);
  }

}


#endif
