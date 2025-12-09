#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	template <typename T>
	class IResourceLoader
	{
	public:
		explicit IResourceLoader(VulkanContext* context) : m_context(context) {}

		virtual ~IResourceLoader() = default;
		virtual std::unique_ptr<T> Load(const std::string& path) = 0;

	protected:
		VulkanContext* m_context;
	};
};