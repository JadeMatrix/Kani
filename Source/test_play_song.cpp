#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>
// #include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CFURL.h>
#include <CoreServices/CoreServices.h>

#include <iostream>
#include <cstdlib>
#include <string>


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


void KaniTestMusicHandleErrorDebug( OSStatus e, int line )
{
    // https://developer.apple.com/documentation/audiotoolbox/audio_queue_services?language=objc
    switch( e )
    {
    case kAudioQueueErr_InvalidBuffer:
        std::cout << "InvalidBuffer on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_BufferEmpty:
        std::cout << "BufferEmpty on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_DisposalPending:
        std::cout << "DisposalPending on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidProperty:
        std::cout << "InvalidProperty on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidPropertySize:
        std::cout << "InvalidPropertySize on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidParameter:
        std::cout << "InvalidParameter on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_CannotStart:
        std::cout << "CannotStart on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidDevice:
        std::cout << "InvalidDevice on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_BufferInQueue:
        std::cout << "BufferInQueue on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidRunState:
        std::cout << "InvalidRunState on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidQueueType:
        std::cout << "InvalidQueueType on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_Permissions:
        std::cout << "Permissions on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidPropertyValue:
        std::cout << "InvalidPropertyValue on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_PrimeTimedOut:
        std::cout << "PrimeTimedOut on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_CodecNotFound:
        std::cout << "CodecNotFound on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidCodecAccess:
        std::cout << "InvalidCodecAccess on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_QueueInvalidated:
        std::cout << "QueueInvalidated on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_RecordUnderrun:
        std::cout << "RecordUnderrun on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_EnqueueDuringReset:
        std::cout << "EnqueueDuringReset on line " << std::to_string( line ) << std::endl;
        break;
    case kAudioQueueErr_InvalidOfflineMode:
        std::cout << "InvalidOfflineMode on line " << std::to_string( line ) << std::endl;
        break;
    default:
        return;
    }
    
    std::exit( -1 );
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
    
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
    if( numPackets > 0 )
    {
        inBuffer -> mAudioDataByteSize = numBytesReadFromFile;
        
        result = AudioQueueEnqueueBuffer(
            pAqData -> mQueue,
            inBuffer,
            ( pAqData -> mPacketDescs ? numPackets : 0 ),
            pAqData -> mPacketDescs
        );
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
        
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
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
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
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
     
    CFRelease( audioFileURL );
    
    UInt32 dataFormatSize = sizeof( aqData.mDataFormat );
    result = AudioFileGetProperty(
        aqData.mAudioFile,
        kAudioFilePropertyDataFormat,
        &dataFormatSize,
        &aqData.mDataFormat
    );
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
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
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
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
    
    bool couldNotGetProperty = AudioFileGetPropertyInfo(
        aqData.mAudioFile,
        kAudioFilePropertyMagicCookieData,
        &cookieSize,
        NULL
    );
     
    if( !couldNotGetProperty && cookieSize )
    {
        char* magicCookie = ( char* )malloc( cookieSize );
        
        result = AudioFileGetProperty(
            aqData.mAudioFile,
            kAudioFilePropertyMagicCookieData,
            &cookieSize,
            magicCookie
        );
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
        result = AudioQueueSetProperty(
            aqData.mQueue,
            kAudioQueueProperty_MagicCookie,
            magicCookie,
            cookieSize
        );
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
        
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
    
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
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
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
        
        KaniTestMusicPlayCallback(
            &aqData,
            aqData.mQueue,
            aqData.mBuffers[ i ]
        );
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
    }
    
    Float32 gain = 1.0; // Optionally, allow user to override gain setting here
    
    result = AudioQueueSetParameter(
        aqData.mQueue,
        kAudioQueueParam_Volume,
        gain
    );
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
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
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
    std::cout
        << "running queue..."
        << std::endl
    ;
    
    do {
        result = CFRunLoopRunInMode(
            kCFRunLoopDefaultMode,
            0.25,
            false
        );
        KaniTestMusicHandleErrorDebug( result, __LINE__ );
    } while( aqData.mIsRunning );
    
    result = CFRunLoopRunInMode(
        kCFRunLoopDefaultMode,
        1,
        false
    );
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
    std::cout
        << "ran queue"
        << std::endl
    ;
    
    /**************************************************************************/
    
    result = AudioQueueDispose(
        aqData.mQueue,
        true
    );
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
    result = AudioFileClose( aqData.mAudioFile );
    KaniTestMusicHandleErrorDebug( result, __LINE__ );
    
    free( aqData.mPacketDescs );
    
    std::cout
        << "done"
        << std::endl
    ;
    
    /**************************************************************************/
    
    return 0;
}
