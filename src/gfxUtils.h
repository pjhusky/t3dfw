#ifndef _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
#define _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759

#include "statusType.h"
#include "math/linAlg.h"
#include "gfxAPI/shader.h"

#include <filesystem>
#include <stdint.h>
#include <stddef.h>
#include <array>
#include <vector>

namespace FileLoader {
    struct PlyModel;
}

using namespace FileLoader;

struct gfxUtils {

    struct bufferHandles_t {
        uint32_t vaoHandle;
        std::vector< uint32_t > vboHandles;
        uint32_t eboHandle;
    };

    using path_t = std::filesystem::path;
    using srcStr_t = std::string;

    static Status_t createShader( GfxAPI::Shader& shaderProgram, const std::vector< std::pair< srcStr_t, GfxAPI::Shader::eShaderStage > >& shaderBuildInfo );
    static Status_t createShader( GfxAPI::Shader& shaderProgram, const std::vector< std::pair< path_t, GfxAPI::Shader::eShaderStage > >& shaderBuildInfo );

    static bufferHandles_t createScreenTriGfxBuffers();
    static bufferHandles_t createScreenQuadGfxBuffers();

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

    static bufferHandles_t createMeshGfxBuffers(
        const uint32_t numVertexCoordVec3s,
        const float *const pVertexCoordFloats, 
        const uint32_t numNormalVec3s,
        const float *const pNormalFloats,
        const uint32_t numIndices,
        const uint32_t *const pIndices );

    static bufferHandles_t createMeshGfxBuffers(
        const uint32_t numVertexCoordVec3s,
        const std::vector< float >& vertexCoordFloats, 
        const uint32_t numNormalVec3s,
        const std::vector< float >& normalFloats,
        const uint32_t numIndices,
        const std::vector< uint32_t >& indices );

    static bufferHandles_t createCylinderMeshGfxBuffer(
        const linAlg::vec3_t& startPos,
        const linAlg::vec3_t& axis,
        const float btmRadius,
        const float topRadius,
        const float lenAlongAxis,
        const uint32_t circleSegments,
        uint32_t& outNumIndices );

    static void freeMeshGfxBuffers( bufferHandles_t& bufferHandles );

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

    static void screenToWorld(
        linAlg::vec3_t& posWS,
        const float currMouseX, const float currMouseY,
        const linAlg::mat3x4_t& viewMat,
        const linAlg::mat4_t& projMat4,
        const int32_t& fbWidth, const int32_t& fbHeight );

};

#endif // _GFXUTILS_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
