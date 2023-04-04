#include "gfxUtils.h"
#include "fileLoaders/plyModel.h"

#include "fileLoaders/stb/stb_image.h"

#include <iostream>
//#include <fstream>

#include <string>

#include <stdio.h>

#include <glad/glad.h>
#include "gfxAPI/checkErrorGL.h"

#include <thread>
#include <chrono>

#include <cassert>

namespace {

    static Status_t loadFile( const std::string& filepath, std::string& fileContent ) {
        FILE* fp = fopen( filepath.c_str(), "r" );
        fseek( fp, 0L, SEEK_END );
        const auto fileSizeBytes = ftell( fp );
        fseek( fp, 0L, SEEK_SET );
        
        fileContent.reserve( fileSizeBytes );

        for (;;) {
            const auto readChar = fgetc( fp );
            if (readChar == EOF) { break; }

            fileContent.push_back( static_cast< char >( readChar ) );
        }

        fileContent.push_back( '\0' );

        return Status_t::OK();
    }
}

Status_t gfxUtils::createShader( GfxAPI::Shader& shaderProgram, const std::vector< std::pair< gfxUtils::srcStr_t, GfxAPI::Shader::eShaderStage > >& shaderBuildInfo ) {
    for (const auto& buildInfo : shaderBuildInfo) {
        const auto& shaderStageString = buildInfo.first;
        const auto& shaderStage = buildInfo.second;
        shaderProgram.addShaderStage( shaderStage, shaderStageString );
    }
    return shaderProgram.build();
}

Status_t gfxUtils::createShader( GfxAPI::Shader& shaderProgram, const std::vector< std::pair< gfxUtils::path_t, GfxAPI::Shader::eShaderStage > >& shaderBuildInfo ) {
    
    for ( const auto& buildInfo : shaderBuildInfo ) {
        const auto& filePath = buildInfo.first;
        const auto& shaderStage = buildInfo.second;

        Status_t fileReadResult = Status_t::OK();
        std::string shaderStageString = "";
    #if defined( VERBOSE_GFX_DEBUG )
        printf( "now reading file %s\n", filePath.c_str() );
    #endif
        fileReadResult = loadFile( filePath.string(), shaderStageString );
        //printf( "--------------\n%s\n--------------\n", shaderStageString.c_str() );
        assert( fileReadResult == Status_t::OK() );

        shaderProgram.addShaderStage( shaderStage, shaderStageString );
    }

    return shaderProgram.build();
}

gfxUtils::bufferHandles_t gfxUtils::createScreenTriGfxBuffers() { // screen quad

    // this can be calculated in the shader anyway
    float vertices[] = {
        -1.0f, +1.0f, 0.0f,  // top left
        -1.0f, -3.0f, 0.0f,  // bottom left
        +3.0f, -1.0f, 0.0f,  // top right
    };

    uint32_t screenVAO;
    glGenVertexArrays( 1, &screenVAO ); 
    uint32_t VBO;
    glGenBuffers( 1, &VBO );

    // 1. bind Vertex Array Object
    glBindVertexArray( screenVAO );

    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    // 1. then set the vertex attributes pointers
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), reinterpret_cast< void* >( 0 ) );

    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    return bufferHandles_t{
        .vaoHandle = screenVAO,
        .vboHandles = std::vector< uint32_t >{ VBO },
        .eboHandle = static_cast<uint32_t>(-1),
    };
}

gfxUtils::bufferHandles_t gfxUtils::createScreenQuadGfxBuffers() { // screen quad

    float vertices[] = {
        -1.0f, +1.0f, 0.0f,  // top left
        -1.0f, -1.0f, 0.0f,  // bottom left
        +1.0f, -1.0f, 0.0f,  // bottom right
        +1.0f, +1.0f, 0.0f   // top right
    };
    uint16_t indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        2    // second triangle
    };

    uint32_t screenVAO;
    glGenVertexArrays( 1, &screenVAO ); 
    uint32_t VBO;
    glGenBuffers( 1, &VBO );
    uint32_t EBO;
    glGenBuffers(1, &EBO);

    // 1. bind Vertex Array Object
    glBindVertexArray( screenVAO );

    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    // 1. then set the vertex attributes pointers
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), reinterpret_cast< void* >( 0 ) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );

    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    return bufferHandles_t{
        .vaoHandle = screenVAO,
        .vboHandles = std::vector< uint32_t >{ VBO },
        .eboHandle = EBO,
    };
}

gfxUtils::bufferHandles_t gfxUtils::createMeshGfxBuffers(
    const uint32_t numVertexCoordVec3s,
    const float* const pVertexCoordFloats,
    const uint32_t numNormalVec3s,
    const float* const pNormalFloats,
    const uint32_t numIndices,
    const uint32_t* const pIndices ) {

    uint32_t stlModel_VAO;
    glGenVertexArrays(1, &stlModel_VAO);

    uint32_t stlCoords_VBO;
    glGenBuffers(1, &stlCoords_VBO);

    uint32_t stlNormals_VBO;
    glGenBuffers(1, &stlNormals_VBO);

    uint32_t stlModel_EBO;
    glGenBuffers(1, &stlModel_EBO);

    glBindVertexArray(stlModel_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, stlCoords_VBO);
    const size_t numBytes = numVertexCoordVec3s * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numBytes, pVertexCoordFloats, GL_STATIC_DRAW);
    const uint32_t attribIdx = 0;
    const int32_t components = 3;
    glVertexAttribPointer(attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(attribIdx);

    glBindBuffer(GL_ARRAY_BUFFER, stlNormals_VBO);
    const size_t numNormalBytes = numNormalVec3s * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numNormalBytes, pNormalFloats, GL_STATIC_DRAW);
    const uint32_t normalAttribIdx = 1;
    glVertexAttribPointer(normalAttribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(normalAttribIdx);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stlModel_EBO);
    {
        const size_t numBytes = numIndices * sizeof( uint32_t );
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numBytes, pIndices, GL_STATIC_DRAW);
    }

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return bufferHandles_t{
        .vaoHandle = stlModel_VAO,
        .vboHandles = std::vector< uint32_t >{ stlCoords_VBO, stlNormals_VBO },
        .eboHandle = stlModel_EBO,
    };
}

gfxUtils::bufferHandles_t gfxUtils::createMeshGfxBuffers(
    const uint32_t numVertexCoordVec3s,
    const std::vector< float >& vertexCoordFloats,
    const uint32_t numNormalVec3s,
    const std::vector< float >& normalFloats,
    const uint32_t numIndices,
    const std::vector< uint32_t >& indices ) {

    return createMeshGfxBuffers(
        numVertexCoordVec3s,
        vertexCoordFloats.data(),
        numNormalVec3s,
        normalFloats.data(),
        numIndices,
        indices.data() );
}

gfxUtils::bufferHandles_t gfxUtils::createCylinderMeshGfxBuffer(    const linAlg::vec3_t& startPos, 
                                                                    const linAlg::vec3_t& axis, 
                                                                    const float btmRadius, 
                                                                    const float topRadius, 
                                                                    const float lenAlongAxis, 
                                                                    const uint32_t circleSegments,
                                                                    uint32_t& outNumIndices ) {
    
    uint32_t cylinder_VAO;
    glGenVertexArrays(1, &cylinder_VAO);

    uint32_t cylinderCoords_VBO;
    glGenBuffers(1, &cylinderCoords_VBO);

    uint32_t cylinderNormals_VBO;
    glGenBuffers(1, &cylinderNormals_VBO);

    uint32_t cylinder_EBO;
    glGenBuffers(1, &cylinder_EBO);

    glBindVertexArray(cylinder_VAO);

    std::vector< linAlg::vec3_t > cylinderVerts;
    cylinderVerts.reserve( ( circleSegments + 1 ) * 2 * 3 * 4 ); // TODO: calculate exactly, but i'm too tired already
    

    // bottom and top circles lie in planes that the axis is perpendicular to
    const linAlg::vec3_t absAxis = { fabsf( axis[0] ), fabsf( axis[1] ), fabsf( axis[2] ) };
    linAlg::vec3_t tmpAxis = { 1.0f, 0.0f, 0.0f };
    if ( absAxis[0] > absAxis[1] ) {
        if (absAxis[0] > absAxis[2]) {
            // absAxis[0] has largest magnitude
            tmpAxis = { 0.0f, 1.0f, 1.0f };
        }
        else {
            // absAxis[2] has largest magnitude
            tmpAxis = { 1.0f, 0.0f, 0.0f };
        }
    }
    else {
        if (absAxis[1] > absAxis[2]) {
            // absAxis[1] has largest magnitude
            tmpAxis = { 0.0f, 0.0f, 1.0f };
        }
        else {
            // absAxis[2] has largest magnitude
            tmpAxis = { 1.0f, 0.0f, 0.0f };
        }
    }

    linAlg::vec3_t axisNormal;
    linAlg::cross( axisNormal, tmpAxis, axis );
    linAlg::normalize( axisNormal );

    linAlg::vec3_t axisNormal2;
    linAlg::cross( axisNormal2, axisNormal, axis );
    linAlg::normalize( axisNormal2 );

    std::vector<uint32_t> indicesCylinder;

    // bottom and top circles span the cylinder's revolution surface

    const float angleInc = 2.0f * M_PI / static_cast<float>( circleSegments );
    const auto btmCircleCenterPos = startPos;
    const auto topCircleCenterPos = btmCircleCenterPos + axis * lenAlongAxis;

    cylinderVerts.push_back( btmCircleCenterPos );

    // draw as indexed triangles 

    for (uint32_t i = 0; i <= circleSegments; i++) {
        const float angRad = i * angleInc;
        const float cosAng = cosf( angRad ) * btmRadius;
        const float sinAng = sinf( angRad ) * btmRadius;

        const auto btmCirclePos = btmCircleCenterPos + cosAng * axisNormal + sinAng * axisNormal2;

        cylinderVerts.push_back( btmCirclePos );
        
        indicesCylinder.push_back( 0 );
        indicesCylinder.push_back( i + 1 );
        indicesCylinder.push_back( ( i + 1 ) % circleSegments + 1 );
    }

    const uint32_t topStartIdx = cylinderVerts.size();
    cylinderVerts.push_back( topCircleCenterPos );

    for (uint32_t i = 0; i <= circleSegments; i++) {
        const float angRad = i * angleInc;
        const float cosAng = cosf( angRad ) * btmRadius;
        const float sinAng = sinf( angRad ) * btmRadius;

        const auto topCirclePos = topCircleCenterPos + cosAng * axisNormal + sinAng * axisNormal2;

        cylinderVerts.push_back( topCirclePos );

        indicesCylinder.push_back( topStartIdx + 0 );
        indicesCylinder.push_back( topStartIdx + i + 1 );
        indicesCylinder.push_back( topStartIdx + ( i + 1 ) % circleSegments + 1 );
    }

    for (uint32_t i = 0; i <= circleSegments; i++) {
        indicesCylinder.push_back( i + 1 );
        indicesCylinder.push_back( topStartIdx + ( i + 1 ) % circleSegments + 1 );
        indicesCylinder.push_back( topStartIdx + i + 1 );

        indicesCylinder.push_back( i + 1 );
        indicesCylinder.push_back( ( i + 1 ) % circleSegments + 1 );
        indicesCylinder.push_back( topStartIdx + ( i + 1 ) % circleSegments + 1 );
    }


    const uint32_t numVertexCoordVec3s  = static_cast<uint32_t>( cylinderVerts.size() );
    const uint32_t numNormalVec3s       = numVertexCoordVec3s;

    const float* const pVertexCoordFloats = reinterpret_cast<const float* const>( cylinderVerts.data() );
    const float* const pNormalFloats = pVertexCoordFloats;

    const uint32_t* const pIndices = reinterpret_cast<const uint32_t* const>( indicesCylinder.data() );
    const uint32_t numIndices = static_cast<uint32_t>(indicesCylinder.size());

    outNumIndices = numIndices;

    glBindBuffer(GL_ARRAY_BUFFER, cylinderCoords_VBO);
    const size_t numBytes = numVertexCoordVec3s * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numBytes, pVertexCoordFloats, GL_STATIC_DRAW);
    const uint32_t attribIdx = 0;
    const int32_t components = 3;
    glVertexAttribPointer(attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(attribIdx);

    glBindBuffer(GL_ARRAY_BUFFER, cylinderNormals_VBO);
    const size_t numNormalBytes = numNormalVec3s * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numNormalBytes, pNormalFloats, GL_STATIC_DRAW);
    const uint32_t normalAttribIdx = 1;
    glVertexAttribPointer(normalAttribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(normalAttribIdx);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_EBO);
    {
        const size_t numBytes = numIndices * sizeof( uint32_t );
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numBytes, pIndices, GL_STATIC_DRAW);
    }

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return bufferHandles_t{
        .vaoHandle = cylinder_VAO,
        .vboHandles = std::vector< uint32_t >{ cylinderCoords_VBO, cylinderNormals_VBO },
        .eboHandle = cylinder_EBO,
    };
}

void gfxUtils::freeMeshGfxBuffers( gfxUtils::bufferHandles_t& bufferHandles ) {
    glDeleteVertexArrays(1, &bufferHandles.vaoHandle);
    glDeleteBuffers( static_cast<GLsizei>(bufferHandles.vboHandles.size()), bufferHandles.vboHandles.data() );
    glDeleteBuffers( 1, &bufferHandles.eboHandle );
}

uint32_t gfxUtils::createPlyModelGfxBuffers( 
    const PlyModel& plyModel, 
    size_t& numPlyIndices, 
    size_t& elementDrawCallGlType, 
    std::array< uint32_t, gfxUtils::numPosCoords + gfxUtils::numNormalCoords >* pOrigMeshBufferTex ) {

    uint32_t plyModel_VAO;
    glGenVertexArrays( 1, &plyModel_VAO );  

    uint32_t plyCoords_VBO[ 3 ];
    glGenBuffers( 3, plyCoords_VBO );

    uint32_t plyNormals_VBO[ 3 ];
    glGenBuffers( 3, plyNormals_VBO );

    uint32_t plyColors_VBO[ 3 ];
    glGenBuffers( 3, plyColors_VBO );

    uint32_t plyModel_EBO;
    glGenBuffers( 1, &plyModel_EBO );

    glBindVertexArray( plyModel_VAO );

    std::array< const char *const, 3 > propertyCoordNames{ "x", "y", "z" };
    for ( int32_t i = 0; i < propertyCoordNames.size(); i++ ) {
        size_t numElements;
        const auto *const pPlyVertDesc = plyModel.getPropertyByName( propertyCoordNames[ i ], numElements );
        if ( pPlyVertDesc != nullptr ) {
            glBindBuffer( GL_ARRAY_BUFFER, plyCoords_VBO[ i ] );
            const size_t numBytes = numElements * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyVertDesc->dataType ) ];
            glBufferData( GL_ARRAY_BUFFER, numBytes, pPlyVertDesc->data.data(), GL_STATIC_DRAW );
            const uint32_t attribIdx = i;
            const int32_t components = 1;
            assert( pPlyVertDesc->dataType == PlyModel::eDataType::f32 );
            glVertexAttribPointer( attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0 ); // the last two zeros mean "tightly packed"
            glEnableVertexAttribArray( attribIdx );
        }
    }
#if 1 // load vertex normals !!!
    std::array< const char *const, 3 > propertyNormalNames{ "nx", "ny", "nz" };
    for ( size_t i = 0; i < propertyNormalNames.size(); i++ ) {
        size_t numElements;
        const auto *const pPlyNormalsDesc = plyModel.getPropertyByName( propertyNormalNames[ i ], numElements );
        if ( pPlyNormalsDesc != nullptr ) {
            glBindBuffer( GL_ARRAY_BUFFER, plyNormals_VBO[ i ] );
            const size_t numBytes = numElements * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyNormalsDesc->dataType ) ];
            glBufferData( GL_ARRAY_BUFFER, numBytes, pPlyNormalsDesc->data.data(), GL_STATIC_DRAW );
            const uint32_t attribIdx = static_cast<uint32_t>( i + propertyCoordNames.size() );
            const int32_t components = 1;
            assert( pPlyNormalsDesc->dataType == PlyModel::eDataType::f32 );
            glVertexAttribPointer( attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0 ); // the last two zeros mean "tightly packed"
            glEnableVertexAttribArray( attribIdx );
        }
    }
#endif
#if 1
    std::array< const char *const, 3 > propertyColorNames{ "red", "green", "blue" };
    for ( size_t i = 0; i < propertyColorNames.size(); i++ ) {
        size_t numElements;
        const auto *const pPlyVertDesc = plyModel.getPropertyByName( propertyColorNames[ i ], numElements );
        if ( pPlyVertDesc != nullptr ) {
            glBindBuffer( GL_ARRAY_BUFFER, plyColors_VBO[ i ] );
            const size_t numBytes = numElements * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyVertDesc->dataType ) ];
            glBufferData( GL_ARRAY_BUFFER, numBytes, pPlyVertDesc->data.data(), GL_STATIC_DRAW );
            const uint32_t attribIdx = static_cast<uint32_t>( i + propertyCoordNames.size() + propertyNormalNames.size() ); // offset by the amount of vertex coord attribs used
            const int32_t components = 1;
            assert( pPlyVertDesc->dataType == PlyModel::eDataType::u8 );
            glVertexAttribPointer( attribIdx, components, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0 ); // the last two zeros mean "tightly packed"
            //glVertexAttribIPointer() // <== for int attribs!!!
            glEnableVertexAttribArray( attribIdx );
        }
    }
#endif

    // create buffer texture for each plyCoords_VBO and plyNormals_VBO
    if ( pOrigMeshBufferTex != nullptr ) { 
        glGenTextures( static_cast<GLsizei>(pOrigMeshBufferTex->size()), pOrigMeshBufferTex->data() );
        for ( int32_t i = 0; i < propertyCoordNames.size(); i++ ) { 
            glBindTexture( GL_TEXTURE_BUFFER, pOrigMeshBufferTex->data()[ i ] );
            glTexBuffer( GL_TEXTURE_BUFFER, GL_R32F, plyCoords_VBO[ i ] );

            // TODO: assert( GL_MAX_TEXTURE_BUFFER_SIZE < maxEntries ) // we won't be able to access more than GL_MAX_TEXTURE_BUFFER_SIZE texels of the VBO through this TBO
        }
        for ( int32_t i = 0; i < propertyNormalNames.size(); i++ ) { 
            glBindTexture( GL_TEXTURE_BUFFER, pOrigMeshBufferTex->data()[ i + propertyCoordNames.size() ] );
            glTexBuffer( GL_TEXTURE_BUFFER, GL_R32F, plyNormals_VBO[ i ] );
        }
        glBindTexture( GL_TEXTURE_BUFFER, 0 );
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, plyModel_EBO );
    const auto *const pPlyIndicesDesc = plyModel.getPropertyByName( "vertex_indices", numPlyIndices );
    assert( pPlyIndicesDesc->isList );
    numPlyIndices *= pPlyIndicesDesc->numListElements;
    elementDrawCallGlType = 0;
    if ( pPlyIndicesDesc->dataType == PlyModel::eDataType::u16 ) { elementDrawCallGlType = GL_UNSIGNED_SHORT; }
    else if ( pPlyIndicesDesc->dataType == PlyModel::eDataType::i16 ) { elementDrawCallGlType = /*GL_SHORT*/GL_UNSIGNED_SHORT; }
    else if ( pPlyIndicesDesc->dataType == PlyModel::eDataType::u32 ) { elementDrawCallGlType = GL_UNSIGNED_INT; }
    else if ( pPlyIndicesDesc->dataType == PlyModel::eDataType::i32 ) { elementDrawCallGlType = /*GL_INT*/GL_UNSIGNED_INT; }
    {
        const size_t numBytes = numPlyIndices * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyIndicesDesc->dataType ) ];
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, numBytes, pPlyIndicesDesc->data.data(), GL_STATIC_DRAW );        
    }
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray( 0 ); 

    glBindBuffer( GL_ARRAY_BUFFER, 0 ); 
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); 

    glDeleteBuffers(3, plyCoords_VBO);
    glDeleteBuffers(3, plyNormals_VBO);
    glDeleteBuffers(3, plyColors_VBO);
    glDeleteBuffers(1, &plyModel_EBO);

    return plyModel_VAO;
}

void gfxUtils::createModelViewMatrixForModel( 
    const PlyModel& plyModel, 
    const float azimuthRadians, 
    const float elevationRadians,
    const float distFactor, 
    linAlg::mat4_t& modelViewMatrix ) {

    linAlg::mat3x4_t centerTranslationMatrix;
    linAlg::vec4_t boundingSphere;
    plyModel.getBoundingSphere( boundingSphere );
    linAlg::loadTranslationMatrix( centerTranslationMatrix, linAlg::vec3_t{ -boundingSphere[ 0 ], -boundingSphere[ 1 ], -boundingSphere[ 2 ] } );

    constexpr float modelScaleFactor = 0.02f;
    linAlg::mat3x4_t scaleMatrix;
    linAlg::loadScaleMatrix( scaleMatrix, linAlg::vec3_t{ modelScaleFactor, modelScaleFactor, modelScaleFactor } );

    linAlg::mat3x4_t viewTranslationMatrix;
    // TODO: calc required translation based on FOV and bounding-sphere size
    linAlg::loadTranslationMatrix( viewTranslationMatrix, linAlg::vec3_t{ 0.0f, 0.0f, -boundingSphere[ 3 ] * modelScaleFactor * distFactor } );

    linAlg::mat3x4_t modelMatrix3x4;
    linAlg::mat3x4_t tmpModelMatrixPre;
    
    linAlg::multMatrix( tmpModelMatrixPre, scaleMatrix, centerTranslationMatrix );

    linAlg::mat3x4_t azimuthRotMatrix;
    linAlg::loadRotationYMatrix( azimuthRotMatrix, azimuthRadians );

    linAlg::mat3x4_t elevationRotMatrix;
    linAlg::loadRotationXMatrix( elevationRotMatrix, elevationRadians );

    linAlg::mat3x4_t viewRotMatrix;
    linAlg::mat3x4_t tmpViewRotMatrix;

    linAlg::multMatrix( tmpViewRotMatrix, azimuthRotMatrix, tmpModelMatrixPre );
    linAlg::multMatrix( viewRotMatrix, elevationRotMatrix, tmpViewRotMatrix );
    
    linAlg::multMatrix( modelMatrix3x4, viewTranslationMatrix, viewRotMatrix );

    linAlg::castMatrix( modelViewMatrix, modelMatrix3x4 );
}

Status_t gfxUtils::loadImageIntoArray( const std::string& imgFilePath, 
                                       std::vector< uint8_t >& imgData, 
                                       int32_t& imgW, 
                                       int32_t& imgH, 
                                       int32_t& imgNumChannels ) {
#if defined( VERBOSE_GFX_DEBUG )
    printf( "loading image '%s'\n", imgFilePath.c_str() );
#endif
    stbi_set_flip_vertically_on_load( 1 );
    uint8_t* pData = stbi_load( imgFilePath.c_str(), &imgW, &imgH, &imgNumChannels, 0 );
    imgData.resize( imgW * imgH * imgNumChannels );
    std::copy( pData, pData + imgW * imgH * imgNumChannels, imgData.begin() );
    stbi_image_free( pData );
    return Status_t::OK();
}

Status_t gfxUtils::createTexFromImage(  const std::string& imgFilePath, 
                                        std::vector< uint32_t >& imgTex, 
                                        int32_t& imgW, 
                                        int32_t& imgH ) {
    stbi_set_flip_vertically_on_load( 1 );
    int32_t imgNumChannels;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // needed for RGB images with odd width
    uint8_t* pData = stbi_load( imgFilePath.c_str(), &imgW, &imgH, &imgNumChannels, 0 );

    glGenTextures( static_cast<GLsizei>( imgTex.size() ), imgTex.data() );
    for ( size_t i = 0; i < imgTex.size(); i++ ) {
        glBindTexture( GL_TEXTURE_2D, imgTex[ i ] );
        constexpr int32_t mipLvl = 0;

        int32_t memImgFormat = ( imgNumChannels == 3) ? GL_RGB : GL_RED;
        glTexImage2D( GL_TEXTURE_2D, mipLvl, GL_RGBA8, imgW, imgH, 0, memImgFormat, GL_UNSIGNED_BYTE, pData );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
        // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );

        constexpr float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor ); 

        glGenerateMipmap( GL_TEXTURE_2D );
    }
    glBindTexture( GL_TEXTURE_2D, 0 );

    stbi_image_free( pData );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 ); // needed for RGB images with odd width
    return Status_t::OK();        
}

void gfxUtils::limitFrameRate( double deltaFrame_s, const float maxFrameRate /*= 60.0f*/ ) {
    static const float duration_maxFrameRate_ms = 1000.0f / maxFrameRate;
    clock_t sleep_time_ms = static_cast<clock_t>(
        duration_maxFrameRate_ms -
        (deltaFrame_s * 1000.0)
        - 0.1 ); // account for small amount of overhead to not go overboard with idling

    if (sleep_time_ms > 0) {
        std::this_thread::sleep_for
        ( std::chrono::milliseconds( sleep_time_ms ) );
    }
}

void gfxUtils::screenToWorld(   linAlg::vec3_t& posWS, 
                                const float currMouseX, 
                                const float currMouseY, 
                                const linAlg::mat3x4_t& viewMat, 
                                const linAlg::mat4_t& projMat4, 
                                const int32_t& fbWidth, 
                                const int32_t& fbHeight ) {

    glFlush();
    glFinish();

    glCheckError();
#if (VERBOSE_DEBUG != 0)
    printf( "\n" );
#endif

    glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
    if (glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE) {
    #if (VERBOSE_DEBUG != 0)
        printf( "framebuffer not complete!\n" );
    #endif
    }
    glReadBuffer( GL_BACK );

    const GLint glReadX = static_cast<GLint>(currMouseX); 
    const GLint glReadY = (fbHeight - 1) - static_cast<GLint>(currMouseY); // flip y for reading from framebuffer

    const float fReadWindowRelativeX = ( currMouseX / static_cast<float>( fbWidth - 1 ) );
    const float fReadWindowRelativeY = ( currMouseY / static_cast<float>( fbHeight - 1 ) );

    float depthAtMousePos = 0.0f;
    glReadPixels( glReadX, glReadY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthAtMousePos );

#if (VERBOSE_DEBUG != 0)
    printf( "mouse read depth = %f at (%f, %f) for window size (%d, %d)\n", depthAtMousePos, currMouseX, currMouseY, fbWidth, fbHeight );
#endif

    linAlg::vec4_t mousePosNDC{ fReadWindowRelativeX * 2.0f - 1.0f, ( 1.0f - fReadWindowRelativeY ) * 2.0f - 1.0f, depthAtMousePos * 2.0f - 1.0f, 1.0f };
    //printVec( "mousePosNDC", mousePosNDC );

    linAlg::mat4_t viewMat4;
    linAlg::castMatrix( viewMat4, viewMat );

    linAlg::mat4_t viewProjMat4; 
    linAlg::multMatrix( viewProjMat4, projMat4, viewMat4 );

    linAlg::mat4_t invViewProjMat4; 
    linAlg::inverse( invViewProjMat4, viewProjMat4 );

    linAlg::vec4_t mousePosWS = invViewProjMat4 * mousePosNDC;


    mousePosWS[3] = linAlg::maximum( mousePosWS[3], std::numeric_limits<float>::epsilon() );
    mousePosWS[0] /= mousePosWS[3];
    mousePosWS[1] /= mousePosWS[3];
    mousePosWS[2] /= mousePosWS[3];


    posWS = { mousePosWS[0], mousePosWS[1], mousePosWS[2] };
    linAlg::printVec( "picked WS pos", posWS );

    glCheckError();
#if (VERBOSE_DEBUG != 0)
    printf( "\n" );
#endif

}
