#include "epch.h"
#include "WaveLoader.h"

#include <fstream>

namespace Cosmos::sound
{
    WaveLoader::WaveLoader()
    {
        mBitDepth = 16;
        mSampleRate = 44100;
        mSamples.resize(1);
        mSamples[0].resize(0);
    }

    bool WaveLoader::Load(std::string filePath)
    {
        std::ifstream file(filePath, std::ios::binary);

        // check the file exists
        if (!file.good())
        {
            LOG_TO_TERMINAL(Logger::Error, "Could not open file %s", filePath);
            return false;
        }

        file.unsetf(std::ios::skipws);
        std::istream_iterator<uint8_t> begin(file), end;
        std::vector<uint8_t> fileData(begin, end);

        return DecodeWaveFile(fileData);
    }

    bool WaveLoader::WritePCMToBuffer(std::vector<uint8_t>& out)
    {
        out.clear();

        int16_t audioFormat = mBitDepth == 32 ? WavAudioFormat::IEEEFloat : WavAudioFormat::PCM;

        for (uint32_t i = 0; i < GetSamplesPerChannel(); i++)
        {
            for (uint32_t channel = 0; channel < GetChannels(); channel++)
            {
                switch (mBitDepth)
                {
                case 8:
                {
                    uint8_t byte = SampleToSingleByte(mSamples[channel][i]);
                    out.push_back(byte);

                    break;
                }

                case 16:
                {
                    int16_t sampleAsInt = SampleToSixteenBitInt(mSamples[channel][i]);
                    AddInt16ToFileData(out, sampleAsInt);

                    break;
                }

                case 24:
                {
                    int32_t sampleAsIntAgain = (int32_t)(mSamples[channel][i] * (float)8388608.0f);

                    uint8_t bytes[3] = {};
                    bytes[2] = (uint8_t)(sampleAsIntAgain >> 16) & 0xFF;
                    bytes[1] = (uint8_t)(sampleAsIntAgain >> 8) & 0xFF;
                    bytes[0] = (uint8_t)sampleAsIntAgain & 0xFF;

                    out.push_back(bytes[0]);
                    out.push_back(bytes[1]);
                    out.push_back(bytes[2]);

                    break;
                }

                case 32:
                {
                    int32_t sampleAsInt;

                    if (audioFormat == WavAudioFormat::IEEEFloat)
                        sampleAsInt = (int32_t) reinterpret_cast<int32_t&> (mSamples[channel][i]);

                    else // assume PCM
                        sampleAsInt = (int32_t)(mSamples[channel][i] * std::numeric_limits<int32_t>::max());

                    AddInt32ToFileData(out, sampleAsInt, Endianness::LittleEndian);

                    break;
                }

                default:
                {
                    LOG_TO_TERMINAL(Logger::Error, "Error: Trying to write a file with unsupported bit depth");
                    return false;
                }
                }
            }
        }

        return true;
    }

    bool WaveLoader::DecodeWaveFile(std::vector<uint8_t>& fileData)
    {
        // header chunk
        std::string headerChunkID(fileData.begin(), fileData.begin() + 4);
        std::string format(fileData.begin() + 8, fileData.begin() + 12);

        // try and find the start points of key chunks
        int32_t indexOfDataChunk = GetIndexOfChunk(fileData, "data", 12);
        int32_t indexOfFormatChunk = GetIndexOfChunk(fileData, "fmt ", 12);
        int32_t indexOfXMLChunk = GetIndexOfChunk(fileData, "iXML", 12);

        // if we can't find the data or format chunks, or the IDs/formats don't seem to be as expected, abort
        if (indexOfDataChunk == -1 || indexOfFormatChunk == -1 || headerChunkID != "RIFF" || format != "WAVE")
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: This .wav file doesn't follow the standart Wave Format");
            return false;
        }

        // format chunk
        int32_t f = indexOfFormatChunk;
        std::string formatChunkID(fileData.begin() + f, fileData.begin() + f + 4);
        int16_t audioFormat = TwoBytesToInt(fileData, f + 8);
        int16_t numChannels = TwoBytesToInt(fileData, f + 10);
        mSampleRate = (uint32_t)FourBytesToInt(fileData, f + 12);
        int32_t numBytesPerSecond = FourBytesToInt(fileData, f + 16);
        int16_t numBytesPerBlock = TwoBytesToInt(fileData, f + 20);
        mBitDepth = (int32_t)TwoBytesToInt(fileData, f + 22);
        int32_t numBytesPerSample = mBitDepth / 8;

        // check that the audio format is PCM or Float
        if (audioFormat != WavAudioFormat::PCM && audioFormat != WavAudioFormat::IEEEFloat)
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: This wave file is encoded in a unsupported format");
            return false;
        }

        // check the number of channels is mono or stereo
        if (numChannels < 1 || numChannels > 128)
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: This wave file is doesn't have a valid number of channels");
            return false;
        }

        // check header data is consistent
        if ((numBytesPerSecond != (numChannels * mSampleRate * mBitDepth) / 8) || (numBytesPerBlock != (numChannels * numBytesPerSample)))
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: This wave file has invalid header data");
            return false;
        }

        // check bit depth is either 8, 16, 24 or 32 bit
        if (mBitDepth != 8 && mBitDepth != 16 && mBitDepth != 24 && mBitDepth != 32)
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: This wave file has an invalid bit depth");
            return false;
        }

        // data chunk
        int32_t d = indexOfDataChunk;
        std::string dataChunkID(fileData.begin() + d, fileData.begin() + d + 4);
        int32_t dataChunkSize = FourBytesToInt(fileData, d + 4);

        int32_t numSamples = dataChunkSize / (numChannels * mBitDepth / 8);
        int32_t samplesStartIndex = indexOfDataChunk + 8;

        ClearAudioBuffer();
        mSamples.resize(numChannels);

        for (int32_t i = 0; i < numSamples; i++)
        {
            for (int32_t channel = 0; channel < numChannels; channel++)
            {
                int32_t sampleIndex = samplesStartIndex + (numBytesPerBlock * i) + channel * numBytesPerSample;

                switch (mBitDepth)
                {
                case 8:
                {
                    float sample = SingleByteToSample(fileData[sampleIndex]);
                    mSamples[channel].push_back(sample);

                    break;
                }

                case 16:
                {
                    int16_t sampleAsInt = TwoBytesToInt(fileData, sampleIndex);
                    float sample = SixteenBitIntToSample(sampleAsInt);
                    mSamples[channel].push_back(sample);

                    break;
                }

                case 24:
                {
                    int32_t sampleAsInt = 0;
                    sampleAsInt = (fileData[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(sampleIndex) + 2] << 16)
                        | (fileData[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(sampleIndex) + 1] << 8)
                        | fileData[sampleIndex];

                    // if the 24th bit is set, this is a negative number in 24 - bit world, so make sure sign is extended to the 32 bit float
                    if (sampleAsInt & 0x800000)
                        sampleAsInt = sampleAsInt | ~0xFFFFFF;

                    float sample = (float)sampleAsInt / (float)8388608.0f;
                    mSamples[channel].push_back(sample);

                    break;
                }

                case 32:
                {
                    int32_t sampleAsInt = FourBytesToInt(fileData, sampleIndex);
                    float sample;

                    if (audioFormat == WavAudioFormat::IEEEFloat)
                        sample = (float)reinterpret_cast<float&> (sampleAsInt);

                    else // assume PCM
                        sample = (float)sampleAsInt / static_cast<float> (std::numeric_limits<std::int32_t>::max());

                    mSamples[channel].push_back(sample);

                    break;
                }
                }
            }
        }

        // IXML CHUNK
        if (indexOfXMLChunk != -1)
        {
            int32_t chunkSize = FourBytesToInt(fileData, indexOfXMLChunk + 4);
            mIXMLChunk = std::string((const char*)&fileData[indexOfXMLChunk + 8], chunkSize);
        }

        return true;
    }

    void WaveLoader::ClearAudioBuffer()
    {
        for (size_t i = 0; i < mSamples.size(); i++)
        {
            mSamples[i].clear();
        }

        mSamples.clear();
    }

    int32_t WaveLoader::GetIndexOfString(std::vector<uint8_t>& source, std::string s)
    {
        int32_t index = -1;
        int32_t stringLength = (int32_t)s.length();

        for (size_t i = 0; i < source.size() - stringLength; i++)
        {
            std::string section(source.begin() + i, source.begin() + i + stringLength);

            if (section == s)
            {
                index = static_cast<int32_t> (i);
                break;
            }
        }

        return index;
    }

    int32_t WaveLoader::GetIndexOfChunk(std::vector<uint8_t>& source, const std::string& chunkHeaderID, int32_t startIndex, Endianness endianness)
    {
        constexpr size_t dataLen = 4;
        if (chunkHeaderID.size() != dataLen || startIndex < 0)
        {
            LOG_TO_TERMINAL(Logger::Error, "Error: Invalid chunk header ID");
            return -1;
        }

        size_t i = size_t(startIndex);
        while (i < source.size() - dataLen)
        {
            if (memcmp(&source[i], chunkHeaderID.data(), dataLen) == 0)
            {
                return (int32_t)i;
            }

            i += dataLen;
            auto chunkSize = FourBytesToInt(source, (int32_t)i, endianness);
            i += (dataLen + chunkSize);
        }

        return -1;
    }

    float WaveLoader::Clamp(float value, float minValue, float maxValue)
    {
        value = std::min(value, maxValue);
        value = std::max(value, minValue);
        return value;
    }

    int16_t WaveLoader::TwoBytesToInt(std::vector<uint8_t>& source, int32_t startIndex, Endianness endianness)
    {
        int16_t result;

        if (endianness == Endianness::LittleEndian)
            result = (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 1] << 8)
            | source[startIndex];

        else
            result = (source[startIndex] << 8)
            | source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 1];

        return result;
    }

    int32_t WaveLoader::FourBytesToInt(std::vector<uint8_t>& source, int32_t startIndex, Endianness endianness)
    {
        int32_t result;

        if (endianness == Endianness::LittleEndian)
            result = (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 3] << 24)
            | (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 2] << 16)
            | (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 1] << 8)
            | source[startIndex];

        else
            result = (source[startIndex] << 24)
            | (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 1] << 16)
            | (source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 2] << 8)
            | source[static_cast<std::vector<uint8_t, std::allocator<uint8_t>>::size_type>(startIndex) + 3];

        return result;
    }

    void WaveLoader::AddStringToFileData(std::vector<uint8_t>& fileData, std::string s)
    {
        for (size_t i = 0; i < s.length(); i++)
            fileData.push_back((uint8_t)s[i]);
    }

    void WaveLoader::AddInt16ToFileData(std::vector<uint8_t>& fileData, int16_t i, Endianness endianness)
    {
        uint8_t bytes[2] = {};

        if (endianness == Endianness::LittleEndian)
        {
            bytes[1] = (i >> 8) & 0xFF;
            bytes[0] = i & 0xFF;
        }

        else
        {
            bytes[0] = (i >> 8) & 0xFF;
            bytes[1] = i & 0xFF;
        }

        fileData.push_back(bytes[0]);
        fileData.push_back(bytes[1]);
    }

    void WaveLoader::AddInt32ToFileData(std::vector<uint8_t>& fileData, int32_t i, Endianness endianness)
    {
        uint8_t bytes[4] = {};

        if (endianness == Endianness::LittleEndian)
        {
            bytes[3] = (i >> 24) & 0xFF;
            bytes[2] = (i >> 16) & 0xFF;
            bytes[1] = (i >> 8) & 0xFF;
            bytes[0] = i & 0xFF;
        }

        else
        {
            bytes[0] = (i >> 24) & 0xFF;
            bytes[1] = (i >> 16) & 0xFF;
            bytes[2] = (i >> 8) & 0xFF;
            bytes[3] = i & 0xFF;
        }

        for (int32_t i = 0; i < 4; i++)
            fileData.push_back(bytes[i]);
    }

    float WaveLoader::SixteenBitIntToSample(int16_t sample)
    {
        return static_cast<float>(sample) / static_cast<float>(32768.0f);
    }

    float WaveLoader::SingleByteToSample(uint8_t sample)
    {
        return static_cast<float>(sample - 128.0f) / static_cast<float>(128.0f);
    }

    int16_t WaveLoader::SampleToSixteenBitInt(float sample)
    {
        sample = Clamp(sample, -1.0f, 1.0f);
        return static_cast<int16_t> (sample * 32767.0f);
    }

    uint8_t WaveLoader::SampleToSingleByte(float sample)
    {
        sample = Clamp(sample, -1.0f, 1.0f);
        sample = (sample + 1.0f) / 2.0f;
        return static_cast<uint8_t> (sample * 255.0f);
    }
}