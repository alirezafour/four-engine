#pragma once

#include "core/type.hpp"
#include <vulkan/vulkan.hpp>

namespace four
{
enum class VKDeviceType : u8
{
  Other     = 0,
  Integated = 1,
  Discrete  = 2,
  Virtual   = 3,
  CPU       = 4,
  Unknown   = std::numeric_limits<u8>::max(),
};


struct VKDeviceLimits
{
  u32                max_image_dimension1d;
  u32                max_image_dimension2d;
  u32                max_image_dimension3d;
  u32                max_image_dimension_cube;
  u32                max_image_array_layers;
  u32                max_texel_buffer_elements;
  u32                max_uniform_buffer_range;
  u32                max_storage_buffer_range;
  u32                max_push_constants_size;
  u32                max_memory_allocation_count;
  u32                max_sampler_allocation_count;
  u64                buffer_image_granularity;
  u64                sparse_address_space_size;
  u32                max_bound_descriptor_sets;
  u32                max_per_stage_descriptor_samplers;
  u32                max_per_stage_descriptor_uniform_buffers;
  u32                max_per_stage_descriptor_storage_buffers;
  u32                max_per_stage_descriptor_sampled_images;
  u32                max_per_stage_descriptor_storage_images;
  u32                max_per_stage_descriptor_input_attachments;
  u32                max_per_stage_resources;
  u32                max_descriptor_set_samplers;
  u32                max_descriptor_set_uniform_buffers;
  u32                max_descriptor_set_uniform_buffers_dynamic;
  u32                max_descriptor_set_storage_buffers;
  u32                max_descriptor_set_storage_buffers_dynamic;
  u32                max_descriptor_set_sampled_images;
  u32                max_descriptor_set_storage_images;
  u32                max_descriptor_set_input_attachments;
  u32                max_vertex_input_attributes;
  u32                max_vertex_input_bindings;
  u32                max_vertex_input_attribute_offset;
  u32                max_vertex_input_binding_stride;
  u32                max_vertex_output_components;
  u32                max_tessellation_generation_level;
  u32                max_tessellation_patch_size;
  u32                max_tessellation_control_per_vertex_input_components;
  u32                max_tessellation_control_per_vertex_output_components;
  u32                max_tessellation_control_per_patch_output_components;
  u32                max_tessellation_control_total_output_components;
  u32                max_tessellation_evaluation_input_components;
  u32                max_tessellation_evaluation_output_components;
  u32                max_geometry_shader_invocations;
  u32                max_geometry_input_components;
  u32                max_geometry_output_components;
  u32                max_geometry_output_vertices;
  u32                max_geometry_total_output_components;
  u32                max_fragment_input_components;
  u32                max_fragment_output_attachments;
  u32                max_fragment_dual_src_attachments;
  u32                max_fragment_combined_output_resources;
  u32                max_compute_shared_memory_size;
  std::array<u32, 3> max_compute_work_group_count;
  u32                max_compute_work_group_invocations;
  std::array<u32, 3> max_compute_work_group_size;
  u32                sub_pixel_precision_bits;
  u32                sub_texel_precision_bits;
  u32                mipmap_precision_bits;
  u32                max_draw_indexed_index_value;
  u32                max_draw_indirect_count;
  f32                max_sampler_lod_bias;
  f32                max_sampler_anisotropy;
  u32                max_viewports;
  std::array<u32, 2> max_viewport_dimensions;
  std::array<f32, 2> viewport_bounds_range;
  u32                viewport_sub_pixel_bits;
  std::size_t        min_memory_map_alignment;
  u64                min_texel_buffer_offset_alignment;
  u64                min_uniform_buffer_offset_alignment;
  u64                min_storage_buffer_offset_alignment;
  i32                min_texel_offset;
  u32                max_texel_offset;
  i32                min_texel_gather_offset;
  u32                max_texel_gather_offset;
  f32                min_interpolation_offset;
  f32                max_interpolation_offset;
  u32                sub_pixel_interpolation_offset_bits;
  u32                max_framebuffer_width;
  u32                max_framebuffer_height;
  u32                max_framebuffer_layers;
  u32                framebuffer_color_sample_counts;
  u32                framebuffer_depth_sample_counts;
  u32                framebuffer_stencil_sample_counts;
  u32                framebuffer_no_attachments_sample_counts;
  u32                max_color_attachments;
  u32                sampled_image_color_sample_counts;
  u32                sampled_image_integer_sample_counts;
  u32                sampled_image_depth_sample_counts;
  u32                sampled_image_stencil_sample_counts;
  u32                storage_image_sample_counts;
  u32                max_sample_mask_words;
  u32                timestamp_compute_and_graphics;
  f32                timestamp_period;
  u32                max_clip_distances;
  u32                max_cull_distances;
  u32                max_combined_clip_and_cull_distances;
  u32                discrete_queue_priorities;
  std::array<f32, 2> point_size_range;
  std::array<f32, 2> line_width_range;
  f32                point_size_granularity;
  f32                line_width_granularity;
  u32                strict_lines;
  u32                standard_sample_locations;
  u64                optimal_buffer_copy_offset_alignment;
  u64                optimal_buffer_copy_row_pitch_alignment;
  u64                non_coherent_atom_size;
};

// not compatible with vk::physical_device_ray_tracing_propertiesKHR
struct VKRayTracingPipelineProperties
{
  u32 shader_group_handle_size;
  u32 max_ray_recursion_depth;
  u32 max_shader_group_stride;
  u32 shader_group_base_alignment;
  u32 shader_group_handle_capture_replay_size;
  u32 max_ray_dispatch_invocation_count;
  u32 shader_group_handle_alignment;
  u32 max_ray_hit_attribute_size;
};

// must be abi compatible with vk::physical_device_acceleration_structure_propertiesKHR
struct VKAccelerationStructureProperties
{
  u64 max_geometry_count;
  u64 max_instance_count;
  u64 max_primitive_count;
  u32 max_per_stage_descriptor_acceleration_structures;
  u32 max_per_stage_descriptor_update_after_bind_acceleration_structures;
  u32 max_descriptor_set_acceleration_structures;
  u32 max_descriptor_set_update_after_bind_acceleration_structures;
  u32 min_acceleration_structure_scratch_offset_alignment;
};

struct VKRayTracingInvocationReorderProperties
{
  u32 invocation_reorder_mode;
};

// not compatible with vk::physical_device_properties
struct VKMeshShaderProperties
{
  u32                max_task_work_group_total_count;
  std::array<u32, 3> max_task_work_group_count;
  u32                max_task_work_group_invocations;
  std::array<u32, 3> max_task_work_group_size;
  u32                max_task_payload_size;
  u32                max_task_shared_memory_size;
  u32                max_task_payload_and_shared_memory_size;
  u32                max_mesh_work_group_total_count;
  std::array<u32, 3> max_mesh_work_group_count;
  u32                max_mesh_work_group_invocations;
  std::array<u32, 3> max_mesh_work_group_size;
  u32                max_mesh_shared_memory_size;
  u32                max_mesh_payload_and_shared_memory_size;
  u32                max_mesh_output_memory_size;
  u32                max_mesh_payload_and_output_memory_size;
  u32                max_mesh_output_components;
  u32                max_mesh_output_vertices;
  u32                max_mesh_output_primitives;
  u32                max_mesh_output_layers;
  u32                max_mesh_multiview_view_count;
  u32                mesh_output_per_vertex_granularity;
  u32                mesh_output_per_primitive_granularity;
  u32                max_preferred_task_work_group_invocations;
  u32                max_preferred_mesh_work_group_invocations;
  b8                 prefers_local_invocation_vertex_output;
  b8                 prefers_local_invocation_primitive_output;
  b8                 prefers_compact_vertex_output;
  b8                 prefers_compact_primitive_output;
};

enum class VKMissingFeatureFlags : u8
{
  None                                                = 0,
  Image_cube_array                                    = 1,
  Independent_blend                                   = 2,
  Tessellation_shader                                 = 3,
  Multi_draw_indirect                                 = 4,
  Depth_clamp                                         = 5,
  Fill_mode_non_solid                                 = 6,
  Wide_lines                                          = 7,
  Sampler_anisotropy                                  = 8,
  Fragment_stores_and_atomics                         = 9,
  Shader_storage_image_multisample                    = 10,
  Shader_storage_image_read_without_format            = 11,
  Shader_storage_image_write_without_format           = 12,
  Shader_int64                                        = 13,
  Variable_pointers_storage_buffer                    = 14,
  Variable_pointers                                   = 15,
  Buffer_device_address                               = 16,
  Buffer_device_address_capture_replay                = 17,
  Buffer_device_address_multi_device                  = 18,
  Shader_sampled_image_array_non_uniform_indexing     = 19,
  Shader_storage_buffer_array_non_uniform_indexing    = 20,
  Shader_storage_image_array_non_uniform_indexing     = 21,
  Descriptor_binding_sampled_image_update_after_bind  = 22,
  Descriptor_binding_storage_image_update_after_bind  = 23,
  Descriptor_binding_storage_buffer_update_after_bind = 24,
  Descriptor_binding_update_unused_while_pending      = 25,
  Descriptor_binding_partially_bound                  = 26,
  Runtime_descriptor_array                            = 27,
  Host_query_reset                                    = 28,
  Dynamic_rendering                                   = 29,
  Synchronization2                                    = 30,
  Timeline_semaphore                                  = 31,
  Subgroup_size_control                               = 32,
  Compute_full_subgroups                              = 33,
  Scalar_block_layout                                 = 34,
  Acceleration_structure_capture_replay               = 35,
  Vulkan_memory_model                                 = 36,
  Robust_buffer_access2                               = 37,
  Robust_image_access2                                = 38,
  Uknown                                              = std::numeric_limits<u8>::max(),
};


enum class VKExplicitFeatureFlags : u8
{
  None                                  = 0,
  Buffer_device_address_capture_replay  = 0x1 << 0,
  Acceleration_structure_capture_replay = 0x1 << 1,
  Vk_memory_model                       = 0x1 << 2,
  Robustness_2                          = 0x1 << 3,
};

enum class VKImplicitFeatureFlags : u16
{
  None                           = 0,
  Mesh_shader                    = 0x1 << 0,
  Basic_ray_tracing              = 0x1 << 1,
  Ray_tracing_pipeline           = 0x1 << 2,
  Ray_tracing_invocation_reorder = 0x1 << 3,
  Ray_tracing_position_fetch     = 0x1 << 4,
  Conservative_rasterization     = 0x1 << 5,
  Shader_atomic_int64            = 0x1 << 6,
  Image_atomic64                 = 0x1 << 7,
  Shader_float16                 = 0x1 << 8,
  Shader_int8                    = 0x1 << 9,
  Dynamic_state_3                = 0x1 << 10,
  Shader_atomic_float            = 0x1 << 11,
  Swapchain                      = 0x1 << 12,
};

struct VKDeviceProperties
{
  u32                                                    vulkan_api_version;
  u32                                                    driver_version;
  u32                                                    vendor_id;
  u32                                                    device_id;
  VKDeviceType                                           device_type;
  std::string                                            device_name;
  std::string                                            pipeline_cache_uuid;
  VKDeviceLimits                                         limits;
  std::optional<VKMeshShaderProperties>                  mesh_shader_properties;
  std::optional<VKRayTracingPipelineProperties>          ray_tracing_pipeline_properties;
  std::optional<VKAccelerationStructureProperties>       acceleration_structure_properties;
  std::optional<VKRayTracingInvocationReorderProperties> ray_tracing_invocation_reorder_properties;
  u32                                                    compute_queue_count;
  u32                                                    transfer_queue_count;
  VKImplicitFeatureFlags                                 implicit_features;
  VKExplicitFeatureFlags                                 explicit_features;
  VKMissingFeatureFlags                                  missing_required_feature;
};

struct VKDeviceInfo
{
  u32                    physical_device_index{0U}; // Index devices list
  VKExplicitFeatureFlags explicit_features{
    VKExplicitFeatureFlags::Buffer_device_address_capture_replay}; // Explicit features must be manually enabled.
  u32         max_allowed_images{10000};
  u32         max_allowed_buffers{10000};
  u32         max_allowed_samplers{400};
  u32         max_allowed_acceleration_structures{10000};
  std::string name{};
};

enum class VKQueueFamily : u8
{
  Main     = 0,
  Compute  = 1,
  Transfer = 2,
  Uknown   = std::numeric_limits<u8>::max(),
};

struct VKQueue
{
  VKQueueFamily family;
  u32           index;
};

constexpr VKQueue VKMain{.family = VKQueueFamily::Main, .index = 0};
constexpr VKQueue VKCompute_0{.family = VKQueueFamily::Compute, .index = 0};
constexpr VKQueue VKCompute_1{.family = VKQueueFamily::Compute, .index = 1};
constexpr VKQueue VKCompute_2{.family = VKQueueFamily::Compute, .index = 2};
constexpr VKQueue VKCompute_3{.family = VKQueueFamily::Compute, .index = 3};
constexpr VKQueue VKCompute_4{.family = VKQueueFamily::Compute, .index = 4};
constexpr VKQueue VKCompute_5{.family = VKQueueFamily::Compute, .index = 5};
constexpr VKQueue VKCompute_6{.family = VKQueueFamily::Compute, .index = 6};
constexpr VKQueue VKCompute_7{.family = VKQueueFamily::Compute, .index = 7};
constexpr VKQueue VKTransfer_0{.family = VKQueueFamily::Transfer, .index = 0};
constexpr VKQueue VKTransfer_1{.family = VKQueueFamily::Transfer, .index = 1};

struct VKCommandSubmitInfo
{
  VKQueue                        queue;
  vk::PipelineStageFlags         wait_stages;
  VKExecutableCommandList const* command_lists;
  u64                            command_list_count;
  VKBinarySemaphore const*       wait_binary_semaphores;
  u64                            wait_binary_semaphore_count;
  VKBinarySemaphore const*       signal_binary_semaphores;
  u64                            signal_binary_semaphore_count;
  VKTimelinePair const*          wait_timeline_semaphores;
  u64                            wait_timeline_semaphore_count;
  VKTimelinePair const*          signal_timeline_semaphores;
  u64                            signal_timeline_semaphore_count;
};

typedef struct VKPresentInfo
{
  VKBinarySemaphore const* wait_binary_semaphores;
  u64                      wait_binary_semaphore_count;
  VKSwapchain              swapchain;
  VKQueue                  queue;
};

struct VKMemoryBlockBufferInfo
{
  VKBufferInfo   buffer_info;
  VKMemoryBlock* memory_block;
  std::size_t    offset;
};

struct VKMemoryBlockImageInfo
{
  VKImageInfo    image_info;
  VKMemoryBlock* memory_block;
  std::size_t    offset;
};

struct VKBufferTlasInfo
{
  VKTlasInfo tlas_info;
  VKBufferId buffer_id;
  u64        offset;
};

struct VKBufferBlasInfo
{
  VKBlasInfo blas_info;
  VKBufferId buffer_id;
  u64        offset;
};

struct VKAccelerationStructureBuildSizesInfo
{
  u64 acceleration_structure_size;
  u64 update_scratch_size;
  u64 build_scratch_size;
};
}; // namespace four
