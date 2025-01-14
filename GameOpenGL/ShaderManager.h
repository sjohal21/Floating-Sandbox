/***************************************************************************************
* Original Author:      Gabriele Giuseppini
* Created:              2018-10-03
* Copyright:            Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include "GameOpenGL.h"

#include <GameCore/Vectors.h>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

template <typename Traits>
class ShaderManager
{
private:

    static constexpr GLint NoParameterLocation = std::numeric_limits<GLint>::min();

    template <typename T>
    static std::string ToString(T const & v)
    {
        return std::to_string(v);
    }

    std::string ToString(float const & v)
    {
        std::stringstream stream;
        stream << std::fixed << v;
        return stream.str();
    }

public:

    static std::unique_ptr<ShaderManager> CreateInstance(std::filesystem::path const & shadersRoot)
    {
        return std::unique_ptr<ShaderManager>(
            new ShaderManager(shadersRoot));
    }

    template <typename Traits::ProgramType Program>
    inline auto GetProgramOpenGLHandle()
    {
        uint32_t constexpr programIndex = static_cast<uint32_t>(Program);

        return *(mPrograms[programIndex].OpenGLHandle);
    }

    template <typename Traits::ProgramType Program>
    inline void SetTextureParameters()
    {
        size_t programIndex = static_cast<size_t>(Program);

        // Find all texture parameters
        for (size_t parameterIndex = 0; parameterIndex < mPrograms[programIndex].UniformLocations.size(); ++parameterIndex)
        {
            if (mPrograms[programIndex].UniformLocations[parameterIndex] != NoParameterLocation)
            {
                typename Traits::ProgramParameterType parameter = static_cast<typename Traits::ProgramParameterType>(parameterIndex);

                // See if it's a texture/sampler parameter
                if (parameter >= Traits::ProgramParameterType::_FirstTexture
                    && parameter <= Traits::ProgramParameterType::_LastTexture)
                {
                    //
                    // Set it
                    //

                    auto const textureUnitIndex = static_cast<uint8_t>(parameter) - static_cast<uint8_t>(Traits::ProgramParameterType::_FirstTexture);

                    assert(mPrograms[programIndex].UniformLocations[parameterIndex] != NoParameterLocation);

                    glUniform1i(
                        mPrograms[programIndex].UniformLocations[parameterIndex],
                        textureUnitIndex);

                    CheckUniformError(Program, parameter);
                }
            }
        }
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(float value)
    {
        SetProgramParameter<Parameter>(Program, value);
    }

    template <typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(
        typename Traits::ProgramType program,
        float value)
    {
        uint32_t const programIndex = static_cast<uint32_t>(program);
        uint32_t constexpr ParameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[ParameterIndex] != NoParameterLocation);

        glUniform1f(
            mPrograms[programIndex].UniformLocations[ParameterIndex],
            value);

        CheckUniformError(program, Parameter);
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(vec2f const & val)
    {
        SetProgramParameter<Program, Parameter>(val.x, val.y);
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(float val1, float val2)
    {
        constexpr uint32_t programIndex = static_cast<uint32_t>(Program);
        constexpr uint32_t parameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[parameterIndex] != NoParameterLocation);

        glUniform2f(
            mPrograms[programIndex].UniformLocations[parameterIndex],
            val1,
            val2);

        CheckUniformError<Program, Parameter>();
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(vec3f const & val)
    {
        SetProgramParameter<Parameter>(Program, val);
    }

    template <typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(
        typename Traits::ProgramType program,
        vec3f const & val)
    {
        uint32_t const programIndex = static_cast<uint32_t>(program);
        uint32_t constexpr ParameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[ParameterIndex] != NoParameterLocation);

        glUniform3f(
            mPrograms[programIndex].UniformLocations[ParameterIndex],
            val.x,
            val.y,
            val.z);

        CheckUniformError(program, Parameter);
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(vec4f const & val)
    {
        constexpr uint32_t programIndex = static_cast<uint32_t>(Program);
        constexpr uint32_t parameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[parameterIndex] != NoParameterLocation);

        glUniform4f(
            mPrograms[programIndex].UniformLocations[parameterIndex],
            val.x,
            val.y,
            val.z,
            val.w);

        CheckUniformError<Program, Parameter>();
    }

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(float const (*value)[4])
    {
        SetProgramParameter<Parameter>(Program, value);
    }

    template <typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameter(
        typename Traits::ProgramType program,
        float const (*value)[4])
    {
        uint32_t const programIndex = static_cast<uint32_t>(program);
        uint32_t constexpr ParameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[ParameterIndex] != NoParameterLocation);

        glUniformMatrix4fv(
            mPrograms[programIndex].UniformLocations[ParameterIndex],
            1,
            GL_FALSE,
            &(value[0][0]));

        CheckUniformError(program, Parameter);
    }

    template <typename Traits::ProgramParameterType Parameter>
    inline void SetProgramParameterVec4fArray(
        typename Traits::ProgramType program,
        vec4f const * array,
        size_t vectorCount)
    {
        uint32_t const programIndex = static_cast<uint32_t>(program);
        uint32_t constexpr ParameterIndex = static_cast<uint32_t>(Parameter);

        assert(mPrograms[programIndex].UniformLocations[ParameterIndex] != NoParameterLocation);

        glUniform4fv(
            mPrograms[programIndex].UniformLocations[ParameterIndex],
            static_cast<GLsizei>(vectorCount),
            reinterpret_cast<GLfloat const *>(array));
        
        CheckUniformError(program, Parameter);
    }

    // At any given moment, only one program may be active
    template <typename Traits::ProgramType Program>
    inline void ActivateProgram()
    {
        ActivateProgram(Program);
    }

    // At any given moment, only one program may be active
    inline void ActivateProgram(typename Traits::ProgramType program)
    {
        uint32_t const programIndex = static_cast<uint32_t>(program);

        glUseProgram(*(mPrograms[programIndex].OpenGLHandle));

        CheckOpenGLError();
    }

    // At any given moment, only one texture (unit) may be active
    template <typename Traits::ProgramParameterType Parameter>
    inline void ActivateTexture()
    {
        GLenum const textureUnit = static_cast<GLenum>(Parameter) - static_cast<GLenum>(Traits::ProgramParameterType::_FirstTexture);

        glActiveTexture(GL_TEXTURE0 + textureUnit);

        GLenum const glError = glGetError();
        if (GL_NO_ERROR != glError)
        {
            throw GameException("Error activating texture " + std::to_string(textureUnit) + ": " + std::to_string(glError));
        }
    }

private:

    template <typename Traits::ProgramType Program, typename Traits::ProgramParameterType Parameter>
    static inline void CheckUniformError()
    {
        CheckUniformError(Program, Parameter);
    }

    static inline void CheckUniformError(
        typename Traits::ProgramType program,
        typename Traits::ProgramParameterType parameter)
    {
        GLenum const glError = glGetError();
        if (GL_NO_ERROR != glError)
        {
            throw GameException("Error setting uniform for parameter \"" + Traits::ProgramParameterTypeToStr(parameter) + "\" on program \"" + Traits::ProgramTypeToStr(program) + "\"");
        }
    }

private:

    ShaderManager(
        std::filesystem::path const & shadersRoot);

    void CompileShader(
        std::string const & shaderFilename,
        std::string const & shaderSource,
        std::unordered_map<std::string, std::pair<bool, std::string>> const & shaderSources);

    static std::string ResolveIncludes(
        std::string const & shaderSource,
        std::unordered_map<std::string, std::pair<bool, std::string>> const & shaderSources);

    static std::tuple<std::string, std::string> SplitSource(std::string const & source);

    static std::set<std::string> ExtractVertexAttributeNames(GameOpenGLShaderProgram const & shaderProgram);

    static std::set<std::string> ExtractParameterNames(GameOpenGLShaderProgram const & shaderProgram);

private:

    struct ProgramInfo
    {
        // The OpenGL handle to the program
        GameOpenGLShaderProgram OpenGLHandle;

        // The uniform locations, indexed by shader parameter type;
        // set to NoLocation when not specified in the shader
        std::vector<GLint> UniformLocations;
    };

    // All programs, indexed by program type
    std::vector<ProgramInfo> mPrograms;

private:

    friend class ShaderManagerTests_ProcessesIncludes_OneLevel_Test;
    friend class ShaderManagerTests_ProcessesIncludes_MultipleLevels_Test;
    friend class ShaderManagerTests_ProcessesIncludes_DetectsLoops_Test;
    friend class ShaderManagerTests_ProcessesIncludes_ComplainsWhenIncludeNotFound_Test;

    friend class ShaderManagerTests_SplitsShaders_Test;
    friend class ShaderManagerTests_SplitsShaders_DuplicatesCommonSectionToVertexAndFragment_Test;
    friend class ShaderManagerTests_SplitsShaders_ErrorsOnMalformedVertexSection_Test;
    friend class ShaderManagerTests_SplitsShaders_ErrorsOnMissingVertexSection_Test;
    friend class ShaderManagerTests_SplitsShaders_ErrorsOnMissingVertexSection_EmptyFile_Test;
    friend class ShaderManagerTests_SplitsShaders_ErrorsOnMissingFragmentSection_Test;

    friend class ShaderManagerTests_ExtractsShaderParameters_Single_Test;
    friend class ShaderManagerTests_ExtractsShaderParameters_Multiple_Test;
    friend class ShaderManagerTests_ExtractsShaderParameters_IgnoresTrailingComment_Test;
    friend class ShaderManagerTests_ExtractsShaderParameters_IgnoresCommentedOutParameters_Test;
    friend class ShaderManagerTests_ExtractsShaderParameters_ErrorsOnUnrecognizedParameter_Test;
    friend class ShaderManagerTests_ExtractsShaderParameters_ErrorsOnRedefinedParameter_Test;

    friend class ShaderManagerTests_ExtractsVertexAttributeNames_Single_Test;
    friend class ShaderManagerTests_ExtractsVertexAttributeNames_Multiple_Test;
    friend class ShaderManagerTests_ExtractsVertexAttributeNames_ErrorsOnUnrecognizedAttribute_Test;
    friend class ShaderManagerTests_ExtractsVertexAttributeNames_ErrorsOnRedeclaredAttribute_Test;
};

#include "ShaderManager.cpp.inl"
