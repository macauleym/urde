#include "CStreamAudioManager.hpp"
#include "CStringExtras.hpp"
#include "CDvdFile.hpp"
#include "CDvdRequest.hpp"
#include "Audio/CAudioSys.hpp"
#include "amuse/DSPCodec.h"
#include <memory>

namespace urde
{
class CDSPStreamManager;

static u32 s_HandleCounter = 0;
static u32 s_HandleCounter2 = 0;

/* Standard DSPADPCM header */
struct dspadpcm_header
{
    u32 x0_num_samples;
    u32 x4_num_nibbles;
    u32 x8_sample_rate;
    u16 xc_loop_flag;
    u16 xe_format; /* 0 for ADPCM */
    u32 x10_loop_start_nibble;
    u32 x14_loop_end_nibble;
    u32 x18_ca;
    s16 x1c_coef[8][2];
    s16 x3c_gain;
    s16 x3e_ps;
    s16 x40_hist1;
    s16 x42_hist2;
    s16 x44_loop_ps;
    s16 x46_loop_hist1;
    s16 x48_loop_hist2;
    u16 x4a_pad[11];
};

struct SDSPStreamInfo
{
    const char* x0_fileName;
    u32 x4_sampleRate;
    u32 x8_headerSize = sizeof(dspadpcm_header);
    u32 xc_adpcmBytes;
    u32 x10_loopFlag;
    u32 x14_loopStartByte;
    u32 x18_loopEndByte;
    s16 x1c_coef[8][2];

    SDSPStreamInfo() = default;
    SDSPStreamInfo(const CDSPStreamManager& stream);
};

struct SDSPStream : boo::IAudioVoiceCallback
{
    bool x0_active;
    bool x1_oneshot;
    u32 x4_ownerId;
    SDSPStream* x8_stereoLeft;
    SDSPStream* xc_companionRight;
    SDSPStreamInfo x10_info;
    float x4c_vol;
    float m_leftgain, m_rightgain;
    //DVDFileInfo x50_dvdHandle1;
    //DVDFileInfo x8c_dvdHandle2;
    //u32 xc8_streamId = -1; // MusyX stream handle
    u32 xcc_fileCur = 0;
    std::unique_ptr<u8[]> xd4_ringBuffer;
    u32 xd8_ringBytes = 0x11DC0; // 73152 4sec in ADPCM bytes
    u32 xdc_ringSamples = 0x1f410; // 128016 4sec in samples
    s8 xe0_curBuffer = -1;
    u32 xe8_silent = true;
    u8 xec_readState = 0; // 0: NoRead 1: Read 2: ReadWrap

    std::experimental::optional<CDvdFile> m_file;
    std::shared_ptr<IDvdRequest> m_readReqs[2];

    void ReadBuffer(int buf)
    {        
        u32 halfSize = xd8_ringBytes / 2;
        u8* data = xd4_ringBuffer.get() + (buf ? halfSize : 0);

        if (x10_info.x10_loopFlag)
        {
            u32 remFileBytes = x10_info.x18_loopEndByte - xcc_fileCur;

            if (remFileBytes < halfSize)
            {
                //printf("Buffering %d from %d into %d\n", remFileBytes, xcc_fileCur + x10_info.x8_headerSize, buf);
                m_file->AsyncSeekRead(data, remFileBytes,
                                      ESeekOrigin::Begin, xcc_fileCur + x10_info.x8_headerSize);
                xcc_fileCur = x10_info.x14_loopStartByte;
                u32 remBytes = halfSize - remFileBytes;
                //printf("Loop Buffering %d from %d into %d\n", remBytes, xcc_fileCur + x10_info.x8_headerSize, buf);
                m_readReqs[buf] = m_file->AsyncSeekRead(data + remFileBytes, remBytes,
                                  ESeekOrigin::Begin, xcc_fileCur + x10_info.x8_headerSize);
                xcc_fileCur += remBytes;
            }
            else
            {
                //printf("Buffering %d from %d into %d\n", halfSize, xcc_fileCur + x10_info.x8_headerSize, buf);
                m_readReqs[buf] = m_file->AsyncSeekRead(data, halfSize,
                                  ESeekOrigin::Begin, xcc_fileCur + x10_info.x8_headerSize);
                xcc_fileCur += halfSize;
            }
        }
        else
        {
            if (xcc_fileCur == x10_info.xc_adpcmBytes)
            {
                memset(data, 0, halfSize);
                return;
            }

            u32 remFileBytes = x10_info.xc_adpcmBytes - xcc_fileCur;

            if (remFileBytes < halfSize)
            {
                //printf("Buffering %d from %d into %d\n", remFileBytes, xcc_fileCur + x10_info.x8_headerSize, buf);
                m_readReqs[buf] = m_file->AsyncSeekRead(data, remFileBytes,
                                  ESeekOrigin::Begin, xcc_fileCur + x10_info.x8_headerSize);
                memset(data + remFileBytes, 0, halfSize - remFileBytes);
                xcc_fileCur = x10_info.xc_adpcmBytes;
            }
            else
            {
                //printf("Buffering %d from %d into %d\n", halfSize, xcc_fileCur + x10_info.x8_headerSize, buf);
                m_readReqs[buf] = m_file->AsyncSeekRead(data, halfSize,
                                  ESeekOrigin::Begin, xcc_fileCur + x10_info.x8_headerSize);
                xcc_fileCur += halfSize;
            }
        }
    }

    bool BufferStream()
    {
        if (xec_readState == 0)
        {
            ReadBuffer(0);
            ReadBuffer(1);
            xec_readState = 1;
            return false;
        }
        else if (xec_readState == 1)
        {
            if (m_readReqs[0]->IsComplete())
            {
                xe0_curBuffer = 0;
                xec_readState = 2;
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (xec_readState == 2)
        {
            if (xe0_curBuffer == 1 && m_readReqs[0]->IsComplete())
            {
                xe0_curBuffer = 0;
                ReadBuffer(1);
                return true;
            }
            else if (xe0_curBuffer == 0 && m_readReqs[1]->IsComplete())
            {
                xe0_curBuffer = 1;
                ReadBuffer(0);
                return true;
            }
        }
        return false;
    }

    u32 m_curSample = 0;
    u32 m_totalSamples = 0;
    s16 m_prev1 = 0;
    s16 m_prev2 = 0;

    void preSupplyAudio(boo::IAudioVoice&, double) {}

    void decompressChunk(u32 readToSample, int16_t*& data)
    {
        auto sampDiv = std::div(m_curSample, 14);
        if (sampDiv.rem)
        {
            unsigned samps = DSPDecompressFrameRanged(data, xd4_ringBuffer.get() + sampDiv.quot * 8,
                                                      x10_info.x1c_coef, &m_prev1, &m_prev2, sampDiv.rem,
                                                      readToSample - m_curSample);
            m_curSample += samps;
            data += samps;
            ++sampDiv.quot;
        }

        while (m_curSample < readToSample)
        {
            unsigned samps = DSPDecompressFrame(data, xd4_ringBuffer.get() + sampDiv.quot * 8,
                                                x10_info.x1c_coef, &m_prev1, &m_prev2,
                                                readToSample - m_curSample);
            m_curSample += samps;
            data += samps;
            ++sampDiv.quot;
        }
    }

    size_t supplyAudio(boo::IAudioVoice&, size_t frames, int16_t* data)
    {
        if (xe8_silent)
        {
            StopStream();
            memset(data, 0, frames * 2);
            return frames;
        }

        if (xec_readState != 2 || (xe0_curBuffer == 0 && m_curSample >= xdc_ringSamples / 2))
        {
            if (!BufferStream())
            {
                memset(data, 0, frames * 2);
                return frames;
            }
        }

        u32 readToSample = m_curSample + frames;
        if (!x10_info.x10_loopFlag)
        {
            m_totalSamples += frames;
            u32 fileSamples = x10_info.xc_adpcmBytes * 14 / 8;
            if (m_totalSamples >= fileSamples)
            {
                u32 leftover = m_totalSamples - fileSamples;
                readToSample -= leftover;
                memset(data + frames - leftover, 0, leftover * 2);
                StopStream();
            }
        }

        u32 leftoverSamples = 0;
        if (readToSample > xdc_ringSamples)
        {
            leftoverSamples = readToSample - xdc_ringSamples;
            readToSample = xdc_ringSamples;
        }

        decompressChunk(readToSample, data);

        if (leftoverSamples)
        {
            BufferStream();
            m_curSample = 0;
            decompressChunk(leftoverSamples, data);
        }

        return frames;
    }
    std::unique_ptr<boo::IAudioVoice> m_booVoice;

    void DoAllocateStream()
    {
        xd4_ringBuffer.reset(new u8[0x11DC0]);
        m_booVoice = CAudioSys::GetVoiceEngine()->allocateNewMonoVoice(32000.0, this);
    }

    static void Initialize()
    {
        for (int i=0 ; i<4 ; ++i)
        {
            SDSPStream& stream = g_Streams[i];
            stream.x0_active = false;
            stream.xd4_ringBuffer.reset();
            stream.xd8_ringBytes = 0x11DC0;
            stream.xdc_ringSamples = 0x1f410;
            if (i < 2)
            {
                stream.x1_oneshot = false;
                stream.DoAllocateStream();
            }
            else
            {
                stream.x1_oneshot = true;
            }
        }
    }

    static void FreeAllStreams()
    {
        for (int i=0 ; i<4 ; ++i)
        {
            SDSPStream& stream = g_Streams[i];
            stream.m_booVoice.reset();
            stream.x0_active = false;
            stream.xd4_ringBuffer.reset();
        }
    }

    static u32 PickFreeStream(SDSPStream*& streamOut, bool oneshot)
    {
        for (int i=0 ; i<4 ; ++i)
        {
            SDSPStream& stream = g_Streams[i];
            if (stream.x0_active || stream.x1_oneshot != oneshot)
                continue;
            stream.x0_active = true;
            stream.x4_ownerId = ++s_HandleCounter2;
            if (stream.x4_ownerId == -1)
                stream.x4_ownerId = ++s_HandleCounter2;
            stream.x8_stereoLeft = nullptr;
            stream.xc_companionRight = nullptr;
            streamOut = &stream;
            return stream.x4_ownerId;
        }
        return -1;
    }

    static u32 FindStreamIdx(u32 id)
    {
        for (int i=0 ; i<4 ; ++i)
        {
            SDSPStream& stream = g_Streams[i];
            if (stream.x4_ownerId == id)
                return i;
        }
        return -1;
    }

    void UpdateStreamVolume(float vol)
    {
        x4c_vol = vol;
        if (!x0_active || xe8_silent)
            return;
        float coefs[8] = {};
        coefs[int(boo::AudioChannel::FrontLeft)] = m_leftgain * vol * 0.7f;
        coefs[int(boo::AudioChannel::FrontRight)] = m_rightgain * vol * 0.7f;
        m_booVoice->setMonoChannelLevels(nullptr, coefs, true);
    }

    static void UpdateVolume(u32 id, float vol)
    {
        u32 idx = FindStreamIdx(id);
        if (idx == -1)
            return;

        SDSPStream& stream = g_Streams[idx];
        stream.UpdateStreamVolume(vol);
        if (SDSPStream* left = stream.x8_stereoLeft)
            left->UpdateStreamVolume(vol);
        if (SDSPStream* right = stream.xc_companionRight)
            right->UpdateStreamVolume(vol);
    }

    void SilenceStream()
    {
        if (!x0_active || xe8_silent)
            return;
        float coefs[8] = {};
        m_booVoice->setMonoChannelLevels(nullptr, coefs, true);
        xe8_silent = true;
        x0_active = false;
    }

    static void Silence(u32 id)
    {
        u32 idx = FindStreamIdx(id);
        if (idx == -1)
            return;

        SDSPStream& stream = g_Streams[idx];
        stream.SilenceStream();
        if (SDSPStream* left = stream.x8_stereoLeft)
            left->SilenceStream();
        if (SDSPStream* right = stream.xc_companionRight)
            right->SilenceStream();
    }

    void StopStream()
    {
        x0_active = false;
        m_booVoice->stop();
        m_file = std::experimental::nullopt;
    }

    static bool IsStreamActive(u32 id)
    {
        u32 idx = FindStreamIdx(id);
        if (idx == -1)
            return false;

        SDSPStream& stream = g_Streams[idx];
        return stream.x0_active;
    }

    static bool IsStreamAvailable(u32 id)
    {
        u32 idx = FindStreamIdx(id);
        if (idx == -1)
            return false;

        SDSPStream& stream = g_Streams[idx];
        return !stream.x0_active;
    }

    static u32 AllocateMono(const SDSPStreamInfo& info, float vol, bool oneshot)
    {
        SDSPStream* stream;
        u32 id = PickFreeStream(stream, oneshot);
        if (id == -1)
            return -1;

        /* -3dB pan law for mono */
        stream->AllocateStream(info, vol, 0.707f, 0.707f);
        return id;
    }

    static u32 AllocateStereo(const SDSPStreamInfo& linfo,
                              const SDSPStreamInfo& rinfo,
                              float vol, bool oneshot)
    {
        SDSPStream* lstream;
        u32 lid = PickFreeStream(lstream, oneshot);
        if (lid == -1)
            return -1;

        SDSPStream* rstream;
        if (PickFreeStream(rstream, oneshot) == -1)
            return -1;

        rstream->x8_stereoLeft = lstream;
        lstream->xc_companionRight = rstream;

        lstream->AllocateStream(linfo, vol, 1.f, 0.f);
        rstream->AllocateStream(rinfo, vol, 0.f, 1.f);
        return lid;
    }

    void AllocateStream(const SDSPStreamInfo& info, float vol, float left, float right)
    {
        x10_info = info;
        m_file.emplace(x10_info.x0_fileName);
        if (!xd4_ringBuffer)
            DoAllocateStream();
        m_readReqs[0].reset();
        m_readReqs[1].reset();
        x4c_vol = vol;
        m_leftgain = left;
        m_rightgain = right;
        xe8_silent = false;
        xec_readState = 0;
        xe0_curBuffer = -1;
        xd8_ringBytes = 0x11DC0;
        xdc_ringSamples = 0x1f410;
        xcc_fileCur = 0;
        m_curSample = 0;
        m_totalSamples = 0;
        m_prev1 = 0;
        m_prev2 = 0;
        memset(xd4_ringBuffer.get(), 0, 0x11DC0);
        m_booVoice->resetSampleRate(info.x4_sampleRate);
        m_booVoice->start();
        UpdateStreamVolume(vol);
    }

    static SDSPStream g_Streams[4];
};

SDSPStream SDSPStream::g_Streams[4] = {};

class CDSPStreamManager
{
    friend struct SDSPStreamInfo;
public:
    enum class EState
    {
        Looping,
        Oneshot,
        Preparing
    };

private:
    dspadpcm_header x0_header;
    std::string x60_fileName; // arg1
    union
    {
        u8 dummy = 0;
        struct
        {
            bool x70_24_unclaimed : 1;
            bool x70_25_headerReadCancelled : 1;
            u8 x70_26_headerReadState : 2; // 0: not read 1: reading 2: read
        };
    };
    s8 x71_companionRight = -1;
    s8 x72_companionLeft = -1;
    float x73_volume = 0.f;
    bool x74_oneshot;
    u32 x78_handleId = -1; // arg2
    u32 x7c_streamId = -1;
    std::shared_ptr<IDvdRequest> m_dvdReq;
    //DVDFileInfo x80_dvdHandle;
    static CDSPStreamManager g_Streams[4];

public:
    CDSPStreamManager()
    {
        x70_24_unclaimed = true;
    }

    CDSPStreamManager(const std::string& fileName, u32 handle, float volume, bool oneshot)
    : x60_fileName(fileName), x73_volume(volume), x74_oneshot(oneshot), x78_handleId(handle)
    {
        if (!CDvdFile::FileExists(fileName.c_str()))
            x70_24_unclaimed = true;
    }

    static u32 FindUnclaimedStreamIdx()
    {
        for (int i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            if (stream.x70_24_unclaimed)
                return i;
        }
        return -1;
    }

    static bool FindUnclaimedStereoPair(u32& left, u32& right)
    {
        u32 idx = FindUnclaimedStreamIdx();

        for (u32 i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            if (stream.x70_24_unclaimed && idx != i)
            {
                left = idx;
                right = i;
                return true;
            }
        }

        return false;
    }

    static u32 FindClaimedStreamIdx(u32 handle)
    {
        for (int i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            if (!stream.x70_24_unclaimed && stream.x78_handleId == handle)
                return i;
        }
        return -1;
    }

    static u32 GetFreeHandleId()
    {
        u32 handle;
        bool good;
        do
        {
            good = true;
            handle = ++s_HandleCounter;
            if (handle == -1)
            {
                good = false;
                continue;
            }

            for (int i=0 ; i<4 ; ++i)
            {
                CDSPStreamManager& stream = g_Streams[i];
                if (!stream.x70_24_unclaimed && stream.x78_handleId == handle)
                {
                    good = false;
                    break;
                }
            }

        } while(!good);

        return handle;
    }

    static EState GetStreamState(u32 handle)
    {
        u32 idx = FindClaimedStreamIdx(handle);
        if (idx == -1)
            return EState::Oneshot;

        CDSPStreamManager& stream = g_Streams[idx];
        switch (stream.x70_26_headerReadState)
        {
        case 0:
            return EState::Oneshot;
        case 2:
            return EState(!stream.x0_header.xc_loop_flag);
        default:
            return EState::Preparing;
        }
    }

    static bool CanStop(u32 handle)
    {
        u32 idx = FindClaimedStreamIdx(handle);
        if (idx == -1)
            return true;

        CDSPStreamManager& stream = g_Streams[idx];
        if (stream.x70_26_headerReadState == 1)
            return false;

        if (stream.x7c_streamId == -1)
            return true;

        return !SDSPStream::IsStreamActive(stream.x7c_streamId);
    }

    static bool IsStreamAvailable(u32 handle)
    {
        u32 idx = FindClaimedStreamIdx(handle);
        if (idx == -1)
            return false;

        CDSPStreamManager& stream = g_Streams[idx];
        if (stream.x70_26_headerReadState == 1)
            return false;

        if (stream.x7c_streamId == -1)
            return false;

        return SDSPStream::IsStreamAvailable(stream.x7c_streamId);
    }

    static void AllocateStream(u32 idx)
    {
        CDSPStreamManager& stream = g_Streams[idx];
        SDSPStreamInfo info(stream);

        if (stream.x71_companionRight == -1)
        {
            /* Mono */
            if (!stream.x70_25_headerReadCancelled)
                stream.x7c_streamId = SDSPStream::AllocateMono(info, stream.x73_volume, stream.x74_oneshot);
            if (stream.x7c_streamId == -1)
                stream = CDSPStreamManager();
        }
        else
        {
            /* Stereo */
            CDSPStreamManager& rstream = g_Streams[stream.x71_companionRight];
            SDSPStreamInfo rinfo(rstream);

            if (!stream.x70_25_headerReadCancelled)
                stream.x7c_streamId = SDSPStream::AllocateStereo(info, rinfo, stream.x73_volume, stream.x74_oneshot);
            if (stream.x7c_streamId == -1)
            {
                stream = CDSPStreamManager();
                rstream = CDSPStreamManager();
            }
        }
    }

    void HeaderReadComplete()
    {
        u32 selfIdx = -1;
        for (int i=0 ; i<4 ; ++i)
        {
            if (this == &g_Streams[i])
            {
                selfIdx = i;
                break;
            }
        }

        if (x70_24_unclaimed || selfIdx == -1)
        {
            *this = CDSPStreamManager();
            return;
        }

        x70_26_headerReadState = 2;

        u32 companion = -1;
        if (x72_companionLeft != -1)
            companion = x72_companionLeft;
        else if (x71_companionRight != -1)
            companion = x71_companionRight;

        if (companion != -1)
        {
            /* Stereo */
            CDSPStreamManager& companionStream = g_Streams[companion];
            if (companionStream.x70_24_unclaimed ||
                companionStream.x70_26_headerReadState == 0 ||
                (companionStream.x71_companionRight != selfIdx &&
                 companionStream.x72_companionLeft != selfIdx))
            {
                /* No consistent companion available */
                *this = CDSPStreamManager();
                return;
            }

            /* Companion is pending; its completion will continue */
            if (companionStream.x70_26_headerReadState == 1)
                return;

            /* Use whichever stream is the left channel */
            if (companionStream.x71_companionRight != -1)
                AllocateStream(companion);
            else
                AllocateStream(selfIdx);
        }
        else
        {
            /* Mono */
            AllocateStream(selfIdx);
        }
    }

    static void PollHeaderReadCompletions()
    {
        for (int i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            if (stream.m_dvdReq && stream.m_dvdReq->IsComplete())
            {
                stream.m_dvdReq.reset();
                stream.HeaderReadComplete();
            }
        }
    }

    static bool StartMonoHeaderRead(CDSPStreamManager& stream)
    {
        if (stream.x70_26_headerReadState != 0 || stream.x70_24_unclaimed)
            return false;

        CDvdFile file(stream.x60_fileName.c_str());
        if (!file)
            return false;

        stream.x70_26_headerReadState = 1;
        stream.m_dvdReq = file.AsyncRead(&stream.x0_header, sizeof(dspadpcm_header));
        return true;
    }

    static bool StartStereoHeaderRead(CDSPStreamManager& lstream, CDSPStreamManager& rstream)
    {
        if (lstream.x70_26_headerReadState != 0 || lstream.x70_24_unclaimed ||
            rstream.x70_26_headerReadState != 0 || rstream.x70_24_unclaimed)
            return false;

        CDvdFile lfile(lstream.x60_fileName.c_str());
        if (!lfile)
            return false;

        CDvdFile rfile(rstream.x60_fileName.c_str());
        if (!rfile)
            return false;

        lstream.x70_26_headerReadState = 1;
        rstream.x70_26_headerReadState = 1;

        lstream.m_dvdReq = lfile.AsyncRead(&lstream.x0_header, sizeof(dspadpcm_header));
        rstream.m_dvdReq = rfile.AsyncRead(&rstream.x0_header, sizeof(dspadpcm_header));
        return true;
    }

    void WaitForReadCompletion()
    {
        if (std::shared_ptr<IDvdRequest> req = m_dvdReq)
            req->WaitUntilComplete();
        m_dvdReq.reset();
    }

    static u32 StartStreaming(const std::string& fileName, float volume, bool oneshot)
    {
        auto pipePos = fileName.find('|');
        if (pipePos == std::string::npos)
        {
            /* Mono stream */
            u32 idx = FindUnclaimedStreamIdx();
            if (idx == -1)
                return -1;

            u32 handle = GetFreeHandleId();
            CDSPStreamManager tmpStream(fileName, handle, volume, oneshot);
            if (tmpStream.x70_24_unclaimed)
                return -1;

            CDSPStreamManager& stream = g_Streams[idx];
            stream = tmpStream;

            if (!StartMonoHeaderRead(stream))
            {
                stream.x70_25_headerReadCancelled = true;
                stream.WaitForReadCompletion();
                stream = CDSPStreamManager();
                return -1;
            }

            return handle;
        }
        else
        {
            /* Stereo stream */
            u32 leftIdx = 0;
            u32 rightIdx = 0;
            if (!FindUnclaimedStereoPair(leftIdx, rightIdx))
                return -1;

            std::string leftFile(fileName.begin(), fileName.begin() + pipePos);
            std::string rightFile(fileName.begin() + pipePos + 1, fileName.end());

            u32 leftHandle = GetFreeHandleId();
            u32 rightHandle = GetFreeHandleId();
            CDSPStreamManager tmpLeftStream(leftFile, leftHandle, volume, oneshot);
            CDSPStreamManager tmpRightStream(rightFile, rightHandle, volume, oneshot);
            if (tmpLeftStream.x70_24_unclaimed || tmpRightStream.x70_24_unclaimed)
                return -1;

            tmpLeftStream.x71_companionRight = rightIdx;
            tmpRightStream.x72_companionLeft = leftIdx;

            CDSPStreamManager& leftStream = g_Streams[leftIdx];
            CDSPStreamManager& rightStream = g_Streams[rightIdx];
            leftStream = tmpLeftStream;
            rightStream = tmpRightStream;

            if (!StartStereoHeaderRead(leftStream, rightStream))
            {
                leftStream.x70_25_headerReadCancelled = true;
                rightStream.x70_25_headerReadCancelled = true;
                leftStream.WaitForReadCompletion();
                leftStream.WaitForReadCompletion();
                leftStream = CDSPStreamManager();
                rightStream = CDSPStreamManager();
                return -1;
            }

            return leftHandle;
        }
    }

    static void StopStreaming(u32 handle)
    {
        u32 idx = FindClaimedStreamIdx(handle);
        if (idx == -1)
            return;

        CDSPStreamManager& stream = g_Streams[idx];
        if (stream.x70_24_unclaimed)
            return;

        if (stream.x70_26_headerReadState == 1)
        {
            stream.x70_25_headerReadCancelled = true;
            return;
        }

        if (stream.x71_companionRight != -1)
            g_Streams[stream.x71_companionRight] = CDSPStreamManager();

        SDSPStream::Silence(stream.x7c_streamId);

        stream = CDSPStreamManager();
    }

    static void UpdateVolume(u32 handle, float volume)
    {
        u32 idx = FindClaimedStreamIdx(handle);
        if (idx == -1)
            return;

        CDSPStreamManager& stream = g_Streams[idx];
        stream.x73_volume = volume;
        if (stream.x7c_streamId == -1)
            return;

        SDSPStream::UpdateVolume(stream.x7c_streamId, volume);
    }

    static void Initialize()
    {
        SDSPStream::Initialize();
        for (int i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            stream = CDSPStreamManager();
        }
    }

    static void Shutdown()
    {
        SDSPStream::FreeAllStreams();
        for (int i=0 ; i<4 ; ++i)
        {
            CDSPStreamManager& stream = g_Streams[i];
            stream = CDSPStreamManager();
        }
    }
};

CDSPStreamManager CDSPStreamManager::g_Streams[4] = {};


SDSPStreamInfo::SDSPStreamInfo(const CDSPStreamManager& stream)
{
    x0_fileName = stream.x60_fileName.c_str();
    x4_sampleRate = hecl::SBig(stream.x0_header.x8_sample_rate);
    xc_adpcmBytes = (hecl::SBig(stream.x0_header.x4_num_nibbles) / 2) & 0x7FFFFFE0;

    if (stream.x0_header.xc_loop_flag)
    {
        u32 loopStartNibble = hecl::SBig(stream.x0_header.x10_loop_start_nibble);
        u32 loopEndNibble = hecl::SBig(stream.x0_header.x14_loop_end_nibble);
        x10_loopFlag = true;
        x14_loopStartByte = (loopStartNibble / 2) & 0x7FFFFFE0;
        x18_loopEndByte = std::min((loopEndNibble / 2) & 0x7FFFFFE0, xc_adpcmBytes);
    }
    else
    {
        x10_loopFlag = false;
        x14_loopStartByte = 0;
        x18_loopEndByte = 0;
    }

    for (int i=0 ; i<8 ; ++i)
    {
        x1c_coef[i][0] = hecl::SBig(stream.x0_header.x1c_coef[i][0]);
        x1c_coef[i][1] = hecl::SBig(stream.x0_header.x1c_coef[i][1]);
    }
}

enum class EPlayerState
{
    Stopped,
    FadeIn,
    Playing,
    FadeOut,
    FadeOutNoStop
};

struct SDSPPlayer
{
    std::string x0_fileName;
    EPlayerState x10_playState = EPlayerState::Stopped;
    float x14_volume = 0.f;
    float x18_fadeIn = 0.f;
    float x1c_fadeOut = 0.f;
    u32 x20_internalHandle = -1;
    float x24_fadeFactor = 0.f;
    bool x28_music = true;

    SDSPPlayer() = default;
    SDSPPlayer(EPlayerState playing, const std::string& fileName, float volume,
               float fadeIn, float fadeOut, u32 handle, bool music)
    : x0_fileName(fileName), x10_playState(playing), x14_volume(volume),
      x18_fadeIn(fadeIn), x1c_fadeOut(fadeOut), x20_internalHandle(handle), x28_music(music) {}
};
static SDSPPlayer s_Players[2]; // looping, oneshot
static SDSPPlayer s_QueuedPlayers[2]; // looping, oneshot

float CStreamAudioManager::GetTargetDSPVolume(float fileVol, bool music)
{
    if (music)
        return g_MusicUnmute ? (g_MusicVolume * fileVol / 127.f) : 0.f;
    else
        return g_SfxUnmute ? (g_SfxVolume * fileVol / 127.f) : 0.f;
}

void CStreamAudioManager::Start(bool oneshot, const std::string& fileName,
                                u8 volume, bool music, float fadeIn, float fadeOut)
{
    float fvol = volume / 127.f;

    SDSPPlayer& p = s_Players[oneshot];
    SDSPPlayer& qp = s_QueuedPlayers[oneshot];

    if (p.x10_playState != EPlayerState::Stopped &&
        CStringExtras::CompareCaseInsensitive(fileName, p.x0_fileName))
    {
        /* Enque new stream */
        qp = SDSPPlayer(EPlayerState::FadeIn, fileName, fvol, fadeIn, fadeOut, -1, music);
        Stop(oneshot, p.x0_fileName);
    }
    else if (p.x10_playState != EPlayerState::Stopped)
    {
        /* Fade existing stream back in */
        p.x18_fadeIn = fadeIn;
        p.x1c_fadeOut = fadeOut;
        p.x14_volume = fvol;
        if (p.x18_fadeIn <= FLT_EPSILON)
        {
            CDSPStreamManager::UpdateVolume(p.x20_internalHandle,
                                            GetTargetDSPVolume(p.x14_volume, p.x28_music));
            p.x24_fadeFactor = 1.f;
            p.x10_playState = EPlayerState::Playing;
        }
        else
        {
            p.x10_playState = EPlayerState::FadeIn;
        }
    }
    else
    {
        /* Start new stream */
        EPlayerState state;
        float vol;
        if (fadeIn > 0.f)
        {
            state = EPlayerState::FadeIn;
            vol = 0.f;
        }
        else
        {
            state = EPlayerState::Playing;
            vol = fvol;
        }

        u32 handle = CDSPStreamManager::StartStreaming(fileName, GetTargetDSPVolume(vol, music), oneshot);
        if (handle != -1)
            p = SDSPPlayer(state, fileName, fvol, fadeIn, fadeOut, handle, music);
    }
}

void CStreamAudioManager::Stop(bool oneshot, const std::string& fileName)
{
    SDSPPlayer& p = s_Players[oneshot];
    SDSPPlayer& qp = s_QueuedPlayers[oneshot];

    if (!CStringExtras::CompareCaseInsensitive(fileName, qp.x0_fileName))
    {
        /* Cancel enqueued file */
        qp = SDSPPlayer();
    }
    else if (!CStringExtras::CompareCaseInsensitive(fileName, p.x0_fileName) &&
             p.x20_internalHandle != -1 && p.x10_playState != EPlayerState::Stopped)
    {
        /* Fade out or stop */
        if (p.x1c_fadeOut <= FLT_EPSILON)
            StopStreaming(oneshot);
        else
            p.x10_playState = EPlayerState::FadeOut;
    }
}

void CStreamAudioManager::FadeBackIn(bool oneshot, float fadeTime)
{
    SDSPPlayer& p = s_Players[oneshot];
    if (p.x10_playState == EPlayerState::Stopped ||
        p.x10_playState == EPlayerState::Playing)
        return;
    p.x18_fadeIn = fadeTime;
    p.x10_playState = EPlayerState::FadeIn;
}

void CStreamAudioManager::TemporaryFadeOut(bool oneshot, float fadeTime)
{
    SDSPPlayer& p = s_Players[oneshot];
    if (p.x10_playState == EPlayerState::FadeOut ||
        p.x10_playState == EPlayerState::Stopped)
        return;
    p.x1c_fadeOut = fadeTime;
    p.x10_playState = EPlayerState::FadeOutNoStop;
}

void CStreamAudioManager::StopStreaming(bool oneshot)
{
    SDSPPlayer& p = s_Players[oneshot];
    p.x10_playState = EPlayerState::Stopped;
    CDSPStreamManager::StopStreaming(p.x20_internalHandle);
    p.x24_fadeFactor = 0.f;
    p.x20_internalHandle = -1;
}

void CStreamAudioManager::UpdateDSP(bool oneshot, float dt)
{
    SDSPPlayer& p = s_Players[oneshot];

    if (p.x10_playState == EPlayerState::Stopped)
    {
        SDSPPlayer& qp = s_QueuedPlayers[oneshot];
        if (qp.x10_playState != EPlayerState::Stopped)
        {
            Start(oneshot, qp.x0_fileName, qp.x14_volume, qp.x28_music,
                  qp.x18_fadeIn, qp.x1c_fadeOut);
            qp = SDSPPlayer();
        }
    }
    else
    {
        if (p.x10_playState != EPlayerState::Stopped &&
            CDSPStreamManager::GetStreamState(p.x20_internalHandle) == CDSPStreamManager::EState::Oneshot &&
            CDSPStreamManager::CanStop(p.x20_internalHandle))
        {
            StopStreaming(oneshot);
            return;
        }

        if ((p.x10_playState != EPlayerState::FadeIn &&
             p.x10_playState != EPlayerState::FadeOut &&
             p.x10_playState != EPlayerState::FadeOutNoStop))
        {
            if (p.x10_playState == EPlayerState::Playing)
                CDSPStreamManager::UpdateVolume(p.x20_internalHandle,
                                                GetTargetDSPVolume(p.x14_volume, p.x28_music));
            return;
        }

        if (p.x10_playState == EPlayerState::FadeIn)
        {
            float newFadeFactor = p.x24_fadeFactor + dt / p.x18_fadeIn;
            if (newFadeFactor >= 1.f)
            {
                p.x24_fadeFactor = 1.f;
                p.x10_playState = EPlayerState::Playing;
            }
            else
            {
                p.x24_fadeFactor = newFadeFactor;
            }
        }
        else if (p.x10_playState == EPlayerState::FadeOut ||
                 p.x10_playState == EPlayerState::FadeOutNoStop)
        {
            float newFadeFactor = p.x24_fadeFactor - dt / p.x1c_fadeOut;
            if (newFadeFactor <= 0.f)
            {
                if (p.x10_playState == EPlayerState::FadeOutNoStop)
                {
                    p.x24_fadeFactor = 0.f;
                }
                else
                {
                    StopStreaming(oneshot);
                    return;
                }
            }
            else
            {
                p.x24_fadeFactor = newFadeFactor;
            }
        }

        CDSPStreamManager::UpdateVolume(p.x20_internalHandle,
                                        GetTargetDSPVolume(p.x14_volume * p.x24_fadeFactor,
                                                           p.x28_music));
    }
}

void CStreamAudioManager::UpdateDSPStreamers(float dt)
{
    UpdateDSP(false, dt);
    UpdateDSP(true, dt);
}

void CStreamAudioManager::StopAllStreams()
{
    for (int i=0 ; i<2 ; ++i)
    {
        StopStreaming(i);
        SDSPPlayer& p = s_Players[i];
        SDSPPlayer& qp = s_QueuedPlayers[i];
        p = SDSPPlayer();
        qp = SDSPPlayer();
    }
}

void CStreamAudioManager::Update(float dt)
{
    CDSPStreamManager::PollHeaderReadCompletions();
    UpdateDSPStreamers(dt);
}

void CStreamAudioManager::StopAll()
{
    StopAllStreams();
}

void CStreamAudioManager::SetMusicUnmute(bool unmute)
{
    g_MusicUnmute = unmute;
}

void CStreamAudioManager::SetSfxVolume(u8 volume)
{
    g_SfxVolume = std::min(volume, u8(127));
}

void CStreamAudioManager::SetMusicVolume(u8 volume)
{
    g_MusicVolume = std::min(volume, u8(127));
}

void CStreamAudioManager::Initialize()
{
    CDSPStreamManager::Initialize();
}

void CStreamAudioManager::Shutdown()
{
    CDSPStreamManager::Shutdown();
}

u8 CStreamAudioManager::g_MusicVolume = 0x7f;
u8 CStreamAudioManager::g_SfxVolume = 0x7f;
bool CStreamAudioManager::g_MusicUnmute = true;
bool CStreamAudioManager::g_SfxUnmute = true;

}
