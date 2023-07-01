#include <array>
#include <cassert>
#include <span>

#include "MpvPlayer.h"
#include "momuma/spdlog.h"


namespace MpvUtil
{

constexpr const char *STR_NULL = nullptr;

[[nodiscard]] static inline
mpv_error command(mpv_handle &ctx, const std::span<const char*> args)
{
	return static_cast<mpv_error>(mpv_command(&ctx, args.data()));
}

//a thin and type-safe wrapper for `mpv_set_property()` and `mpv_get_property()`.
struct Property
{
	mpv_handle &_ctx;
	const char *const _name;
	
	/*
	! `PropertyProxy` should be constructed with a string literal for the `name` param.
	! @param ctx: reference to a valid `mpv_handle`.
	! @param name: a C-string representing the name of the mpv property.
	*/
	inline Property(mpv_handle &ctx, const char *name) :
		_ctx { ctx }, _name { name }
	{}
	
	[[nodiscard]] mpv_error observe(uint64_t replyUserdata, mpv_format format);
	
	[[nodiscard]] mpv_error set(mpv_format format, const void *value);
	[[nodiscard]] mpv_error get(mpv_format format, void *value) const;
	
	// Equivalent to using `Property::set(MPV_FORMAT_NONE, nullptr)`.
	[[nodiscard]] mpv_error set(void);
	
	[[nodiscard]] mpv_error set_str(const std::string &value);
	[[nodiscard]] mpv_error set_flag(bool value);
	[[nodiscard]] mpv_error set_int(int64_t value);
	[[nodiscard]] mpv_error set_float(double value);
	[[nodiscard]] mpv_error set_time(chrono::microseconds value);
	
	[[nodiscard]] mpv_error get_str(std::string &value) const;
	[[nodiscard]] mpv_error get_flag(bool &value) const;
	[[nodiscard]] mpv_error get_int(int64_t &value) const;
	[[nodiscard]] mpv_error get_float(double &value) const;
	[[nodiscard]] mpv_error get_time(chrono::microseconds &value) const;
};

mpv_error Property::observe(uint64_t replyUserdata, mpv_format format)
{
	return static_cast<mpv_error>(mpv_observe_property(&_ctx, replyUserdata, _name, format));
}

mpv_error Property::set(mpv_format format, const void *value)
{
	return static_cast<mpv_error>(
		mpv_set_property(&_ctx, _name, format, const_cast<void*>(value))
	);
}

mpv_error Property::get(mpv_format format, void *value) const
{
	return static_cast<mpv_error>(mpv_get_property(&_ctx, _name, format, value));
}

mpv_error Property::set(void)
{
	return this->set(MPV_FORMAT_NONE, nullptr);
}

mpv_error Property::set_str(const std::string &value)
{
	const char *s = value.c_str();
	return this->set(MPV_FORMAT_STRING, &s);
}

mpv_error Property::set_flag(const bool value)
{
	const int flag = value;
	return this->set(MPV_FORMAT_FLAG, &flag);
}

mpv_error Property::set_int(int64_t value)
{
	return this->set(MPV_FORMAT_INT64, &value);
}

mpv_error Property::set_float(double value)
{
	return this->set(MPV_FORMAT_DOUBLE, &value);
}

mpv_error Property::set_time(chrono::microseconds value)
{
	double data = chrono::duration<double>(value).count();
	return this->set_float(data);
}

mpv_error Property::get_str(std::string &value) const
{
	char *s = nullptr;
	mpv_error err = this->get(MPV_FORMAT_STRING, &s);
	if (err == MPV_ERROR_SUCCESS) {
		value = s;
		mpv_free(s);
	}
	return err;
}

mpv_error Property::get_flag(bool &value) const
{
	int flag = 0;
	mpv_error err = this->get(MPV_FORMAT_FLAG, &flag);
	value = flag;
	return err;
}

mpv_error Property::get_int(int64_t &value) const
{
	return this->get(MPV_FORMAT_INT64, &value);
}

mpv_error Property::get_float(double &value) const
{
	return this->get(MPV_FORMAT_DOUBLE, &value);
}

mpv_error Property::get_time(chrono::microseconds &value) const
{
	double data;
	const mpv_error err = this->get_float(data);
	value = chrono::duration_cast<chrono::microseconds>(chrono::duration<double>(data));
	return err;
}

}



namespace Momuma
{

// Public:
// -----------------------------------------------------------------------------

chrono::microseconds MpvPlayer::query_duration(const fs::path &media)
{
	constexpr chrono::microseconds FAIL_VALUE(-1);
	static mpv_error initErr;
	static MpvPlayer player(initErr);
	if (!player) {
		SPDLOG_CRITICAL("Construction failed: ({}) {}", initErr, mpv_error_string(initErr));
		return FAIL_VALUE;
	}
	
	if (mpv_error e = player.set_media(media); e != MPV_ERROR_SUCCESS) {
		SPDLOG_ERROR("Failure to player.set_media(): {:s}", mpv_error_string(e));
		return FAIL_VALUE;
	}
	
	static bool loaded = false;
	while (true) {
		const mpv_event_id event = player.wait_event(chrono::seconds(-1))->event_id;
		
		if (event == MPV_EVENT_FILE_LOADED) {
			loaded = true;
			mpv_error err;
			return player.get_duration(err);
		}
		else if (event == MPV_EVENT_END_FILE) {
			if (!loaded) { return FAIL_VALUE; }
			loaded = false;
		}
	}
}

MpvPlayer::MpvPlayer(void) :
	_ctx { mpv_create() }
{
	(void)this->initialize();
}

MpvPlayer::MpvPlayer(mpv_error &err) :
	_ctx { mpv_create() }
{
	err = this->initialize();
}

MpvPlayer::~MpvPlayer(void)
{
	this->destroy();
}

MpvPlayer::operator bool(void) const
{
	return _ctx != nullptr;
}

MpvPlayer MpvPlayer::create_client(void)
{
	return MpvPlayer(*_ctx, nullptr);
}

void MpvPlayer::destroy(void)
{
	if (_ctx != nullptr) {
		mpv_destroy(_ctx);
		_ctx = nullptr;
	}
}

void MpvPlayer::stop_playback(void)
{
	[[maybe_unused]] mpv_error err;
	
	std::array cmd1 { "playlist-clear", MpvUtil::STR_NULL };
	err = MpvUtil::command(*_ctx, cmd1);
	assert(err == MPV_ERROR_SUCCESS);
	
	// will fail when playlist is empty
	std::array cmd2 { "playlist-remove", "0", MpvUtil::STR_NULL };
	err = MpvUtil::command(*_ctx, cmd2);
	SPDLOG_INFO("Playlist-remove: ({}) {}", err, mpv_error_string(err));
	
	this->set_play(false);
}

mpv_error MpvPlayer::set_media(const fs::path &media)
{
	std::array cmd = { "loadfile", media.c_str(), MpvUtil::STR_NULL };
	return MpvUtil::command(*_ctx, cmd);
}

mpv_error MpvPlayer::append_media(const fs::path &media, const bool play)
{
	const int64_t count = this->playlist_size();
	SPDLOG_DEBUG("Previous song count = {}", count);
	
	std::array cmd = {
		"loadfile", media.c_str(),
		((count > 0) ? (play ? "append-play" : "append") : MpvUtil::STR_NULL),
		MpvUtil::STR_NULL
	};
	return MpvUtil::command(*_ctx, cmd);
}

mpv_error MpvPlayer::set_position(const chrono::microseconds position)
{
	return MpvUtil::Property(*_ctx, "playback-time").set_time(position);
}

chrono::microseconds MpvPlayer::get_position(mpv_error &err) const
{
	chrono::microseconds data;
	err = MpvUtil::Property(*_ctx, "playback-time").get_time(data);
	return data;
}

int64_t MpvPlayer::get_index(void) const
{
	int64_t index;
	mpv_error err = MpvUtil::Property(*_ctx, "playlist-pos").get_int(index);
	assert(err == MPV_ERROR_SUCCESS);
	return index;
}

void MpvPlayer::set_index(const int64_t index)
{
	mpv_error err = MpvUtil::Property(*_ctx, "playlist-pos").set_int(index);
	if (err != MPV_ERROR_SUCCESS) {
		SPDLOG_ERROR("Items = {} | ({:d}): {:s}", this->playlist_size(), err, mpv_error_string(err));
	}
	assert(err == MPV_ERROR_SUCCESS);
}

void MpvPlayer::set_volume(const double volume)
{
	[[maybe_unused]] mpv_error err = MpvUtil::Property(*_ctx, "volume").set_float(volume);
	assert(err == MPV_ERROR_SUCCESS);
}

void MpvPlayer::set_mute(const bool mute)
{
	[[maybe_unused]] mpv_error err = MpvUtil::Property(*_ctx, "mute").set_flag(mute);
	assert(err == MPV_ERROR_SUCCESS);
}

void MpvPlayer::set_play(const bool play)
{
	assert(!play || (play && this->playlist_size() > 0));
	[[maybe_unused]] mpv_error err = MpvUtil::Property(*_ctx, "pause").set_flag(!play);
	assert(err == MPV_ERROR_SUCCESS);
}

bool MpvPlayer::is_paused(void) const
{
	bool paused;
	[[maybe_unused]] mpv_error err = MpvUtil::Property(*_ctx, "pause").get_flag(paused);
	assert(err == MPV_ERROR_SUCCESS);
	return paused;
}

chrono::microseconds MpvPlayer::get_duration(mpv_error &err) const
{
	chrono::microseconds data;
	err = MpvUtil::Property(*_ctx, "duration").get_time(data);
	return data;
}

fs::path MpvPlayer::get_current_media(void) const
{
	std::string path;
	const mpv_error err = MpvUtil::Property(*_ctx, "path").get_str(path);
	assert(err == MPV_ERROR_SUCCESS);
	return path;
}

int64_t MpvPlayer::playlist_size(void) const
{
	int64_t count;
	mpv_error err = MpvUtil::Property(*_ctx, "playlist-count").get_int(count);
	assert(err == MPV_ERROR_SUCCESS);
	return count;
}

bool MpvPlayer::playlist_empty(void) const
{
	return this->playlist_size() <= 0;
}

MpvPlayer::State MpvPlayer::get_state(void) const
{
	const bool paused = this->is_paused();
	if (!paused) {
		return State::PLAY;
	}
	else if (this->playlist_size() > 0) {
		return State::PAUSE;
	}
	return State::STOP;
}

mpv_event* MpvPlayer::wait_event(const chrono::microseconds timeout)
{
	const auto t = chrono::duration_cast<chrono::duration<double>>(timeout);
	mpv_event &event = *mpv_wait_event(_ctx, t.count());
	
	const State currState = this->get_state();
	if (currState != m_lastState) {
		signal_stateChanged.emit(*this, m_lastState, currState);
		m_lastState = currState;
	}
	
	switch (event.event_id)
	{
	case MPV_EVENT_NONE:
		signal_eventNone.emit();
		break;
	case MPV_EVENT_SHUTDOWN:
		signal_eventShutdown.emit();
		break;
	case MPV_EVENT_LOG_MESSAGE:
		signal_eventLogMessage.emit(*static_cast<mpv_event_log_message*>(event.data));
		break;
	case MPV_EVENT_GET_PROPERTY_REPLY:
		signal_eventGetPropertyReply.emit(
			event.error, event.reply_userdata,
			*static_cast<mpv_event_property*>(event.data)
		);
		break;
	case MPV_EVENT_SET_PROPERTY_REPLY:
		signal_eventSetPropertyReply.emit(event.error, event.reply_userdata);
		break;
	case MPV_EVENT_COMMAND_REPLY:
		signal_eventCommandReply.emit(
			event.error, event.reply_userdata,
			*static_cast<mpv_event_command*>(event.data)
		);
		break;
	case MPV_EVENT_START_FILE:
		signal_eventStartFile.emit(*static_cast<mpv_event_start_file*>(event.data));
		break;
	case MPV_EVENT_END_FILE:
		signal_eventEndFile.emit(*static_cast<mpv_event_end_file*>(event.data));
		signal_streamEnded.emit(*this);
		break;
	case MPV_EVENT_FILE_LOADED:
		signal_eventFileLoaded.emit();
		signal_streamStarted.emit(*this);
		break;
	case MPV_EVENT_CLIENT_MESSAGE:
		signal_eventClientMessage.emit(*static_cast<mpv_event_client_message*>(event.data));
		break;
	case MPV_EVENT_VIDEO_RECONFIG:
		signal_eventVideoReconfig.emit();
		break;
	case MPV_EVENT_AUDIO_RECONFIG:
		signal_eventAudioReconfig.emit();
		break;
	case MPV_EVENT_SEEK:
		signal_eventSeek.emit();
		break;
	case MPV_EVENT_PLAYBACK_RESTART:
		signal_eventPlaybackRestart.emit();
		break;
	case MPV_EVENT_PROPERTY_CHANGE:
		signal_eventPropertyChange.emit(
			event.reply_userdata, *static_cast<mpv_event_property*>(event.data)
		);
		break;
	case MPV_EVENT_QUEUE_OVERFLOW:
		signal_eventQueueOverflow.emit();
		break;
	case MPV_EVENT_HOOK:
		signal_eventHook.emit(
			event.reply_userdata, *static_cast<mpv_event_hook*>(event.data)
		);
		break;
	default:
		signal_eventUnknown(event);
	}
	
	//SPDLOG_TRACE("event: ({:d}, {:d}) {:s}",
	//	event.event_id, event.reply_userdata, mpv_event_name(event.event_id)
	//);
	return &event;
}



// Private:
// -----------------------------------------------------------------------------

MpvPlayer::MpvPlayer(mpv_handle &ctx, const char *name) :
	_ctx { mpv_create_client(&ctx, name) }
{
	if (_ctx == nullptr || this->initialize() != MPV_ERROR_SUCCESS) {
		this->destroy();
	}
}

mpv_error MpvPlayer::initialize(void)
{
	(void)MpvUtil::Property(*_ctx, "keep-open").set_flag(true);
	(void)MpvUtil::Property(*_ctx, "keep-open-pause").set_flag(false);
	this->set_play(false);
	return static_cast<mpv_error>(mpv_initialize(_ctx));
}

bool MpvPlayer::is_idle(void) const
{
	bool isIdle;
	const MpvUtil::Property property(*_ctx, "core-idle");
	[[maybe_unused]] mpv_error err = property.get_flag(isIdle);
	assert(err == MPV_ERROR_SUCCESS);
	return isIdle;
}

}
