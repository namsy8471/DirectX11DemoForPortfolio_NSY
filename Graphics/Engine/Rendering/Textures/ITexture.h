#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace Engine::Rendering
{
	class ITexture
	{
	public:
		virtual ~ITexture() = default;

		[[nodiscard]] virtual ID3D11ShaderResourceView* GetShaderResourceView() const noexcept = 0;
	};

	class IRenderTexture : public ITexture
	{
	public:
		~IRenderTexture() override = default;

		virtual void SetRenderTarget(ID3D11DeviceContext* deviceContext) noexcept = 0;
		virtual void ClearRenderTarget(
			ID3D11DeviceContext* deviceContext,
			float red,
			float green,
			float blue,
			float alpha) noexcept = 0;
		virtual void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) const noexcept = 0;
		virtual void GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix) const noexcept = 0;
	};

	using TexturePtr = std::unique_ptr<ITexture>;
	using RenderTexturePtr = std::unique_ptr<IRenderTexture>;

	template <typename TextureType, typename... Arguments>
	[[nodiscard]] std::unique_ptr<TextureType> MakeTexture(Arguments&&... arguments)
	{
		static_assert(
			std::is_base_of_v<ITexture, TextureType>,
			"MakeTexture can only construct ITexture implementations.");

		auto texture = std::make_unique<TextureType>();
		if (!texture->Initialize(std::forward<Arguments>(arguments)...))
		{
			return nullptr;
		}

		return texture;
	}
}
