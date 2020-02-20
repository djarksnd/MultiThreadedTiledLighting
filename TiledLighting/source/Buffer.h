#pragma once

#include <D3D11.h>

#include "ResourceBindFlags.h"
#include "AccessFlags.h"

class Buffer
{
public:
    bool Create(ID3D11Device* device, DXGI_FORMAT format,
        unsigned int bufferSize, unsigned int elementSize,
        const ResourceBindFlags& bindFlags,
        const AccessFlags& cpuAccessFlags,
        const void* initData = nullptr);

    void Destroy();
    bool Update(ID3D11DeviceContext*deviceContext, const void* data, unsigned int updateBytes = 0) const;

    unsigned int GetSize() const {
        return bufferSize;
    }

    operator ID3D11Buffer* () const {
        return buff;
    }
    operator ID3D11ShaderResourceView* () const {
        return srv;
    }
    operator ID3D11UnorderedAccessView* () const {
        return uav;
    }

    Buffer() : bufferSize(0), buff(nullptr), srv(nullptr), uav(nullptr) {}
    ~Buffer() {
        Destroy();
    }

    Buffer(const Buffer&) = delete;
    const Buffer& operator = (const Buffer&) = delete;

private:
    ID3D11Buffer* buff;
    ID3D11ShaderResourceView* srv;
    ID3D11UnorderedAccessView* uav;
    unsigned int bufferSize;
};
