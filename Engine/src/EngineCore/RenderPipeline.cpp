#include "pch.h"

#include "RenderPipeline.h"

namespace Luxel
{
	PipelineConfigInfo RenderPipeline::DefaultPipelineConfigInfo(const VkPipelineLayout& pipelineLayout, ui32 width, ui32 height, const VkRenderPass& renderPass)
	{
		Info("Use default pipeline config info.");
		PipelineConfigInfo configInfo{};
		
		// vertex input
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		configInfo.inputAssemblyInfo.pNext = nullptr;
		configInfo.inputAssemblyInfo.flags = 0;

		// viewport
		configInfo.viewport.x = 0.f;
		configInfo.viewport.y = 0.f;
		configInfo.viewport.width = static_cast<float>(width);
		configInfo.viewport.height = static_cast<float>(height);
		configInfo.viewport.minDepth = 0.f;
		configInfo.viewport.maxDepth = 1.f;

		configInfo.scissor.offset = { 0, 0 };
		configInfo.scissor.extent = { width, height };

		// rasterization
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.f;
		configInfo.rasterizationInfo.depthBiasClamp = 0.f;
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.f;

		// multisample
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		// color blend
		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};  // Optional
		configInfo.depthStencilInfo.back = {};   // Optional

		configInfo.pipelineLayout = nullptr;
		configInfo.renderPass = nullptr;
		configInfo.subpass = 0;

		configInfo.renderPass = renderPass;
		configInfo.pipelineLayout = pipelineLayout;

		return configInfo;
	}

	void RenderPipeline::Bind(VkCommandBuffer commandBuffer)
	{
		Info("Bind graphics pipeline with command buffers.");
		if (graphicsPipeline == VK_NULL_HANDLE) {
			Error("Cannot bind graphics pipeline: no graphics pipeline provided.");
			throw std::runtime_error("Cannot bind graphics pipeline: no graphics pipeline provided.");
		}
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	std::vector<char> RenderPipeline::readFile(const std::string& filePath)
	{
		std::ifstream file{
			filePath, std::ios::ate | std::ios::binary
		};

		if (!file.is_open()) {
			Fatal("The file:", filePath, "does not exist.");
			throw std::runtime_error("failed to open file.");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	void RenderPipeline::CreateGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo)
	{
		Info("Create graphics pipeline.");

		if (configInfo.pipelineLayout == VK_NULL_HANDLE) {
			Error("Cannot create graphics pipeline: no pipelineLayout provided.");
			throw std::runtime_error("Cannot create graphics pipeline: no pipelineLayout provided.");
		}
		if (configInfo.renderPass == VK_NULL_HANDLE) {
			Error("Cannot create graphics pipeline: no renderPass provided.");
			throw std::runtime_error("Cannot create graphics pipeline: no renderPass provided.");
		}

		auto vert = readFile(vertPath);
		auto frag = readFile(fragPath);
		Info("Vertex shader size:", vert.size());
		Info("Fragment shader size:", frag.size());

		CreateShaderModule(vert, &vertShaderModule);
		CreateShaderModule(frag, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shadersCreateInfo[2]{};
		shadersCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shadersCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shadersCreateInfo[0].module = vertShaderModule;
		shadersCreateInfo[0].pName = "main";
		shadersCreateInfo[0].pNext = nullptr;
		shadersCreateInfo[0].pSpecializationInfo = nullptr;

		shadersCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shadersCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shadersCreateInfo[1].module = fragShaderModule;
		shadersCreateInfo[1].pName = "main";
		shadersCreateInfo[1].pNext = nullptr;
		shadersCreateInfo[1].pSpecializationInfo = nullptr;


		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.viewportCount = 1;
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.scissorCount = 1;
		viewportInfo.pViewports = &configInfo.viewport;
		viewportInfo.pScissors = &configInfo.scissor;

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shadersCreateInfo;
		pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineCreateInfo.pViewportState = &viewportInfo;
		pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineCreateInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineCreateInfo.pDynamicState = nullptr;

		pipelineCreateInfo.layout = configInfo.pipelineLayout;
		pipelineCreateInfo.renderPass = configInfo.renderPass;
		pipelineCreateInfo.subpass = configInfo.subpass;

		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pNext = nullptr;

		if (device == nullptr || device->GetDevice() == VK_NULL_HANDLE) {
			Error("Invalid Vulkan device.");
			throw std::runtime_error("Invalid Vulkan device.");
		}
		if (vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			Error("Failed to create graphics pipeline.");
			throw std::runtime_error("Failed to create graphics pipeline.");
		}
	}

	void RenderPipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const ui32*>(code.data());

		if (vkCreateShaderModule(device->GetDevice(), &shaderModuleCreateInfo, nullptr, shaderModule) != VK_SUCCESS) {
			Error("Failed to create shader module.");
			throw std::runtime_error("Feiled to create shader module.");
		}
	}

	RenderPipeline::RenderPipeline(Device* const d, SwapChain* const s, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo) : device{ d }, swapChain{ s }
	{
		CreateGraphicsPipeline(vertPath, fragPath, configInfo);
	}

	RenderPipeline::~RenderPipeline()
	{
		Info("Destory render pipeline.");
		vkDestroyShaderModule(device->GetDevice(), vertShaderModule, nullptr);
		vkDestroyShaderModule(device->GetDevice(), fragShaderModule, nullptr);
		vkDestroyPipeline(device->GetDevice(), graphicsPipeline, nullptr);
	}
}
