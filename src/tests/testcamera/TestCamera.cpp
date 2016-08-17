#include "TestCamera.h"

// TODO: zooming should update the far and near plane of the render camera (maybe alt + ctrl pressed)
// TODO: render the render camera frustum
// TODO: onMouseMotion for renderCamera (maybe also while ctrl or alt is held)
TestCamera::TestCamera(io::FilesystemPtr filesystem, core::EventBusPtr eventBus) :
		Super(filesystem, eventBus) {
	setCameraMotion(true);
	setRenderPlane(false);
	setRenderAxis(true);
}

core::AppState TestCamera::onInit() {
	core::AppState state = Super::onInit();

	if (!_colorShader.setup()) {
		return core::AppState::Cleanup;
	}

	_renderCamera.init(dimension());
	_renderCamera.setAspectRatio(_aspect);
	_renderCamera.setRotationType(video::CameraRotationType::Target);
	_renderCamera.setPosition(glm::vec3(1.0f, 10.0f, 1.0f));
	_renderCamera.setOmega(glm::vec3(0.0f, 0.001f, 0.0f));
	_renderCamera.setTarget(glm::vec3(10.0f, 70.0f, 10.0f));
	_renderCamera.setNearPlane(5.0f);
	_renderCamera.setFarPlane(40.0f);
	_renderCamera.update(0l);

	_camera.setRotationType(video::CameraRotationType::Target);
	_camera.setTarget(_renderCamera.position());

	// allocate buffer space
	glm::vec3 out[video::FRUSTUM_VERTICES_MAX];
	glm::vec4 out4[video::FRUSTUM_VERTICES_MAX];
	glm::vec4 colors[video::FRUSTUM_VERTICES_MAX];
	uint32_t indices[24];

	// fill buffers
	glm::vec4 color = core::Color::Red;
	for (size_t i = 0; i < video::FRUSTUM_VERTICES_MAX; ++i) {
		colors[i] = color;
		out4[i] = glm::vec4(out[i], 1.0f);
		color = core::Color::Brighter(color, 0.5f);
	}
	_renderCamera.frustumCorners(out, indices);

	// upload to gpu
	_vertexIndex = _frustumBuffer.create(out4, sizeof(out4));
	_indexIndex = _frustumBuffer.create(indices, sizeof(indices), GL_ELEMENT_ARRAY_BUFFER);
	const int32_t cIndex = _frustumBuffer.create(colors, sizeof(colors));

	// configure shader attributes
	_frustumBuffer.addAttribute(_colorShader.getLocationPos(), _vertexIndex, 4);
	_frustumBuffer.addAttribute(_colorShader.getLocationColor(), cIndex, 4);

	return state;
}

core::AppState TestCamera::onRunning() {
	core::AppState state = Super::onRunning();
	_renderCamera.update(_deltaFrame);
	_camera.setTarget(_renderCamera.position());
	return state;
}

core::AppState TestCamera::onCleanup() {
	core::AppState state = Super::onCleanup();
	_colorShader.shutdown();
	_frustumBuffer.shutdown();
	return state;
}

void TestCamera::doRender() {
	// update the vertex buffer, because the reference camera might have changed
	glm::vec3 out[video::FRUSTUM_VERTICES_MAX];
	glm::vec4 out4[video::FRUSTUM_VERTICES_MAX];
	_renderCamera.frustumCorners(out, nullptr);
	for (size_t i = 0; i < video::FRUSTUM_VERTICES_MAX; ++i) {
		out4[i] = glm::vec4(out[i], 1.0f);
	}
	_frustumBuffer.update(_vertexIndex, out4, sizeof(out4));

	video::ScopedShader scoped(_colorShader);
	_colorShader.setView(_camera.viewMatrix());
	_colorShader.setProjection(_camera.projectionMatrix());
	core_assert_always(_frustumBuffer.bind());
	const GLuint indices = _frustumBuffer.elements(_indexIndex, 1, sizeof(uint32_t));
	glDrawElements(GL_LINES, indices, GL_UNSIGNED_INT, 0);
	_frustumBuffer.unbind();
	GL_checkError();
}

int main(int argc, char *argv[]) {
	return core::getApp<TestCamera>()->startMainLoop(argc, argv);
}
