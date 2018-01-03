#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>
// #include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CFURL.h>
#include <CoreServices/CoreServices.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <list>
#include <cmath>

#include "KaniErrors.hpp"


namespace
{
    static const int kNumberBuffers = 3;
    static const double buffer_seconds = 0.5f;
    static const double play_loop_timeout_seconds = 0.25f;
    
    struct converter_state
    {
        AudioConverterRef             converter;
        AudioFileID                   file;
        AudioStreamBasicDescription   format;
        AudioStreamPacketDescription* packet_descriptions;
        AudioBuffer                   buffer;
        UInt32                        buffer_bytes;
        Float64                       duration_seconds;
        
        UInt32                        packets_to_read;
        SInt64                        current_packet;
    };
    
    struct player_state
    {
        AudioQueueRef                 queue;
        AudioStreamBasicDescription   format;
        AudioStreamPacketDescription* packet_descriptions;
        AudioQueueBufferRef           buffers[ kNumberBuffers ];
        UInt32                        buffer_bytes;
        
        UInt32                        packets_to_read;
        
        bool                          running;
        
        std::list< converter_state* > songs;
    };
}


/******************************************************************************/


void DeriveBufferSize(
    AudioStreamBasicDescription &ASBDesc,
    UInt32                      maxPacketSize,
    Float64                     seconds,
    UInt32                      *outBufferSize,
    UInt32                      *outNumPacketsToRead
);
void initConverter(
    converter_state   & c_state,
    const player_state& p_state,
    const std::string & filename
);
void disposeConverter( converter_state& c_state );
void initPlayer( player_state& p_state );
void disposePlayer( player_state& p_state );
void runPlayer( player_state& p_state );
OSStatus KaniCoreAudioConverterComplexInputDataProc(
    AudioConverterRef             converter,
    UInt32                      * packet_count,
    AudioBufferList             * buffers,
    AudioStreamPacketDescription* packet_descriptions,
    void                        * user_data
);
void KaniCoreAudioQueueCallbackProc(
    void*               user_data,
    AudioQueueRef       queue,
    AudioQueueBufferRef buffer
);


/******************************************************************************/


void DeriveBufferSize(
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
        // TODO: may want to estimate larger
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


void initConverter(
    converter_state   & c_state,
    const player_state& p_state,
    const std::string & filename
)
{
    OSStatus result;
    
    /* Open audio file ********************************************************/
    
    CFURLRef audioFileURL = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, // aka NULL
        ( UInt8* )filename.c_str(),
        filename.length(),
        false // "not a directory"
    );
    
    result = AudioFileOpenURL(
        audioFileURL,
        ( AudioFilePermissions )fsRdPerm,
        0,
        &c_state.file
    );
    KaniHandleOSErrorDebug( result );
     
    CFRelease( audioFileURL );
    
    UInt32 dataFormatSize = sizeof( c_state.format );
    result = AudioFileGetProperty(
        c_state.file,
        kAudioFilePropertyDataFormat,
        &dataFormatSize,
        &c_state.format
    );
    KaniHandleOSErrorDebug( result );
    
    dataFormatSize = sizeof( c_state.duration_seconds );
    result = AudioFileGetProperty(
        c_state.file,
        kAudioFilePropertyEstimatedDuration,
        &dataFormatSize,
        &c_state.duration_seconds
    );
    KaniHandleOSErrorDebug( result );
    
    // DEBUG:
    std::cout
        << "loaded audio file "
        << filename
        << " (approx. duration: "
        << ( long )( c_state.duration_seconds / 60 )
        << ":"
        << fmod( c_state.duration_seconds, 60 )
        << ")"
        << std::endl
    ;
    
    /* Buffers & packet descriptions ******************************************/
    
    UInt32 maxPacketSize;
    UInt32 propertySize = sizeof( maxPacketSize );
    
    result = AudioFileGetProperty(
        c_state.file,
        kAudioFilePropertyPacketSizeUpperBound,
        &propertySize,
        &maxPacketSize
    );
    KaniHandleOSErrorDebug( result );
    
    // DeriveBufferSize(
    //     c_state.format,
    //     maxPacketSize,
    //     buffer_seconds,
    //     &c_state.buffer_bytes,
    //     &c_state.packets_to_read
    // );
    c_state.packets_to_read = 1;
    c_state.buffer_bytes = maxPacketSize * c_state.packets_to_read;
    
    // DEBUG:
    std::cout
        << "want "
        << c_state.buffer_bytes
        << " bytes for "
        << c_state.packets_to_read
        << " packets ("
        << ( ( float )c_state.buffer_bytes / ( float )c_state.packets_to_read )
        << " per)"
        << std::endl
    ;
    
    if( // Variable Bit Rate (VBR)
           c_state.format.mBytesPerPacket  == 0
        || c_state.format.mFramesPerPacket == 0
    )
    {
        c_state.packet_descriptions = ( AudioStreamPacketDescription* )malloc(
            c_state.packets_to_read * sizeof( AudioStreamPacketDescription )
        );
    }
    else
    {
        c_state.packet_descriptions = NULL;
    }
    
    c_state.buffer.mData = malloc( c_state.buffer_bytes );
    c_state.buffer.mNumberChannels = c_state.format.mChannelsPerFrame;
    
    c_state.current_packet = 0;
    
    // DEBUG:
    std::cout
        << "got packet descriptions & allocated buffers for audio file "
        << filename
        << std::endl
    ;
    
    /* Audio converter initialization *****************************************/
    
    result = AudioConverterNew(
        &c_state.format,
        &p_state.format,
        &c_state.converter
    );
    KaniHandleOSErrorDebug( result );
    
    // DEBUG:
    std::cout
        << "created converter for audio file "
        << filename
        << std::endl
    ;
    
    /* Magic cookie ***********************************************************/
    
    UInt32 cookieSize = sizeof( UInt32 );
    
    result = AudioFileGetPropertyInfo(
        c_state.file,
        kAudioFilePropertyMagicCookieData,
        &cookieSize,
        NULL
    );
    
    if( result == noErr && cookieSize > 0 )
    {
        char* magicCookie = ( char* )malloc( cookieSize );
        
        result = AudioFileGetProperty(
            c_state.file,
            kAudioFilePropertyMagicCookieData,
            &cookieSize,
            magicCookie
        );
        KaniHandleOSErrorDebug( result );
        
        // // DEBUG:
        // std::cout
        //     << "magic cookie of size "
        //     << cookieSize
        //     << ": "
        //     << std::string( magicCookie, cookieSize )
        //     << std::endl
        // ;
        
        result = AudioConverterSetProperty(
            c_state.converter,
            kAudioConverterDecompressionMagicCookie,
            cookieSize,
            magicCookie
        );
        // DEBUG:
        if( result == paramErr )
            std::cout
                << "Could not set magic cookie on queue, playback may still succeed"
                << std::endl
            ;
        else
            KaniHandleOSErrorDebug( result );
        
        free( magicCookie );
        
        // DEBUG:
        std::cout
            << "set magic cookie on converter for audio file "
            << filename
            << std::endl
        ;
    }
    // DEBUG:
    else
        std::cout
            << "no magic cookie for audio file "
            << filename
            << std::endl
        ;
}


void disposeConverter( converter_state& c_state )
{
    OSStatus result;
    
    result = AudioConverterDispose( c_state.converter );
    KaniHandleOSErrorDebug( result );
    
    free( c_state.buffer.mData );
    
    if( c_state.packet_descriptions )
        free( c_state.packet_descriptions );
    
    result = AudioFileClose( c_state.file );
    KaniHandleOSErrorDebug( result );
    
    // DEBUG:
    std::cout
        << "cleaned up converter data"
        << std::endl
    ;
}


void initPlayer( player_state& p_state )
{
    OSStatus result;
    
    /* Linear PCM format description ******************************************/
    
    p_state.format.mFormatID         = kAudioFormatLinearPCM;
    p_state.format.mSampleRate       = 44100.0;
    p_state.format.mChannelsPerFrame = 2;
    p_state.format.mBitsPerChannel   = 8 * sizeof( Float32 );
    p_state.format.mBytesPerPacket   =
       p_state.format.mBytesPerFrame =
          p_state.format.mChannelsPerFrame * sizeof( Float32 );
    p_state.format.mFramesPerPacket  = 1;
    
    AudioFileTypeID fileType = kAudioFileAIFFType;
    p_state.format.mFormatFlags =
    (
          /*kLinearPCMFormatFlagIsBigEndian
        |*/ /*kLinearPCMFormatFlagIsSignedInteger*/ kLinearPCMFormatFlagIsFloat
        | kLinearPCMFormatFlagIsPacked
    );
    
    /* Create queue ***********************************************************/
    
    // result = AudioQueueNewOutput(
    //     &aqData.mDataFormat, // const AudioStreamBasicDescription *inFormat,
    //     KaniTestMusicPlayCallback, // AudioQueueOutputCallback inCallbackProc,
    //     &aqData, // void *inUserData,
    //     NULL, // CFRunLoopRef inCallbackRunLoop,
    //     kCFRunLoopCommonModes /* same as NULL */, // CFStringRef inCallbackRunLoopMode,
    //     0, // UInt32 inFlags,
    //     &aqData.mQueue // AudioQueueRef  _Nullable *outAQ
    // );
    result = AudioQueueNewOutput(
        &p_state.format,
        KaniCoreAudioQueueCallbackProc,
        &p_state,
        CFRunLoopGetCurrent(),
        kCFRunLoopCommonModes,
        0,
        &p_state.queue
    );
    KaniHandleOSErrorDebug( result );
    
    Float32 gain = 1.0; // Optionally, allow user to override gain setting here
    
    result = AudioQueueSetParameter(
        p_state.queue,
        kAudioQueueParam_Volume,
        gain
    );
    KaniHandleOSErrorDebug( result );
    
    // DEBUG:
    std::cout
        << "created player queue & set parameters"
        << std::endl
    ;
    
    /* Queue buffers **********************************************************/
    
    DeriveBufferSize(
        p_state.format,
        p_state.format.mBytesPerPacket /* maxPacketSize */,
        buffer_seconds,
        &p_state.buffer_bytes,
        &p_state.packets_to_read
    );
    
    for( int i = 0; i < kNumberBuffers; ++i )
    {
        result = AudioQueueAllocateBuffer(
            p_state.queue,
            p_state.buffer_bytes,
            &p_state.buffers[ i ]
        );
        KaniHandleOSErrorDebug( result );
    }
    
    // DEBUG:
    std::cout
        << "created player queue buffers"
        << std::endl
    ;
}


void disposePlayer( player_state& p_state )
{
    OSStatus result;
    
    result = AudioQueueDispose(
        p_state.queue,
        true
    );
    KaniHandleOSErrorDebug( result );
    
    // DEBUG:
    std::cout
        << "cleaned up player data"
        << std::endl
    ;
}


void runPlayer( player_state& p_state )
{
    OSStatus result;
    
    p_state.running = true;
    
    result = AudioQueueStart(
        p_state.queue,
        NULL
    );
    KaniHandleOSErrorDebug( result );
    
    for( int i = 0; i < kNumberBuffers; ++i )
    {
        // DEBUG:
        std::cout
            << "prepping buffer "
            << i
            << std::endl
        ;
        
        KaniCoreAudioQueueCallbackProc(
            &p_state,
            p_state.queue,
            p_state.buffers[ i ]
        );
    }
    
    // DEBUG:
    std::cout
        << "running queue..."
        << std::endl
    ;
    
    // NOTE: CFRunLoopRunInMode() returns
    // https://developer.apple.com/documentation/corefoundation/cfrunlooprunresult?language=objc
    
    do
    {
        CFRunLoopRunInMode(
            kCFRunLoopDefaultMode,
            play_loop_timeout_seconds,
            false
        );
        // DEBUG:
        std::cout
            << "loop"
            << std::endl
        ;
    } while( p_state.running );
    
    CFRunLoopRunInMode(
        kCFRunLoopDefaultMode,
        1.0f,
        false
    );
    
    // DEBUG:
    std::cout
        << "ran queue"
        << std::endl
    ;
}


OSStatus KaniCoreAudioConverterComplexInputDataProc(
    AudioConverterRef              converter,
    UInt32                       * packet_count,
    AudioBufferList              * buffers,
    AudioStreamPacketDescription** packet_descriptions,
    void                         * user_data
)
{
    // // DEBUG:
    // std::cout
    //     << "KaniCoreAudioConverterComplexInputDataProc()"
    //     << std::endl
    // ;
    
    converter_state* c_state = ( converter_state* )user_data;
    OSStatus result;
    
    UInt32 bytes_read;
    
    // TODO: Reallocate as needed in case player wants larger buffers?
    *packet_count = (
        *packet_count < c_state -> packets_to_read ?
        *packet_count : c_state -> packets_to_read
    );
    
    buffers -> mNumberBuffers = 1;
    buffers -> mBuffers[ 0 ].mData = malloc( c_state -> buffer_bytes );
    buffers -> mBuffers[ 0 ].mDataByteSize   = c_state -> buffer_bytes;
    buffers -> mBuffers[ 0 ].mNumberChannels = c_state -> format.mChannelsPerFrame;
    
    result = AudioFileReadPackets(
    // result = AudioFileReadPacketData(
        c_state -> file,
        false,
        &bytes_read,
        c_state -> packet_descriptions,
        c_state -> current_packet,
        packet_count,
        buffers -> mBuffers[ 0 ].mData
    );
    KaniHandleOSErrorDebug( result );
    
    // // DEBUG:
    // std::cout
    //     << "converter: read "
    //     << bytes_read
    //     << " bytes ("
    //     << *packet_count
    //     << " packets) from file"
    //     << std::endl
    // ;
    
    if( packet_descriptions )
    {
        for( int i = 0; i < *packet_count; ++i )
            packet_descriptions[ i ] = &( c_state -> packet_descriptions[ i ] );
        
        // // DEBUG:
        // std::cout
        //     << "converter: relayed packet descriptions"
        //     << std::endl
        // ;
    }
    
    c_state -> current_packet += *packet_count;
    
    return noErr;
}


void KaniCoreAudioQueueCallbackProc(
    void*               user_data,
    AudioQueueRef       queue,
    AudioQueueBufferRef buffer
)
{
    // // DEBUG:
    // std::cout
    //     << "KaniCoreAudioQueueCallbackProc()"
    //     << std::endl
    // ;
    
    player_state* p_state = ( player_state* )user_data;
    OSStatus result;
    static unsigned long buffer_count = 0;
    
    if( p_state -> songs.size() < 1 )
    {
        // DEBUG:
        std::cout
            << "player: out of songs, stopping"
            << std::endl
        ;
        
        result = AudioQueueStop(
            p_state -> queue,
            false
        );
        KaniHandleOSErrorDebug( result );
        
        p_state -> running = false;
        
        return;
    }
    
    UInt32 packets_read = p_state -> packets_to_read;
    
    // AudioBufferList play_buffers{
    //     {
    //         buffer -> mAudioData,
    //         buffer -> mAudioDataBytesCapacity,
    //         p_state -> format.mChannelsPerFrame
    //     },
    //     1
    // };
    AudioBufferList play_buffers;
    play_buffers.mNumberBuffers = 1;
    play_buffers.mBuffers[ 0 ].mData           = buffer  -> mAudioData;
    play_buffers.mBuffers[ 0 ].mDataByteSize   = buffer  -> mAudioDataBytesCapacity;
    play_buffers.mBuffers[ 0 ].mNumberChannels = p_state -> format.mChannelsPerFrame;
    
    // // DEBUG:
    // std::cout
    //     << "player: reading "
    //     << packets_read
    //     << " packets"
    //     << std::endl
    // ;
    
    result = AudioConverterFillComplexBuffer(
        p_state -> songs.front() -> converter,
        KaniCoreAudioConverterComplexInputDataProc,
        p_state -> songs.front(),
        &packets_read,
        &play_buffers,
        NULL
    );
    KaniHandleOSErrorDebug( result );
    
    // // DEBUG:
    // std::cout
    //     << "player: read "
    //     << packets_read
    //     << " packets"
    //     << std::endl
    // ;
    
    if( packets_read > 0 )
    {
        buffer -> mAudioDataByteSize = (
            packets_read * ( p_state -> format.mBytesPerPacket )
        );
        
        // // DOWNSAMPLE:
        // for( int i = 0; i < packets_read * 2; ++i )
        //     ( ( Float32* )( buffer -> mAudioData ) )[ i ] = (
        //         ( ( Float32* )( buffer -> mAudioData ) )[ i ] / 256
        //     ) * 256;
        
        result = AudioQueueEnqueueBuffer(
            p_state -> queue,
            buffer,
            ( p_state -> packet_descriptions ? packets_read : 0 ),
            p_state -> packet_descriptions
        );
        KaniHandleOSErrorDebug( result );
        
        // DEBUG:
        std::cout
            << "player: buffered audio file data buffer "
            << ++buffer_count
            << std::endl
        ;
    }
    else
    {
        // DEBUG:
        std::cout
            << "player: no more audio file data to buffer, moving to next song"
            << std::endl
        ;
        
        p_state -> songs.pop_front();
        
        // TEMPORARY:
        KaniCoreAudioQueueCallbackProc(
            user_data,
            queue,
            buffer
        );
    }
}


int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cerr << "missing audio file" << std::endl;
        return -1;
    }
    
    int song_count = argc - 1;
    
    /**************************************************************************/
    
    player_state     p_state { 0 };
    converter_state* c_states = new converter_state[ song_count ];
    
    initPlayer( p_state );
    
    for( int i = 0; i < song_count; ++i )
    {
        std::string song_file( argv[ i + 1 ] );
        
        // DEBUG:
        std::cout
            << "adding file "
            << song_file
            << " to player queue"
            << std::endl
        ;
        
        initConverter(
            c_states[ i ],
            p_state,
            song_file
        );
        p_state.songs.push_back( &c_states[ i ] );
    }
    
    runPlayer( p_state );
    
    disposePlayer( p_state );
    
    for( int i = 0; i < song_count; ++i )
        disposeConverter( c_states[ i ] );
    
    delete[] c_states;
    
    std::cout
        << "done"
        << std::endl
    ;
    
    return 0;
}
