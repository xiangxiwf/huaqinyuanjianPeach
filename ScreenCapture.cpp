#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <stdexcept>
#include <algorithm>
#include <QPixmap>
#include <QImage>
#include "ScreenCapture.h"
#include "mainwindow.h"
#include <QApplication>
using Microsoft::WRL::ComPtr;

ScreenCapture::ScreenCapture(int model, int width, int height) {
    this->model = model;
    this->width = width;
    this->height = height;
}

void ScreenCapture::reviseParameter(int model, int width, int height) {
    this->model = model;
    this->width = width;
    this->height = height;
}

cv::Mat ScreenCapture::runCapture() {
    switch (this->model) {
        case 1: {
#ifdef _WIN32
            using Microsoft::WRL::ComPtr;

            static ComPtr<ID3D11Device> d3dDevice;
            static ComPtr<ID3D11DeviceContext> d3dContext;
            static ComPtr<IDXGIOutputDuplication> deskDupl;
            static bool initialized = false;

            if (!initialized) {
                D3D_FEATURE_LEVEL featureLevel;
                HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                               D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &d3dContext);
                if (FAILED(hr)) {
                    std::cerr << "Failed to create D3D11 device." << std::endl;
                    return cv::Mat();
                }

                ComPtr<IDXGIDevice> dxgiDevice;
                d3dDevice.As(&dxgiDevice);
                ComPtr<IDXGIAdapter> dxgiAdapter;
                dxgiDevice->GetAdapter(&dxgiAdapter);

                ComPtr<IDXGIOutput> dxgiOutput;
                dxgiAdapter->EnumOutputs(0, &dxgiOutput);
                ComPtr<IDXGIOutput1> dxgiOutput1;
                dxgiOutput.As(&dxgiOutput1);

                dxgiOutput1->DuplicateOutput(d3dDevice.Get(), &deskDupl);
                initialized = true;
            }

            ComPtr<IDXGIResource> desktopResource;
            DXGI_OUTDUPL_FRAME_INFO frameInfo;
            HRESULT hr = deskDupl->AcquireNextFrame(500, &frameInfo, &desktopResource);
            if (FAILED(hr)) {
                std::cerr << "Failed to acquire next frame." << std::endl;
                return cv::Mat();
            }

            ComPtr<ID3D11Texture2D> acquiredDesktopImage;
            desktopResource.As(&acquiredDesktopImage);

            D3D11_TEXTURE2D_DESC desc;
            acquiredDesktopImage->GetDesc(&desc);

            int offsetX = (desc.Width - width) / 2;
            int offsetY = (desc.Height - height) / 2;

            D3D11_TEXTURE2D_DESC stagingDesc = desc;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.BindFlags = 0;
            stagingDesc.MiscFlags = 0;

            ComPtr<ID3D11Texture2D> stagingTexture;
            d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);

            d3dContext->CopyResource(stagingTexture.Get(), acquiredDesktopImage.Get());

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            hr = d3dContext->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
            if (FAILED(hr)) {
                std::cerr << "Failed to map staging texture." << std::endl;
                return cv::Mat();
            }

            cv::Mat img(height, width, CV_8UC4);

            for (int y = 0; y < height; ++y) {
                std::memcpy(
                    img.ptr(y),
                    (uint8_t *) mappedResource.pData + ((offsetY + y) * mappedResource.RowPitch) + (offsetX * 4),
                    width * 4);
            }

            cv::Mat imgBGR;
            cv::cvtColor(img, imgBGR, cv::COLOR_BGRA2BGR);

            d3dContext->Unmap(stagingTexture.Get(), 0);
            deskDupl->ReleaseFrame();

            return imgBGR;
#else
    std::cerr << "Screen capture is not implemented for non-Windows platforms." << std::endl;
    return cv::Mat();
#endif
        }
        case 2: {
            HDC hScreenDC = GetDC(nullptr);
            HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);

            int left = std::max(0, (screenWidth - this->width) / 2);
            int top = std::max(0, (screenHeight - this->height) / 2);
            int regionWidth = std::min(this->width, screenWidth - left);
            int regionHeight = std::min(this->height, screenHeight - top);

            HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, regionWidth, regionHeight);
            SelectObject(hMemoryDC, hBitmap);

            BitBlt(hMemoryDC, 0, 0, regionWidth, regionHeight, hScreenDC, left, top, SRCCOPY);

            BITMAPINFOHEADER bi = {0};
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = regionWidth;
            bi.biHeight = -regionHeight;
            bi.biPlanes = 1;
            bi.biBitCount = 32;
            bi.biCompression = BI_RGB;

            BYTE *pBits = new BYTE[regionWidth * regionHeight * 4];
            GetDIBits(hMemoryDC, hBitmap, 0, regionHeight, pBits, (BITMAPINFO *) &bi, DIB_RGB_COLORS);

            cv::Mat img(regionHeight, regionWidth, CV_8UC4, pBits);

            cv::Mat imgBGR;
            cv::cvtColor(img, imgBGR, cv::COLOR_BGRA2BGR);

            delete[] pBits;

            DeleteObject(hBitmap);
            DeleteDC(hMemoryDC);
            ReleaseDC(nullptr, hScreenDC);

            return imgBGR;
        }
        case 3: {
#ifdef _WIN32
            QPixmap pixmap = static_cast<MainWindow *>(QApplication::activeWindow())->getscaledPixmap();

            if (pixmap.isNull()) {
                std::cerr << "Failed to get QPixmap from getscaledPixmap()." << std::endl;
                return cv::Mat();
            }
            QImage image = pixmap.toImage();
            cv::Mat img(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                        image.bytesPerLine());
            cv::Mat imgCopy = img.clone();
            cv::Mat imgBGR;
            cv::cvtColor(imgCopy, imgBGR, cv::COLOR_BGRA2BGR);

            return imgBGR;
#else
            std::cerr << "Drawing on QPixmap is not implemented for non-Windows platforms." << std::endl;
            return cv::Mat();
#endif
        }
    }
}

