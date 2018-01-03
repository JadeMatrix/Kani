#include "KaniErrors.hpp"

#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioConverter.h>

#include <iostream>


void KaniHandleOSErrorDebugLine( OSStatus e, std::string location )
{
    if( e == noErr )
        return;
    
    std::string errcode_string( "????" );
    std::string errdesc_string( "????" );
    
    switch( e )
    {
    /*
        Audio Queue Services Errors
        https://developer.apple.com/documentation/audiotoolbox/audio_queue_services?language=objc
    */
    
    case kAudioQueueErr_InvalidBuffer:
        errcode_string = "kAudioQueueErr_InvalidBuffer";
        errdesc_string = "The specified audio queue buffer does not belong to the specified audio queue.";
        break;
    case kAudioQueueErr_BufferEmpty:
        errcode_string = "kAudioQueueErr_BufferEmpty";
        errdesc_string = "The audio queue buffer is empty (that is, the mAudioDataByteSize field = 0).";
        break;
    case kAudioQueueErr_DisposalPending:
        errcode_string = "kAudioQueueErr_DisposalPending";
        errdesc_string = "The function cannot act on the audio queue because it is being asynchronously disposed of.";
        break;
    case kAudioQueueErr_InvalidProperty:
        errcode_string = "kAudioQueueErr_InvalidProperty";
        errdesc_string = "The specified property ID is invalid.";
        break;
    case kAudioQueueErr_InvalidPropertySize:
        errcode_string = "kAudioQueueErr_InvalidPropertySize";
        errdesc_string = "The size of the specified property is invalid.";
        break;
    case kAudioQueueErr_InvalidParameter:
        errcode_string = "kAudioQueueErr_InvalidParameter";
        errdesc_string = "The specified parameter ID is invalid.";
        break;
    case kAudioQueueErr_CannotStart:
        errcode_string = "kAudioQueueErr_CannotStart";
        errdesc_string = "The audio queue has encountered a problem and cannot start.";
        break;
    case kAudioQueueErr_InvalidDevice:
        errcode_string = "kAudioQueueErr_InvalidDevice";
        errdesc_string = "The specified audio hardware device could not be located.";
        break;
    case kAudioQueueErr_BufferInQueue:
        errcode_string = "kAudioQueueErr_BufferInQueue";
        errdesc_string = "The audio queue buffer cannot be disposed of when it is enqueued.";
        break;
    case kAudioQueueErr_InvalidRunState:
        errcode_string = "kAudioQueueErr_InvalidRunState";
        errdesc_string = "The queue is running but the function can only operate on the queue when it is stopped, or vice versa.";
        break;
    case kAudioQueueErr_InvalidQueueType:
        errcode_string = "kAudioQueueErr_InvalidQueueType";
        errdesc_string = "The queue is an input queue but the function can only operate on an output queue, or vice versa.";
        break;
    case kAudioQueueErr_Permissions:
        errcode_string = "kAudioQueueErr_Permissions";
        errdesc_string = "You do not have the required permissions to call the function.";
        break;
    case kAudioQueueErr_InvalidPropertyValue:
        errcode_string = "kAudioQueueErr_InvalidPropertyValue";
        errdesc_string = "The property value used is not valid.";
        break;
    case kAudioQueueErr_PrimeTimedOut:
        errcode_string = "kAudioQueueErr_PrimeTimedOut";
        errdesc_string = "During a call to the `AudioQueuePrime()` function, the audio queue’s audio converter failed to convert the requested number of sample frames.";
        break;
    case kAudioQueueErr_CodecNotFound:
        errcode_string = "kAudioQueueErr_CodecNotFound";
        errdesc_string = "The requested codec was not found.";
        break;
    case kAudioQueueErr_InvalidCodecAccess:
        errcode_string = "kAudioQueueErr_InvalidCodecAccess";
        errdesc_string = "The codec could not be accessed.";
        break;
    case kAudioQueueErr_QueueInvalidated:
        errcode_string = "kAudioQueueErr_QueueInvalidated";
        errdesc_string = "In iPhone OS, the audio server has exited, causing the audio queue to become invalid.";
        break;
    case kAudioQueueErr_RecordUnderrun:
        errcode_string = "kAudioQueueErr_RecordUnderrun";
        errdesc_string = "During recording, data was lost because there was no enqueued buffer to store it in.";
        break;
    case kAudioQueueErr_EnqueueDuringReset:
        errcode_string = "kAudioQueueErr_EnqueueDuringReset";
        errdesc_string = "During a call to the `AudioQueueReset()`, `AudioQueueStop()`, or `AudioQueueDispose()`functions, the system does not allow you to enqueue buffers.";
        break;
    case kAudioQueueErr_InvalidOfflineMode:
        errcode_string = "kAudioQueueErr_InvalidOfflineMode";
        errdesc_string = "The operation requires the audio queue to be in offline mode but it isn’t, or vice versa.";
        break;
    // case kAudioFormatUnsupportedDataFormatError: See kAudioFileUnsupportedDataFormatError below
    
    /*
        Audio File Services Errors
        https://developer.apple.com/documentation/audiotoolbox/audio_file_services?language=objc
    */
    
    case kAudioFileUnspecifiedError:
        errcode_string = "kAudioFileUnspecifiedError";
        errdesc_string = "An unspecified error has occurred.";
        break;
    case kAudioFileUnsupportedFileTypeError:
        errcode_string = "kAudioFileUnsupportedFileTypeError";
        errdesc_string = "The file type is not supported.";
        break;
    case kAudioFileUnsupportedDataFormatError:
        errcode_string = "kAudioFileUnsupportedDataFormatError or kAudioConverterErr_FormatNotSupported";
        errdesc_string = "The (playback) data format is unsupported / not supported by this file type.";
        break;
    case kAudioFileUnsupportedPropertyError:
        errcode_string = "kAudioFileUnsupportedPropertyError";
        errdesc_string = "The property is not supported.";
        break;
    case kAudioFileBadPropertySizeError:
        errcode_string = "kAudioFileBadPropertySizeError or kAudioConverterErr_BadPropertySizeError";
        errdesc_string = "The size of the property data was not correct.";
        break;
    case kAudioFilePermissionsError:
        errcode_string = "kAudioFilePermissionsError";
        errdesc_string = "The operation violated the file permissions. For example, an attempt was made to write to a file opened with the `kAudioFileReadPermission` constant.";
        break;
    case kAudioFileNotOptimizedError:
        errcode_string = "kAudioFileNotOptimizedError";
        errdesc_string = "The chunks following the audio data chunk are preventing the extension of the audio data chunk. To write more data, you must optimize the file.";
        break;
    case kAudioFileInvalidChunkError:
        errcode_string = "kAudioFileInvalidChunkError";
        errdesc_string = "Either the chunk does not exist in the file or it is not supported by the file.";
        break;
    case kAudioFileDoesNotAllow64BitDataSizeError:
        errcode_string = "kAudioFileDoesNotAllow64BitDataSizeError";
        errdesc_string = "The file offset was too large for the file type. The AIFF and WAVE file format types have 32-bit file size limits.";
        break;
    case kAudioFileInvalidPacketOffsetError:
        errcode_string = "kAudioFileInvalidPacketOffsetError";
        errdesc_string = "A packet offset was past the end of the file, or not at the end of the file when a VBR format was written, or a corrupt packet size was read when the packet table was built.";
        break;
    case kAudioFileInvalidFileError:
        errcode_string = "kAudioFileInvalidFileError";
        errdesc_string = "The file is malformed, or otherwise not a valid instance of an audio file of its type.";
        break;
    case kAudioFileOperationNotSupportedError:
        errcode_string = "kAudioFileOperationNotSupportedError or kAudioConverterErr_OperationNotSupported";
        errdesc_string = "The operation cannot be performed. For example, setting the `kAudioFilePropertyAudioDataByteCount` constant to increase the size of the audio data in a file is not a supported operation. Write the data instead.";
        break;
    case kAudioFileNotOpenError:
        errcode_string = "kAudioFileNotOpenError";
        errdesc_string = "The file is closed.";
        break;
    case kAudioFileEndOfFileError:
        errcode_string = "kAudioFileEndOfFileError";
        errdesc_string = "End of file.";
        break;
    case kAudioFilePositionError:
        errcode_string = "kAudioFilePositionError";
        errdesc_string = "Invalid file position.";
        break;
    case kAudioFileFileNotFoundError:
        errcode_string = "kAudioFileFileNotFoundError";
        errdesc_string = "File not found.";
        break;
    
    /*
        Audio Converter Services Errors
        https://developer.apple.com/documentation/audiotoolbox/audio_converter_services?language=objc
    */
    
    // case kAudioConverterErr_FormatNotSupported:
    //     errcode_string = "kAudioConverterErr_FormatNotSupported";
    //     break;
    // case kAudioConverterErr_OperationNotSupported:
    //     errcode_string = "kAudioConverterErr_OperationNotSupported";
    //     break;
    case kAudioConverterErr_PropertyNotSupported:
        errcode_string = "kAudioConverterErr_PropertyNotSupported";
        break;
    case kAudioConverterErr_InvalidInputSize:
        errcode_string = "kAudioConverterErr_InvalidInputSize";
        break;
    case kAudioConverterErr_InvalidOutputSize:
        errcode_string = "kAudioConverterErr_InvalidOutputSize";
        errdesc_string = "The byte size is not an integer multiple of the frame size.";
        break;
    case kAudioConverterErr_UnspecifiedError:
        errcode_string = "kAudioConverterErr_UnspecifiedError";
        break;
    // case kAudioConverterErr_BadPropertySizeError:
    //     errcode_string = "kAudioConverterErr_BadPropertySizeError";
    //     break;
    case kAudioConverterErr_RequiresPacketDescriptionsError:
        errcode_string = "kAudioConverterErr_RequiresPacketDescriptionsError";
        break;
    case kAudioConverterErr_InputSampleRateOutOfRange:
        errcode_string = "kAudioConverterErr_InputSampleRateOutOfRange";
        break;
    case kAudioConverterErr_OutputSampleRateOutOfRange:
        errcode_string = "kAudioConverterErr_OutputSampleRateOutOfRange";
        break;
    // Unavailable on macOS (iOS & tvOS only):
    // https://developer.apple.com/documentation/audiotoolbox/1624334-anonymous/kaudioconvertererr_hardwareinuse?language=objc
    // case kAudioConverterErr_HardwareInUse:
    //     errcode_string = "kAudioConverterErr_HardwareInUse";
    //     errdesc_string = "Returned from the `AudioConverterFillComplexBuffer` function if the underlying hardware codec has become unavailable, probably due to an audio interruption.";
    //     break;
    // Unavailable on macOS (iOS & tvOS only):
    // https://developer.apple.com/documentation/audiotoolbox/1624334-anonymous/kaudioconvertererr_nohardwarepermission?language=objc
    // case kAudioConverterErr_NoHardwarePermission:
    //     errcode_string = "kAudioConverterErr_NoHardwarePermission";
    //     errdesc_string = "Returned from the `AudioConverterNew` function if the new converter would use a hardware codec which the application does not have permission to use.";
    //     break;
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    const char* osstatus_name_string    = GetMacOSStatusErrorString(   e );
    const char* osstatus_comment_string = GetMacOSStatusCommentString( e );
#pragma clang diagnostic pop
    
    std::string err_string;
    char err_string_buffer[ 4 ];
    *( UInt32* )err_string_buffer = CFSwapInt32HostToBig( e );
    if(
           isprint( err_string_buffer[ 0 ] )
        && isprint( err_string_buffer[ 1 ] )
        && isprint( err_string_buffer[ 2 ] )
        && isprint( err_string_buffer[ 3 ] )
    )
        err_string = " \"" + std::string( err_string_buffer, 4 ) + "\"";
    
    if( osstatus_name_string && *osstatus_name_string )
        std::cerr
            << osstatus_name_string
            << " ("
            << errcode_string
            << " = "
            << ( long long )e
            << err_string
            << ")"
            << location
            << " - "
            << osstatus_comment_string
            << " ("
            << errdesc_string
            << ")"
            << std::endl
        ;
    else
        std::cerr
            << errcode_string
            << " ("
            << ( long long )e
            << err_string
            << ")"
            << location
            << " - "
            << errdesc_string
            << std::endl
        ;
    
    std::exit( -1 );
}
