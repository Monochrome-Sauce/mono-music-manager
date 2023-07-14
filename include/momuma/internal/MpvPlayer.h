#ifndef MONO_MUSIC_MANAGER__INTERNAL__MPV_PLAYER_H
#define MONO_MUSIC_MANAGER__INTERNAL__MPV_PLAYER_H

#include <filesystem>
#include <mpv/client.h>

#include "momuma/sigc.h"


namespace Momuma
{

class MpvPlayer
{
public:
	enum class State {
		STOP, /* Playlist is empty */
		PAUSE, /* Player is stopped and playlist has items */
		PLAY, /* Player is active and playlist has items */
	};
	
	/* #Query the duration of a random media file using a private MpvPlayer object.
	! Due to the nature of libmpv's `mpv_wait_event()`, this function is *not* MT thread-safe.
	! @param media: path to a media file.
	! @return: the duration of the file at `media`, or -1 in case of failure.
	*/
	[[nodiscard]] static
	std::chrono::microseconds query_duration(const std::filesystem::path &media);
	
	
	mpv_handle *_ctx;
	
	// Equivalent to `mpv_create()` followed by `mpv_initialize()`.
	MpvPlayer(void);
	MpvPlayer(mpv_error &err);
	~MpvPlayer(void);
	
	MpvPlayer(MpvPlayer &&other) = default;
	MpvPlayer& operator=(MpvPlayer &&other) = default;
	
	MpvPlayer(const MpvPlayer &other) = delete;
	MpvPlayer& operator=(const MpvPlayer &other) = delete;
	
	[[nodiscard]] explicit operator bool(void) const;
	
	[[nodiscard]] MpvPlayer create_client(void);
	
	// Called by the destructor.
	void destroy(void);
	
	// Clears the playlist and pauses the playback.
	void stop_playback(void);
	
	/* #Sets `media` as the only item in the current playlist.
	! @param media: path to a playable media file.
	*/
	[[nodiscard]] mpv_error set_media(const std::filesystem::path &media);
	
	/* #Appends `media` to the end of the current playlist.
	! @param media: path to a playable media file.
	! @param play: when set to `true`, the appended media starts playing
	instead of the current one.
	*/
	[[nodiscard]] mpv_error append_media(const std::filesystem::path &media, bool play=false);
	
	[[nodiscard]] mpv_error set_position(std::chrono::microseconds position);
	[[nodiscard]] std::chrono::microseconds get_position(mpv_error &err) const;
	
	/* # Index of the "playing" media on playlist.
	! A playlist item is "playing" if it's being loaded, actually playing, or
	being unloaded. This property is set during the `MPV_EVENT_START_FILE` (start-file)
	and the `MPV_EVENT_END_FILE` (end-file) events. Outside of that, it returns `-1`.
	! If the playlist entry was somehow removed during playback, but playback hasn't
	stopped yet, or is in progress of being stopped, it also returns `-1`. (This
	can happen at least during state transitions.)
	*/
	[[nodiscard]] int64_t get_index(void) const;
	
	/* #Set the index of the currently playing media.
	*/
	void set_index(int64_t index);
	
	void set_volume(double volume);
	void set_mute(bool mute);
	void set_play(bool play);
	[[nodiscard]] bool is_paused(void) const;
	
	[[nodiscard]] std::chrono::microseconds get_duration(mpv_error &err) const;
	[[nodiscard]] std::filesystem::path get_current_media(void) const;
	
	// the result is negative when the state is `State::STOP`
	[[nodiscard]] int64_t playlist_size(void) const;
	
	// equivalent to `get_media_count() <= 0`.
	[[nodiscard]] bool playlist_empty(void) const;
	
	State get_state(void) const;
	
	mpv_event* wait_event(std::chrono::microseconds timeout);
	
	sigc::signal<void(MpvPlayer &src)> signal_streamStarted;
	sigc::signal<void(MpvPlayer &src)> signal_streamEnded;
	sigc::signal<void(MpvPlayer &src, State prevState, State newState)> signal_stateChanged;
	
	sigc::signal<void()> signal_eventNone;
	sigc::signal<void()> signal_eventShutdown;
	sigc::signal<void(mpv_event_log_message &data)> signal_eventLogMessage;
	sigc::signal<void(int error, uint64_t replyUserdata, mpv_event_property &data)> signal_eventGetPropertyReply;
	sigc::signal<void(int error, uint64_t replyUserdata)> signal_eventSetPropertyReply;
	sigc::signal<void(int error, uint64_t replyUserdata, mpv_event_command &data)> signal_eventCommandReply;
	sigc::signal<void(mpv_event_start_file &data)> signal_eventStartFile;
	sigc::signal<void(mpv_event_end_file &data)> signal_eventEndFile;
	sigc::signal<void()> signal_eventFileLoaded;
	sigc::signal<void(mpv_event_client_message &data)> signal_eventClientMessage;
	sigc::signal<void()> signal_eventVideoReconfig;
	sigc::signal<void()> signal_eventAudioReconfig;
	sigc::signal<void()> signal_eventSeek;
	sigc::signal<void()> signal_eventPlaybackRestart;
	sigc::signal<void(uint64_t replyUserdata, mpv_event_property &data)> signal_eventPropertyChange;
	sigc::signal<void()> signal_eventQueueOverflow;
	sigc::signal<void(uint64_t replyUserdata, mpv_event_hook &data)> signal_eventHook;
	
	// a signal for all events not listed above
	sigc::signal<void(mpv_event &event)> signal_eventUnknown;
	
private:
	State m_lastState = State::STOP;
	
	// Equivalent to `mpv_create_client()`.
	MpvPlayer(mpv_handle &ctx, const char *name);
	
	[[nodiscard]] mpv_error initialize(void);
	
	[[nodiscard]] bool is_idle(void) const;
};

}

#endif /* MONO_MUSIC_MANAGER__INTERNAL__MPV_PLAYER_H */
