#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <chrono>
#include <filesystem>
#include <gstreamermm/audiobasesink.h>
#include <gstreamermm/filesrc.h>
#include <gstreamermm/pipeline.h>


namespace Momuma
{

/* #Provides non-blocking audio-playing capabilities for audio files.
! Requires GStreamer to be initiated (e.g by calling `Gst::init()`).
*/
class AudioPlayer
{
public:
	enum class State
	{
		STOPPED, // disconnected from the tracked media and allows to change it.
		PAUSED, // tracks a media, but doesn't play it.
		PLAYING, // plays the tracked media.
	};
	[[nodiscard]] static
	std::string state_string(State state);
	
	
	AudioPlayer(const AudioPlayer &) = delete;
	AudioPlayer& operator=(const AudioPlayer &) = delete;
	
	AudioPlayer(void);
	
	AudioPlayer(AudioPlayer &&other);
	AudioPlayer& operator=(AudioPlayer &&other);
	
	~AudioPlayer(void);
	
	/* #Set the playbin to `State::STOPPED` in 3 steps:
	! set_state(State::PAUSED);
	! seek(0);
	! set_state(State::STOPPED);
	*/
	void stop(void);
	
	/* # Set player to `State::PAUSED`.
	! Does nothing if already set, but if the change fails the state is set to `State::STOPPED`.
	*/
	[[nodiscard]] bool pause(void);
	
	/* # Set player to `State::PLAYING`.
	! Does nothing if already set, but if the change fails the state is set to `State::STOPPED`.
	! @return: `true` if the final state is `State::PLAYING`.
	*/
	[[nodiscard]] bool play(void);
	
	/* #Set the audio source.
	! The object must be in `State::STOPPED`.
	! @param audioFile: the file which will act as the audio source. Must not be a URI.
	*/
	void set_audio_file(const std::filesystem::path &audioFile);
	
	/* #Set the volume level of the player.
	! @param volume: a floating-point value which must be in the range [0, 10.0].
	*/
	void set_volume(double volume);
	
	// Mute the player (this does NOT change the volume).
	void set_mute(bool mute);
	
	/* #Seeks the position in time relative to the start of the current audio file.
	! The object must be in State::PAUSED or State::PLAYING.
	*/
	void set_position(std::chrono::nanoseconds position);
	
	[[nodiscard]]
	State get_state(void) const;
	
	/* #Gets the audio source that was set with `set_audio_file()`
	*/
	[[nodiscard]]
	std::filesystem::path get_audio_file(void) const;
	
	/* #Retrieves the position in time relative to the start of the current audio file.
	! The object must be in State::PAUSED or State::PLAYING.
	*/
	[[nodiscard]]
	std::chrono::nanoseconds get_position(void) const;
	
	/* #Retrieves the length of the current audio file.
	! The object must be in State::PAUSED or State::PLAYING.
	*/
	[[nodiscard]]
	std::chrono::nanoseconds get_duration(void) const;
	
	
	[[nodiscard]]
	sigc::signal<void()> signal_end_of_stream(void);
	
	[[nodiscard]]
	sigc::signal<void(const std::string& debugInfo, Glib::Error)> signal_error(void);
	
	[[nodiscard]]
	sigc::signal<void(State newState)> signal_state_changed(void);
	
private:
	Glib::RefPtr<Gst::Pipeline> m_pipeline;
	Glib::RefPtr<Gst::BaseSrc> m_source; // kept as an instance to set the source
	Glib::RefPtr<Gst::AudioBaseSink> m_sink; // kept as an instance to set the volume
	
	Glib::RefPtr<Gst::QueryPosition> m_positionQuery;
	Glib::RefPtr<Gst::QueryDuration> m_durationQuery;
	
	sigc::signal<void()> m_sigMsg_Eos;
	sigc::signal<void(const std::string&, Glib::Error)> m_sigMsg_Error;
	sigc::signal<void(State)> m_sigMsg_StateChanged;
	
	
	[[nodiscard]] Gst::StateChangeReturn set_state(State state);
	
	void cb__bus_message(Glib::RefPtr<Gst::Message> msg);
};

}

#endif /* AUDIO_PLAYER_H */
