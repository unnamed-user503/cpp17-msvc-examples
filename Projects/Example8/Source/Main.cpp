#include <iostream>
#include <vector>

#include <Windows.h>
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <wil/result.h>
#include <wil/resource.h>
#include <wil/com.h>

namespace N503::XAudio2
{
    struct VoiceDeleter
    {
        void operator()(IXAudio2Voice* pVoice) const
        {
            if (pVoice)
            {
                pVoice->DestroyVoice();
            }
        };
    };
}

namespace N503::MediaFoundation
{
    //! A type that calls CoUninitialize on destruction (or reset()).
    using unique_mfshutdown_call = wil::unique_call<decltype(&::MFShutdown), ::MFShutdown>;

    inline unique_mfshutdown_call MFStartup(DWORD flags = 0)
    {
        THROW_IF_FAILED(::MFStartup(MF_VERSION, flags));
        return {};
    }
}

int main()
{
    // COMライブラリを初期化し必要に応じてスレッドの新しいアパートメントを作成します。
    auto&& CoUninitializeReservedCall = wil::CoInitializeEx();

    // MediaEngineの初期化
    auto&& MFShutdownReservedCall = N503::MediaFoundation::MFStartup(MFSTARTUP_LITE);

    try
    {
        wil::com_ptr<IMFSourceReader> mfSourceReader;
        THROW_IF_FAILED(::MFCreateSourceReaderFromURL(LR"(G:\Develop\Assets\Audio\file_example_WAV_1MG.wav)", nullptr, mfSourceReader.put()));

        // 最初のオーディオストリームを選択し、他のすべてのストリームの選択を解除します。
        THROW_IF_FAILED(mfSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE));
        THROW_IF_FAILED(mfSourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));

        // 非圧縮PCMオーディオを指定する部分メディアタイプを作成する。
        {
            wil::com_ptr<IMFMediaType> mfMediaType;
            THROW_IF_FAILED(::MFCreateMediaType(mfMediaType.put()));
            THROW_IF_FAILED(mfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
            THROW_IF_FAILED(mfMediaType->SetGUID(MF_MT_SUBTYPE   , MFAudioFormat_PCM));

            // ソース・リーダーにこのタイプを設定する。ソースリーダは必要なデコーダをロードする。
            THROW_IF_FAILED(mfSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mfMediaType.get()));
        }

        // 完全な非圧縮フォーマットを取得する。
        wil::com_ptr<IMFMediaType> mfMediaType;
        THROW_IF_FAILED(mfSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, mfMediaType.put()));

        // ストリームが選択されていることを確認する。
        THROW_IF_FAILED(mfSourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));

        // PCMオーディオフォーマットをWAVEFORMATEX構造に変換する。
        WAVEFORMATEX* pWaveFormat{};
        std::uint32_t waveFormatSize{};
        THROW_IF_FAILED(::MFCreateWaveFormatExFromMFMediaType(mfMediaType.get(), &pWaveFormat, &waveFormatSize, MFWaveFormatExConvertFlag_Normal));

        // 完全にするならwaveFormatSizeのサイズを確認し、WAVEFORMATEXかWAVEFORMATEXTENSIBLEなのかを判断する
        WAVEFORMATEX waveFormat{};
        std::memcpy(&waveFormat, pWaveFormat, sizeof(WAVEFORMATEX));
        ::CoTaskMemFree(pWaveFormat);

        // CalculateMaxAudioDataSize
        /*
        {
            std::uint32_t audioBlockSize = 0; // オーディオフレームサイズ(バイト単位)
            std::uint32_t bytesPerSecond = 0; // 1秒あたりのバイト数

            // オーディオ・ブロック・サイズとバイト数/秒をオーディオ・フォーマットから取得する。
            audioBlockSize = ::MFGetAttributeUINT32(mfMediaType.get(), MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
            bytesPerSecond = ::MFGetAttributeUINT32(mfMediaType.get(), MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);

            std::cout << "WaveFormat.nBlockAlign     = " << waveFormat.nBlockAlign     << ", audioBlockSize=" << audioBlockSize << std::endl;
            std::cout << "WaveFormat.nAvgBytesPerSec = " << waveFormat.nAvgBytesPerSec << ", bytesPerSecond=" << bytesPerSecond << std::endl;
            std::cout << "WaveFormat.cbSize          = " << waveFormat.cbSize << std::endl;
        }
        */
        wil::com_ptr<IXAudio2> xaudio2;
        THROW_IF_FAILED(::XAudio2Create(xaudio2.put()));

        IXAudio2MasteringVoice* pMasteringVoice{};
        THROW_IF_FAILED(xaudio2->CreateMasteringVoice(&pMasteringVoice));
        std::unique_ptr<IXAudio2MasteringVoice, N503::XAudio2::VoiceDeleter> masteringVoice(pMasteringVoice);

        IXAudio2SourceVoice* pSourceVoice{};
        THROW_IF_FAILED(xaudio2->CreateSourceVoice(&pSourceVoice, &waveFormat));
        std::unique_ptr<IXAudio2SourceVoice, N503::XAudio2::VoiceDeleter> sourceVoice(pSourceVoice);

        // Audio Decode
        wil::com_ptr<IMFSample>      mfSample;
        wil::com_ptr<IMFMediaBuffer> mfMediaBuffer;

        while (true)
        {
            DWORD streamFlags{};

            THROW_IF_FAILED(mfSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &streamFlags, nullptr, mfSample.put()));

            if (streamFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
            {
                std::cerr << "Type change - not supported by WAVE file format." << std::endl;
                break;
            }

            if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                std::cout << "End of input file" << std::endl;
                break;
            }

            if (!mfSample)
            {
                std::cerr << "No sample." << std::endl;
                continue;
            }

            // サンプル内のオーディオデータへのポインタを取得します。
            THROW_IF_FAILED(mfSample->ConvertToContiguousBuffer(mfMediaBuffer.put()));

            BYTE* pAudioData{};
            DWORD bufferCurrentLength{};

            std::vector<std::uint8_t> audioBuffer;

            THROW_IF_FAILED(mfMediaBuffer->Lock(&pAudioData, nullptr, &bufferCurrentLength));

            std::cout << "Buffer.CurrentLength = " << bufferCurrentLength << std::endl;
            audioBuffer.resize(bufferCurrentLength);
            std::memcpy(&audioBuffer[0], pAudioData, bufferCurrentLength);

            THROW_IF_FAILED(mfMediaBuffer->Unlock());

            XAUDIO2_BUFFER xaudio2Buffer{};
            xaudio2Buffer.Flags = 0;
            xaudio2Buffer.AudioBytes = bufferCurrentLength;
            xaudio2Buffer.pAudioData = &audioBuffer[0];

            sourceVoice->SubmitSourceBuffer(&xaudio2Buffer);
            sourceVoice->Start();

            XAUDIO2_VOICE_STATE xaudio2VoiceState{};
            while (sourceVoice->GetState(&xaudio2VoiceState), xaudio2VoiceState.BuffersQueued > 0)
            {
            };
        }

        XAUDIO2_VOICE_STATE xaudio2VoiceState{};
        while (sourceVoice->GetState(&xaudio2VoiceState), xaudio2VoiceState.BuffersQueued > 0)
        {
        };
    }
    CATCH_LOG();

    return 0;
}
