#include <opendaq/component_factory.h>
#include <opendaq/context_factory.h>
#include <coreobjects/property_factory.h>
#include <gtest/gtest.h>
#include <coreobjects/property_object_internal_ptr.h>
#include <opendaq/mock/mock_fb_module.h>
#include <opendaq/mock/mock_device_module.h>
#include <opendaq/instance_factory.h>
#include <opendaq/data_descriptor_factory.h>
#include <opendaq/signal_config_ptr.h>
#include <opendaq/folder_config_ptr.h>
#include <opendaq/input_port_factory.h>
#include <opendaq/signal_factory.h>
#include <opendaq/tags_private_ptr.h>
#include <coreobjects/property_object_factory.h>
#include <coreobjects/property_object_protected_ptr.h>
#include "config_protocol/config_protocol_server.h"
#include "config_protocol/config_protocol_client.h"

using namespace daq;
using namespace daq::config_protocol;

class ConfigCoreEventTest : public testing::Test
{
public:
    void SetUp() override
    {
        const auto logger = Logger();
        const auto moduleManager = ModuleManager("");
        const auto typeManager = TypeManager();
        const auto context = Context(nullptr, logger, typeManager, moduleManager);

        const ModulePtr deviceModule(MockDeviceModule_Create(context));
        moduleManager.addModule(deviceModule);

        const ModulePtr fbModule(MockFunctionBlockModule_Create(context));
        moduleManager.addModule(fbModule);

        instance = InstanceCustom(context, "test");
        instance.addDevice("mock_phys_device");
        instance.addFunctionBlock("mock_fb_uid");

        server = std::make_unique<ConfigProtocolServer>(instance, std::bind(&ConfigCoreEventTest::serverNotificationReady, this, std::placeholders::_1));

        clientContext = NullContext();
        client = std::make_unique<ConfigProtocolClient>(clientContext, std::bind(&ConfigCoreEventTest::sendRequest, this, std::placeholders::_1), nullptr);

        client->connect();
        client->getDevice().asPtr<IPropertyObjectInternal>().enableCoreEventTrigger();
    }

protected:
    InstancePtr instance;
    std::unique_ptr<ConfigProtocolServer> server;
    std::unique_ptr<ConfigProtocolClient> client;
    ContextPtr clientContext;
    BaseObjectPtr notificationObj;

    // server handling
    void serverNotificationReady(const PacketBuffer& notificationPacket) const
    {
        client->triggerNotificationPacket(notificationPacket);
    }

    // client handling
    PacketBuffer sendRequest(const PacketBuffer& requestPacket) const
    {
        auto replyPacket = server->processRequestAndGetReply(requestPacket);
        return replyPacket;
    }
};

TEST_F(ConfigCoreEventTest, PropertyValueChanged)
{
    const auto clientComponent = client->getDevice().findComponent("Dev/mockdev");
    const auto serverComponent = instance.findComponent("Dev/mockdev");
    int callCount = 0;
    clientContext.getOnCoreEvent() +=
        [&](const ComponentPtr& comp, const CoreEventArgsPtr& args)
        {
            ASSERT_EQ(args.getEventId(), static_cast<Int>(CoreEventId::PropertyValueChanged));
            ASSERT_EQ(args.getEventName(), "PropertyValueChanged");
            ASSERT_EQ(comp, clientComponent);
            ASSERT_TRUE(args.getParameters().hasKey("Name"));
            ASSERT_TRUE(args.getParameters().hasKey("Value"));
            ASSERT_TRUE(args.getParameters().hasKey("Path"));
            ASSERT_EQ(comp, args.getParameters().get("Owner"));

            callCount++;
        };
    
    serverComponent.setPropertyValue("TestProperty", "foo");
    serverComponent.setPropertyValue("TestProperty", "bar");
    serverComponent.clearPropertyValue("TestProperty");
    ASSERT_EQ(callCount, 3);
}

TEST_F(ConfigCoreEventTest, PropertyObjectUpdateEnd)
{
    
}

TEST_F(ConfigCoreEventTest, PropertyAdded)
{
    
}

TEST_F(ConfigCoreEventTest, PropertyRemoved)
{
    
}

TEST_F(ConfigCoreEventTest, ComponentAdded)
{
    
}

TEST_F(ConfigCoreEventTest, ComponentRemoved)
{
    
}

TEST_F(ConfigCoreEventTest, SignalConnected)
{
    
}

TEST_F(ConfigCoreEventTest, SignalDisconnected)
{
    
}

TEST_F(ConfigCoreEventTest, DataDescriptorChanged)
{
    
}

TEST_F(ConfigCoreEventTest, ComponentUpdateEnd)
{
    
}

TEST_F(ConfigCoreEventTest, AttributeChanged)
{
    
}

TEST_F(ConfigCoreEventTest, TagsChanged)
{
    
}
