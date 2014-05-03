# This file is read into the GN build.

# Files are relative to third_party/skia.
{
  'skia_library_sources': [
    '<(skia_src_path)/core/SkFlate.cpp',  # this should likely be moved into src/utils in skia

    '<(skia_include_path)/images/SkImageRef_GlobalPool.h',
    '<(skia_include_path)/images/SkImageRef.h',
    '<(skia_include_path)/images/SkMovie.h',
    '<(skia_include_path)/images/SkPageFlipper.h',
    '<(skia_include_path)/ports/SkTypeface_win.h',
    '<(skia_include_path)/utils/mac/SkCGUtils.h',
    '<(skia_include_path)/utils/SkDeferredCanvas.h',
    '<(skia_include_path)/utils/SkEventTracer.h',
    '<(skia_include_path)/utils/SkMatrix44.h',
    '<(skia_include_path)/utils/SkNullCanvas.h',
    '<(skia_include_path)/utils/SkNWayCanvas.h',
    '<(skia_include_path)/utils/SkPictureUtils.h',
    '<(skia_include_path)/utils/SkProxyCanvas.h',
    '<(skia_include_path)/utils/SkRTConf.h',
    '<(skia_src_path)/fonts/SkFontMgr_fontconfig.cpp',
    '<(skia_src_path)/images/SkScaledBitmapSampler.cpp',
    '<(skia_src_path)/images/SkScaledBitmapSampler.h',
    '<(skia_src_path)/ports/SkFontConfigInterface_android.cpp',
    '<(skia_src_path)/ports/SkFontConfigInterface_direct.cpp',
    '<(skia_src_path)/ports/SkFontConfigParser_android.cpp',
    '<(skia_src_path)/ports/SkFontHost_fontconfig.cpp',
    '<(skia_src_path)/ports/SkFontHost_FreeType_common.cpp',
    '<(skia_src_path)/ports/SkFontHost_FreeType_common.h',
    '<(skia_src_path)/ports/SkFontHost_FreeType.cpp',
    '<(skia_src_path)/ports/SkFontHost_linux.cpp',
    '<(skia_src_path)/ports/SkFontHost_mac.cpp',
    '<(skia_src_path)/ports/SkFontHost_win.cpp',
    '<(skia_src_path)/ports/SkFontHost_win_dw.cpp',
    '<(skia_src_path)/ports/SkFontMgr_default_gdi.cpp',
    '<(skia_src_path)/ports/SkGlobalInitialization_chromium.cpp',
    '<(skia_src_path)/ports/SkImageDecoder_empty.cpp',
    '<(skia_src_path)/ports/SkOSFile_posix.cpp',
    '<(skia_src_path)/ports/SkOSFile_stdio.cpp',
    '<(skia_src_path)/ports/SkOSFile_win.cpp',
    '<(skia_src_path)/ports/SkTime_Unix.cpp',
    '<(skia_src_path)/ports/SkTLS_pthread.cpp',
    '<(skia_src_path)/ports/SkTLS_win.cpp',
    '<(skia_src_path)/sfnt/SkOTTable_name.cpp',
    '<(skia_src_path)/sfnt/SkOTTable_name.h',
    '<(skia_src_path)/sfnt/SkOTUtils.cpp',
    '<(skia_src_path)/sfnt/SkOTUtils.h',
    '<(skia_src_path)/utils/debugger/SkDebugCanvas.cpp',
    '<(skia_src_path)/utils/debugger/SkDebugCanvas.h',
    '<(skia_src_path)/utils/debugger/SkDrawCommand.cpp',
    '<(skia_src_path)/utils/debugger/SkDrawCommand.h',
    '<(skia_src_path)/utils/debugger/SkObjectParser.cpp',
    '<(skia_src_path)/utils/debugger/SkObjectParser.h',
    '<(skia_src_path)/utils/SkBase64.cpp',
    '<(skia_src_path)/utils/SkBase64.h',
    '<(skia_src_path)/utils/SkBitmapHasher.cpp',
    '<(skia_src_path)/utils/SkBitmapHasher.h',
    '<(skia_src_path)/utils/SkBitSet.cpp',
    '<(skia_src_path)/utils/SkBitSet.h',
    '<(skia_src_path)/utils/SkBoundaryPatch.cpp',
    '<(skia_src_path)/utils/SkFrontBufferedStream.cpp',
    '<(skia_src_path)/utils/SkCamera.cpp',
    '<(skia_src_path)/utils/SkCanvasStack.h',
    '<(skia_src_path)/utils/SkCanvasStack.cpp',
    '<(skia_src_path)/utils/SkCanvasStateUtils.cpp',
    '<(skia_src_path)/utils/SkCubicInterval.cpp',
    '<(skia_src_path)/utils/SkCullPoints.cpp',
    '<(skia_src_path)/utils/SkDeferredCanvas.cpp',
    '<(skia_src_path)/utils/SkDumpCanvas.cpp',
    '<(skia_src_path)/utils/SkEventTracer.cpp',
    '<(skia_src_path)/utils/SkFloatUtils.h',
    '<(skia_src_path)/utils/SkGatherPixelRefsAndRects.cpp',
    '<(skia_src_path)/utils/SkGatherPixelRefsAndRects.h',
    '<(skia_src_path)/utils/SkInterpolator.cpp',
    '<(skia_src_path)/utils/SkLayer.cpp',
    '<(skia_src_path)/utils/SkMatrix22.cpp',
    '<(skia_src_path)/utils/SkMatrix22.h',
    '<(skia_src_path)/utils/SkMatrix44.cpp',
    '<(skia_src_path)/utils/SkMD5.cpp',
    '<(skia_src_path)/utils/SkMD5.h',
    '<(skia_src_path)/utils/SkMeshUtils.cpp',
    '<(skia_src_path)/utils/SkNinePatch.cpp',
    '<(skia_src_path)/utils/SkNWayCanvas.cpp',
    '<(skia_src_path)/utils/SkNullCanvas.cpp',
    '<(skia_src_path)/utils/SkOSFile.cpp',
    '<(skia_src_path)/utils/SkParse.cpp',
    '<(skia_src_path)/utils/SkParseColor.cpp',
    '<(skia_src_path)/utils/SkParsePath.cpp',
    '<(skia_src_path)/utils/SkPictureUtils.cpp',
    '<(skia_src_path)/utils/SkPathUtils.cpp',
    '<(skia_src_path)/utils/SkProxyCanvas.cpp',
    '<(skia_src_path)/utils/SkSHA1.cpp',
    '<(skia_src_path)/utils/SkSHA1.h',
    '<(skia_src_path)/utils/SkRTConf.cpp',
    '<(skia_src_path)/utils/SkThreadUtils.h',
    '<(skia_src_path)/utils/SkThreadUtils_pthread.cpp',
    '<(skia_src_path)/utils/SkThreadUtils_pthread.h',
    '<(skia_src_path)/utils/SkThreadUtils_pthread_linux.cpp',
    '<(skia_src_path)/utils/SkThreadUtils_pthread_mach.cpp',
    '<(skia_src_path)/utils/SkThreadUtils_pthread_other.cpp',
    '<(skia_src_path)/utils/SkThreadUtils_win.cpp',
    '<(skia_src_path)/utils/SkThreadUtils_win.h',
    '<(skia_src_path)/utils/SkTFitsIn.h',
    '<(skia_src_path)/utils/SkTLogic.h',
    '<(skia_src_path)/utils/SkUnitMappers.cpp',

    #mac
    '<(skia_include_path)/utils/mac/SkCGUtils.h',
    '<(skia_src_path)/utils/mac/SkCreateCGImageRef.cpp',
    '<(skia_src_path)/utils/mac/SkStream_mac.cpp',

    #windows
    '<(skia_include_path)/utils/win/SkAutoCoInitialize.h',
    '<(skia_include_path)/utils/win/SkHRESULT.h',
    '<(skia_include_path)/utils/win/SkIStream.h',
    '<(skia_include_path)/utils/win/SkTScopedComPtr.h',
    '<(skia_src_path)/utils/win/SkAutoCoInitialize.cpp',
    '<(skia_src_path)/utils/win/SkDWrite.h',
    '<(skia_src_path)/utils/win/SkDWrite.cpp',
    '<(skia_src_path)/utils/win/SkDWriteFontFileStream.cpp',
    '<(skia_src_path)/utils/win/SkDWriteFontFileStream.h',
    '<(skia_src_path)/utils/win/SkDWriteGeometrySink.cpp',
    '<(skia_src_path)/utils/win/SkDWriteGeometrySink.h',
    '<(skia_src_path)/utils/win/SkHRESULT.cpp',
    '<(skia_src_path)/utils/win/SkIStream.cpp',
    '<(skia_src_path)/utils/win/SkWGL_win.cpp',

    #testing
    '<(skia_src_path)/fonts/SkGScalerContext.cpp',
    '<(skia_src_path)/fonts/SkGScalerContext.h',
  ],
}
