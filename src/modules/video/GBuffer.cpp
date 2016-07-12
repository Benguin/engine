/**
 * @file
 */

#include "GBuffer.h"
#include "ScopedFrameBuffer.h"

#include <cstddef>
#include "core/Common.h"

namespace video {

GBuffer::GBuffer() :
		_fbo(0), _depthTexture(0) {
	for (std::size_t i = 0; i < SDL_arraysize(_textures); ++i) {
		_textures[i] = 0;
	}
}

GBuffer::~GBuffer() {
	core_assert_msg(_fbo == 0u, "GBuffer was not properly shut down");
	shutdown();
}

void GBuffer::shutdown() {
	if (_fbo != 0) {
		glDeleteFramebuffers(1, &_fbo);
		_fbo = 0;
	}

	if (_textures[0] != 0) {
		glDeleteTextures(SDL_arraysize(_textures), _textures);
		for (int i = 0; i < (int)SDL_arraysize(_textures); ++i) {
			_textures[i] = 0;
		}
	}

	if (_depthTexture != 0) {
		glDeleteTextures(1, &_depthTexture);
		_depthTexture = 0;
	}
	core_assert(_oldDrawFramebuffer == -1);
	core_assert(_oldReadFramebuffer == -1);
}

bool GBuffer::init(const glm::ivec2& dimension) {
	glGenFramebuffers(1, &_fbo);
	GL_setName(GL_FRAMEBUFFER, _fbo, "gbuffer");
	ScopedFrameBuffer scopedFrameBuffer(_fbo);

	// +1 for the depth texture
	glGenTextures(SDL_arraysize(_textures) + 1, _textures);
	for (std::size_t i = 0; i < SDL_arraysize(_textures); ++i) {
		GL_setName(GL_TEXTURE, _textures[i], "gbuffertexture");
		glBindTexture(GL_TEXTURE_2D, _textures[i]);
		// we are going to write vec3 into the out vars in the shaders
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, dimension.x, dimension.y, 0, GL_RGB, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _textures[i], 0);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GL_checkError();
	}

	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	GL_setName(GL_TEXTURE, _depthTexture, "gbufferdepthtexture");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, dimension.x, dimension.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
	GL_checkError();

	const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	static_assert(SDL_arraysize(drawBuffers) == SDL_arraysize(_textures), "buffers and textures don't match");
	glDrawBuffers(SDL_arraysize(drawBuffers), drawBuffers);
	GL_checkError();

	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		Log::error("FB error, status: %i", (int)status);
		return false;
	}

	return true;
}

void GBuffer::bindForWriting() {
	if (_oldDrawFramebuffer == -1) {
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_oldDrawFramebuffer);
		GL_checkError();
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	GL_checkError();
}

void GBuffer::bindForReading(bool gbuffer) {
	if (gbuffer) {
		if (_oldReadFramebuffer == -1) {
			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &_oldReadFramebuffer);
			GL_checkError();
		}
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		return;
	}

	if (_oldDrawFramebuffer == -1) {
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_oldDrawFramebuffer);
		GL_checkError();
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// activate the textures to read from
	for (int i = 0; i < (int) SDL_arraysize(_textures); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		core_assert(_textures[i] != 0);
		glBindTexture(GL_TEXTURE_2D, _textures[i]);
	}
	glActiveTexture(GL_TEXTURE0);
}

void GBuffer::unbind() {
	if (_oldDrawFramebuffer != -1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _oldDrawFramebuffer);
		_oldDrawFramebuffer = -1;
	}
	if (_oldReadFramebuffer != -1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _oldReadFramebuffer);
		_oldReadFramebuffer = -1;
	}

	// activate the textures to read from
	for (int i = 0; i < (int) SDL_arraysize(_textures); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glActiveTexture(GL_TEXTURE0);
}

void GBuffer::setReadBuffer(GBufferTextureType textureType) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + textureType);
	GL_checkError();
}

}
