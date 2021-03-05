#pragma once
#include "dxptr.h"
#include "dxStructures.h"
#include <vector>
#include <string>

namespace mini
{
	class Window;
}

class DxDevice
{
public:
	explicit DxDevice(const mini::Window& window);

	const mini::dx_ptr<ID3D11DeviceContext>& context() const { return m_context; }

	const mini::dx_ptr<IDXGISwapChain>& swapChain() const { return m_swapChain; }

	ID3D11Device* operator->() const { return m_device.get(); }

	mini::dx_ptr<ID3D11RenderTargetView> CreateRenderTargetView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

	mini::dx_ptr<ID3D11Texture2D> CreateTexture(const D3D11_TEXTURE2D_DESC& desc) const;

	mini::dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

	mini::dx_ptr<ID3D11Buffer> CreateBuffer(const void* data, const D3D11_BUFFER_DESC& desc) const;

	static std::vector<BYTE> LoadByteCode(const std::wstring& filename);

	mini::dx_ptr<ID3D11VertexShader> CreateVertexShader(std::vector<BYTE> vsCode) const;

	mini::dx_ptr<ID3D11PixelShader> CreatePixelShader(std::vector<BYTE> psCode) const;

	mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> elements, std::vector<BYTE> vsCode) const;

	mini::dx_ptr <ID3D11DepthStencilView> CreateDepthStencilView(SIZE size) const;

	template <typename T>
	mini::dx_ptr<ID3D11Buffer> CreateConstantBuffer() const
	{
		auto desc = BufferDescription::ConstantBufferDescription(sizeof(T));
		return CreateBuffer(nullptr, desc);
	}

	template<class T>
	mini::dx_ptr <ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& vertices) const {
		auto desc = BufferDescription::VertexBufferDescription(vertices.size() * sizeof(T));
		return CreateBuffer(reinterpret_cast<const void*>(vertices.data()), desc);
	}

	template<class T>
	mini::dx_ptr <ID3D11Buffer> CreateIndexBuffer(const std::vector<T>& indices) const {
		auto desc = BufferDescription::IndexBufferDescription(indices.size() * sizeof(T));
		return CreateBuffer(reinterpret_cast<const void*>(indices.data()), desc);
	}

private:

	mini::dx_ptr<ID3D11Device> m_device;
	mini::dx_ptr<ID3D11DeviceContext> m_context;
	mini::dx_ptr<IDXGISwapChain> m_swapChain;
};