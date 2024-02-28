#include <iostream>

#include <vorbis/vorbisfile.h>

int main()
{
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

    try
    {
        auto pVorbisInfo = ::ov_info(&file, 0);

        if (!pVorbisInfo)
        {
            throw std::runtime_error("vorbis_info failed.");
        }

        std::cout << "vorbis_info.version         = " << pVorbisInfo->version         << std::endl; // Vorbisのエンコーダバージョンです。再生にはあまり関係がありません。
        std::cout << "vorbis_info.channels        = " << pVorbisInfo->channels        << std::endl; // ビットストリームのチャンネル数です。モノラルなら1、ステレオなら2などです。
        std::cout << "vorbis_info.rate            = " << pVorbisInfo->rate            << std::endl; // ンプリングレートです。44100のようなサンプリングレート数が入ります。
        std::cout << "vorbis_info.bitrate_upper   = " << pVorbisInfo->bitrate_upper   << std::endl; // 可変長ビットレート（VBR : Variable Bit Rate）の情報です。
        std::cout << "vorbis_info.bitrate_nominal = " << pVorbisInfo->bitrate_nominal << std::endl; // 可変長ビットレート（VBR : Variable Bit Rate）の情報です。
        std::cout << "vorbis_info.bitrate_lower   = " << pVorbisInfo->bitrate_lower   << std::endl; // 可変長ビットレート（VBR : Variable Bit Rate）の情報です。
        std::cout << "vorbis_info.bitrate_window  = " << pVorbisInfo->bitrate_window  << std::endl; // 現在使用されていません。
        std::cout << "vorbis_info.codec_setup     = " << reinterpret_cast<std::uint64_t>(pVorbisInfo->codec_setup) << std::endl; // コーデックに必要な情報へのポインタが返ります。これも再生には必要ありません。
    }
    catch (std::exception const& exception)
    {
        std::cerr << exception.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown error." << std::endl;
    }

    ::ov_clear(&file);

    return 0;
}
