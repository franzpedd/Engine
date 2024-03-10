#pragma once

#include <vector>
#include <string>

namespace Cosmos::sound
{
    class WaveLoader
    {
    public:

        // wave audio formats the class supports
        enum WavAudioFormat
        {
            PCM = 0x0001,
            IEEEFloat = 0x0002
        };

        // used to determine on how to write data
        enum class Endianness
        {
            LittleEndian,
            BigEndian
        };

    public:

        // constructor
        WaveLoader();

        // destructor
        ~WaveLoader() = default;

    public:

        // returns the sample rate
        inline uint32_t GetSampleRate() const { return mSampleRate; }

        // returns the number of audio channels in the buffer
        inline uint32_t GetChannels() const { return (uint32_t)mSamples.size(); }

        // returns true if the audio file is mono
        inline bool IsMono() const { return GetChannels() == 1; }

        // returns true if the audio file is stereo
        inline bool IsStereo() const { return GetChannels() == 2; }

        // returns the bit depth of each sample
        inline uint32_t GetBitDepth() const { return mBitDepth; }

        // returns the number of samples per channel
        inline uint32_t GetSamplesPerChannel() const { return mSamples.size() > 0 ? (uint32_t)mSamples[0].size() : 0; }

        // returns the length in seconds of the audio file based on the number of samples and sample rate 
        inline double GetLengthInSeconds() const { return (double)GetSamplesPerChannel() / (double)mSampleRate; }

    public:

        // loads a file, returns true on success
        bool Load(std::string filePath);

        // writes the pcm data into an output buffer
        bool WritePCMToBuffer(std::vector<uint8_t>& out);

    private:

        // decodes a wave audio
        bool DecodeWaveFile(std::vector<uint8_t>& fileData);

        // erases the audio buffer content
        void ClearAudioBuffer();

        // returns the index of a string
        int32_t GetIndexOfString(std::vector<uint8_t>& source, std::string s);

        // returns the index of a chunk of data
        int32_t GetIndexOfChunk(std::vector<uint8_t>& source, const std::string& chunkHeaderID, int32_t startIndex, Endianness endianness = Endianness::LittleEndian);

        // restricts a value
        float Clamp(float value, float minValue, float maxValue);

    public:

        // converts two bytes to an integer
        int16_t TwoBytesToInt(std::vector<uint8_t>& source, int32_t startIndex, Endianness endianness = Endianness::LittleEndian);

        // converts four bytes into an integer
        int32_t FourBytesToInt(std::vector<uint8_t>& source, int32_t startIndex, Endianness endianness = Endianness::LittleEndian);

        // writes a string into a vector of data
        void AddStringToFileData(std::vector<uint8_t>& fileData, std::string s);

        // writes a 16bits integer into a veector of data
        void AddInt16ToFileData(std::vector<uint8_t>& fileData, int16_t i, Endianness endianness = Endianness::LittleEndian);

        // writes a 32bits integer into a vector of data
        void AddInt32ToFileData(std::vector<uint8_t>& fileData, int32_t i, Endianness endianness = Endianness::LittleEndian);

        // converts a 16bits sample into a sample (float)
        float SixteenBitIntToSample(int16_t sample);

        // converts a 8bits sample into a sample (float)
        float SingleByteToSample(uint8_t sample);

        // converts the sample(float) into a 16bits integer
        int16_t SampleToSixteenBitInt(float sample);

        // converts the sample(float) into a 8bits integer
        uint8_t SampleToSingleByte(float sample);

    private:

        std::vector<std::vector<float>> mSamples;
        std::string mIXMLChunk;
        uint32_t mSampleRate;
        int32_t mBitDepth;
    };
}