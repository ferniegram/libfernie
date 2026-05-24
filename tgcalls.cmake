add_library(tgcalls STATIC)

set_target_properties(tgcalls PROPERTIES POSITION_INDEPENDENT_CODE ON)

include(FindPkgConfig)

set(tgcalls_loc ${CMAKE_CURRENT_SOURCE_DIR}/tgcalls/tgcalls)

target_include_directories(tgcalls PUBLIC
    ${tgcalls_loc}

    ${CMAKE_CURRENT_SOURCE_DIR}/tg_owt/include/tg_owt
)

target_sources(tgcalls PRIVATE
    ${tgcalls_loc}/Instance.cpp
    ${tgcalls_loc}/Instance.h

    ${tgcalls_loc}/AudioDeviceHelper.cpp
    ${tgcalls_loc}/AudioDeviceHelper.h
    ${tgcalls_loc}/ChannelManager.cpp
    ${tgcalls_loc}/ChannelManager.h
    ${tgcalls_loc}/CodecSelectHelper.cpp
    ${tgcalls_loc}/CodecSelectHelper.h
    ${tgcalls_loc}/CryptoHelper.cpp
    ${tgcalls_loc}/CryptoHelper.h
    ${tgcalls_loc}/DirectConnectionChannel.h
    ${tgcalls_loc}/EncryptedConnection.cpp
    ${tgcalls_loc}/EncryptedConnection.h
    ${tgcalls_loc}/FakeAudioDeviceModule.cpp
    ${tgcalls_loc}/FakeAudioDeviceModule.h
    ${tgcalls_loc}/FieldTrialsConfig.cpp
    ${tgcalls_loc}/FieldTrialsConfig.h
    ${tgcalls_loc}/InstanceImpl.cpp
    ${tgcalls_loc}/InstanceImpl.h
    ${tgcalls_loc}/LogSinkImpl.cpp
    ${tgcalls_loc}/LogSinkImpl.h
    ${tgcalls_loc}/Manager.cpp
    ${tgcalls_loc}/Manager.h
    ${tgcalls_loc}/MediaManager.cpp
    ${tgcalls_loc}/MediaManager.h
    ${tgcalls_loc}/Message.cpp
    ${tgcalls_loc}/Message.h
    ${tgcalls_loc}/NetworkManager.cpp
    ${tgcalls_loc}/NetworkManager.h
    ${tgcalls_loc}/SctpDataChannelProviderInterfaceImpl.cpp
    ${tgcalls_loc}/SctpDataChannelProviderInterfaceImpl.h
    ${tgcalls_loc}/StaticThreads.cpp
    ${tgcalls_loc}/StaticThreads.h
    ${tgcalls_loc}/ThreadLocalObject.h
    ${tgcalls_loc}/TurnCustomizerImpl.cpp
    ${tgcalls_loc}/TurnCustomizerImpl.h
    ${tgcalls_loc}/VideoCaptureInterface.cpp
    ${tgcalls_loc}/VideoCaptureInterface.h
    ${tgcalls_loc}/VideoCaptureInterfaceImpl.cpp
    ${tgcalls_loc}/VideoCaptureInterfaceImpl.h
    ${tgcalls_loc}/VideoCapturerInterface.h

    ${tgcalls_loc}/utils/gzip.cpp
    ${tgcalls_loc}/utils/gzip.h

    ${tgcalls_loc}/v2/ContentNegotiation.cpp
    ${tgcalls_loc}/v2/ContentNegotiation.h
    ${tgcalls_loc}/v2/DirectNetworkingImpl.cpp
    ${tgcalls_loc}/v2/DirectNetworkingImpl.h
    ${tgcalls_loc}/v2/ExternalSignalingConnection.cpp
    ${tgcalls_loc}/v2/ExternalSignalingConnection.h
    ${tgcalls_loc}/v2/InstanceNetworking.h
    ${tgcalls_loc}/v2/InstanceV2ReferenceImpl.cpp
    ${tgcalls_loc}/v2/InstanceV2ReferenceImpl.h
    ${tgcalls_loc}/v2/InstanceV2Impl.cpp
    ${tgcalls_loc}/v2/InstanceV2Impl.h
    ${tgcalls_loc}/v2/NativeNetworkingImpl.cpp
    ${tgcalls_loc}/v2/NativeNetworkingImpl.h
    ${tgcalls_loc}/v2/RawTcpSocket.cpp
    ${tgcalls_loc}/v2/RawTcpSocket.h
    ${tgcalls_loc}/v2/ReflectorPort.cpp
    ${tgcalls_loc}/v2/ReflectorPort.h
    ${tgcalls_loc}/v2/ReflectorRelayPortFactory.cpp
    ${tgcalls_loc}/v2/ReflectorRelayPortFactory.h
    ${tgcalls_loc}/v2/Signaling.cpp
    ${tgcalls_loc}/v2/Signaling.h
    ${tgcalls_loc}/v2/SignalingConnection.cpp
    ${tgcalls_loc}/v2/SignalingConnection.h
    ${tgcalls_loc}/v2/SignalingEncryption.cpp
    ${tgcalls_loc}/v2/SignalingEncryption.h
    ${tgcalls_loc}/v2/SignalingSctpConnection.cpp
    ${tgcalls_loc}/v2/SignalingSctpConnection.h
    ${tgcalls_loc}/v2/CustomDcSctpSocket.cpp
    ${tgcalls_loc}/v2/CustomDcSctpSocket.h

    ${tgcalls_loc}/platform/fake/FakeInterface.cpp
    ${tgcalls_loc}/platform/fake/FakeInterface.h

    # Group calls
    ${tgcalls_loc}/group/AVIOContextImpl.cpp
    ${tgcalls_loc}/group/AVIOContextImpl.h
    ${tgcalls_loc}/group/AudioStreamingPart.cpp
    ${tgcalls_loc}/group/AudioStreamingPart.h
    ${tgcalls_loc}/group/AudioStreamingPartInternal.cpp
    ${tgcalls_loc}/group/AudioStreamingPartInternal.h
    ${tgcalls_loc}/group/AudioStreamingPartPersistentDecoder.cpp
    ${tgcalls_loc}/group/AudioStreamingPartPersistentDecoder.h
    ${tgcalls_loc}/group/GroupInstanceCustomImpl.cpp
    ${tgcalls_loc}/group/GroupInstanceCustomImpl.h
    ${tgcalls_loc}/group/GroupInstanceImpl.h
    ${tgcalls_loc}/group/GroupJoinPayloadInternal.cpp
    ${tgcalls_loc}/group/GroupJoinPayloadInternal.h
    ${tgcalls_loc}/group/GroupJoinPayload.h
    ${tgcalls_loc}/group/GroupNetworkManager.cpp
    ${tgcalls_loc}/group/GroupNetworkManager.h
    ${tgcalls_loc}/group/StreamingMediaContext.cpp
    ${tgcalls_loc}/group/StreamingMediaContext.h
    ${tgcalls_loc}/group/VideoStreamingPart.cpp
    ${tgcalls_loc}/group/VideoStreamingPart.h

    ${tgcalls_loc}/platform/PlatformInterface.h

    # third-party
    ${tgcalls_loc}/third-party/json11.cpp
    ${tgcalls_loc}/third-party/json11.hpp
)

# target_link_libraries(tgcalls
# PRIVATE
#     desktop-app::external_webrtc
#     desktop-app::external_ffmpeg
#     desktop-app::external_openssl
#     desktop-app::external_rnnoise
#     desktop-app::external_zlib
# )

# rnnoise
add_library(rnnoise STATIC)

execute_process(
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise_download_model.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise
    RESULT_VARIABLE DOWNLOAD_RESULT
)

if(NOT DOWNLOAD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to download model")
endif()

target_include_directories(rnnoise PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/include
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src
)

target_sources(rnnoise PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/denoise.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/rnn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/pitch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/kiss_fft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/celt_lpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/nnet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/nnet_default.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/parse_lpcnet_weights.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/rnnoise_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rnnoise/src/rnnoise_tables.c
)

target_link_libraries(tgcalls PRIVATE rnnoise)


target_compile_definitions(tgcalls PRIVATE WEBRTC_POSIX WEBRTC_LINUX)

# target_compile_definitions(lib_tgcalls
# PUBLIC
#     TGCALLS_USE_STD_OPTIONAL
# PRIVATE
#     WEBRTC_APP_TDESKTOP
#     RTC_ENABLE_H265
#     RTC_ENABLE_VP9
# )

target_compile_options(tgcalls PRIVATE
    -Wno-deprecated-declarations
    -Wno-unused-function
    -Wno-attributes
)
