#include <iostream>
#include <memory>
#include <vector>

#include <Windows.h>
#include <xaudio2.h>

#include <wil/result.h>
#include <wil/com.h>
#include <vorbis/vorbisfile.h>

namespace N503::Audio::Format::Wave
{
#pragma pack(push)
#pragma pack(1)

    struct WaveFormatEx // msut be 18 bytes
    {
        std::uint16_t AudioFormat;
        std::uint16_t Channels;
        std::uint32_t SamplePerSecond;
        std::uint32_t BytesPerSecond;
        std::uint16_t BlockAlign;
        std::uint16_t BitsPerSample;
        std::uint16_t ExtraFormatSize;
    };

#pragma pack(pop)
}

int main()
{
    std::locale::global(std::locale(".UTF8"));

    // COMライブラリを初期化し必要に応じてスレッドの新しいアパートメントを作成します。
    auto&& CoUninitializeReservedCall = wil::CoInitializeEx();

    OggVorbis_File file{};

    auto error = ::ov_fopen(R"(G:\Develop\Assets\Audio\Ogg\sample-file-1.ogg)", &file);

    switch (error)
    {
        case OV_EREAD:
        case OV_ENOTVORBIS:
        case OV_EVERSION:
        case OV_EBADHEADER:
        case OV_EFAULT:
        {
            return 0;
        }
    }

    wil::com_ptr<IXAudio2> xaudio2;
    IXAudio2MasteringVoice* pMasteringVoice{};
    IXAudio2SourceVoice*    pSourceVoice{};

    try
    {
        auto pVorbisInfo = ::ov_info(&file, 0);

        if (!pVorbisInfo)
        {
            throw std::runtime_error("vorbis_info failed.");
        }

        N503::Audio::Format::Wave::WaveFormatEx waveFormat{};
        waveFormat.AudioFormat     = 1; // WAVE_FORMAT_PCM
        waveFormat.BitsPerSample   = 16;
        waveFormat.Channels        = pVorbisInfo->channels;
        waveFormat.SamplePerSecond = pVorbisInfo->rate;
        waveFormat.BlockAlign      = waveFormat.BitsPerSample / 8 * waveFormat.Channels;
        waveFormat.BytesPerSecond  = waveFormat.SamplePerSecond * waveFormat.BlockAlign;
        waveFormat.ExtraFormatSize = 0;

        std::cout << "WaveFormat.AudioFormat     = " << waveFormat.AudioFormat     << std::endl;
        std::cout << "WaveFormat.BitsPerSample   = " << waveFormat.BitsPerSample   << std::endl;
        std::cout << "WaveFormat.Channels        = " << waveFormat.Channels        << std::endl;
        std::cout << "WaveFormat.SamplePerSecond = " << waveFormat.SamplePerSecond << std::endl;
        std::cout << "WaveFormat.BlockAlign      = " << waveFormat.BlockAlign      << std::endl;
        std::cout << "WaveFormat.BytesPerSecond  = " << waveFormat.BytesPerSecond  << std::endl;
        std::cout << "WaveFormat.ExtraFormatSize = " << waveFormat.ExtraFormatSize << std::endl;

        std::cout << "ov_raw_total  = " << ::ov_raw_total(&file, -1)  << " bytes." << std::endl;
        std::cout << "ov_pcm_total  = " << ::ov_pcm_total(&file, -1)  << " bytes." << std::endl;
        std::cout << "ov_time_total = " << ::ov_time_total(&file, -1) << " seconds." << std::endl;

        THROW_IF_FAILED(::XAudio2Create(xaudio2.put()));
        THROW_IF_FAILED(xaudio2->CreateMasteringVoice(&pMasteringVoice));
        THROW_IF_FAILED(xaudio2->CreateSourceVoice(&pSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&waveFormat)));
        THROW_IF_FAILED(pSourceVoice->Start());

        XAUDIO2_BUFFER      xaudio2Buffer{};
        XAUDIO2_VOICE_STATE xaudio2VoiceState{};

        int bitStream{};

        std::vector<std::unique_ptr<char[]>> buffers;
        buffers.emplace_back(std::make_unique<char[]>(4096));
        buffers.emplace_back(std::make_unique<char[]>(4096));
        buffers.emplace_back(std::make_unique<char[]>(4096));
        buffers.emplace_back(std::make_unique<char[]>(4096));
        buffers.emplace_back(std::make_unique<char[]>(4096));

        int buffersIndex = 0;

        while (true)
        {
            if (pSourceVoice->GetState(&xaudio2VoiceState), xaudio2VoiceState.BuffersQueued >= buffers.size())
            {
                ::Sleep(1);
                continue;
            }

            auto result = ::ov_read(&file, buffers.at(buffersIndex).get(), 4096, 0, 2, 1, &bitStream);

            if (result == 0)
            {
                std::cout << "End of stream. (length = " << result << ")" << std::endl;
                break;
            }
            else if (result == OV_HOLE)
            {
                std::cerr << "OV_HOLEはデータに中断があった。(ページ間のゴミ、同期が失われた後の再キャプチャ、破損したページのいずれか)" << std::endl;
                continue;
            }
            else if (result == OV_EBADLINK)
            {
                throw std::runtime_error("libvorbisfile に無効なストリームセクションが与えられたか、 要求されたリンクが壊れている。");
            }
            else if (result == OV_EINVAL)
            {
                throw std::runtime_error("最初のファイルヘッダが読み取れなかったか、壊れているか、ov_fopenの最初のオープンコールに失敗した。");
            }
            else if (result < 0)
            {
                throw std::runtime_error("unknown error.");
            }

            xaudio2Buffer.Flags      = 0;
            xaudio2Buffer.AudioBytes = result;
            xaudio2Buffer.pAudioData = reinterpret_cast<BYTE const*>(buffers.at(buffersIndex).get());

            THROW_IF_FAILED(pSourceVoice->SubmitSourceBuffer(&xaudio2Buffer));

            buffersIndex = (++buffersIndex) % buffers.size();
        }

        while (pSourceVoice->GetState(&xaudio2VoiceState), xaudio2VoiceState.BuffersQueued > 0)
        {
            ::Sleep(1);
        }
    }
    catch (std::exception const& exception)
    {
        std::cerr << exception.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown error." << std::endl;
    }

    if (pSourceVoice)
    {
        pSourceVoice->Stop();
        pSourceVoice->FlushSourceBuffers();
        pSourceVoice->DestroyVoice();
    }

    if (pMasteringVoice)
    {
        pMasteringVoice->DestroyVoice();
    }

    ::ov_clear(&file);

    return 0;
}
