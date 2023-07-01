#include <gstreamermm/audioconvert.h>
#include <gstreamermm/bus.h>
#include <gstreamermm/decodebin.h>
#include <gstreamermm/discoverer.h>
#include <gstreamermm/discovererinfo.h>

#include "AudioPlayer.h"
#include "momuma/spdlog.h"


namespace Momuma
{

[[nodiscard]] static
AudioPlayer::State State_from_GstState(const Gst::State state)
{
	assert(state >= Gst::STATE_READY);
	switch (state)
	{
		case Gst::STATE_READY: return AudioPlayer::State::STOPPED;
		case Gst::STATE_PAUSED: return AudioPlayer::State::PAUSED;
		case Gst::STATE_PLAYING: return AudioPlayer::State::PLAYING;
		default: std::terminate();
	}
}

[[nodiscard]] static
Gst::State State_to_GstState(const AudioPlayer::State state)
{
	switch (state)
	{
		case AudioPlayer::State::STOPPED: return Gst::STATE_READY;
		case AudioPlayer::State::PAUSED: return Gst::STATE_PAUSED;
		case AudioPlayer::State::PLAYING: return Gst::STATE_PLAYING;
		default: std::terminate();
	}
}

static Glib::RefPtr<Gst::AudioBaseSink> create_GstAudioSink(void)
{
	Glib::RefPtr<Gst::Element> sink = Gst::ElementFactory::create_element("pulsesink");
	return Glib::RefPtr<Gst::AudioBaseSink>::cast_static(sink);
}

static void on_pad_added(const Glib::RefPtr<Gst::Pad> &srcPad, Gst::Element &dstElement)
{
	const Gst::Structure capabilities = srcPad->get_current_caps()->get_structure(0);
	const Glib::ustring padType = capabilities.get_name();
	
	if G_UNLIKELY (padType.find("audio/x-raw") == Glib::ustring::npos) {
		SPDLOG_DEBUG("AudioPlayer - Ignoring pad (type: {:s}).", padType);
		return;
	}
	else {
		SPDLOG_DEBUG("AudioPlayer - Got pad (type: {:s}).", padType);
	}
	
	Gst::PadLinkReturn res = srcPad->link(dstElement.get_static_pad("sink"));
	if (res != Gst::PAD_LINK_OK) {
		SPDLOG_CRITICAL("AudioPlayer - Failed to link pads (reason: {:d})", res);
	}
}

// public
// ==================================================

std::string AudioPlayer::state_string(State state)
{
#ifndef CASE_STRING
	#define CASE_STRING(x) case x: return G_STRINGIFY(x)
	switch (state)
	{
		CASE_STRING(State::STOPPED);
		CASE_STRING(State::PAUSED);
		CASE_STRING(State::PLAYING);
	default:
		throw std::logic_error("Invalid enum value");
	}
	#undef CASE_STRING
#else
	#error "Missing switch case"
#endif
}


AudioPlayer::AudioPlayer(void) :
	m_pipeline { Gst::Pipeline::create() },
	m_source { Gst::FileSrc::create() },
	m_sink { create_GstAudioSink() },
	m_positionQuery { Gst::QueryPosition::create(Gst::FORMAT_TIME) },
	m_durationQuery { Gst::QueryDuration::create(Gst::FORMAT_TIME) }
{
	// this is too error-prone in case of modification...
	if (!m_pipeline) { throw std::runtime_error("Failed to create Gst pipeline"); }
	if (!m_source) { throw std::runtime_error("Failed to create Gst source"); }
	if (!m_sink) { throw std::runtime_error("Failed to create Gst sink"); }
	if (!m_positionQuery) { throw std::runtime_error("Failed to create Gst position query"); }
	if (!m_durationQuery) { throw std::runtime_error("Failed to create Gst duration query"); }
	
	const Glib::RefPtr decoder = Gst::DecodeBin::create();
	if (!decoder) { throw std::runtime_error("Failed to create Gstreamer decoder"); }
	
	const Glib::RefPtr converter = Gst::AudioConvert::create();
	if (!converter) { throw std::runtime_error("Failed to create Gstreamer converter"); }
	
	
	m_pipeline->add(m_source)->add(decoder)->add(converter)->add(m_sink);
	m_source->link(decoder);
	converter->link(m_sink);
	
	decoder->signal_pad_added().connect( // link decoder to converter at run-time
		sigc::bind(&on_pad_added, sigc::ref(*converter.get()))
	);
	
	const Glib::RefPtr<Gst::Bus> bus = m_pipeline->get_bus();
	bus->add_signal_watch();
	bus->signal_message().connect(sigc::mem_fun(*this, &AudioPlayer::cb__bus_message));
	
	if (this->set_state(State::STOPPED) == Gst::STATE_CHANGE_FAILURE) {
		throw std::runtime_error("Unable to set the pipeline to State::STOPPED");
	}
}

AudioPlayer::AudioPlayer(AudioPlayer &&other)
{
	*this = std::move(other);
}

AudioPlayer& AudioPlayer::operator=(AudioPlayer &&other)
{
	assert(this != &other);
	m_pipeline = std::move(other.m_pipeline);
	m_source = std::move(other.m_source);
	m_sink = std::move(other.m_sink);
	
	m_positionQuery = std::move(other.m_positionQuery);
	m_durationQuery = std::move(other.m_durationQuery);
	
	m_sigMsg_Eos = std::move(other.m_sigMsg_Eos);
	m_sigMsg_Error = std::move(other.m_sigMsg_Error);
	m_sigMsg_StateChanged = std::move(other.m_sigMsg_StateChanged);
	return *this;
}

AudioPlayer::~AudioPlayer(void)
{
	m_pipeline->set_state(Gst::STATE_NULL);
}

bool AudioPlayer::play(void)
{
	const bool changed = (this->set_state(State::PLAYING) != Gst::STATE_CHANGE_FAILURE);
	if (!changed) {
		[[maybe_unused]] const bool good = this->set_state(State::STOPPED);
		SPDLOG_DEBUG("set_state(State::STOPPED) returned `{}`", good);
	}
	return changed;
}

bool AudioPlayer::pause(void)
{
	const bool changed = (this->set_state(State::PAUSED) != Gst::STATE_CHANGE_FAILURE);
	if (!changed) {
		[[maybe_unused]] const bool good = this->set_state(State::STOPPED);
		SPDLOG_DEBUG("set_state(State::STOPPED) returned `{}`", good);
	}
	return changed;
}

void AudioPlayer::stop(void)
{
	if (this->get_state() == State::STOPPED) { return; }
	
	if G_UNLIKELY (!this->pause()) {
		SPDLOG_ERROR("Failed to change to State::PAUSED");
	}
	this->set_position(chrono::seconds(0));
	
	if G_UNLIKELY (this->set_state(State::STOPPED) == Gst::STATE_CHANGE_FAILURE) {
		SPDLOG_ERROR("Failed to change to State::STOPPED");
	}
}

void AudioPlayer::set_audio_file(const fs::path &audioFile)
{
	assert(this->get_state() == State::STOPPED);
	SPDLOG_TRACE("{:s}(\"{:s}\")", SPDLOG_FUNCTION, audioFile);
	m_source->set_property("location", audioFile.string());
}

void AudioPlayer::set_volume(const double volume)
{
	assert(0.0 <= volume && volume <= 10.0);
	m_sink->set_property("volume", volume);
}

void AudioPlayer::set_mute(const bool mute)
{
	m_sink->set_property("mute", mute);
}

void AudioPlayer::set_position(const chrono::nanoseconds time)
{
	SPDLOG_TRACE("AudioPlayer::{:s}(time={})", SPDLOG_FUNCTION, time);
	assert(this->get_state() != State::STOPPED);
	m_pipeline->seek(
		Gst::FORMAT_TIME,
		Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_KEY_UNIT,
		chrono::duration_cast<chrono::nanoseconds>(time).count()
	);
}

AudioPlayer::State AudioPlayer::get_state(void) const
{
	Gst::State state, pending;
	m_pipeline->get_state(state, pending, Gst::CLOCK_TIME_NONE);
	return State_from_GstState(state);
}

fs::path AudioPlayer::get_audio_file(void) const
{
	Glib::ustring path;
	m_source->get_property("location", path);
	return path.raw();
}

chrono::nanoseconds AudioPlayer::get_position(void) const
{
	assert(this->get_state() != State::STOPPED);
	[[maybe_unused]] const bool x = m_pipeline->query(m_positionQuery);
	assert(x);
	return chrono::nanoseconds(m_positionQuery->parse());
}

chrono::nanoseconds AudioPlayer::get_duration(void) const
{
	assert(this->get_state() != State::STOPPED);
	[[maybe_unused]] const bool x = m_pipeline->query(m_durationQuery);
	assert(x);
	
	return chrono::nanoseconds(m_durationQuery->parse());
}

sigc::signal<void()> AudioPlayer::signal_end_of_stream(void)
{
	return m_sigMsg_Eos;
}

sigc::signal<void(const std::string&, Glib::Error)> AudioPlayer::signal_error(void)
{
	return m_sigMsg_Error;
}

sigc::signal<void(AudioPlayer::State)> AudioPlayer::signal_state_changed(void)
{
	return m_sigMsg_StateChanged;
}

// private
// ==================================================

Gst::StateChangeReturn AudioPlayer::set_state(State state)
{
	return m_pipeline->set_state(State_to_GstState(state));
}

void AudioPlayer::cb__bus_message(Glib::RefPtr<Gst::Message> msg)
{
	const Glib::RefPtr<Gst::Object> src = msg->get_source();
	if (src != decltype(src)::cast_dynamic(m_pipeline)) {
		return;
	}
	
	switch (msg->get_message_type())
	{
	case Gst::MESSAGE_EOS:
		m_sigMsg_Eos.emit();
		break;
	case Gst::MESSAGE_ERROR:
	{
		const auto m = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
		Glib::Error error;
		std::string debugInfo;
		m->parse(error, debugInfo);
		
		SPDLOG_TRACE("Error received from element {:s}", m->get_source()->get_name());
		m_sigMsg_Error.emit(debugInfo, error);
		break;
	}
	case Gst::MESSAGE_STATE_CHANGED:
	{
		const auto m = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
		const Gst::State old = m->parse_old_state(), curr = m->parse_new_state();
		SPDLOG_TRACE("State {:s} => {:s}",
			gst_element_state_get_name(static_cast<GstState>(old)),
			gst_element_state_get_name(static_cast<GstState>(curr))
		);
		
		if G_LIKELY (curr != old && curr >= Gst::STATE_READY) {
			m_sigMsg_StateChanged.emit(State_from_GstState(curr));
		}
		break;
	}
	default:
		//SPDLOG_TRACE("{:s} msg type: {}", SPDLOG_FUNCTION, __builtin_ctz((unsigned)type));
		break; // silence -Wswitch error
	}
}

}
