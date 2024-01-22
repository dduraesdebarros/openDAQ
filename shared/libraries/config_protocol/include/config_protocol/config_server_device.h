/*
 * Copyright 2022-2023 Blueberry d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <opendaq/device_ptr.h>
#include <config_protocol/component_holder_ptr.h>
#include <config_protocol/component_holder_factory.h>

namespace daq::config_protocol
{

class ConfigServerDevice
{
public:
    static BaseObjectPtr getAvailableFunctionBlockTypes(const DevicePtr& device, const ParamsDictPtr& params);
    static BaseObjectPtr addFunctionBlock(const DevicePtr& device, const ParamsDictPtr& params);
    static BaseObjectPtr removeFunctionBlock(const DevicePtr& device, const ParamsDictPtr& params);
};

inline BaseObjectPtr ConfigServerDevice::getAvailableFunctionBlockTypes(const DevicePtr& device,
    const ParamsDictPtr& params)
{
    const auto fbTypes = device.getAvailableFunctionBlockTypes();
    return fbTypes;
}

inline BaseObjectPtr ConfigServerDevice::addFunctionBlock(const DevicePtr& device, const ParamsDictPtr& params)
{
    const auto fbTypeId = params.get("TypeId");
    PropertyObjectPtr config;
    if (params.hasKey("Config"))
        config = params.get("Config");

    const auto fb = device.addFunctionBlock(fbTypeId, config);
    return ComponentHolder(fb);
}

inline BaseObjectPtr ConfigServerDevice::removeFunctionBlock(const DevicePtr& device, const ParamsDictPtr& params)
{
    return nullptr;
}

}