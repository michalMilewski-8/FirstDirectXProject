#pragma once
#include "windowApplication.h"
#include "dxDevice.h"
#include <DirectXMath.h>

using namespace mini;

class DxApplication : public mini::WindowApplication {
public:
	explicit DxApplication(HINSTANCE hInstance);
protected:
	int MainLoop() override;
	bool ProcessMessage(WindowMessage& msg) override;
private:
	struct VertexPositionColor {
		DirectX::XMFLOAT3 position, color;
	};

	static std::vector<VertexPositionColor> CreateCubeVertices();
	static std::vector<unsigned short> CreateCubeIndices();
	
	void Render();
	void Update();

	std::vector<DirectX::XMFLOAT2> CreateTriangleVertices();

	float rotation_angle;
	double old_timestamp;
	double new_timestamp;
	int old_mouse_position;
	int new_mouse_position;

	float camera_x_rotation;
	float camera_z_translation;

	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;

	DxDevice m_device;

	DirectX::XMFLOAT4X4 m_modelMtx, m_viewMtx, m_projMtx;

	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;

};

