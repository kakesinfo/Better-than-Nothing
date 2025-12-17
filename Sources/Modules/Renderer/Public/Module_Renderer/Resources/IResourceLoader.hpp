#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	template <typename input_type, typename output_type>
	class IResourceLoader
	{
	public:
		explicit IResourceLoader(VulkanContext* context) : m_context(context) {}

		virtual ~IResourceLoader() = default;
		virtual std::unique_ptr<output_type> Load(const input_type& input) = 0;

	protected:
		VulkanContext* m_context;
	};
};