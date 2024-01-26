#pragma once
#include <opendaq/device_ptr.h>
#include <opendaq/function_block_impl.h>
#include <opendaq/device_impl.h>
#include <opendaq/channel_impl.h>

namespace daq::config_protocol::test_utils
{
    DevicePtr createServerDevice();
    ComponentPtr createAdvancedPropertyComponent(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);

    class MockFb1Impl final : public FunctionBlock
    {
    public:
        MockFb1Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
    };

    class MockFb2Impl final : public FunctionBlock
    {
    public:
        MockFb2Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
    };

    class MockChannel1Impl final : public Channel
    {
    public:
        MockChannel1Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
    };

    class MockChannel2Impl final : public Channel
    {
    public:
        MockChannel2Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
    };

    class MockDevice1Impl final : public Device
    {
    public:
        MockDevice1Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
        DictPtr<IString, IFunctionBlockType> onGetAvailableFunctionBlockTypes() override;
        FunctionBlockPtr onAddFunctionBlock(const StringPtr& typeId, const PropertyObjectPtr& config) override;
    };

    class MockDevice2Impl final : public Device
    {
    public:
        MockDevice2Impl(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId);
    };

}
