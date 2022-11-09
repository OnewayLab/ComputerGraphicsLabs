#include "TRTexture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace TinyRenderer
{
	//----------------------------------------------TRTexture2D----------------------------------------------

	TRTexture2D::TRTexture2D() :
		m_width(0), m_height(0), m_channel(0), m_pixels(nullptr),
		m_warp_mode(TRTextureWarpMode::TR_REPEAT),
		m_filtering_mode(TRTextureFilterMode::TR_NEAREST) {}

	TRTexture2D::~TRTexture2D() { freeLoadedImage(); }

	void TRTexture2D::setWarpingMode(TRTextureWarpMode mode) { m_warp_mode = mode; }

	void TRTexture2D::setFilteringMode(TRTextureFilterMode mode) { m_filtering_mode = mode; }

	bool TRTexture2D::loadTextureFromFile(
		const std::string &filepath,
		TRTextureWarpMode warpMode,
		TRTextureFilterMode filterMode)
	{
		freeLoadedImage();

		m_warp_mode = warpMode;
		m_filtering_mode = filterMode;

		//Load image from given file using stb_image.h
		//Refs: https://github.com/nothings/stb
		{
			stbi_set_flip_vertically_on_load(true);
			m_pixels = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channel, 0);
		}

		if (m_pixels == nullptr)
		{
			std::cerr << "Failed to load image from " << filepath << std::endl;
			exit(1);
		}

		return true;
	}

	void TRTexture2D::readPixel(int u, int v, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) const
	{
		//Handling out of range situation
		{
			if (u < 0 || u >= m_width)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					u = u > 0 ? (u % m_width) : (m_width - 1 + u % m_width);
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					u = (u < 0) ? 0 : m_width - 1;
					break;
				default:
					u = (u < 0) ? 0 : m_width - 1;
					break;
				}
			}

			if (v < 0 || v >= m_height)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					v = v > 0 ? (v % m_height) : (m_height - 1 + v % m_height);
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					v = (v < 0) ? 0 : m_height - 1;
					break;
				default:
					v = (v < 0) ? 0 : m_height - 1;
					break;
				}
			}
		}

		int index = (v * m_height + u) * m_channel;
		r = m_pixels[index + 0];
		g = m_pixels[index + 1];
		b = m_pixels[index + 2];
		a = (m_channel >= 4) ? m_pixels[index + 3] : a;

		return;
	}

	void TRTexture2D::freeLoadedImage()
	{
		if (m_pixels != nullptr)
		{
			stbi_image_free(m_pixels);
		}

		m_pixels = nullptr;
		m_width = m_height = m_channel = 0;
	}

	glm::vec4 TRTexture2D::sample(const glm::vec2 &uv) const
	{
		//Perform sampling procedure
		//Note: return texel that ranges from 0.0f to 1.0f instead of [0,255]
		glm::vec4 texel(1.0f);
		switch (m_filtering_mode)
		{
		case TRTextureFilterMode::TR_NEAREST:
			texel = TRTexture2DSampler::textureSampling_nearest(*this, uv);
			break;
		case TRTextureFilterMode::TR_LINEAR:
			texel = TRTexture2DSampler::textureSampling_bilinear(*this, uv);
			break;
		default:
			break;
		}

		return texel;
	}

	//----------------------------------------------TRTexture2DSampler----------------------------------------------

	glm::vec4 TRTexture2DSampler::textureSampling_nearest(const TRTexture2D &texture, glm::vec2 uv)
	{
		unsigned char r = 255, g = 255, b = 255, a = 255;

		//Task1: Implement nearest sampling algorithm for texture sampling
		// Note: You should use texture.readPixel() to read the pixel, and for instance,
		//       use texture.readPixel(25,35,r,g,b,a) to read the pixel in (25, 35).
		//       But before that, you need to map uv from [0,1]*[0,1] to [0,width-1]*[0,height-1].
		{
			int u = uv.x * (texture.getWidth() - 1) + 0.5;
			int v = uv.y * (texture.getHeight() - 1) + 0.5;
			texture.readPixel(u, v, r, g, b, a);
		}

		constexpr float denom = 1.0f / 255.0f;
		return glm::vec4(r, g, b, a) * denom;
	}

	glm::vec4 TRTexture2DSampler::textureSampling_bilinear(const TRTexture2D &texture, glm::vec2 uv)
	{
		//Task4: Implement bilinear sampling algorithm for texture sampling
		// Note: You should use texture.readPixel() to read the pixel, and for instance,
		//       use texture.readPixel(25,35,r,g,b,a) to read the pixel in (25, 35).

		// Get the texture coordinates of the four surrounding texels
		float u = uv.x * (texture.getWidth() - 1);
		float v = uv.y * (texture.getHeight() - 1);
		int u1 = u;
		int v1 = v;
		int u2 = u1 + 1;
		int v2 = v1 + 1;

		// Get the four surrounding texels
		unsigned char r1, g1, b1, a1;
		unsigned char r2, g2, b2, a2;
		unsigned char r3, g3, b3, a3;
		unsigned char r4, g4, b4, a4;
		texture.readPixel(u1, v1, r1, g1, b1, a1);
		texture.readPixel(u2, v1, r2, g2, b2, a2);
		texture.readPixel(u1, v2, r3, g3, b3, a3);
		texture.readPixel(u2, v2, r4, g4, b4, a4);

		// Get the weights
		float w1 = (u2 - u) * (v2 - v);
		float w2 = (u - u1) * (v2 - v);
		float w3 = (u2 - u) * (v - v1);
		float w4 = (u - u1) * (v - v1);

		// Get the weighted average
		unsigned char r = w1 * r1 + w2 * r2 + w3 * r3 + w4 * r4;
		unsigned char g = w1 * g1 + w2 * g2 + w3 * g3 + w4 * g4;
		unsigned char b = w1 * b1 + w2 * b2 + w3 * b3 + w4 * b4;
		unsigned char a = w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4;

		constexpr float denom = 1.0f / 255.0f;
		return glm::vec4(r, g, b, a) * denom;
	}
}