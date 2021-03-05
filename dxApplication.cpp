#include "dxApplication.h"
#include <DirectXMath.h>
#include <windowsx.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace mini;
using namespace std;
using namespace DirectX;


static inline double GetCurrentTimeSp() {
	LARGE_INTEGER ticks;
	LARGE_INTEGER frequency;

	if (!QueryPerformanceFrequency(&frequency)) {
		//Do smth with error
	}

	if (!QueryPerformanceCounter(&ticks)) {
		//Do smth with error
	}

	return ticks.QuadPart / (double)frequency.QuadPart;
}

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance), m_device(m_window) {
	ID3D11Texture2D* temp = nullptr;
	m_device.swapChain()->GetBuffer(0,
		__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	const dx_ptr<ID3D11Texture2D> backTexture{ temp };
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);
	SIZE wndSize = m_window.getClientSize();
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1,
		&backBuffer, m_depthBuffer.get());
	Viewport viewport{ wndSize };
	m_device.context()->RSSetViewports(1, &viewport);
	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);
	const auto vertices = CreateCubeVertices();
	m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
	const auto indices = CreateCubeIndices();
	m_indexBuffer = m_device.CreateIndexBuffer(indices);

	vector<D3D11_INPUT_ELEMENT_DESC> elements{ { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPositionColor, color), D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	m_layout = m_device.CreateInputLayout(elements, vsBytes);

	camera_x_rotation = -M_PI_2 / 3.0f;
	camera_z_translation = 10.0f;

	XMStoreFloat4x4(&m_modelMtx, XMMatrixIdentity());
	XMStoreFloat4x4(&m_viewMtx, XMMatrixRotationX(camera_x_rotation) * XMMatrixTranslation(0.0f, 0.0f, camera_z_translation));
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XMConvertToRadians(45), static_cast<float>(wndSize.cx) / wndSize.cy, 0.1f, 100.0f));
	m_cbMVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();

	rotation_angle = 0.0f;
	old_timestamp = GetCurrentTimeSp();
}

int DxApplication::MainLoop() {
	MSG msg{};
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else {
			Update();
			Render();
			m_device.swapChain()->Present(0, 0);

		}

	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

bool DxApplication::ProcessMessage(WindowMessage& msg)
{
	if (msg.message == WM_MOUSEMOVE) {
		int diff;
		switch (msg.wParam) {
		case MK_LBUTTON:
			new_mouse_position = GET_Y_LPARAM(msg.lParam);
			diff = new_mouse_position - old_mouse_position;
			camera_x_rotation -= diff / 300.0f;
			if (camera_x_rotation > M_PI)
				camera_x_rotation = -M_PI;
			else if (camera_x_rotation < -M_PI)
				camera_x_rotation = M_PI;
			break;
		case MK_RBUTTON:
			new_mouse_position = GET_Y_LPARAM(msg.lParam);
			diff = new_mouse_position - old_mouse_position;
			camera_z_translation += diff / 100.0f;
			if (camera_z_translation > 50.0f)
				camera_z_translation = 50.0f;
			else if (camera_z_translation < 0.0f)
				camera_z_translation = 0.0f;
			break;
		default:
			old_mouse_position = GET_Y_LPARAM(msg.lParam);
			return false;
		}
		XMStoreFloat4x4(&m_viewMtx, XMMatrixRotationX(camera_x_rotation) * XMMatrixTranslation(0.0f, 0.0f, camera_z_translation));
		old_mouse_position = GET_Y_LPARAM(msg.lParam);
		return true;
	}
	return false;
}

void DxApplication::Update() {
	static const float s = 1.0f;
	new_timestamp = GetCurrentTimeSp();
	auto diff = new_timestamp - old_timestamp;
	old_timestamp = new_timestamp;
	rotation_angle += diff * M_PI_4 * s;
	XMStoreFloat4x4(&m_modelMtx, XMMatrixRotationY(rotation_angle) * XMMatrixTranslation(+5.0f, 0.0f, 0.0f));
	D3D11_MAPPED_SUBRESOURCE res;
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	XMMATRIX mvp = XMLoadFloat4x4(&m_modelMtx) * XMLoadFloat4x4(&m_viewMtx) * XMLoadFloat4x4(&m_projMtx);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);
}

std::vector<DirectX::XMFLOAT2> DxApplication::CreateTriangleVertices()
{
	auto vec = std::vector<DirectX::XMFLOAT2>();
	vec.push_back({ -0.5f,-0.5f });
	vec.push_back({ -0.5f,0.5f });
	vec.push_back({ 0.5f,-0.5f });
	return vec;
}

std::vector<DxApplication::VertexPositionColor> DxApplication::CreateCubeVertices()
{
	return {
		//Front Face
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { +0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { +0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { -0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		//Upper Face
		{ { +0.5f, +0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, +0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, +0.5f, +0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { +0.5f, +0.5f, +0.5f }, { 0.0f, 1.0f, 0.0f } },
		//Back face
		{ { +0.5f, +0.5f, +0.5f }, { 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, +0.5f, +0.5f }, { 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 0.0f } },
		{ { +0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 0.0f } },
		//Left face
		{ { -0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, +0.5f }, { 1.0f, 0.0f, 1.0f } },
		//Right face
		{ { +0.5f, +0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, +0.5f, +0.5f }, { 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, -0.5f, +0.5f }, { 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
		//Down face
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f } },
		{ { +0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f } },
		{ { +0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
	};
}

std::vector<unsigned short> DxApplication::CreateCubeIndices()
{
	return {
		 0,2,1, 0,3,2,
		 8,9,10, 8,10,11,
		 12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23,
		4,5,6, 4,6,7,
	};
}

void DxApplication::Render() {
	const float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_device.context()->ClearDepthStencilView(
		m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_device.context()->VSSetShader(
		m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(
		m_pixelShader.get(), nullptr, 0);
	m_device.context()->IASetInputLayout(m_layout.get());
	m_device.context()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	m_device.context()->IASetVertexBuffers(
		0, 1, vbs, strides, offsets);
	m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	m_device.context()->DrawIndexed(36, 0, 0);

	ID3D11Buffer* cbs[] = { m_cbMVP.get() };
	m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	XMStoreFloat4x4(&m_modelMtx, XMMatrixTranslation(-5.0f, 0.0f, 0.0f));
	D3D11_MAPPED_SUBRESOURCE res;
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	XMMATRIX mvp = XMLoadFloat4x4(&m_modelMtx) * XMLoadFloat4x4(&m_viewMtx) * XMLoadFloat4x4(&m_projMtx);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);

	m_device.context()->DrawIndexed(36, 0, 0);


}
