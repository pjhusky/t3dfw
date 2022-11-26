#include "gfxUtils.h"
#include "fileLoaders/plyModel.h"

#include "fileLoaders/stb/stb_image.h"

#include <iostream>
//#include <fstream>

#include <string>

#include <stdio.h>

#include <glad/glad.h>

#include <cassert>

namespace {
    //static Status_t loadFile( const std::string& filepath, std::string& fileContent ) {
    //    std::ifstream fileStream( filepath );
    //    if ( !fileStream.good() ) { return Status_t::ERROR(); }

    //    fileContent = std::string{ 
    //        std::istreambuf_iterator< char >( fileStream ),
    //        std::istreambuf_iterator< char >() };

    //    return Status_t::OK();
    //}

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

Status_t gfxUtils::createShader( Shader& shaderProgram, const std::vector< std::pair< path_t, Shader::eShaderStage > >& shaderBuildInfo ) {
    
    for ( const auto& buildInfo : shaderBuildInfo ) {
        const auto& filePath = buildInfo.first;
        const auto& shaderStage = buildInfo.second;

        Status_t fileReadResult = Status_t::OK();
        std::string shaderStageString = "";
        printf( "now reading file %s\n", filePath.c_str() );
        fileReadResult = loadFile( filePath, shaderStageString );
        //printf( "--------------\n%s\n--------------\n", shaderStageString.c_str() );
        assert( fileReadResult == Status_t::OK() );

        shaderProgram.addShaderStage( shaderStage, shaderStageString );
    }

    return shaderProgram.build();

    //return Status_t::OK();
}

uint32_t gfxUtils::createScreenQuadGfxBuffers() { // screen quad
    
    //float vertices[] = {
    //    -1.0f, +1.0f, 1.0f,  // top right
    //    -1.0f, -1.0f, 1.0f,  // bottom right
    //    +1.0f, -1.0f, 1.0f,  // bottom left
    //    +1.0f, +1.0f, 1.0f   // top left 
    //};
    //uint32_t indices[] = {  // note that we start from 0!
    //    0, 1, 2,   // first triangle
    //    0, 2, 3    // second triangle
    //}; 

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
    //glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );

    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    return screenVAO;
}

uint32_t gfxUtils::createMeshGfxBuffers(
    const size_t& numVertexCoordVec3s,
    const std::vector< float >& vertexCoordFloats,
    const size_t& numNormalVec3s,
    const std::vector< float >& normalFloats,
    const size_t& numIndices,
    const std::vector< uint32_t >& indices ) {

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
    glBufferData(GL_ARRAY_BUFFER, numBytes, vertexCoordFloats.data(), GL_STATIC_DRAW);
    const uint32_t attribIdx = 0;
    const int32_t components = 3;
    glVertexAttribPointer(attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(attribIdx);
    
    glBindBuffer(GL_ARRAY_BUFFER, stlNormals_VBO);
    const size_t numNormalBytes = numNormalVec3s * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numNormalBytes, normalFloats.data(), GL_STATIC_DRAW);
    const uint32_t normalAttribIdx = 1;
    glVertexAttribPointer(normalAttribIdx, components, GL_FLOAT, GL_FALSE, 0, 0); // the last two zeros mean "tightly packed"
    glEnableVertexAttribArray(normalAttribIdx);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stlModel_EBO);
    {
        const size_t numBytes = numIndices * sizeof( uint32_t );
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numBytes, indices.data(), GL_STATIC_DRAW);
    }

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &stlCoords_VBO);
    glDeleteBuffers(1, &stlNormals_VBO);
    glDeleteBuffers(1, &stlModel_EBO);

    return stlModel_VAO;

}

void gfxUtils::freeMeshGfxBuffers(uint32_t vaoHandle) {
    glDeleteVertexArrays(1, &vaoHandle);
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
    for ( int32_t i = 0; i < propertyNormalNames.size(); i++ ) {
        size_t numElements;
        const auto *const pPlyNormalsDesc = plyModel.getPropertyByName( propertyNormalNames[ i ], numElements );
        if ( pPlyNormalsDesc != nullptr ) {
            glBindBuffer( GL_ARRAY_BUFFER, plyNormals_VBO[ i ] );
            const size_t numBytes = numElements * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyNormalsDesc->dataType ) ];
            glBufferData( GL_ARRAY_BUFFER, numBytes, pPlyNormalsDesc->data.data(), GL_STATIC_DRAW );
            const uint32_t attribIdx = i + propertyCoordNames.size();
            const int32_t components = 1;
            assert( pPlyNormalsDesc->dataType == PlyModel::eDataType::f32 );
            glVertexAttribPointer( attribIdx, components, GL_FLOAT, GL_FALSE, 0, 0 ); // the last two zeros mean "tightly packed"
            glEnableVertexAttribArray( attribIdx );
        }
    }
#endif
#if 1
    std::array< const char *const, 3 > propertyColorNames{ "red", "green", "blue" };
    for ( int32_t i = 0; i < propertyColorNames.size(); i++ ) {
        size_t numElements;
        const auto *const pPlyVertDesc = plyModel.getPropertyByName( propertyColorNames[ i ], numElements );
        if ( pPlyVertDesc != nullptr ) {
            glBindBuffer( GL_ARRAY_BUFFER, plyColors_VBO[ i ] );
            const size_t numBytes = numElements * PlyModel::dataTypeNumBytes[ static_cast< int32_t >( pPlyVertDesc->dataType ) ];
            glBufferData( GL_ARRAY_BUFFER, numBytes, pPlyVertDesc->data.data(), GL_STATIC_DRAW );
            const uint32_t attribIdx = i + propertyCoordNames.size() + propertyNormalNames.size(); // offset by the amount of vertex coord attribs used
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
        glGenTextures( pOrigMeshBufferTex->size(), pOrigMeshBufferTex->data() );
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

    linAlg::cast( modelViewMatrix, modelMatrix3x4 );
}

Status_t gfxUtils::loadImageIntoArray( const std::string& imgFilePath, 
                                       std::vector< uint8_t >& imgData, 
                                       int32_t& imgW, 
                                       int32_t& imgH, 
                                       int32_t& imgNumChannels ) {
    printf( "loading image '%s'\n", imgFilePath.c_str() );
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

    glGenTextures( imgTex.size(), imgTex.data() );
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
