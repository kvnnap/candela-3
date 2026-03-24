module candela.renderer;

import core.util;

using candela::renderer::VulkanPipeline;

VulkanPipeline::VulkanPipeline(const VulkanDevice& device, const VulkanSwapchain& swapchain)
    : device(device), swapchain(swapchain), pipelineLayout(nullptr), graphicsPipeline(nullptr)
{
}

void VulkanPipeline::init()
{
    createGraphicsPipeline();
}

const vk::raii::Pipeline& VulkanPipeline::getGraphicsPipeline() const
{
    return graphicsPipeline;
}

vk::raii::ShaderModule VulkanPipeline::createShaderModule(const std::vector<std::byte>& code)
{
    vk::ShaderModuleCreateInfo createInfo{ 
        .codeSize = code.size() * sizeof(std::byte), 
        .pCode = reinterpret_cast<const std::uint32_t*>(code.data()) 
    };
    return { device.getDevice(), createInfo };
}

void VulkanPipeline::createGraphicsPipeline()
{
    // Load shader
    auto code = core::util::loadBinaryFile("shaders/shaders.spv");
    auto shaderModule = createShaderModule(code);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo { 
        .stage = vk::ShaderStageFlagBits::eVertex, 
        .module = shaderModule,
        .pName = "vertMain",
        .pSpecializationInfo = nullptr // For specifying constants at pipeline build time
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ 
        .stage = vk::ShaderStageFlagBits::eFragment, 
        .module = shaderModule, 
        .pName = "fragMain" 
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    //vertex description
    auto binding = vk::VertexInputBindingDescription{ 0, 20, vk::VertexInputRate::eVertex };
    auto attribs = std::array<vk::VertexInputAttributeDescription, 2> { //location,binding,format,offset
            vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32Sfloat, 0 ), // pos
            vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, 8 ) //color
    };
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{ .vertexBindingDescriptionCount   = 1,
                                                        .pVertexBindingDescriptions      = &binding,
                                                        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>( attribs.size() ),
                                                        .pVertexAttributeDescriptions    = attribs.data() };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{  .topology = vk::PrimitiveTopology::eTriangleList };
    // vk::Viewport { 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f };
    // vk::Rect2D { vk::Offset2D{ 0, 0 }, swapChainExtent };

    std::vector dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() };

    // vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1);
    vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };

    vk::PipelineRasterizationStateCreateInfo rasterizer{  
        .depthClampEnable = vk::False, 
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill, .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise, .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f, .lineWidth = 1.0f 
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable    = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable = vk::False, 
        .logicOp =  vk::LogicOp::eCopy, 
        .attachmentCount = 1, 
        .pAttachments =  &colorBlendAttachment 
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

    // Uniforms
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{  .setLayoutCount = 0, .pushConstantRangeCount = 0 };
    pipelineLayout = vk::raii::PipelineLayout( device.getDevice(), pipelineLayoutInfo );

    // Dynamic rendering
    auto format = swapchain.getFormat();
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &format };

    vk::GraphicsPipelineCreateInfo pipelineInfo{ 
        .pNext = &pipelineRenderingCreateInfo, // This structure specifies the formats of the attachments that will be used during rendering
        .stageCount = 2, .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState, .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling, .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState, 
        .layout = pipelineLayout, // The fixed layout
        .renderPass = nullptr // null because we're using dynamic rendering
    };

    graphicsPipeline = vk::raii::Pipeline(device.getDevice(), nullptr, pipelineInfo);
}