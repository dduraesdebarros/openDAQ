// ReSharper disable CppClangTidyModernizeAvoidBind
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <config_protocol/config_protocol_server.h>
#include <config_protocol/config_protocol_client.h>
#include "test_utils.h"
#include <config_protocol/config_client_object_ptr.h>
#include "coreobjects/argument_info_factory.h"
#include "coreobjects/callable_info_factory.h"
#include "opendaq/context_factory.h"

using namespace daq;
using namespace config_protocol;
using namespace testing;
using namespace std::placeholders;


class ConfigProtocolIntegrationTest : public Test
{
public:
    static StringPtr serializeComponent(const ComponentPtr& component)
    {
        const auto serializer = JsonSerializer(True);
        component.serialize(serializer);
        const auto str = serializer.getOutput();
        return str;
    }

    static PacketBuffer sendPacket(ConfigProtocolServer& server, const PacketBuffer& requestPacket)
    {
        auto replyPacket = server.processRequestAndGetReply(requestPacket);
        return replyPacket;
    }
};

TEST_F(ConfigProtocolIntegrationTest, Connect)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverDeviceSerialized = serializeComponent(serverDevice);

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();
    const auto clientDeviceSerialized = serializeComponent(clientDevice);
    ASSERT_EQ(serverDeviceSerialized, clientDeviceSerialized);
}

TEST_F(ConfigProtocolIntegrationTest, RemoteGlobalIds)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverDeviceSerialized = serializeComponent(serverDevice);

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    ASSERT_EQ(clientDevice.asPtr<IConfigClientObject>(true).getRemoteGlobalId(), serverDevice.getGlobalId());
    ASSERT_EQ(clientDevice.getDevices()[0].asPtr<IConfigClientObject>(true).getRemoteGlobalId(), serverDevice.getDevices()[0].getGlobalId());
    ASSERT_EQ(clientDevice.getDevices()[0].getFunctionBlocks()[0].asPtr<IConfigClientObject>(true).getRemoteGlobalId(),
              serverDevice.getDevices()[0].getFunctionBlocks()[0].getGlobalId());
    ASSERT_EQ(clientDevice.getDevices()[0].getFunctionBlocks()[0].getSignals()[0].asPtr<IConfigClientObject>(true).getRemoteGlobalId(),
              serverDevice.getDevices()[0].getFunctionBlocks()[0].getSignals()[0].getGlobalId());
    ASSERT_EQ(clientDevice.getChannels()[0].getSignals()[0].asPtr<IConfigClientObject>(true).getRemoteGlobalId(),
              serverDevice.getChannels()[0].getSignals()[0].getGlobalId());
}

TEST_F(ConfigProtocolIntegrationTest, ConnectWithParent)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverDeviceSerialized = serializeComponent(serverDevice);

    ConfigProtocolServer server(serverDevice, nullptr);

    const auto clientContext = NullContext();
    ConfigProtocolClient client(clientContext, std::bind(sendPacket, std::ref(server), _1), nullptr);

    const auto parentComponent = Component(clientContext, nullptr, "cmp");

    client.connect(parentComponent);
    const auto clientDevice = client.getDevice();

    ASSERT_EQ(clientDevice.asPtr<IConfigClientObject>(true).getRemoteGlobalId(), serverDevice.getGlobalId());
    ASSERT_EQ(clientDevice.getGlobalId(), "/cmp/" + clientDevice.getLocalId().toStdString());
}

void checkComponentForConfigClientObject(const ComponentPtr& component)
{
    if (!component.supportsInterface<IConfigClientObject>())
        throw std::runtime_error(fmt::format("Component {} not a config client object", component.getGlobalId()));

    const auto folder = component.asPtrOrNull<IFolder>(true);
    if (folder.assigned())
        for (const auto& item: folder.getItems())
            checkComponentForConfigClientObject(item);
}

TEST_F(ConfigProtocolIntegrationTest, CheckConfigClientObject)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverDeviceSerialized = serializeComponent(serverDevice);

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();
    checkComponentForConfigClientObject(clientDevice);
}

TEST_F(ConfigProtocolIntegrationTest, GetInitialPropertyValue)
{
    const auto serverDevice = test_utils::createServerDevice();
    serverDevice.getChannels()[0].setPropertyValue("StrProp", "SomeValue");

    const auto serverDeviceSerialized = serializeComponent(serverDevice);

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrProp"), clientDevice.getChannels()[0].getPropertyValue("StrProp"));

    const auto clientDeviceSerialized = serializeComponent(clientDevice);
    ASSERT_EQ(serverDeviceSerialized, clientDeviceSerialized);
}


TEST_F(ConfigProtocolIntegrationTest, SetPropertyValue)
{
    const auto serverDevice = test_utils::createServerDevice();
    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    clientDevice.getChannels()[0].setPropertyValue("StrProp", "SomeValue");

    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrProp"), "SomeValue");
    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrProp"), clientDevice.getChannels()[0].getPropertyValue("StrProp"));
}

TEST_F(ConfigProtocolIntegrationTest, SetProtectedPropertyValue)
{
    const auto serverDevice = test_utils::createServerDevice();
    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    ASSERT_THROW(clientDevice.getChannels()[0].setPropertyValue("StrPropProtected", "SomeValue"), AccessDeniedException);

    clientDevice.getChannels()[0].asPtr<IPropertyObjectProtected>(true).setProtectedPropertyValue("StrPropProtected", "SomeValue");

    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrPropProtected"), "SomeValue");
    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrPropProtected"), clientDevice.getChannels()[0].getPropertyValue("StrPropProtected"));
}

TEST_F(ConfigProtocolIntegrationTest, ClearPropertyValue)
{
    const auto serverDevice = test_utils::createServerDevice();
    serverDevice.getChannels()[0].setPropertyValue("StrProp", "SomeValue");

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    clientDevice.getChannels()[0].clearPropertyValue("StrProp");
    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrProp"), "-");
    ASSERT_EQ(serverDevice.getChannels()[0].getPropertyValue("StrProp"), clientDevice.getChannels()[0].getPropertyValue("StrProp"));
}

TEST_F(ConfigProtocolIntegrationTest, CallFuncProp)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverCh = serverDevice.getChannels()[0];
    const auto funcProp =
        FunctionPropertyBuilder("FuncProp", FunctionInfo(ctInt, List<IArgumentInfo>(ArgumentInfo("A", ctInt), ArgumentInfo("B", ctInt))))
            .setReadOnly(True)
            .build();
    serverCh.addProperty(funcProp);

    serverCh.asPtr<IPropertyObjectProtected>(true).setProtectedPropertyValue("FuncProp", Function([](Int a, Int b) { return a + b; }));

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    const auto clientFuncPropValue = clientDevice.getChannels()[0].getPropertyValue("FuncProp");

    Int r = clientFuncPropValue.call(2, 4);
    ASSERT_EQ(r, 6);
}

TEST_F(ConfigProtocolIntegrationTest, CallProcProp)
{
    const auto serverDevice = test_utils::createServerDevice();
    const auto serverCh = serverDevice.getChannels()[0];
    const auto procProp =
        FunctionPropertyBuilder("ProcProp", ProcedureInfo())
            .setReadOnly(True)
            .build();
    serverCh.addProperty(procProp);

    bool called = false;
    serverCh.asPtr<IPropertyObjectProtected>(true).setProtectedPropertyValue("ProcProp", Procedure([&called] { called = true; }));

    ConfigProtocolServer server(serverDevice, nullptr);

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();

    const auto clientProcPropValue = clientDevice.getChannels()[0].getPropertyValue("ProcProp");

    clientProcPropValue.dispatch();
    ASSERT_TRUE(called);
}

TEST_F(ConfigProtocolIntegrationTest, GetAvailableFunctionBlockTypes)
{
    const auto serverDevice = test_utils::createServerDevice();

    ConfigProtocolServer server(serverDevice, nullptr);
    const auto serverSubDevice = serverDevice.getDevices()[0];

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();
    const auto clientSubDevice = clientDevice.getDevices()[0];

    const auto fbTypes = clientSubDevice.getAvailableFunctionBlockTypes();
    ASSERT_EQ(fbTypes, serverSubDevice.getAvailableFunctionBlockTypes());
}

TEST_F(ConfigProtocolIntegrationTest, AddFunctionBlockNotFound)
{
    const auto serverDevice = test_utils::createServerDevice();

    ConfigProtocolServer server(serverDevice, nullptr);
    const auto serverSubDevice = serverDevice.getDevices()[0];

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();
    const auto clientSubDevice = clientDevice.getDevices()[0];

    ASSERT_THROW(clientSubDevice.addFunctionBlock("someFb"), NotFoundException);
}

TEST_F(ConfigProtocolIntegrationTest, AddFunctionBlock)
{
    const auto serverDevice = test_utils::createServerDevice();

    ConfigProtocolServer server(serverDevice, nullptr);
    const auto serverSubDevice = serverDevice.getDevices()[0];

    ConfigProtocolClient client(NullContext(), std::bind(sendPacket, std::ref(server), _1), nullptr);

    client.connect();
    const auto clientDevice = client.getDevice();
    const auto clientSubDevice = clientDevice.getDevices()[0];

    const auto config = PropertyObject();
    config.addProperty(StringPropertyBuilder("Param", "Value").build());

    const auto fb = clientSubDevice.addFunctionBlock("mockfb1", config);

    ASSERT_EQ(fb, clientSubDevice.getFunctionBlocks()[1]);

    ASSERT_EQ(fb.asPtr<IConfigClientObject>().getRemoteGlobalId(), serverDevice.getDevices()[0].getFunctionBlocks()[1].getGlobalId());
}
