/**
 * @file
 */

#pragma once

#include "voxel/World.h"
#include "voxel/generator/PlantGenerator.h"
#include "video/Shader.h"
#include "video/Texture.h"
#include "video/Camera.h"
#include "video/VertexBuffer.h"
#include "video/GBuffer.h"
#include "video/DepthBuffer.h"
#include "FrontendShaders.h"
#include "video/GLMeshData.h"
#include "core/GLM.h"
#include "core/Var.h"
#include "core/Color.h"
#include "ClientEntity.h"

#include <unordered_map>
#include <list>

namespace frontend {

class WorldRenderer {
private:
	struct NoiseGenerationTask {
		NoiseGenerationTask(uint8_t *_buffer, int _width, int _height, int _depth) :
				buffer(_buffer), width(_width), height(_height), depth(_depth) {
		}
		// pointer to preallocated buffer that was hand into the noise task
		uint8_t *buffer;
		const int width;
		const int height;
		const int depth;
	};

	typedef std::future<NoiseGenerationTask> NoiseFuture;
	std::vector<NoiseFuture> _noiseFuture;

	// Index/vertex buffer data
	typedef std::list<video::GLMeshData> GLMeshDatas;
	GLMeshDatas _meshDataOpaque;
	GLMeshDatas _meshDataWater;
	GLMeshDatas _meshDataPlant;

	typedef std::unordered_map<ClientEntityId, ClientEntityPtr> Entities;
	Entities _entities;

	float _fogRange = 250.0f;
	// TODO: get the view distance from the server - entity attributes
	float _viewDistance = 0.1f;
	long _now = 0l;
	long _deltaFrame = 0l;

	glm::vec4 _clearColor;
	video::TexturePtr _colorTexture;
	glm::vec3 _lightDir = glm::vec3(10000.0, 10000.0, 10000.0);
	glm::vec3 _diffuseColor = glm::vec3(1.0, 1.0, 1.0);
	glm::mat4 _lightProjection;
	glm::mat4 _lightView;
	glm::mat4 _lightSpaceMatrix;
	// the position of the last extraction - we only care for x and z here
	glm::ivec3 _lastGridPosition = { std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min() };
	voxel::WorldPtr _world;
	video::GBuffer _gbuffer;
	core::VarPtr _debugGeometry;
	core::VarPtr _deferred;
	core::VarPtr _deferredDebug;
	core::VarPtr _shadowMap;
	core::VarPtr _shadowMapDebug;
	core::VarPtr _cameraSun;
	video::VertexBuffer _fullscreenQuad;
	video::VertexBuffer _texturedFullscreenQuad;
	video::DepthBuffer _depthBuffer;

	shader::ShadowmapRenderShader _shadowMapRenderShader;
	shader::WorldShader _worldShader;
	shader::WorldInstancedShader _plantShader;
	shader::ShadowmapInstancedShader _shadowMapInstancedShader;
	shader::WaterShader _waterShader;
	shader::MeshShader _meshShader;
	shader::DeferredLightDirShader _deferredDirLightShader;
	shader::ShadowmapShader _shadowMapShader;

	voxel::PlantGenerator _plantGenerator;

	// Convert a PolyVox mesh to OpenGL index/vertex buffers.
	video::GLMeshData createMeshInternal(video::Shader& shader, voxel::Mesh &mesh, int buffers);
	video::GLMeshData createMesh(video::Shader& shader, voxel::Mesh& mesh);
	video::GLMeshData createInstancedMesh(video::Shader& shader, voxel::Mesh &mesh, int amount);
	void updateMesh(voxel::Mesh& surfaceMesh, video::GLMeshData& meshData);
	void handleMeshQueue(video::Shader& shader);
	void distributePlants(int amount, video::GLMeshData& meshData);
	void fillPlantPositionsFromMeshes();

	// we might want to get an answer for this question in two contexts, once for 'should-i-render-this' and once for
	// 'should-i-create/destroy-the-mesh'.
	bool isDistanceCulled(int distance2, bool queryForRendering = true) const;
	int getDistance2(const glm::ivec3& pos) const;
	// schedule mesh extraction around the grid position with the given radius
	void extractMeshAroundCamera(const glm::ivec3& gridPos, int radius = 1);

	int renderWorldMeshes(video::Shader& shader, const video::Camera& camera, GLMeshDatas& meshes, int* vertices, bool culling = true);
	void renderWorldDeferred(const video::Camera& camera, const int width, const int height);

	bool checkShaders() const;

public:
	WorldRenderer(const voxel::WorldPtr& world);
	~WorldRenderer();

	void reset();

	bool onInit(const glm::ivec2& dimension);
	void onRunning(long dt);
	void shutdown();

	// called to initialed the player position
	void onSpawn(const glm::vec3& pos, int initialExtractionRadius = 5);

	ClientEntityPtr getEntity(ClientEntityId id);
	bool addEntity(const ClientEntityPtr& entity);
	bool removeEntity(ClientEntityId id);

	void stats(int& meshes, int& extracted, int& pending) const;

	// world coordinates x/z
	void deleteMesh(const glm::ivec3& pos);

	float getViewDistance() const;

	void setVoxel(const glm::ivec3& pos, const voxel::Voxel& voxel);

	bool extractNewMeshes(const glm::vec3& position, bool force = false);
	int renderWorld(const video::Camera& camera, int* vertices = nullptr);
	int renderEntities(const video::Camera& camera);
};

inline float WorldRenderer::getViewDistance() const {
	return _viewDistance;
}

}
