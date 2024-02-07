#include "Pch.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <wil/result.h>
#include <wil/resource.h>

// Declarations
namespace Audio
{
#pragma pack(push)
#pragma pack(1)

    struct Guid // msut be 16 bytes
    {
        std::uint32_t Data1;
        std::uint16_t Data2;
        std::uint16_t Data3;
        std::uint8_t  Data4[8];
    };

    struct WaveFormat // msut be 14 bytes
    {
        std::uint16_t AudioFormat;
        std::uint16_t Channels;
        std::uint32_t SamplePerSecond;
        std::uint32_t BytesPerSecond;
        std::uint16_t BlockAlign;
    };

    struct PcmWaveFormat // msut be 16 bytes
    {
        std::uint16_t AudioFormat;
        std::uint16_t Channels;
        std::uint32_t SamplePerSecond;
        std::uint32_t BytesPerSecond;
        std::uint16_t BlockAlign;
        std::uint16_t BitsPerSample;
    };

    struct WaveFormatEx // msut be 18 bytes
    {
        std::uint16_t AudioFormat;
        std::uint16_t Channels;
        std::uint32_t SamplePerSecond;
        std::uint32_t BytesPerSecond;
        std::uint16_t BlockAlign;
        std::uint16_t BitsPerSample;
        std::uint16_t ExtendSize;
    };

    struct WaveFormatExtensible // msut be 40 bytes
    {
        WaveFormatEx Format;

        union
        {
            std::uint16_t ValidBitsPerSample;
            std::uint16_t SamplesPerBlock;
            std::uint16_t Reserved;
        }
        Samples;

        std::uint32_t ChannelMask;
        Guid SubFormat;
    };

    static_assert(sizeof(Guid) == 16, "Guid msut be 16 bytes");
    static_assert(sizeof(WaveFormat) == 14, "WaveFormat msut be 14 bytes");
    static_assert(sizeof(PcmWaveFormat) == 16, "PcmWaveFormat msut be 16 bytes");
    static_assert(sizeof(WaveFormatEx) == 18, "WaveFormatEx msut be 18 bytes");
    static_assert(sizeof(WaveFormatExtensible) == 40, "WaveFormatExtensible msut be 40 bytes");

#pragma pack(pop)
}

// Definitions
namespace Audio
{
    constexpr std::uint32_t MakeFourCC(std::uint8_t const code1, std::uint8_t const code2, std::uint8_t const code3, std::uint8_t const code4)
    {
        return ((static_cast<std::uint32_t>(code1)) | (static_cast<std::uint32_t>(code2) << 8) | (static_cast<std::uint32_t>(code3) << 16) | (static_cast<std::uint32_t>(code4) << 24));
    }
}

// Declarations
namespace Audio::Format::Riff
{
    namespace ChunkId
    {
        constexpr std::uint32_t Riff = MakeFourCC('R', 'I', 'F', 'F');
        constexpr std::uint32_t Junk = MakeFourCC('J', 'U', 'N', 'K');
        constexpr std::uint32_t Fmt  = MakeFourCC('f', 'm', 't', ' ');
        constexpr std::uint32_t Data = MakeFourCC('d', 'a', 't', 'a');
        constexpr std::uint32_t List = MakeFourCC('L', 'I', 'S', 'T');
        constexpr std::uint32_t Id3  = MakeFourCC('i', 'd', '3', ' ');
    }

    namespace FormType
    {
        constexpr std::uint32_t Wave = MakeFourCC('W', 'A', 'V', 'E');
        constexpr std::uint32_t Xwma = MakeFourCC('X', 'W', 'M', 'A');
        constexpr std::uint32_t Dpds = MakeFourCC('d', 'p', 'd', 's');
    }

    struct Chunk // msut be 8 bytes
    {
        std::uint32_t Id;
        std::uint32_t Size;
    };

    static_assert(sizeof(Chunk) == 8, "Chunk msut be 8 bytes");
}

std::string FourCCToString(std::uint32_t chunkId)
{
    std::stringstream buffer;
    buffer << static_cast<std::int8_t>(chunkId >> 0);
    buffer << static_cast<std::int8_t>(chunkId >> 8);
    buffer << static_cast<std::int8_t>(chunkId >> 16);
    buffer << static_cast<std::int8_t>(chunkId >> 24);
    return buffer.str();
}

void ExampleCallback(Audio::Format::Riff::Chunk const& chunk, std::uint32_t const formType)
{
    std::cout << "Chunk.Id       = " << FourCCToString(chunk.Id) << std::endl;
    std::cout << "Chunk.Size     = " << chunk.Size << std::endl;

    if (formType)
    {
        std::cout << "Chunk.FormType = " << FourCCToString(formType) << std::endl;
    }
}

int main()
{
    try
    {
        wil::unique_hfile file(::CreateFile("Assets/file_example_WAV_1MG.wav", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL));

        THROW_LAST_ERROR_IF(!file);

        DWORD numberOfBytesRead{};

        while (1)
        {
            Audio::Format::Riff::Chunk chunk{};
            std::uint32_t formType{};

            auto result = ::ReadFile(file.get(), &chunk, sizeof(Audio::Format::Riff::Chunk), &numberOfBytesRead, nullptr);

            if (!result)
            {
                THROW_LAST_ERROR();
            }

            if (numberOfBytesRead == 0)
            {
                break; // end of file
            }

            if (chunk.Id == Audio::Format::Riff::ChunkId::Riff || chunk.Id == Audio::Format::Riff::ChunkId::List)
            {
                auto result = ::ReadFile(file.get(), &formType, sizeof(std::uint32_t), &numberOfBytesRead, nullptr);

                if (!result)
                {
                    THROW_LAST_ERROR();
                }

                if (numberOfBytesRead == 0)
                {
                    break; // end of file
                }
            }
            else
            {
                if (::SetFilePointer(file.get(), chunk.Size, nullptr, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                {
                    THROW_LAST_ERROR();
                }
            }

            ExampleCallback(chunk, formType);
        }
    }
    catch (wil::ResultException& e)
    {
        std::cerr << e.what() << std::endl;
    }
    CATCH_LOG();

    return 0;
}
