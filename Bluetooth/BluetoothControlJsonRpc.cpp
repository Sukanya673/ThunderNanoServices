
#include "Module.h"
#include "BluetoothControl.h"
#include <interfaces/json/JsonData_BluetoothControl.h>

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::BluetoothControl;

    // Utility function
    inline DeviceData ConstructDeviceData(const BluetoothControl::DeviceImpl* device) {
        DeviceData output;
        output.Address = device->Address();
        output.Name = device->Name();
        output.Lowenergy = device->LowEnergy();
        output.Connected = device->IsConnected();
        output.Paired = device->IsPaired();

        return output;
    }


    // Registration
    //
    void BluetoothControl::RegisterAll()
    {
        JSONRPC::Register<PairParamsInfo,void>(_T("pair"), &BluetoothControl::endpoint_pair, this);
        JSONRPC::Register<PairParamsInfo,void>(_T("connect"), &BluetoothControl::endpoint_connect, this);
        JSONRPC::Register<ScanParamsData,void>(_T("scan"), &BluetoothControl::endpoint_scan, this);
        JSONRPC::Register<void,void>(_T("stopscan"), &BluetoothControl::endpoint_stopscan, this);
        JSONRPC::Register<PairParamsInfo,void>(_T("unpair"), &BluetoothControl::endpoint_unpair, this);
        JSONRPC::Register<DisconnectParamsData,void>(_T("disconnect"), &BluetoothControl::endpoint_disconnect, this);
        Property<Core::JSON::Boolean>(_T("scanning"), &BluetoothControl::get_scanning, nullptr, this);
        Property<Core::JSON::Boolean>(_T("advertising"), &BluetoothControl::get_advertising, &BluetoothControl::set_advertising, this);
        Property<Core::JSON::ArrayType<DeviceData>>(_T("device"), &BluetoothControl::get_device, nullptr, this);
    }

    void BluetoothControl::UnregisterAll()
    {
        JSONRPC::Unregister(_T("disconnect"));
        JSONRPC::Unregister(_T("unpair"));
        JSONRPC::Unregister(_T("stopscan"));
        JSONRPC::Unregister(_T("scan"));
        JSONRPC::Unregister(_T("connect"));
        JSONRPC::Unregister(_T("pair"));
        JSONRPC::Unregister(_T("device"));
        JSONRPC::Unregister(_T("advertising"));
        JSONRPC::Unregister(_T("scanning"));
    }

    // API implementation
    //

    // Method: pair - Pair host with bluetooth device
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Device not found
    //  - ERROR_GENERAL: Failed to pair
    //  - ERROR_ASYNC_ABORTED: Pairing aborted
    uint32_t BluetoothControl::endpoint_pair(const PairParamsInfo& params)
    {
        const string& address = params.Address.Value();

        DeviceImpl* device = Find(address);
        if (device == nullptr) 
            return Core::ERROR_UNKNOWN_KEY;

        return device->Pair();
    }

    // Method: connect - Connect with bluetooth device
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Device not found
    //  - ERROR_GENERAL: Failed to connect
    //  - ERROR_ASYNC_ABORTED: Connecting aborted
    uint32_t BluetoothControl::endpoint_connect(const PairParamsInfo& params)
    {
        const string& address = params.Address.Value();

        DeviceImpl* device = Find(address);
        if (device == nullptr) 
            return Core::ERROR_UNKNOWN_KEY;

        return device->Connect();
    }

    // Method: scan - Scan environment for bluetooth devices
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t BluetoothControl::endpoint_scan(const ScanParamsData& params)
    {
        // Default values
        bool lowenergy = true;
        bool limited = false; 
        bool passive = false; 
        uint32_t duration = 10 /*seconds*/; 

        if (params.Lowenergy.IsSet() == true) 
            lowenergy = params.Lowenergy.Value();
        if (params.Limited.IsSet() == true) 
            limited = params.Limited.Value();
        if (params.Passive.IsSet() == true) 
            passive = params.Passive.Value();
        if (params.Duration.IsSet() == true) 
            duration = params.Duration.Value();

        printf("### LE: %d\n", lowenergy);
        printf("### LIMITED: %d\n", limited);
        printf("### PASSIVE: %d\n", passive);
        printf("### DURATION: %d\n", duration);

        uint8_t flags = 0;
        uint32_t type = 0x338B9E;

        if (lowenergy == true) {
            _application.Scan(duration, limited, passive);
        } else {
            _application.Scan(duration, type, flags);
        }

        return Core::ERROR_NONE;
    }

    // Method: stopscan - Connect with bluetooth device
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t BluetoothControl::endpoint_stopscan()
    {
        _application.Abort();

        return Core::ERROR_NONE;
    }

    // Method: unpair - Unpair host from a bluetooth device
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Device not found
    uint32_t BluetoothControl::endpoint_unpair(const PairParamsInfo& params)
    {
        const string& address = params.Address.Value();

        DeviceImpl* device = Find(address);
        if (device == nullptr) 
            return Core::ERROR_UNKNOWN_KEY;

        return device->Unpair();
    }

    // Method: disconnect - Disconnects host from bluetooth device
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Device not found
    uint32_t BluetoothControl::endpoint_disconnect(const DisconnectParamsData& params)
    {
        uint32_t result = Core::ERROR_NONE;
        const string& address = params.Address.Value();
        const uint32_t& reason = params.Reason.Value();

        DeviceImpl* device = Find(address);
        if (device == nullptr) 
            return Core::ERROR_UNKNOWN_KEY;

        return device->Disconnect(reason);

        return result;
    }

    // Property: scanning - Tells if host is currently scanning for bluetooth devices
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t BluetoothControl::get_scanning(Core::JSON::Boolean& response) const
    {
        response = IsScanning();

        return Core::ERROR_NONE;
    }

    // Property: advertising - Defines if device is advertising to be visible by other bluetooth devices
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t BluetoothControl::get_advertising(Core::JSON::Boolean& response) const
    {
        // response = ...

        return Core::ERROR_NONE;
    }

    // Property: advertising - Defines if device is advertising to be visible by other bluetooth devices
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t BluetoothControl::set_advertising(const Core::JSON::Boolean& param)
    {
        uint32_t result = Core::ERROR_NONE;

        // TODO...

        return result;
    }

    // Property: device - Informations about devices found during scanning
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Device not found
    uint32_t BluetoothControl::get_device(const string& index, Core::JSON::ArrayType<DeviceData>& response) const
    {
        uint32_t result = Core::ERROR_NONE;

        if (index.empty() == true) {
            for (auto const device : _devices) {
                response.Add(ConstructDeviceData(device));
            }
        } else {
            DeviceImpl* device = Find(index);

            if (device != nullptr) 
                response.Add(ConstructDeviceData(device));
            else 
                result = Core::ERROR_UNKNOWN_KEY;
        }

        return result;
    }
} // namespace Plugin

}

