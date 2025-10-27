#pragma once

#include "Renderer/Mesh.hpp"

#include <Renderer/Shader.hpp>

namespace libprojectM {
namespace MilkdropPreset {

class PresetState;
class PerFrameContext;
class PerPixelContext;
class MilkdropShader;

/**
 * @brief The "per-pixel" transformation mesh.
 *
 * This mesh is responsible for most of the motion types in presets. Each mesh vertex
 * is transposed (also scaled, from the center) or rotated to create a frame-by-frame motion.
 * Fragment shader interpolation is then used to create smooth transitions in the space
 * between the grid points.
 *
 * A higher resolution grid means better quality, especially for rotations, but also quickly
 * increases the CPU usage as the per-pixel expression needs to be run for every grid point.
 *
 * The mesh size can be changed between frames, the class will reallocate the buffers if needed.
 */
class PerPixelMesh
{
public:
    PerPixelMesh();

    /**
     * @brief Loads the warp shader, if the preset uses one.
     * @param presetState The preset state to retrieve the shader from.
     */
    void LoadWarpShader(const PresetState& presetState);

    /**
     * @brief Loads the required textures and compiles the warp shader.
     * @param presetState The preset state to retrieve the configuration values from.
     */
    void CompileWarpShader(PresetState& presetState);

    /**
     * @brief Renders the transformation mesh.
     * @param presetState The preset state to retrieve the configuration values from.
     * @param presetPerFrameContext The per-frame context to retrieve the initial vars from.
     * @param perPixelContext The per-pixel code context to use.
     */
    void Draw(const PresetState& presetState,
              const PerFrameContext& perFrameContext,
              PerPixelContext& perPixelContext);


private:
    /**
     * Vertex attributes for radius and angle.
     */
    struct RadiusAngle {
        float radius{};
        float angle{};

        static void InitializeAttributePointer(uint32_t attributeIndex)
        {
            glVertexAttribPointer(attributeIndex, sizeof(RadiusAngle) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(RadiusAngle), nullptr);
        }
    };

    /**
     * Vertex attributes for zoom, zoom exponent, rotation and warp strength.
     */
    struct ZoomRotWarp {
        float zoom{};
        float zoomExp{};
        float rot{};
        float warp{};

        static void InitializeAttributePointer(uint32_t attributeIndex)
        {
            glVertexAttribPointer(attributeIndex, sizeof(ZoomRotWarp) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(ZoomRotWarp), nullptr);
        }
    };

    /**
     * @brief Initializes the vertex array and fills in static data if needed.
     *
     * The vertices will be reallocated if the grid size has changed. If either this happened,
     * or the viewport size changed, the static values will be recalculated.
     *
     * @param presetState The preset state to retrieve the configuration values from.
     */
    void InitializeMesh(const PresetState& presetState);

    /**
     * @brief Executes the per-pixel code and calculates the u/v coordinates.
     * The x/y coordinates are either a static grid or computed by the per-vertex expression.
     * @param presetState The preset state to retrieve the configuration values from.
     * @param presetPerFrameContext The per-frame context to retrieve the initial vars from.
     * @param perPixelContext The per-pixel code context to use.
     */
    void CalculateMesh(const PresetState& presetState,
                       const PerFrameContext& perFrameContext,
                       PerPixelContext& perPixelContext);

    /**
     * @brief Draws the warp mesh with or without a warp shader.
     * If the preset doesn't use a warp shader, a default textured shader is used.
     */
    void WarpedBlit(const PresetState& presetState, const PerFrameContext& perFrameContext);

    /**
     * @brief Creates or retrieves the default warp shader.
     * @param presetState The preset state to retrieve the rendering context from.
     * @return A shared pointer to the default warp shader instance.
     */
    auto GetDefaultWarpShader(const PresetState& presetState) -> std::shared_ptr<Renderer::Shader>;

    int m_gridSizeX{}; //!< Warp mesh X resolution.
    int m_gridSizeY{}; //!< Warp mesh Y resolution.

    int m_viewportWidth{};  //!< Last known viewport width.
    int m_viewportHeight{}; //!< Last known viewport height.

    Renderer::Mesh m_warpMesh;                                //!< The Warp effect mesh
    Renderer::VertexBuffer<RadiusAngle> m_radiusAngleBuffer;  //!< Vertex attribute buffer for radius and angle values.
    Renderer::VertexBuffer<ZoomRotWarp> m_zoomRotWarpBuffer;  //!< Vertex attribute buffer for zoom, roation and warp values.
    Renderer::VertexBuffer<Renderer::Point> m_centerBuffer;   //!< Vertex attribute buffer for center coordinate values.
    Renderer::VertexBuffer<Renderer::Point> m_distanceBuffer; //!< Vertex attribute buffer for distance values.
    Renderer::VertexBuffer<Renderer::Point> m_stretchBuffer;  //!< Vertex attribute buffer for stretch values.

    std::weak_ptr<Renderer::Shader> m_perPixelMeshShader;             //!< Special shader which calculates the per-pixel UV coordinates.
    std::unique_ptr<MilkdropShader> m_warpShader;                     //!< The warp shader. Either preset-defined or a default shader.
    Renderer::Sampler m_perPixelSampler{GL_CLAMP_TO_EDGE, GL_LINEAR}; //!< The main texture sampler.
};

} // namespace MilkdropPreset
} // namespace libprojectM
