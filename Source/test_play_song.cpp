#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>
// #include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CFURL.h>
#include <CoreServices/CoreServices.h>

#include <iostream>
#include <cstdlib>
#include <string>

#include "KaniErrors.hpp"


namespace
{
    static const int kNumberBuffers = 3;
    
    struct AQPlayerState
    {
        AudioStreamBasicDescription  mDataFormat;
        AudioQueueRef                mQueue;
        AudioQueueBufferRef          mBuffers[ kNumberBuffers ];
        AudioFileID                  mAudioFile;
        UInt32                       bufferByteSize;
        SInt64                       mCurrentPacket;
        UInt32                       mNumPacketsToRead;
        AudioStreamPacketDescription *mPacketDescs;
        bool                         mIsRunning;
    };
}


void KaniTestMusicPlayCallback(
    void* pAqData_v,
    AudioQueueRef inAQ,
    AudioQueueBufferRef inBuffer
)
{
    AQPlayerState* pAqData = ( AQPlayerState* )pAqData_v;
    
    OSStatus result;
    static unsigned long buffer_count = 0;
    
    if( !( pAqData -> mIsRunning ) )
        return;
    
    UInt32 numBytesReadFromFile;
    UInt32 numPackets = pAqData -> mNumPacketsToRead;
    
    result = AudioFileReadPackets(
    // result = AudioFileReadPacketData(
        pAqData -> mAudioFile,
        false,
        &numBytesReadFromFile,
        pAqData -> mPacketDescs,
        pAqData -> mCurrentPacket,
        &numPackets,
        inBuffer -> mAudioData
    );
    
    KaniHandleOSErrorDebug( result );
    
    if( numPackets > 0 )
    {
        inBuffer -> mAudioDataByteSize = numBytesReadFromFile;
        
        result = AudioQueueEnqueueBuffer(
            pAqData -> mQueue,
            inBuffer,
            ( pAqData -> mPacketDescs ? numPackets : 0 ),
            pAqData -> mPacketDescs
        );
        KaniHandleOSErrorDebug( result );
        
        pAqData -> mCurrentPacket += numPackets;
        
        std::cout
            << "buffered audio file data buffer "
            << ++buffer_count
            << std::endl
        ;
    }
    else
    {
        result = AudioQueueStop( pAqData -> mQueue, false );
        KaniHandleOSErrorDebug( result );
        pAqData -> mIsRunning = false;
        
        std::cout
            << "no more audio file data to buffer"
            << std::endl
        ;
    }
}


void DeriveBufferSize (
    /*
    The AudioStreamBasicDescription structure for the audio queue.
    */
    AudioStreamBasicDescription &ASBDesc,
    /*
    The estimated maximum packet size for the data in the audio file you’re
    playing. You can determine this value by invoking the AudioFileGetProperty
    function (declared in the AudioFile.h header file) with a property ID of
    kAudioFilePropertyPacketSizeUpperBound. See Set Sizes for a Playback Audio
    Queue.
    */
    UInt32                      maxPacketSize,
    /*
    The size you are specifying for each audio queue buffer, in terms of seconds
    of audio.
    */
    Float64                     seconds,
    /*
    On output, the size for each audio queue buffer, in bytes.
    */
    UInt32                      *outBufferSize,
    /*
    On output, the number of packets of audio data to read from the file on each
    invocation of the playback audio queue callback.
    */
    UInt32                      *outNumPacketsToRead
)
{
    /*
    An upper bound for the audio queue buffer size, in bytes. In this example,
    the upper bound is set to 320 KB. This corresponds to approximately five
    seconds of stereo, 24 bit audio at a sample rate of 96 kHz.
    */
    static const int maxBufferSize = 0x50000;
    /*
    A lower bound for the audio queue buffer size, in bytes. In this example,
    the lower bound is set to 16 KB.
    */
    static const int minBufferSize = 0x4000;
    
    if (ASBDesc.mFramesPerPacket != 0) {
        /*
        For audio data formats that define a fixed number of frames per packet,
        derives the audio queue buffer size.
        */
        Float64 numPacketsForTime =
            ASBDesc.mSampleRate / ASBDesc.mFramesPerPacket * seconds;
        *outBufferSize = numPacketsForTime * maxPacketSize;
    } else {
        /*
        For audio data formats that do not define a fixed number of frames per
        packet, derives a reasonable audio queue buffer size based on the
        maximum packet size and the upper bound you’ve set.
        */
        *outBufferSize =
            maxBufferSize > maxPacketSize ?
                maxBufferSize : maxPacketSize;
    }
    
    if (
        *outBufferSize > maxBufferSize &&
        *outBufferSize > maxPacketSize
    )
        /*
        If the derived buffer size is above the upper bound you’ve set, adjusts
        it the bound—taking into account the estimated maximum packet size.
        */
        *outBufferSize = maxBufferSize;
    else {
        /*
        If the derived buffer size is below the lower bound you’ve set, adjusts
        it to the bound.
        */
        if (*outBufferSize < minBufferSize)
            *outBufferSize = minBufferSize;
    }
    
    /*
    Calculates the number of packets to read from the audio file on each
    invocation of the callback.
    */
    *outNumPacketsToRead = *outBufferSize / maxPacketSize;
}


int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cerr << "missing audio file" << std::endl;
        return -1;
    }
    
    std::string song_file( argv[ 1 ] );
    
    OSStatus result;
    
    AQPlayerState aqData = { 0 };
    
    std::cout
        << "playing file "
        << song_file
        << std::endl
    ;
    
    /**************************************************************************/
    
    CFURLRef audioFileURL = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, // same as NULL
        ( UInt8* )song_file.c_str(),
        song_file.length(),
        false // not a directory
    );
    
    result = AudioFileOpenURL(
        audioFileURL,
        ( AudioFilePermissions )fsRdPerm,
        0,
        &aqData.mAudioFile
    );
    KaniHandleOSErrorDebug( result );
     
    CFRelease( audioFileURL );
    
    UInt32 dataFormatSize = sizeof( aqData.mDataFormat );
    result = AudioFileGetProperty(
        aqData.mAudioFile,
        kAudioFilePropertyDataFormat,
        &dataFormatSize,
        &aqData.mDataFormat
    );
    KaniHandleOSErrorDebug( result );
    
    std::cout
        << "loaded audio file"
        << std::endl
    ;
    
    /**************************************************************************/
    
    UInt32 maxPacketSize;
    UInt32 propertySize = sizeof( maxPacketSize );
    result = AudioFileGetProperty(
        aqData.mAudioFile,
        kAudioFilePropertyPacketSizeUpperBound,
        &propertySize,
        &maxPacketSize
    );
    KaniHandleOSErrorDebug( result );
    
    DeriveBufferSize(
        aqData.mDataFormat,
        maxPacketSize,
        0.5,
        &aqData.bufferByteSize,
        &aqData.mNumPacketsToRead
    );
    
    bool isFormatVBR = (
           aqData.mDataFormat.mBytesPerPacket  == 0
        || aqData.mDataFormat.mFramesPerPacket == 0
    );
    
    if( isFormatVBR )
    {
        aqData.mPacketDescs = ( AudioStreamPacketDescription* )malloc(
            aqData.mNumPacketsToRead * sizeof( AudioStreamPacketDescription )
        );
    }
    else
    {
        aqData.mPacketDescs = NULL;
    }
    
    std::cout
        << "got audio file metadata"
        << std::endl
    ;
    
    /**************************************************************************/
    
    UInt32 cookieSize = sizeof( UInt32 );
    
    result = AudioFileGetPropertyInfo(
        aqData.mAudioFile,
        kAudioFilePropertyMagicCookieData,
        &cookieSize,
        NULL
    );
     
    if( result == noErr && cookieSize )
    {
        char* magicCookie = ( char* )malloc( cookieSize );
        
        result = AudioFileGetProperty(
            aqData.mAudioFile,
            kAudioFilePropertyMagicCookieData,
            &cookieSize,
            magicCookie
        );
        KaniHandleOSErrorDebug( result );
        result = AudioQueueSetProperty(
            aqData.mQueue,
            kAudioQueueProperty_MagicCookie,
            magicCookie,
            cookieSize
        );
        if( result == paramErr )
            std::cout
                << "Could not set magic cookie on queue, playback may still succeed"
                << std::endl
            ;
        else
            KaniHandleOSErrorDebug( result );
        
        free( magicCookie );
    }
    
    std::cout
        << "got audio file cookie"
        << std::endl
    ;
    
    /**************************************************************************/
    
    // result = AudioQueueNewOutput(
    //     &aqData.mDataFormat, // const AudioStreamBasicDescription *inFormat,
    //     KaniTestMusicPlayCallback, // AudioQueueOutputCallback inCallbackProc,
    //     &aqData, // void *inUserData,
    //     NULL, // CFRunLoopRef inCallbackRunLoop,
    //     kCFRunLoopCommonModes /* same as NULL */, // CFStringRef inCallbackRunLoopMode,
    //     0, // UInt32 inFlags,
    //     &aqData.mQueue // AudioQueueRef  _Nullable *outAQ
    // );
    result = AudioQueueNewOutput (
        &aqData.mDataFormat,
        KaniTestMusicPlayCallback,
        &aqData,
        CFRunLoopGetCurrent(),
        kCFRunLoopCommonModes,
        0,
        &aqData.mQueue
    );
    
    KaniHandleOSErrorDebug( result );
    
    aqData.mCurrentPacket = 0;
    
    aqData.mIsRunning = true;
    
    for( int i = 0; i < kNumberBuffers; ++i )
    {
        std::cout
            << "prepping buffer "
            << i
            << std::endl
        ;
        
        result = AudioQueueAllocateBuffer(
            aqData.mQueue,
            aqData.bufferByteSize,
            &aqData.mBuffers[ i ]
        );
        KaniHandleOSErrorDebug( result );
        
        KaniTestMusicPlayCallback(
            &aqData,
            aqData.mQueue,
            aqData.mBuffers[ i ]
        );
    }
    
    Float32 gain = 1.0; // Optionally, allow user to override gain setting here
    
    result = AudioQueueSetParameter(
        aqData.mQueue,
        kAudioQueueParam_Volume,
        gain
    );
    KaniHandleOSErrorDebug( result );
    
    std::cout
        << "created queue & set parameters"
        << std::endl
    ;
    
    /**************************************************************************/
    
    // aqData.mIsRunning = true;
     
    result = AudioQueueStart(
        aqData.mQueue,
        NULL
    );
    KaniHandleOSErrorDebug( result );
    
    std::cout
        << "running queue..."
        << std::endl
    ;
    
    do {
        // returns https://developer.apple.com/documentation/corefoundation/cfrunlooprunresult?language=objc
        CFRunLoopRunInMode(
            kCFRunLoopDefaultMode,
            0.25,
            false
        );
    } while( aqData.mIsRunning );
    
    CFRunLoopRunInMode(
        kCFRunLoopDefaultMode,
        1,
        false
    );
    
    std::cout
        << "ran queue"
        << std::endl
    ;
    
    /**************************************************************************/
    
    result = AudioQueueDispose(
        aqData.mQueue,
        true
    );
    KaniHandleOSErrorDebug( result );
    
    result = AudioFileClose( aqData.mAudioFile );
    KaniHandleOSErrorDebug( result );
    
    free( aqData.mPacketDescs );
    
    std::cout
        << "done"
        << std::endl
    ;
    
    /**************************************************************************/
    
    return 0;
}
