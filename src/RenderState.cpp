/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Matrix4.h>

#include "Color.h"
#include "RenderState.h"
#include "RenderSystem.h"
#include "Texture.h"

namespace april
{
	RenderState::RenderState()
	{
		this->reset();
	}
	
	RenderState::~RenderState()
	{
	}
	
	void RenderState::reset()
	{
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->textureId = 0;
		this->textureFilter = Texture::FILTER_UNDEFINED;
		this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->systemColor = Color::Black;
		this->modelviewMatrixChanged = false;
		this->projectionMatrixChanged = false;
		this->blendMode = BM_UNDEFINED;
		this->colorMode = CM_UNDEFINED;
		this->colorModeFactor = 1.0f;
		this->depthBuffer = false;
		this->depthBufferWrite = false;
	}

}
