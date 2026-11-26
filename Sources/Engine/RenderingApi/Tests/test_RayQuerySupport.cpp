#include <RenderingApi/Core/Device.hpp>
#include <gtest/gtest.h>

#include <cstring>
#include <vector>

using namespace BetterThanNothing::RenderingApi;

namespace
{
	VkExtensionProperties MakeExtension(const char* name)
	{
		VkExtensionProperties props{};
		std::strncpy(props.extensionName, name, VK_MAX_EXTENSION_NAME_SIZE - 1);
		props.specVersion = 1;
		return props;
	}
}

TEST(RayQuerySupport, ReturnsTrueWhenAllThreeExtensionsArePresent)
{
	const std::vector<VkExtensionProperties> extensions = {
		MakeExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME),
		MakeExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME),
		MakeExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME),
		MakeExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME),
	};

	EXPECT_TRUE(Device::HasRequiredRayQueryExtensions(extensions));
}

TEST(RayQuerySupport, ReturnsFalseWhenAnyRequiredExtensionIsMissing)
{
	const std::vector<VkExtensionProperties> missingRayQuery = {
		MakeExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME),
		MakeExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME),
	};
	EXPECT_FALSE(Device::HasRequiredRayQueryExtensions(missingRayQuery));

	const std::vector<VkExtensionProperties> missingAS = {
		MakeExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME),
		MakeExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME),
	};
	EXPECT_FALSE(Device::HasRequiredRayQueryExtensions(missingAS));

	const std::vector<VkExtensionProperties> missingDeferred = {
		MakeExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME),
		MakeExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME),
	};
	EXPECT_FALSE(Device::HasRequiredRayQueryExtensions(missingDeferred));
}

TEST(RayQuerySupport, ReturnsFalseForEmptyExtensionList)
{
	const std::vector<VkExtensionProperties> empty;
	EXPECT_FALSE(Device::HasRequiredRayQueryExtensions(empty));
}
