/*
 *  Copyright 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "tg_peer_connection_factory.h"

#include <memory>
#include <utility>
#include <vector>

#include "api/fec_controller.h"
#include "api/media_stream_proxy.h"
#include "api/media_stream_track_proxy.h"
#include "api/network_state_predictor.h"
#include "api/peer_connection_factory_proxy.h"
#include "api/peer_connection_proxy.h"
#include "api/rtc_event_log/rtc_event_log.h"
#include "api/transport/field_trial_based_config.h"
#include "api/transport/media/media_transport_interface.h"
#include "api/turn_customizer.h"
#include "api/units/data_rate.h"
#include "api/video_track_source_proxy.h"
#include "media/sctp/sctp_transport.h"
#include "p2p/base/basic_packet_socket_factory.h"
#include "p2p/base/default_ice_transport_factory.h"
#include "p2p/client/basic_port_allocator.h"
#include "pc/audio_track.h"
#include "pc/local_audio_source.h"
#include "pc/media_stream.h"
#include "pc/peer_connection.h"
#include "pc/rtp_parameters_conversion.h"
#include "pc/video_track.h"
#include "rtc_base/bind.h"
#include "rtc_base/checks.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/experiments/field_trial_units.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/system/file_wrapper.h"

#include "tg_rtp_data_engine.h"
#include "tg_peer_connection.h"

namespace webrtc {

rtc::scoped_refptr<TgPeerConnectionInterface>
TgPeerConnectionFactoryInterface::CreatePeerConnection(
    const PeerConnectionInterface::RTCConfiguration& configuration,
    std::unique_ptr<cricket::PortAllocator> allocator,
    std::unique_ptr<rtc::RTCCertificateGeneratorInterface> cert_generator,
    PeerConnectionObserver* observer) {
  return nullptr;
}

rtc::scoped_refptr<TgPeerConnectionInterface>
TgPeerConnectionFactoryInterface::CreatePeerConnection(
    const PeerConnectionInterface::RTCConfiguration& configuration,
    PeerConnectionDependencies dependencies) {
  return nullptr;
}

RtpCapabilities TgPeerConnectionFactoryInterface::GetRtpSenderCapabilities(
    cricket::MediaType kind) const {
  return {};
}

RtpCapabilities TgPeerConnectionFactoryInterface::GetRtpReceiverCapabilities(
    cricket::MediaType kind) const {
  return {};
}

BEGIN_SIGNALING_PROXY_MAP(TgPeerConnection)
PROXY_SIGNALING_THREAD_DESTRUCTOR()
PROXY_METHOD0(rtc::scoped_refptr<StreamCollectionInterface>, local_streams)
PROXY_METHOD0(rtc::scoped_refptr<StreamCollectionInterface>, remote_streams)
PROXY_METHOD1(bool, AddStream, MediaStreamInterface*)
PROXY_METHOD1(void, RemoveStream, MediaStreamInterface*)
PROXY_METHOD2(RTCErrorOr<rtc::scoped_refptr<RtpSenderInterface>>,
              AddTrack,
              rtc::scoped_refptr<MediaStreamTrackInterface>,
              const std::vector<std::string>&)
PROXY_METHOD1(bool, RemoveTrack, RtpSenderInterface*)
PROXY_METHOD1(RTCError, RemoveTrackNew, rtc::scoped_refptr<RtpSenderInterface>)
PROXY_METHOD1(RTCErrorOr<rtc::scoped_refptr<RtpTransceiverInterface>>,
              AddTransceiver,
              rtc::scoped_refptr<MediaStreamTrackInterface>)
PROXY_METHOD2(RTCErrorOr<rtc::scoped_refptr<RtpTransceiverInterface>>,
              AddTransceiver,
              rtc::scoped_refptr<MediaStreamTrackInterface>,
              const RtpTransceiverInit&)
PROXY_METHOD1(RTCErrorOr<rtc::scoped_refptr<RtpTransceiverInterface>>,
              AddTransceiver,
              cricket::MediaType)
PROXY_METHOD2(RTCErrorOr<rtc::scoped_refptr<RtpTransceiverInterface>>,
              AddTransceiver,
              cricket::MediaType,
              const RtpTransceiverInit&)
PROXY_METHOD2(rtc::scoped_refptr<RtpSenderInterface>,
              CreateSender,
              const std::string&,
              const std::string&)
PROXY_CONSTMETHOD0(std::vector<rtc::scoped_refptr<RtpSenderInterface>>,
                   GetSenders)
PROXY_CONSTMETHOD0(std::vector<rtc::scoped_refptr<RtpReceiverInterface>>,
                   GetReceivers)
PROXY_CONSTMETHOD0(std::vector<rtc::scoped_refptr<RtpTransceiverInterface>>,
                   GetTransceivers)
PROXY_METHOD0(void, ClearStatsCache)
PROXY_METHOD2(rtc::scoped_refptr<DataChannelInterface>,
              CreateDataChannel,
              const std::string&,
              const DataChannelInit*)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*, local_description)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*, remote_description)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*,
                   current_local_description)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*,
                   current_remote_description)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*,
                   pending_local_description)
PROXY_CONSTMETHOD0(const SessionDescriptionInterface*,
                   pending_remote_description)
PROXY_METHOD0(void, RestartIce)
PROXY_METHOD2(void,
              CreateOffer,
              CreateSessionDescriptionObserver*,
              const PeerConnectionInterface::RTCOfferAnswerOptions&)
PROXY_METHOD2(void,
              CreateAnswer,
              CreateSessionDescriptionObserver*,
              const PeerConnectionInterface::RTCOfferAnswerOptions&)
PROXY_METHOD2(void,
              SetLocalDescription,
              SetSessionDescriptionObserver*,
              SessionDescriptionInterface*)
PROXY_METHOD1(void, SetLocalDescription, SetSessionDescriptionObserver*)
PROXY_METHOD2(void,
              SetRemoteDescription,
              SetSessionDescriptionObserver*,
              SessionDescriptionInterface*)
PROXY_METHOD2(void,
              SetRemoteDescription,
              std::unique_ptr<SessionDescriptionInterface>,
              rtc::scoped_refptr<SetRemoteDescriptionObserverInterface>)
PROXY_METHOD0(PeerConnectionInterface::RTCConfiguration, GetConfiguration)
PROXY_METHOD1(RTCError,
              SetConfiguration,
              const PeerConnectionInterface::RTCConfiguration&)
PROXY_METHOD1(bool, AddIceCandidate, const IceCandidateInterface*)
PROXY_METHOD2(void,
              AddIceCandidate,
              std::unique_ptr<IceCandidateInterface>,
              std::function<void(RTCError)>)
PROXY_METHOD1(bool, RemoveIceCandidates, const std::vector<cricket::Candidate>&)
PROXY_METHOD1(RTCError, SetBitrate, const BitrateSettings&)
PROXY_METHOD1(void, SetAudioPlayout, bool)
PROXY_METHOD1(void, SetAudioRecording, bool)
PROXY_METHOD1(rtc::scoped_refptr<DtlsTransportInterface>,
              LookupDtlsTransportByMid,
              const std::string&)
PROXY_CONSTMETHOD0(rtc::scoped_refptr<SctpTransportInterface>, GetSctpTransport)
PROXY_METHOD0(PeerConnectionInterface::SignalingState, signaling_state)
PROXY_METHOD0(PeerConnectionInterface::IceConnectionState, ice_connection_state)
PROXY_METHOD0(PeerConnectionInterface::IceConnectionState, standardized_ice_connection_state)
PROXY_METHOD0(PeerConnectionInterface::PeerConnectionState, peer_connection_state)
PROXY_METHOD0(PeerConnectionInterface::IceGatheringState, ice_gathering_state)
PROXY_METHOD2(bool,
              StartRtcEventLog,
              std::unique_ptr<RtcEventLogOutput>,
              int64_t)
PROXY_METHOD1(bool, StartRtcEventLog, std::unique_ptr<RtcEventLogOutput>)
PROXY_METHOD0(void, StopRtcEventLog)
PROXY_METHOD0(void, Close)
END_PROXY_MAP()

TgPeerConnectionFactory::TgPeerConnectionFactory(
    PeerConnectionFactoryDependencies dependencies)
    : wraps_current_thread_(false),
      network_thread_(dependencies.network_thread),
      worker_thread_(dependencies.worker_thread),
      signaling_thread_(dependencies.signaling_thread),
      task_queue_factory_(std::move(dependencies.task_queue_factory)),
      media_engine_(std::move(dependencies.media_engine)),
      call_factory_(std::move(dependencies.call_factory)),
      event_log_factory_(std::move(dependencies.event_log_factory)),
      fec_controller_factory_(std::move(dependencies.fec_controller_factory)),
      network_state_predictor_factory_(
          std::move(dependencies.network_state_predictor_factory)),
      injected_network_controller_factory_(
          std::move(dependencies.network_controller_factory)),
      media_transport_factory_(std::move(dependencies.media_transport_factory)),
      neteq_factory_(std::move(dependencies.neteq_factory)),
      trials_(dependencies.trials ? std::move(dependencies.trials)
                                  : std::make_unique<FieldTrialBasedConfig>()) {
  if (!network_thread_) {
    owned_network_thread_ = rtc::Thread::CreateWithSocketServer();
    owned_network_thread_->SetName("pc_network_thread", nullptr);
    owned_network_thread_->Start();
    network_thread_ = owned_network_thread_.get();
  }

  if (!worker_thread_) {
    owned_worker_thread_ = rtc::Thread::Create();
    owned_worker_thread_->SetName("pc_worker_thread", nullptr);
    owned_worker_thread_->Start();
    worker_thread_ = owned_worker_thread_.get();
  }

  if (!signaling_thread_) {
    signaling_thread_ = rtc::Thread::Current();
    if (!signaling_thread_) {
      // If this thread isn't already wrapped by an rtc::Thread, create a
      // wrapper and own it in this class.
      signaling_thread_ = rtc::ThreadManager::Instance()->WrapCurrentThread();
      wraps_current_thread_ = true;
    }
  }
          
  options_.disable_encryption = true;
}

TgPeerConnectionFactory::~TgPeerConnectionFactory() {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  channel_manager_.reset(nullptr);

  // Make sure |worker_thread_| and |signaling_thread_| outlive
  // |default_socket_factory_| and |default_network_manager_|.
  default_socket_factory_ = nullptr;
  default_network_manager_ = nullptr;

  if (wraps_current_thread_)
    rtc::ThreadManager::Instance()->UnwrapCurrentThread();
}

bool TgPeerConnectionFactory::Initialize() {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  rtc::InitRandom(rtc::Time32());

  default_network_manager_.reset(new rtc::BasicNetworkManager());
  if (!default_network_manager_) {
    return false;
  }

  default_socket_factory_.reset(
      new rtc::BasicPacketSocketFactory(network_thread_));
  if (!default_socket_factory_) {
    return false;
  }

  channel_manager_ = std::make_unique<cricket::ChannelManager>(
      std::move(media_engine_), std::make_unique<cricket::TgRtpDataEngine>(),
      worker_thread_, network_thread_);

  channel_manager_->SetVideoRtxEnabled(true);
  if (!channel_manager_->Init()) {
    return false;
  }

  return true;
}

void TgPeerConnectionFactory::SetOptions(const PeerConnectionFactory::Options& options) {
  options_ = options;
}

RtpCapabilities TgPeerConnectionFactory::GetRtpSenderCapabilities(
    cricket::MediaType kind) const {
  RTC_DCHECK_RUN_ON(signaling_thread_);
  switch (kind) {
    case cricket::MEDIA_TYPE_AUDIO: {
      cricket::AudioCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channel_manager_->GetSupportedAudioSendCodecs(&cricket_codecs);
      channel_manager_->GetSupportedAudioRtpHeaderExtensions(
          &cricket_extensions);
      return ToRtpCapabilities(cricket_codecs, cricket_extensions);
    }
    case cricket::MEDIA_TYPE_VIDEO: {
      cricket::VideoCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channel_manager_->GetSupportedVideoCodecs(&cricket_codecs);
      channel_manager_->GetSupportedVideoRtpHeaderExtensions(
          &cricket_extensions);
      return ToRtpCapabilities(cricket_codecs, cricket_extensions);
    }
    case cricket::MEDIA_TYPE_DATA:
      return RtpCapabilities();
  }
  // Not reached; avoids compile warning.
  FATAL();
}

RtpCapabilities TgPeerConnectionFactory::GetRtpReceiverCapabilities(
    cricket::MediaType kind) const {
  RTC_DCHECK_RUN_ON(signaling_thread_);
  switch (kind) {
    case cricket::MEDIA_TYPE_AUDIO: {
      cricket::AudioCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channel_manager_->GetSupportedAudioReceiveCodecs(&cricket_codecs);
      channel_manager_->GetSupportedAudioRtpHeaderExtensions(
          &cricket_extensions);
      return ToRtpCapabilities(cricket_codecs, cricket_extensions);
    }
    case cricket::MEDIA_TYPE_VIDEO: {
      cricket::VideoCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channel_manager_->GetSupportedVideoCodecs(&cricket_codecs);
      channel_manager_->GetSupportedVideoRtpHeaderExtensions(
          &cricket_extensions);
      return ToRtpCapabilities(cricket_codecs, cricket_extensions);
    }
    case cricket::MEDIA_TYPE_DATA:
      return RtpCapabilities();
  }
  // Not reached; avoids compile warning.
  FATAL();
}

rtc::scoped_refptr<AudioSourceInterface>
TgPeerConnectionFactory::CreateAudioSource(const cricket::AudioOptions& options) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  rtc::scoped_refptr<LocalAudioSource> source(
      LocalAudioSource::Create(&options));
  return source;
}

bool TgPeerConnectionFactory::StartAecDump(FILE* file, int64_t max_size_bytes) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  return channel_manager_->StartAecDump(FileWrapper(file), max_size_bytes);
}

void TgPeerConnectionFactory::StopAecDump() {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  channel_manager_->StopAecDump();
}

rtc::scoped_refptr<TgPeerConnectionInterface>
TgPeerConnectionFactory::CreatePeerConnection(
    const PeerConnectionInterface::RTCConfiguration& configuration,
    std::unique_ptr<cricket::PortAllocator> allocator,
    std::unique_ptr<rtc::RTCCertificateGeneratorInterface> cert_generator,
    PeerConnectionObserver* observer) {
  // Convert the legacy API into the new dependency structure.
  PeerConnectionDependencies dependencies(observer);
  dependencies.allocator = std::move(allocator);
  dependencies.cert_generator = std::move(cert_generator);
  // Pass that into the new API.
  return CreatePeerConnection(configuration, std::move(dependencies));
}

rtc::scoped_refptr<TgPeerConnectionInterface>
TgPeerConnectionFactory::CreatePeerConnection(
    const PeerConnectionInterface::RTCConfiguration& configuration,
    PeerConnectionDependencies dependencies) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  RTC_DCHECK(!(dependencies.allocator && dependencies.packet_socket_factory))
      << "You can't set both allocator and packet_socket_factory; "
         "the former is going away (see bugs.webrtc.org/7447";

  // Set internal defaults if optional dependencies are not set.
  if (!dependencies.cert_generator) {
    dependencies.cert_generator =
        std::make_unique<rtc::RTCCertificateGenerator>(signaling_thread_,
                                                       network_thread_);
  }
  if (!dependencies.allocator) {
    rtc::PacketSocketFactory* packet_socket_factory;
    if (dependencies.packet_socket_factory)
      packet_socket_factory = dependencies.packet_socket_factory.get();
    else
      packet_socket_factory = default_socket_factory_.get();

    network_thread_->Invoke<void>(RTC_FROM_HERE, [this, &configuration,
                                                  &dependencies,
                                                  &packet_socket_factory]() {
      dependencies.allocator = std::make_unique<cricket::BasicPortAllocator>(
          default_network_manager_.get(), packet_socket_factory,
          configuration.turn_customizer);
    });
  }

  if (!dependencies.ice_transport_factory) {
    dependencies.ice_transport_factory =
        std::make_unique<DefaultIceTransportFactory>();
  }

  // TODO(zstein): Once chromium injects its own AsyncResolverFactory, set
  // |dependencies.async_resolver_factory| to a new
  // |rtc::BasicAsyncResolverFactory| if no factory is provided.

  network_thread_->Invoke<void>(
      RTC_FROM_HERE,
      rtc::Bind(&cricket::PortAllocator::SetNetworkIgnoreMask,
                dependencies.allocator.get(), options_.network_ignore_mask));

  std::unique_ptr<RtcEventLog> event_log =
      worker_thread_->Invoke<std::unique_ptr<RtcEventLog>>(
          RTC_FROM_HERE,
          rtc::Bind(&TgPeerConnectionFactory::CreateRtcEventLog_w, this));

  std::unique_ptr<Call> call = worker_thread_->Invoke<std::unique_ptr<Call>>(
      RTC_FROM_HERE,
      rtc::Bind(&TgPeerConnectionFactory::CreateCall_w, this, event_log.get()));

  rtc::scoped_refptr<TgPeerConnection> pc(
      new rtc::RefCountedObject<TgPeerConnection>(this, std::move(event_log),
                                                std::move(call)));
  if (!pc->Initialize(configuration, std::move(dependencies))) {
    return nullptr;
  }
  return TgPeerConnectionProxy::Create(signaling_thread(), pc);
}

rtc::scoped_refptr<MediaStreamInterface>
TgPeerConnectionFactory::CreateLocalMediaStream(const std::string& stream_id) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  return MediaStreamProxy::Create(signaling_thread_,
                                  MediaStream::Create(stream_id));
}

rtc::scoped_refptr<VideoTrackInterface> TgPeerConnectionFactory::CreateVideoTrack(
    const std::string& id,
    VideoTrackSourceInterface* source) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  rtc::scoped_refptr<VideoTrackInterface> track(
      VideoTrack::Create(id, source, worker_thread_));
  return VideoTrackProxy::Create(signaling_thread_, worker_thread_, track);
}

rtc::scoped_refptr<AudioTrackInterface> TgPeerConnectionFactory::CreateAudioTrack(
    const std::string& id,
    AudioSourceInterface* source) {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  rtc::scoped_refptr<AudioTrackInterface> track(AudioTrack::Create(id, source));
  return AudioTrackProxy::Create(signaling_thread_, track);
}

std::unique_ptr<cricket::SctpTransportInternalFactory>
TgPeerConnectionFactory::CreateSctpTransportInternalFactory() {
#ifdef HAVE_SCTP
  return std::make_unique<cricket::SctpTransportFactory>(network_thread());
#else
  return nullptr;
#endif
}

cricket::ChannelManager* TgPeerConnectionFactory::channel_manager() {
  return channel_manager_.get();
}

std::unique_ptr<RtcEventLog> TgPeerConnectionFactory::CreateRtcEventLog_w() {
  RTC_DCHECK_RUN_ON(worker_thread_);

  auto encoding_type = RtcEventLog::EncodingType::Legacy;
  if (IsTrialEnabled("WebRTC-RtcEventLogNewFormat"))
    encoding_type = RtcEventLog::EncodingType::NewFormat;
  return event_log_factory_
             ? event_log_factory_->CreateRtcEventLog(encoding_type)
             : std::make_unique<RtcEventLogNull>();
}

std::unique_ptr<Call> TgPeerConnectionFactory::CreateCall_w(
    RtcEventLog* event_log) {
  RTC_DCHECK_RUN_ON(worker_thread_);

  webrtc::Call::Config call_config(event_log);
  if (!channel_manager_->media_engine() || !call_factory_) {
    return nullptr;
  }
  call_config.audio_state =
      channel_manager_->media_engine()->voice().GetAudioState();

  FieldTrialParameter<DataRate> min_bandwidth("min", DataRate::kbps(30));
  FieldTrialParameter<DataRate> start_bandwidth("start", DataRate::kbps(300));
  FieldTrialParameter<DataRate> max_bandwidth("max", DataRate::kbps(2000));
  ParseFieldTrial({&min_bandwidth, &start_bandwidth, &max_bandwidth},
                  trials_->Lookup("WebRTC-PcFactoryDefaultBitrates"));

  call_config.bitrate_config.min_bitrate_bps =
      rtc::saturated_cast<int>(min_bandwidth->bps());
  call_config.bitrate_config.start_bitrate_bps =
      rtc::saturated_cast<int>(start_bandwidth->bps());
  call_config.bitrate_config.max_bitrate_bps =
      rtc::saturated_cast<int>(max_bandwidth->bps());

  call_config.fec_controller_factory = fec_controller_factory_.get();
  call_config.task_queue_factory = task_queue_factory_.get();
  call_config.network_state_predictor_factory =
      network_state_predictor_factory_.get();
  call_config.neteq_factory = neteq_factory_.get();

  if (IsTrialEnabled("WebRTC-Bwe-InjectedCongestionController")) {
    RTC_LOG(LS_INFO) << "Using injected network controller factory";
    call_config.network_controller_factory =
        injected_network_controller_factory_.get();
  } else {
    RTC_LOG(LS_INFO) << "Using default network controller factory";
  }

  call_config.trials = trials_.get();

  return std::unique_ptr<Call>(call_factory_->CreateCall(call_config));
}

bool TgPeerConnectionFactory::IsTrialEnabled(absl::string_view key) const {
  RTC_DCHECK(trials_);
  return trials_->Lookup(key).find("Enabled") == 0;
}

}  // namespace webrtc