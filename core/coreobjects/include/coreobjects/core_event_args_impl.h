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
#include <coretypes/common.h>
#include <coreobjects/core_event_args.h>
#include <coretypes/event_args_impl.h>
#include <coretypes/validation.h>
#include <core_event_args_ptr.h>

BEGIN_NAMESPACE_OPENDAQ

namespace core_event_args_impl
{
    static std::string getCoreEventName(const Int id)
    {
        switch(id)
        {
            case core_event_ids::PropertyValueChanged:
                return "PropertyValueChanged";
            case core_event_ids::PropertyObjectUpdateEnd:
                return "PropertyObjectUpdateEnd";
            case core_event_ids::PropertyAdded:
                return "PropertyAdded";
            case core_event_ids::PropertyRemoved:
                return "PropertyRemoved";
            case core_event_ids::ComponentAdded:
                return "ComponentAdded";
            case core_event_ids::ComponentRemoved:
                return "ComponentRemoved";
            case core_event_ids::SignalConnected:
                return "SignalConnected";
            case core_event_ids::SignalDisconnected:
                return "SignalDisconnected";
            case core_event_ids::DataDescriptorChanged:
                return "DataDescriptorChanged";
            case core_event_ids::ComponentUpdateEnd:
                return "ComponentUpdateEnd";
            case core_event_ids::AttributeChanged:
                return "AttributeChanged";
            case core_event_ids::TagsChanged:
                return "TagsChanged";
            default:
                break;
        }

        return "Unknown";
    }
}


class CoreEventArgsImpl : public EventArgsBase<ICoreEventArgs, ISerializable>
{
public:
    explicit CoreEventArgsImpl (Int id, const DictPtr<IString, IBaseObject>& parameters);

    ErrCode INTERFACE_FUNC getParameters(IDict** parameters) override;
    
    ErrCode INTERFACE_FUNC serialize(ISerializer* serializer) override;
    ErrCode INTERFACE_FUNC getSerializeId(ConstCharPtr* id) const override;
    
    static ConstCharPtr SerializeId();

    static ErrCode Deserialize(ISerializedObject* ser, IBaseObject* context, IFunction* factoryCallback, IBaseObject** obj);

private:
    DictPtr<IString, IBaseObject> parameters;
    bool validateParameters() const;
};


inline CoreEventArgsImpl::CoreEventArgsImpl(Int id, const DictPtr<IString, IBaseObject>& parameters)
    : EventArgsImplTemplate<ICoreEventArgs, ISerializable>(id, core_event_args_impl::getCoreEventName(id))
    , parameters(parameters)
{
    if (!validateParameters())
        throw InvalidParameterException{"Core event parameters for event type \"{}\" are invalid", this->eventName};
}

inline ErrCode CoreEventArgsImpl::getParameters(IDict** parameters)
{
    OPENDAQ_PARAM_NOT_NULL(parameters);

    *parameters = this->parameters.addRefAndReturn();
    return OPENDAQ_SUCCESS;
}

inline ErrCode CoreEventArgsImpl::serialize(ISerializer* serializer)
{
    serializer->startTaggedObject(this);

    serializer->key("id");
    serializer->writeInt(this->eventId);

    serializer->key("params");
    ISerializable* serializableParams;
    ErrCode errCode = this->parameters->borrowInterface(ISerializable::Id, reinterpret_cast<void**>(&serializableParams));

    if (errCode == OPENDAQ_ERR_NOINTERFACE)
        return OPENDAQ_ERR_NOT_SERIALIZABLE;

    if (OPENDAQ_FAILED(errCode))
        return errCode;

    errCode = serializableParams->serialize(serializer);

    if (OPENDAQ_FAILED(errCode))
        return errCode;

    serializer->endObject();

    return OPENDAQ_SUCCESS;
}

inline ErrCode CoreEventArgsImpl::getSerializeId(ConstCharPtr* id) const
{
    *id = SerializeId();

    return OPENDAQ_SUCCESS;
}

inline ConstCharPtr CoreEventArgsImpl::SerializeId()
{
    return "CoreEventArgs";
}

inline ErrCode CoreEventArgsImpl::Deserialize(ISerializedObject* ser, IBaseObject* context, IFunction* factoryCallback, IBaseObject** obj)
{
    Int id;
    ErrCode errCode = ser->readInt("id"_daq, &id);
    if (OPENDAQ_FAILED(errCode))
        return errCode;

    BaseObjectPtr params;
    errCode = ser->readObject("params"_daq, context, factoryCallback, &params);
    if (OPENDAQ_FAILED(errCode))
        return errCode;

    try
    {
        CoreEventArgsPtr argsPtr; 
        createCoreEventArgs(&argsPtr, id, params.asPtr<IDict>());
        *obj = argsPtr.detach();
    }
    catch(const DaqException& e)
    {
        return e.getErrCode();
    }
    catch(...)
    {
        return OPENDAQ_ERR_GENERALERROR;
    }

    return OPENDAQ_SUCCESS;
}

inline bool CoreEventArgsImpl::validateParameters() const
{
    switch(eventId)
    {
        case core_event_ids::PropertyValueChanged:
            return parameters.hasKey("Name") && parameters.hasKey("Value") && parameters.hasKey("Path");
        case core_event_ids::PropertyObjectUpdateEnd:
            return parameters.hasKey("UpdatedProperties") && parameters.get("UpdatedProperties").asPtrOrNull<IDict>().assigned() && parameters.hasKey("Path");
        case core_event_ids::PropertyAdded:
            return parameters.hasKey("Property") && parameters.hasKey("Path");
        case core_event_ids::PropertyRemoved:
            return parameters.hasKey("Name") && parameters.hasKey("Path");
        case core_event_ids::ComponentAdded:
            return parameters.hasKey("Component");
        case core_event_ids::ComponentRemoved:
            return parameters.hasKey("Id");
        case core_event_ids::SignalConnected:
            return parameters.hasKey("Signal");
        case core_event_ids::DataDescriptorChanged:
            return parameters.hasKey("DataDescriptor");
        case core_event_ids::AttributeChanged:
            return parameters.hasKey("AttributeName");
        case core_event_ids::TagsChanged:
            return parameters.hasKey("Tags");
        default:
            break;
    }

    return true;
}

OPENDAQ_REGISTER_DESERIALIZE_FACTORY(CoreEventArgsImpl)

END_NAMESPACE_OPENDAQ
