#ifndef _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
#define _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759

#include "statusType.h"
#include "math/linAlg.h"
#include "gfxAPI/shader.h"

#include <stdint.h>
#include <stddef.h>
#include <array>
#include <vector>

namespace FileLoader {
    struct PlyModel;
}

using namespace GfxAPI;
using namespace FileLoader;

struct gfxUtils {

    using path_t = std::string;

    static Status_t createShader( Shader& shaderProgram, const std::vector< std::pair< path_t, Shader::eShaderStage > >& shaderBuildInfo );

    static uint32_t createScreenQuadGfxBuffers();

    static constexpr size_t numPosCoords = 3;
    static constexpr size_t numNormalCoords = 3;
    static uint32_t createPlyModelGfxBuffers( 
        const PlyModel& plyModel, 
        size_t& numPlyIndices, 
        size_t& elementDrawCallGlType, 
        std::array< uint32_t, numPosCoords + numNormalCoords >* pOrigMeshBufferTex = nullptr );

    static void createModelViewMatrixForModel( 
        const PlyModel& plyModel, 
        const float azimuthRadians, 
        const float elevationRadians, 
        const float distFactor,
        linAlg::mat4_t& modelViewMatrix );

    static uint32_t createMeshGfxBuffers(
        const size_t& numVertexCoordVec3s,
        const std::vector< float >& vertexCoordFloats, 
        const size_t& numNormalVec3s,
        const std::vector< float >& normalFloats,
        const size_t& numIndices,
        const std::vector< uint32_t >& indices );
    
    static void freeMeshGfxBuffers( uint32_t vaoHandle );

    static Status_t loadImageIntoArray( const std::string& imgFilePath, 
                                        std::vector< uint8_t >& imgData, 
                                        int32_t& imgW, 
                                        int32_t& imgH, 
                                        int32_t& imgNumChannels );

    static Status_t createTexFromImage( const std::string& imgFilePath, 
                                        std::vector< uint32_t >& imgTex, 
                                        int32_t& imgW, 
                                        int32_t& imgH );

    static int32_t glCheckError_( const char *file, const int line );


    static void limitFrameRate( double deltaFrame_s, const float maxFrameRate = 60.0f );
};

#endif // _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
