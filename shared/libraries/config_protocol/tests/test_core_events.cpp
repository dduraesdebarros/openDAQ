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
#include "test_utils.h"
#include "config_protocol/config_protocol_server.h"
#include "config_protocol/config_protocol_client.h"

using namespace daq;
using namespace daq::config_protocol;

class ConfigCoreEventTest : public testing::Test
{
public:
    void SetUp() override
    {
        serverDevice = test_utils::createServerDevice();
        serverDevice.asPtrOrNull<IPropertyObjectInternal>().enableCoreEventTrigger();
        server = std::make_unique<ConfigProtocolServer>(serverDevice, std::bind(&ConfigCoreEventTest::serverNotificationReady, this, std::placeholders::_1));

        clientContext = NullContext();
        client = std::make_unique<ConfigProtocolClient>(clientContext, std::bind(&ConfigCoreEventTest::sendRequest, this, std::placeholders::_1), nullptr);

        client->connect();
        client->getDevice().asPtr<IPropertyObjectInternal>().enableCoreEventTrigger();
        clientDevice = client->getDevice();
    }

protected:
    DevicePtr serverDevice;
    DevicePtr clientDevice;
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
    const auto clientComponent = client->getDevice().findComponent("IO/ai/ch");
    const auto serverComponent = serverDevice.findComponent("IO/ai/ch");

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
    
    serverComponent.setPropertyValue("StrProp", "foo");
    ASSERT_EQ(clientComponent.getPropertyValue("StrProp"), "foo");
    serverComponent.setPropertyValue("StrProp", "bar");
    ASSERT_EQ(clientComponent.getPropertyValue("StrProp"), "bar");
    serverComponent.clearPropertyValue("StrProp");
    ASSERT_EQ(clientComponent.getPropertyValue("StrProp"), "-");
    ASSERT_EQ(callCount, 3);
}

TEST_F(ConfigCoreEventTest, PropertyChangedNested)
{
    const auto clientComponent = client->getDevice().findComponent("AdvancedPropertiesComponent");
    const auto serverComponent = serverDevice.findComponent("AdvancedPropertiesComponent");

    int callCount = 0;
    
    const auto obj1 = clientComponent.getPropertyValue("ObjectWithMetadata");
    const auto obj2 = clientComponent.getPropertyValue("ObjectWithMetadata.Child");

    clientContext.getOnCoreEvent() += [&](const ComponentPtr& comp, const CoreEventArgsPtr& args)
    {
        ASSERT_EQ(args.getEventId(), core_event_ids::PropertyValueChanged);
        ASSERT_EQ(args.getEventName(), "PropertyValueChanged");
        ASSERT_EQ(comp, clientComponent);
        ASSERT_TRUE(args.getParameters().hasKey("Value"));
        ASSERT_EQ(args.getParameters().get("Name"), "String");

        if (callCount == 0)
        {
            ASSERT_EQ(obj1, args.getParameters().get("Owner"));
            ASSERT_EQ("ObjectWithMetadata", args.getParameters().get("Path"));
        }
        else
        {
            ASSERT_EQ(obj2, args.getParameters().get("Owner"));
            ASSERT_EQ("ObjectWithMetadata.Child", args.getParameters().get("Path"));
        }

        callCount++;
    };

    serverComponent.setPropertyValue("ObjectWithMetadata.String", "foo");
    serverComponent.setPropertyValue("ObjectWithMetadata.Child.String", "bar");

    ASSERT_EQ(clientComponent.getPropertyValue("ObjectWithMetadata.String"), "foo");
    ASSERT_EQ(clientComponent.getPropertyValue("ObjectWithMetadata.Child.String"), "bar");

    ASSERT_EQ(callCount, 2);
}

TEST_F(ConfigCoreEventTest, PropertyObjectUpdateEnd)
{
    const auto clientComponent = client->getDevice().findComponent("AdvancedPropertiesComponent");
    const auto serverComponent = serverDevice.findComponent("AdvancedPropertiesComponent");

    int propChangeCount = 0;
    int updateCount = 0;
    int otherCount = 0;
    
    clientContext.getOnCoreEvent() +=
        [&](const ComponentPtr& comp, const CoreEventArgsPtr& args)
        {
            DictPtr<IString, IBaseObject> updated;
            switch (args.getEventId())
            {
                case core_event_ids::PropertyValueChanged:
                    propChangeCount++;
                    break;
                case core_event_ids::PropertyObjectUpdateEnd:
                    updateCount++;
                    updated = args.getParameters().get("UpdatedProperties");
                    ASSERT_EQ(updated.getCount(), 2);
                    ASSERT_EQ(args.getEventName(), "PropertyObjectUpdateEnd");
                    ASSERT_EQ(comp, args.getParameters().get("Owner"));
                    break;
                default:
                    otherCount++;
                    break;
            }            
        };

    serverComponent.beginUpdate();
    serverComponent.setPropertyValue("Integer", 3);
    serverComponent.setPropertyValue("String", "bar");
    serverComponent.endUpdate();
    
    serverComponent.beginUpdate();
    serverComponent.clearPropertyValue("String");
    serverComponent.clearPropertyValue("Integer");
    serverComponent.endUpdate();

    ASSERT_EQ(propChangeCount, 0);
    ASSERT_EQ(updateCount, 2);
    ASSERT_EQ(otherCount, 0);
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
